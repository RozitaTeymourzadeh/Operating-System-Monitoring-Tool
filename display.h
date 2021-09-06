/**
 * @file
 *
 * Handles text-based UI functionality.
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdbool.h>

/**
 * display_init
 *
 * Display feature
 *
 * @param bool enable_curses
 *
 **/
void display_init(bool enable_curses);

/**
 * display_refresh
 *
 * To convert int to char array
 *
 * @param char *procfs_loc
 *
 **/
void display_refresh(char *procfs_loc);

/**
 * display_quit
 *
 * Display feature
 *
 *
 **/
bool display_quit(void);

/**
 * display_stop
 *
 * Display stop
 *
 *
 **/
void display_stop(void);

#endif
