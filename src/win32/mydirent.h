/*  dirent.h

    Definitions for POSIX directory operations.

*/

#ifndef __DIRENT_H
#define __DIRENT_H

#include <windows.h>  /* For WIN32_FIND_DATA */

#pragma pack(push, 1)

#ifdef FG_NAMESPACE
namespace fgl {
#endif


/**
 dirent structure returned by readdir().
 */
struct dirent
{
	char        d_name[260];
};


/**
 DIR type returned by opendir().  The members of this structure
 must not be accessed by application programs.
*/
typedef struct
{
	unsigned long _d_hdir;              /* directory handle */
	char         *_d_dirname;           /* directory name */
	unsigned      _d_magic;             /* magic cookie for verifying handle */
	unsigned      _d_nfiles;            /* no. of files remaining in buf */
	char          _d_buf[sizeof(WIN32_FIND_DATA)];  /* buffer for a single file */
} DIR;



/* Prototypes.
 */
DIR             * opendir  (const char *__dirname);
struct dirent   * readdir  (DIR *__dir);
int               closedir (DIR *__dir);
void              rewinddir(DIR *__dir);

#define S_ISDIR

#ifdef FG_NAMESPACE
}
#endif


/* restore default packing */
#pragma pack(pop)
#endif  /* __DIRENT_H */

