#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
//#include <sys/dir.h>
//#include <sys/file.h>
//#include <sys/unistd.h>
//#include <sys/ndir.h>
#include <time.h>
#include <errno.h>
//#include <unistd.h>
//#include <curses.h>
#include <windows.h>
#include <stddef.h>
#include <crtdefs.h>
#include <crtdbg.h>
#include <io.h>
#include <wchar.h>
#include <malloc.h>
#include <stdlib.h>
#include <share.h>
#define BUFSIS 7500000
//#define PERMS 0666
#define PERMS _S_IWRITE
#define MAXDECKS 1000
#define MAXCARDS 20000