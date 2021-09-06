

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Program: procfs.c
 *
 * Organizer for this project: Rozita Teymourzadeh, Mathew Malensek 
 * Date: 2020
 * Demonstrates string tokenization in C using the strspn(3) and strcspn(3)
 * functions. Unlike strtok(3), this implementation is thread safe. The code
 * is based on the following newsgroup post:
 *
 * https://groups.google.com/forum/message/raw?msg=comp.lang.c/ff0xFqRPH_Y/Cen0mgciXn8J
 */
#include "debug.h"
#include "procfs.h"
#include "util.h"

int arr_size = 255;

/**
 * struct load_avg
 *
 * To hold load average
 *
 **/
struct date_time {
    int year;
    int day;
    int hour;
    int min;
    int sec;
};

char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz)
{

    char fp[arr_size];
    strcpy(fp, proc_dir);
    strcat(fp, "/sys/kernel/hostname");

    //LOG("fname: %s\n", fp);

    ssize_t read_sz;
    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");
        return -1;
    }
    //int file_size = 0;
    while((read_sz = read(fd, hostname_buf, buf_sz)) >0) {

        int i;
        for(i = 0; i < read_sz; ++i) {
            if(hostname_buf[i] == EOF) {
                //file_size+=i;
            }
        }
        //file_size+=read_sz;
    }
    size_t loc = strcspn(hostname_buf, "\n");
    hostname_buf[loc] = '\0';
    close(fd);
    return 0;
}

int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz)
{
    char fp[arr_size];
    ssize_t read_sz;
    strcpy(fp, proc_dir);
    int file_size = 0;
    strcat(fp, "/version");
    LOG("fname: %s\n", fp);

    char *buf = calloc(buf_sz, sizeof(char));
    

    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");
        return -1;
    }
    while((read_sz = read(fd, buf, buf_sz)) >0) {
        for(int i = 0; i < read_sz; ++i) {
            if(buf[i] == EOF) {
                file_size+=i;
                strncat(version_buf, buf, i);
            }
        }
        strncat(version_buf, buf, read_sz);
        file_size += read_sz;
    }

    version_buf[file_size-1] = '\0';
    close(fd);
    free(buf);

    //token version string
    char *ver_tok = version_buf;
    char *ker_version = 0;
    int tokens = 0;
    while((ker_version = next_token(&ver_tok, " ")) != NULL){
        //printf("Token %d is %s\n", tokens, ker_version);
        if(tokens == 2) {
            size_t loc = strcspn(ker_version, "-");
            ker_version[loc] = '\0';
            break;
        }
        tokens++;
    }
    if(ker_version != 0){
        strcpy(version_buf, ker_version);
    }
    return 0;
}

int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz)
{
    char *pre = "model name";
    char fp[arr_size];
    strcpy(fp, proc_dir);
    strcat(fp, "/cpuinfo");

    char one_line[buf_sz];
    int length = 0;
    char buf[1];
    
    ssize_t read_sz;

    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");
        return -1;
    }

    while((read_sz = read(fd, buf, 1)) >0) {

        if(buf[0] == '\n') {
            //one line has been read
            one_line[length] = '\0';
            int result =strncmp(pre, one_line, strlen(pre));
            if(result == 0) {
                strcpy(model_buf, one_line);
                break;
            }
            one_line[0] = '\0';
            length = 0;
        } else {
            //in the middle of the line
            one_line[length] = buf[0];
            length++;
        }
    }

    close(fd);
    
    //token string
    char *cpu_tok = model_buf;
    char *cpu_mode;
    int tokens = 0;
    while((cpu_mode = next_token(&cpu_tok, "\t:")) != NULL){
        if(tokens == 1) {
            break;
        }
        tokens++;
    }
    if (cpu_mode != 0){
    strcpy(model_buf, cpu_mode+1);
    }
    return 0;
}

