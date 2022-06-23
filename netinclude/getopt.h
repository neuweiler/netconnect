#ifndef GETOPT_H
#define GETOPT_H \
       "$Id: getopt.h,v 1.1.1.1 1999/08/08 14:20:30 zapek Exp $"
/*
 *      Prototypes and declarations for getopt() package
 *
 *      Copyright © 1994 AmiTCP/IP Group, 
 *                       Network Solutions Development Inc.
 *                       All rights reserved.
 */

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;
int getopt(int argc, char * const argv[], char const *opts);

#endif /* GETOPT_H */
