/*!
 * 
 * Program: .cutil
 * Description: Utility functions being used by inspetor and display
 * Author: Rozita Teymourzadeh, Mathew Malensek 
 * Date: 2020
 *
 * 
 */

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

#include "util.h"
#include "procfs.h"
#include "debug.h"

void draw_percbar(char *buf, double frac) {
    // memset(buf, 0, 30);

    buf[0] = 0;

    //null input
    if (isnan(frac)) {
        strcat(buf, "[--------------------] 0.0%");
        LOG("%s\n", "null");
        return;
    }
    //negative imput
    if (frac <= 0) {
        strcat(buf, "[--------------------] 0.0%");
        LOG("%s\n", "neg");
        return;
    }

    //>100 input
    if (frac >= 1) {
        strcat(buf, "[####################] 100.0%");
        LOG("%s\n", ">100");
        return;
    }

    //count number of #
    int draw = (((round(frac*100))/5));

    char temp_buf[20] = "";
    strcat(buf, "[");
    for (int i=0; i<draw; i++) {
        strcat(buf, "#");
    }
    for (int i=0; i<20-draw; i++) {
        strcat(buf, "-");
    }
    
    strcat(buf, "] ");
    sprintf(temp_buf, "%0.1f", frac*100);
    strcat(buf, temp_buf);
    strcat(buf, "%");
}

ssize_t lineread(int fd, char *buf, size_t buf_sz)
{
    ssize_t bytes_read = 0;
    char c = 0;
    while (c != '\n' && bytes_read < buf_sz) {
        ssize_t read_sz = read(fd, &c, 1);
        if (read_sz == 0) {
            return bytes_read;
        } else if (read_sz == -1) {
            return -1;
        }
        bytes_read += read_sz;
        *buf++ = c;
    }
    return bytes_read;
}


void uid_to_uname(char *name_buf, uid_t uid)
{
    char line[1024];
    int fd = open("/etc/passwd", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    memset(name_buf, 0, 15);

    while (true) {
        ssize_t read_sz = lineread(fd, line, 1024);
        if (read_sz == 0 || read_sz == -1) {
            break;
        }

        char *next_tok = line;
        char *username = next_token(&next_tok, ":x");
        char *uid_str = next_token(&next_tok, ":x");
        uid_t uid_num = atoi(uid_str);
        if (uid_num == uid) {
            /* Found a match */
            strncpy(name_buf, username, 15);
            name_buf[15] = '\0';
            break;
        }
    }

    if (strlen(name_buf) == 0) {
        /* Did not find a match */
        sprintf(name_buf, "%d", uid);
    }
}

char* itoa(int val, int base){
    
    static char buf[32] = {0};
    
    int i = 30;
    
    for(; val && i ; --i, val /= base)
    
        buf[i] = "0123456789abcdef"[val % base];
    
    return &buf[i+1];
    
}