int pfs_cpu_units(char *proc_dir)
{
    size_t buf_sz = 128;
    int result = 0;
    char *pre = "cpu";
    char *stop = "intr";
    char fp[arr_size];
    strcpy(fp, proc_dir);
    strcat(fp, "/stat");

    char one_line[buf_sz];
    int length = 0;
    char buf[1];
    
    ssize_t read_sz;

    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        //fprintf(stderr, "Cannot open file. Try again later.\n");
        memset(&buf[0], 0, sizeof(buf));
        close(fd);
        return 0;
    }
    while((read_sz = read(fd, buf, 1)) >0) {

        if(buf[0] == '\n') {
            //one line has been read
            one_line[length] = '\0';
            if(strncmp(pre, one_line, strlen(pre)) == 0) {
                result++;
            }

            //stop reading when find "intr"
            if(strncmp(stop, one_line, strlen(stop)) == 0) {
                break;
            }

            one_line[0] = '\0';
            length = 0;
        } else {
            if(strlen(one_line) >= 127) {
                //previent too many unnessary '0'
                break;
            }

            //in the middle of the line
            one_line[length] = buf[0];
            length++;
        }
    }
    close(fd);
    return result-1;
}

/** function to get a hostname */
double pfs_uptime(char *proc_dir)
{
    size_t buf_sz = 128;
    char fp[arr_size];
    strcpy(fp, proc_dir);
    strcat(fp, "/uptime");
    char *uptime = calloc(1024, sizeof(char));
    char *uptime_clean = uptime;
    char *buf = calloc(buf_sz, sizeof(char));
    char *buf_clean = buf;
    ssize_t read_sz;

    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");
        free(buf_clean),
        free(uptime_clean);
        return 0.0;
    }
    int file_size = 0;
    while((read_sz = read(fd, buf, buf_sz)) >0) {

        int i;
        for(i = 0; i < read_sz; ++i) {
            if(buf[i] == EOF) {
                //if read to the end of the file
                file_size+=i;
                strncat(uptime, buf, i);
            }
        }
        strncat(uptime, buf, read_sz);
        file_size+=read_sz;
    }

    uptime[file_size-1] = '\0';
    close(fd);
    free(buf);

    //token string
    char *up_tok = uptime;
    char *up_time;
    int tokens = 0;
    while((up_time = next_token(&up_tok, " ")) != NULL){

        if(tokens == 0) {
            break;
        }
        tokens++;
    }
    
    if(up_time != 0){
        strcpy(uptime, up_time);
    }

    char *ptr;
    double result = strtod(uptime, &ptr);

    free(uptime_clean);

    return result;
}

/** function to get a hostname */
int pfs_format_uptime(double time, char *uptime_buf)
{
    if (isnan(time) || time == 0.0){
        strcat(uptime_buf, "Uptime: 0 minutes, -1 seconds");
    }

    struct date_time dt = {
        0,
        0,
        0,
        0,
        0
    };

    /* calculate uptime */
    int addComma = 0;
    dt.sec = (int) time;
    dt.min = (dt.sec / 60) % 60;
    dt.hour = (dt.sec / 3600) % 24;
    dt.day = (dt.sec / 86400) % 365;
    dt.year = dt.sec / 31536000;
    dt.sec = dt.sec % 60;

    char buf[50];
    memset(buf, 0, sizeof buf);

    if (dt.year != 0) {
        sprintf(buf, "%d", dt.year);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, "years, ");
        addComma = 1;
        memset(buf, 0, sizeof buf);
    }
    if (dt.day != 0 && addComma) {
        sprintf(buf, "%d", dt.day);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, " days, ");
        addComma = 1;
        memset(buf, 0, sizeof buf);
    } else if (dt.day != 0) {
        sprintf(buf, "%d", dt.day);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, " days, ");
        addComma = 1;
        memset(buf, 0, sizeof buf);
    }
    if (dt.hour != 0 && addComma) {
        sprintf(buf, "%d", dt.hour);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, " hours, ");
        addComma = 1;
        memset(buf, 0, sizeof buf);
    } else if (dt.hour != 0) {
        sprintf(buf, "%d", dt.hour);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, " hours, ");
        addComma = 1;
        memset(buf, 0, sizeof buf);
    }
    if (addComma) {
        sprintf(buf, "%d", dt.min);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, " minutes, ");
        memset(buf, 0, sizeof buf);
    } else {
        sprintf(buf, "%d", dt.min);
        strcat(uptime_buf, buf);
        strcat(uptime_buf, " minutes, ");
        memset(buf, 0, sizeof buf);
    }
    sprintf(buf, "%d", dt.sec);
    strcat(uptime_buf, buf);
    strcat(uptime_buf, " seconds");
    memset(buf, 0, sizeof buf);
    //strcat(uptime_buf, "\n\n");
    return 0;
}

