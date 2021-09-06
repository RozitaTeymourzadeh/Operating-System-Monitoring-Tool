/**
 * @file
 *
 * Retrieves raw information from procfs, parses it, and provides easy-to-use
 * functions for displaying the data.
 * Author: Rozita Teymourzadeh, Mathew Malensek 
 */

#ifndef _PROCFS_H_
#define _PROCFS_H_
#include <sys/types.h>

/**
 * struct load_avg
 *
 * To hold load average
 *
 **/
struct load_avg {
    double one;
    double five;
    double fifteen;
};



/**
 * cpu_stats
 *
 * To hold CPU state
 *
 *
 **/
struct cpu_stats {
    long idle;
    long total;
};

/**
 * mem_stats
 *
 * To show mem_stats
 *
 *
 **/
struct mem_stats {
    double used;
    double total;
};

/**
 * task_stats
 *
 * To hold task info
 *
 *
 **/
struct task_stats {
    unsigned int total;
    unsigned int running;
    unsigned int waiting;
    unsigned int sleeping;
    unsigned int stopped;
    unsigned int zombie;
    struct task_info *active_tasks;

};

/**
 * cpu_usage
 *
 * To holdindividual CPU usage 
 *
 *
 **/
struct cpu_usage {
    unsigned int utime;
    unsigned int stime;
    unsigned int cutime;
    unsigned int cstime;
    unsigned int startime;
    //char cpu_buf[15];
};
/**
 * task_info
 *
 * To hold task information
 *
 *
 **/
struct task_info {
    pid_t pid;
    uid_t uid;
    char name[26];
    char state[13];
    char cpu_usage[15];
    unsigned int utime;
    unsigned int stime;
    unsigned int cutime;
    unsigned int cstime;
    unsigned int startime;
};

/**
 * pfs_hostname
 *
 * To convert int to char array
 *
 * @param char *proc_dir
 * @param char *hostname_buf
 * @param size_t buf_sz
 *
 **/
int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz);

/**
 * pfs_kernel_version
 *
 * To retreive kernel version
 *
 * @param char *proc_dir
 * @param char *version_buf
 * @param size_t buf_sz
 *
 **/
int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz);

/**
 * pfs_cpu_model
 *
 * To get cpu model
 *
 * @param char *proc_dir
 * @param char *model_buf
 * @param size_t buf_sz
 *
 **/
int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz);

/** pfs_cpu_units
*
* Get CPU unit
* @ param char *proc_dir
 */
int pfs_cpu_units(char *proc_dir);

/** pfs_uptime
* function to get a uptime
* @ param char *proc_dir
 */
double pfs_uptime(char *proc_dir);

/** pfs_format_uptime
* function to get a format
* @ double time
* @ char *uptime_buf
 */
int pfs_format_uptime(double time, char *uptime_buf);

/** pfs_load_avg
* function to get load average
* @ param char *proc_dir
* 
 */
struct load_avg pfs_load_avg(char *proc_dir);
/* Note: 'prev' is an input that represents the *last* CPU usage state. 'curr'
 * is an *output* that represents the current CPU usage state, and will be
 * passed back in to pfs_load_avg on its next call. */

/** pfs_cpu_usage
* function to get a hostname
*
* @ char *procfs_dir
* @ struct cpu_stats *prev
* @ struct cpu_stats *curr
 */
double pfs_cpu_usage(char *procfs_dir, struct cpu_stats *prev, struct cpu_stats *curr);

/** pfs_mem_usage
* function to get a hostname 
* 
* @ param char *procfs_dir
 */
struct mem_stats pfs_mem_usage(char *procfs_dir);
/* Note: these two functions create and destroy task_stats structs. Depending on
 * your implementation, this might just be a malloc/free. */

/**
 * task_stats
 *
 * to create task state
 *
 *
 **/
struct task_stats *pfs_create_tstats();

/** pfs_destroy_tstats
*
* function to destroy the function
* @ struct task_stats *tstats
 */
void pfs_destroy_tstats(struct task_stats *tstats);

/**
 * pfs_tasks
 *
 * pfs task report
 *
 * @param char *proc_dir
 * @param struct task_stats *tstats
 *
 **/
int pfs_tasks(char *proc_dir, struct task_stats *tstats);

/** get_task_list
* To parse through the task list
* @param char* procfs_loc
* @param char* process
* @param struct task_stats *tstats
*
 */
void get_task_list(char* procfs_loc, char* process, struct task_stats *tstats, int *count);

/**
 * is_digit
 *
 * To check the directory for process folder
 *
 * @param char d_name[]
 * @param int len
 *
 **/
int is_digit(char d_name[], int len);

/** next_token
* To parse through the task list
* @param char **str_ptr
* @param const char *delim
* @return char
*
 */
char *next_token(char **str_ptr, const char *delim);

/** get_process_usage
* To get individual cpu process
* @param char* proc_dir
* @param char* process
* @struct cpu_stats *tcpu_usage
* @int *count
*
 */
void get_process_usage(char* proc_dir, char* process, struct task_stats *tstats, int *count);
long total_cpu_usage(char* procfs_loc);

#endif
