
#include "nuttx/drivers/ramdisk.h"
#include <nuttx/fs/fs.h>

typedef struct iqa_file_s
{
    //the array size may be changed & the program won't be affected
    char                                buf[512];
    unsigned int                        bufPtr;
    struct file                         theFile;
} IQA_FILE;

struct iqa_fs_driver
{
    const char                          *rootdir;
    struct inode                        *pinode;
};

//Nodes for gMounts linked list
typedef struct iqa_mount
{
    const char                              *pMtPointStr;
    const struct mountpt_operations         *pOps;
    struct inode                            *pinode;
    void                                    *pDrvData;
} iqa_mount_t;

int                 iqa_fs_init(void);
IQA_FILE*           iqa_fopen(const char*, const char *);
char*               iqa_fgets(char *, int, IQA_FILE*);
IQA_FILE*           iqa_fclose(IQA_FILE *);
int                 iqa_romfs_mount(const char*, const char*);
int                 retrieve_filemode(const char*);
void                clear_file_buf(IQA_FILE *pFile);