struct load_avg pfs_load_avg(char *proc_dir)
{
   struct load_avg lavg = { 
        0,
        0,
        0
    };

    size_t buf_sz = 128;
    char fp[arr_size];
    strcpy(fp, proc_dir);
    strcat(fp, "/loadavg");

    char one_line[buf_sz];
    int length = 0;
    char buf[1];
    
    ssize_t read_sz;

    int fd = open(fp, O_RDONLY);

    while((read_sz = read(fd, buf, 1)) >0) {
        one_line[length] = buf[0];
        length++;
    }
    one_line[length-1] = '\0';
    close(fd);

    //token string 
    char *load_tok = one_line;
    char *temp;
    int tokens = 0;

    char *ptr;

    while((temp = next_token(&load_tok, " ")) != NULL){
        if(tokens == 0) {
            //strcpy(lavg.one, temp);
            lavg.one = strtod(temp, &ptr);
        }

        if (tokens == 1) {
            //strcpy(lavg.five, temp);
            lavg.five = strtod(temp, &ptr);
        }

        if (tokens == 2) {
            //strcpy(lavg.fifteen, temp);
            lavg.fifteen = strtod(temp, &ptr);
            break;
        }
        tokens++;
    }
   return lavg;
}

/** function to get a hostname */
double pfs_cpu_usage(char *procfs_dir, struct cpu_stats *prev, struct cpu_stats *curr)
{

    char fp[arr_size];
    strcpy(fp, procfs_dir);
    strcat(fp, "/stat");

    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");
        return -1;
    }
    ssize_t read_sz;
    char buf[1];
    char one_line[100];
    int length = 0;

    char one_cpu[80];

    //read idle  and total 
    while((read_sz = read(fd, buf, 1)) >0) {
        if(buf[0] == '\n') {
            //if readed one line
            one_line[length] = '\0';

            strcpy(one_cpu, one_line);
            one_line[0] = '\0';
            //length = 0;
            break;

        } else {
            //in the middle of the line
            one_line[length] = buf[0];
            length++;
        }
    }
    close(fd);

    char *one_tok = one_cpu;
    char *one_temp;
    long one_total = 0;
    int token = 0;
    long idle1;

    //token string
    while((one_temp = next_token(&one_tok, " ")) != NULL){
        if(token == 4) {
            idle1 = atol(one_temp);
        }
        if(token > 0){
            one_total+=atol(one_temp);
        }
        token++;
    }


    curr->idle = (double)idle1;
    curr->total = (double)one_total;

    double idle_diff = (curr->idle) - (prev->idle);
    double total_diff = (curr->total) - (prev->total);

    double usage = 1 - (idle_diff / total_diff);


    if (isnan(usage) || usage < 0 || idle_diff < 0 || total_diff < 0) {
        return 0.0;
    } else {
        return usage;
    }
}

struct mem_stats pfs_mem_usage(char *procfs_dir)
{
    struct mem_stats mstats = { 
        0,
        0
    };

    char *total_pre = "MemTotal:";
    char *active_pre = "MemAvailable:";

    char *total = calloc(20, sizeof(char));
    char *used = calloc(20, sizeof(char));
    char *total_line = calloc(30, sizeof(char));
    char *active_line = calloc(30, sizeof(char));
    char *one_line = calloc(100, sizeof(char));

    ssize_t read_sz;
    char buf[1];
    int length = 0;
    char fp[arr_size];

