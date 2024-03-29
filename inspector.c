/*!
 * 
 * Program: inspector.c
 * Description: Functions to inspect the memory usage and print the active process
 * Author: Rozita Teymourzadeh, Mathew Malensek 
 * Date: 2020
 * @file
 *
 * System inspector implementation: a Unix utility that inspects the system it
 * runs on and creates a summarized report for the user using the proc pseudo
 * file system.
 *
 * 
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "display.h"

/**
 * Contains program command line options.
 */
struct inspector_opts {
    char *procfs_loc;
    bool one_shot;
    int interval;
};

/* Function prototypes */
void print_usage(char *argv[]);

/**
 * Prints help/program usage information.
 *
 * This output is displayed if there are issues with command line option parsing
 * or the user passes in the -h flag.
 */
void print_usage(char *argv[])
{
    printf("Usage: %s [-ho] [-i interval] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -h              Display help/usage information\n"
"    * -i interval     Set the update interval (default: 1000ms)\n"
"    * -p procfs_dir   Set the expected procfs mount point (default: /proc)\n"
"    * -o              Operate in one-shot mode (no curses or live updates)\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    /* Set up the default options: */
    struct inspector_opts options = {
        "/proc",
        false,
        1000
    };

    /* Set to true if we are using a non-default proc location */
    bool alt_proc = false;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "hi:p:o")) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv);
                return 0;
            case 'i':
                options.interval = atoi(optarg);
                break;
            case 'p':
                options.procfs_loc = optarg;
                alt_proc = true;
                break;
            case 'o':
                options.one_shot = true;
                break;
            case '?':
                if (optopt == 'p') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (alt_proc == true) {
        LOG("Using alternative proc directory: %s\n", options.procfs_loc);
        if (0 != access(options.procfs_loc, F_OK)) {
            return -1;
        }
    }

    if (options.one_shot == true) {
        display_init(false);
        display_refresh(options.procfs_loc);
        return 0;
    }

    display_init(true);
    bool running = true;
    while (running) {
        display_refresh(options.procfs_loc);
        for (int i = 0; i < 5; ++i) {
            usleep(options.interval * 1000 / 5);
            if (display_quit() == true) {
                running = false;
            }
        }
    }

    display_stop();
    return 0;
}

