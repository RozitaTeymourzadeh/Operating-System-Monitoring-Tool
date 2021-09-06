// Author: Rozita Teymourzadeh, Mathew Malensek 
#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/types.h>

/**
 * draw_percbar
 *
 * To print bar chart
 *
 * @param char *buf
 * @param double frac
 *
 **/
void draw_percbar(char *buf, double frac);

/**
 * uid_to_uname
 *
 * To convert uid to name
 *
 * @param char *name_buf
 * @param uid_t uid
 *
 **/
void uid_to_uname(char *name_buf, uid_t uid);

/**
 * itoa
 *
 * To convert int to char array
 *
 * @param int val
 * @param int base
 *
 **/
char* itoa(int val, int base);

/**
 * lineread
 *
 * To one line from the file 
 *
 * @param int fd
 * @param char *buf
 * @param size_t buf_sz
 *
 **/
ssize_t lineread(int fd, char *buf, size_t buf_sz);

#endif