    strcpy(fp, procfs_dir);
    strcat(fp, "/meminfo");
    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");
        //exit(1);
    }

    while(((read_sz = read(fd, buf, 1)) >0) && (total_line[0] =='\0' || active_line[0] == '\0')){

        if(buf[0] == '\n') {
            //read one line
            one_line[length] = '\0';

            if(strncmp(total_pre, one_line, strlen(total_pre)) == 0) {
                //if the line is start with "MemTotal:"
                strcpy(total_line, one_line);
            }

            if(strncmp(active_pre, one_line, strlen(active_pre)) == 0) {
                //if the line is start with "MemAvailable:"
                strcpy(active_line, one_line);
            }

            one_line[0] = '\0';
            length = 0;
        } else {
            //in the middle of the line
            one_line[length] = buf[0];
            length++;
        }
    }
    close(fd);

    //token string
    int i;
    int flag = 0;
    int index = 0;
    for(i = 0; total_line[i] != '\0'; i++) {

        if(flag == 1) {
            total[index] = total_line[i];
            index++;
        } else if(total_line[i] > 47 && total_line[i] < 58) {
            total[index] = total_line[i];
            flag = 1;
            index++;
        }
        if(total_line[i] == 32) {
            flag = 0;
        }
    }

    total[index] = '\0';

    index = 0;
    flag = 0;

    for(i = 0; active_line[i] != '\0'; i++) {

        if(flag == 1) {
            used[index] = active_line[i];
            index++;
        } else if(active_line[i] > 47 && active_line[i] < 58) {
            used[index] = active_line[i];
            flag = 1;
            index++;
        }
        if(active_line[i] == 32) {
            if(index > 0) {
                break;
            }
            flag = 0;
        }
    }
    used[index] = '\0';

    double total_double = ((double) atoi(total))/1024/1024;
    double free_double = ((double) atoi(used))/1024/1024;
    double used_double = (double)(total_double - free_double);

    
    if (isnan(total_double) || isnan(used_double)|| total_double < 0 || used_double < 0 || free_double < 0) {
        mstats.total = 0;
        mstats.used = 0;
    } else {
        mstats.total = total_double;
        mstats.used = used_double;
    }

    free(used);
    free(total);
    free(total_line);
    free(active_line);
    free(one_line);

    return mstats;
}

/** function to get a hostname */
struct task_stats *pfs_create_tstats()
{
    struct task_stats *stats = calloc(1,sizeof(struct task_stats));
    stats->active_tasks = calloc(1000, sizeof(struct task_info));
    return stats;
}

/** 
* pfs_destroy_tstats
* 
* @ struct task_stats *tstats
 */
void pfs_destroy_tstats(struct task_stats *tstats)
{
    free(tstats->active_tasks);
    free(tstats);
}

int pfs_tasks(char *proc_dir, struct task_stats *tstats)
{

    DIR *directory;
    if ((directory = opendir(proc_dir)) == NULL) {
        perror("opendir");
        closedir(directory);
        return EXIT_FAILURE;
    }

    int counter = 0;

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if((is_digit(entry->d_name, strlen(entry->d_name)) == 1) && (entry->d_type == 4)) {
            
            get_task_list(proc_dir, entry->d_name, tstats, &counter);
            get_process_usage(proc_dir,entry->d_name, tstats, &counter);
        }

        
    }
    closedir(directory);

    return 0;
}

