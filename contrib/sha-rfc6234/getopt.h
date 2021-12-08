#pragma once

/* Quick and dirty replacement for POSIX function getopt() and its environment
   Borrwoed from: https://stackoverflow.com/questions/10404448/getopt-h-compiling-linux-c-code-in-windows
*/

extern int opterr;      /* if error message should be printed */
extern int optind;      /* index into parent argv vector */
extern int optopt;      /* character checked for validity */
extern int optreset;    /* reset getopt */
extern char* optarg;    /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

/*
* getopt --
*      Parse argc/argv argument vector.
*/
int getopt(int nargc, char* const nargv[], const char* ostr);