void get_task_list(char* proc_dir, char* process, struct task_stats *tstats, int *count) {
    char *thread_pre = "Threads:";
    char *state_pre = "State:";
    char *uid_pre = "Uid:";
    char *name_pre = "Name:";

    char *state_line = calloc(30, sizeof(char));
    char *uid_line = calloc(30, sizeof(char));
    char *thread_line = calloc(15, sizeof(char));
    char *name_line = calloc(99, sizeof(char));

    char state[13];
    char task_name[26];
    char user[30]; 
    //char task[15];

    char fp[arr_size];

    strcpy(fp, proc_dir);
    strcat(fp, "/");
    strcat(fp, process);
    strcat(fp, "/status");

    int fd = open(fp, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "Cannot open file. Try again later.\n");

        //exit(1);
    }
    ssize_t read_sz;
    char buf[1];
    char one_line[100];
    int length = 0;

    while((read_sz = read(fd, buf, 1)) >0) {
        if(buf[0] == '\n') {
            one_line[length] = '\0';

            if(strncmp(thread_pre, one_line, strlen(thread_pre)) == 0) {
                //if the line is start with "Threads:"
                strcpy(thread_line, one_line);
                //no information needed after this line
                break;
            }

            if(strncmp(name_pre, one_line, strlen(name_pre)) == 0) {
                strcpy(name_line, one_line);
            }

            if(strncmp(state_pre, one_line, strlen(state_pre)) == 0) {
                strcpy(state_line, one_line);
            }

            if(strncmp(uid_pre, one_line, strlen(uid_pre)) == 0) {
                strcpy(uid_line, one_line);
            }

            one_line[0] = '\0';
            length = 0;
        } else {
            //in the middle of the line
            one_line[length] = buf[0];
            length++;
        }

    }
    close(fd);

    //token string
    int i;
    int flag = 0;
    int index = 0;
    for(i = 0; i < 31; i++) {
        if(flag == 1) {
            task_name[index] = name_line[i];
            index++;
            // to check name smaller than 26
            if (index > 25) {
                break;
            }
        }
        if(name_line[i] == 9) {
            flag = 1;
        }
    }

    task_name[index] = '\0';

    //printf("taskname  %s \n", task_name);
    //strcpy(tstats->active_tasks[count].name, task_name);
    index = 0;
    flag = 0;

    for(i = 0; state_line[i] != 41; i++) {
        if(flag == 1) {
            state[index] = state_line[i];
            index++;
            if (index > 12) {
                break;
            }
        }
        if(state_line[i] == 40) {
            flag = 1;
        }
    }

    state[index] = '\0';

    //printf("state %s \n", state);
    //strcpy(tstats->active_tasks[count].state, state);


    // index = 0;
    // flag = 0;

    // for(i = 0; thread_line[i] != '\0'; i++) {
    //     if(flag == 1) {
    //         task[index] = thread_line[i];
    //         index++;
    //     }
    //     if(thread_line[i] == 9) {
    //         flag = 1;
    //     }
    // }

    // task[index] = '\0';

    //printf("TASK- PID %d \n", atoi(&task[index]) );

    index = 0;
    flag = 0;

    for(i = 0; uid_line[i] != '\0'; i++) {
        if(flag == 1) {
            user[index] = uid_line[i];
            index++;
        }
        if(uid_line[i] == 9) {
            flag = 1;
        }
    }

    user[index] = '\0';

    for(i = 0; user[i] != 9; i++) {
        if(user[i+1] == 9){
            user[i+1] = '\0';
            //printf("uid - user %d \n", user[i+1]);
            break;
        }
    }

    //count processes

    if (strcmp(state, "running") == 0) {
        tstats->running++;
    }
    if (strcmp(state, "disk sleep") == 0){
        tstats->waiting++;
    }
    if ((strcmp(state, "sleeping") == 0) || (strcmp(state, "idle") == 0)){
         tstats->sleeping++;
    }
    if ((strcmp(state, "stopped") == 0) || (strcmp(state, "tracing stop") == 0)){
        tstats->stopped++;
    }
    if (strcmp(state, "zombie") == 0){
        tstats->zombie++;
    }
    tstats->total++;


    if (strcmp(state, "idle") != 0 && strcmp(state, "sleeping") != 0){
        strcpy(tstats->active_tasks[*count].name, task_name);
        strcpy(tstats->active_tasks[*count].state, state);
        tstats->active_tasks[*count].pid = atoi(process);
        tstats->active_tasks[*count].uid = atoi(&user[i+1]);
        *count = *count + 1;
        if (*count > 999){
            *count = 0;
        }
    }
            
    free(state_line);
    free(uid_line);
    free(thread_line);
    free(name_line);
}



int is_digit(char d_name[], int len) {

    //if the string is all digit
    int i;
    for(i = 0; i < len; i++) {
        if(d_name[i]<48 || d_name[i]>57) {
            return 0;
        }
    }
    return 1;
}



void get_process_usage(char* proc_dir, char* process, struct task_stats *tstats, int *count) {
    
    char line[1024];
    char fp[arr_size];
    strcpy(fp, proc_dir);
    strcat(fp, "/");
    strcat(fp, process);
    strcat(fp, "/stat");

    //printf("fp is ..... %s\n", fp);

    int fd = open(fp, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    //memset(name_buf, 0, 15);
    int tokens = 0;

    char *tok = 0;
    int hertz = 100;
    int total_time = 0;
    double individual_cpu_usage = 0.0;
    // unsigned int utime = 0;
    // unsigned int stime = 0;

    // unsigned int utime2 = 0;
    // unsigned int stime2 = 0;

    while (true) {
        ssize_t read_sz = lineread(fd, line, 1024);
        if (read_sz == 0 || read_sz == -1) {
            break;
        }

        char *next_tok = line;
        //printf("*************** Line is %s\n", line);
        while  (tokens < 22){
            tok = next_token(&next_tok, " ");
            //printf("Token %d is %s\n", tokens, tok);
            if (tokens == 13) {
                /* Found a match */
                char *utime = tok;
                //strncpy(name_buf, username, 15);
                size_t loc = strcspn(utime, " ");
                utime[loc] = '\0';
                tstats->active_tasks[*count].utime = (unsigned int)atoi(utime);
            } 
            else if(tokens == 14) {
                /* Found a match */
                char *stime = tok;
                size_t loc = strcspn(stime, " ");
                stime[loc] = '\0';
                tstats->active_tasks[*count].stime = (unsigned int)atoi(stime);
                
            } 
            else if(tokens == 15) {
                /* Found a match */
                char *cutime = tok;
                size_t loc = strcspn(cutime, " ");
                cutime[loc] = '\0';
                tstats->active_tasks[*count].cutime = (unsigned int)atoi(cutime);
            }
            else if(tokens == 16) {
                /* Found a match */
                char *cstime = tok;
                size_t loc = strcspn(cstime, " ");
                cstime[loc] = '\0';
                tstats->active_tasks[*count].cstime = (unsigned int)atoi(cstime);
            } 

            else if(tokens == 21) {
                /* Found a match */
                char *startime = tok;
                size_t loc = strcspn(startime, " ");
                startime[loc] = '\0';
                tstats->active_tasks[*count].startime = (unsigned int)atoi(startime);
            } 
            tokens++;
        }
    }

    close(fd);

        // utime = tstats->active_tasks[*count].utime;
        // stime = tstats->active_tasks[*count].stime;
        //long total_time1 = total_cpu_usage(proc_dir);

        //sleep(1);

    // tokens = 0;
    // while (true) {
    //     ssize_t read_sz = lineread(fd, line, 1024);
    //     if (read_sz == 0 || read_sz == -1) {
    //         break;
    //     }

    //     char *next_tok = line;
    //     //printf("*************** Line is %s\n", line);
    //     while  (tokens < 15){
    //         tok = next_token(&next_tok, " ");
    //         //printf("Token %d is %s\n", tokens, tok);
    //         if (tokens == 13) {
    //             /* Found a match */
    //             char *utime = tok;
    //             //strncpy(name_buf, username, 15);
    //             size_t loc = strcspn(utime, " ");
    //             utime[loc] = '\0';
    //             tstats->active_tasks[*count].utime = (unsigned int)atoi(utime);
    //         } 
    //         else if(tokens == 14) {
    //             /* Found a match */
    //             char *stime = tok;
    //             size_t loc = strcspn(stime, " ");
    //             stime[loc] = '\0';
    //             tstats->active_tasks[*count].stime = (unsigned int)atoi(stime);  
    //         } 
    //         tokens++;
    //     }
    // }

    //close(fd);

        // utime2 = tstats->active_tasks[*count].utime;
        // stime2 = tstats->active_tasks[*count].stime;
        // long total_time2 = total_cpu_usage(proc_dir);


       // long user_util = 100 * (utime2 - utime1) / (total_time2 - total_time1);
        //long sys_util = 100 * (stime2 - stime1) / (total_time2 - total_time1);


        //printf("utime2 %d\n, utime1 %d\n, stime2 %d\n, stime1 %d\n, time_total2 %ld\n, time_total1 %ld\n", utime2, utime1, stime2, stime1, total_time2, total_time1);
        //printf("user_util %ld\n, sys_util %ld\n",user_util, sys_util);







        double uptime = pfs_uptime(proc_dir);
        printf("uptime is ::::, %f\n", uptime);


        total_time = tstats->active_tasks[*count].utime + tstats->active_tasks[*count].stime;
        total_time = total_time + tstats->active_tasks[*count].cutime + tstats->active_tasks[*count].cstime;

        printf("total_time is ::::, %d\n", total_time);

        useconds_t seconds = uptime - (tstats->active_tasks[*count].startime / hertz);

        printf("seconds is ::::, %d\n", seconds);
        if (seconds != 0){
            individual_cpu_usage = 100 * ((total_time / hertz) / seconds);
        } else {
            individual_cpu_usage = 0.0;
        }
        printf(" utime : %d, stime: %d , cutime: %d, cstime : %d, startime :%d, individual_cpu_usage: %f\n", tstats->active_tasks[*count].utime, tstats->active_tasks[*count].stime, tstats->active_tasks[*count].cutime, tstats->active_tasks[*count].cstime, tstats->active_tasks[*count].startime, individual_cpu_usage);
        printf("*************individual_cpu_usage is ....%f \n", individual_cpu_usage);
        if (strlen(tstats->active_tasks[*count].cpu_usage) == 0) {
            sprintf(tstats->active_tasks[*count].cpu_usage, "%.2f", individual_cpu_usage);
            printf("individual_cpu_usage is .... %s\n", tstats->active_tasks[*count].cpu_usage);
        }
    
    return;
}


// long total_cpu_usage(char* procfs_loc){

//     char fp[255];
//     strcpy(fp, procfs_loc);
//     strcat(fp, "/stat");

//     int fd = open(fp, O_RDONLY);
//     ssize_t read_sz;
//     char buf[1];
//     char one_line[100];
//     int length = 0;

//     char one_cpu[80];
//     //char two_cpu[80];

//     //read idel 1 and total 1
//     while((read_sz = read(fd, buf, 1)) >0) {
//         if(buf[0] == '\n') {
//             //if readed one line
//             one_line[length] = '\0';

//             strcpy(one_cpu, one_line);
//             one_line[0] = '\0';
//             length = 0;
//             break;

//         } else {
//             //in the middle of the line
//             one_line[length] = buf[0];
//             length++;
//         }
//     }
//     close(fd);


    // sleep(1);

    // //read idel 2 and total 2
    // fd = open(fp, O_RDONLY);

    // while((read_sz = read(fd, buf, 1)) >0) {

    //     if(buf[0] == '\n') {
    //         //if read one line
    //         one_line[length] = '\0';
    //         strcpy(two_cpu, one_line);
    //         one_line[0] = '\0';
    //         length = 0;
    //         break;

    //     } else {
    //         //in the middle of the line
    //         one_line[length] = buf[0];
    //         length++;
    //     }
    // }
    // close(fd);

    // char *one_tok = one_cpu;
    // char *one_temp;
    // long one_total = 0;
    // int token = 0;
    // //long idel1;

    // //token string
    // while((one_temp = next_token(&one_tok, " ")) != NULL){
    //     if(token == 4) {
    //         idel1 = atol(one_temp);
    //     }
    //     if(token > 0){
    //         one_total+=atol(one_temp);
    //     }
    //     token++;
    // }

    // long two_total = 0;
    // long idel2;
    // char *two_tok = two_cpu;
    // char *two_temp;
    // token = 0;

    // while((two_temp = next_token(&two_tok, " ")) != NULL){
    //     if(token == 4) {
    //         idel2 = atol(two_temp);
    //     }
    //     if(token > 0){
    //         two_total+=atol(two_temp);
    //     }
    //     token++;
    // }

    //calculate usage
    // float usage = 1-(float)(idel2-idel1)/(two_total-one_total);

//     if (one_total <=0){
//         return 0;
//     } else {
//         return one_total;
//     }

// }







