/****************************************************************************
 * iqa_fs - A virtual filesystem for use in IqaGui or other apps
 *
 *   Contains wrapper functions and virtual mount services to emulate
 *   to emulate NuttX operations.
 *
 *   Copyright (C) 2016 Ken Pettit. All rights reserved.
 *   Author: Ken Pettit <pettitkd@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/config.h>
#include <nuttx/fs/fs.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/fs/smart.h>
#include <nuttx/fs/nxffs.h>

#include <list>
#include "romfs/fs_romfs.c"
#include "romfs/fs_romfs.h"
#include "iqa_fs.h"

/****************************************************************************
 * Global Variables
 ****************************************************************************/

//linked list of mounted filesystems
std::list <iqa_mount_t> gMounts;

//romfs image array & length
extern unsigned char diagrams_fs[];
extern unsigned int diagrams_fs_len;


#ifndef CONFIG_EXAMPLES_ROMFS_SECTORSIZE
#  define CONFIG_EXAMPLES_ROMFS_SECTORSIZE 64
#endif

#define NSECTORS(b)        (((b)+CONFIG_EXAMPLES_ROMFS_SECTORSIZE-1)/CONFIG_EXAMPLES_ROMFS_SECTORSIZE)


/****************************************************************************
 * Name: iqa_init
 ****************************************************************************/

int iqa_fs_init(void)
{
    int ret;
    
    //Create ramdisk blkdriver
    ret = romdisk_register(0, diagrams_fs, NSECTORS(diagrams_fs_len), CONFIG_EXAMPLES_ROMFS_SECTORSIZE);
    if (ret < 0)
    {
        puts("ERROR: Failed to create RAM disk");
        return -1;
    }


    // Mount the romfs image, mount point is "./"
    ret = iqa_romfs_mount("./", "/dev/ram0");
    if(ret < 0)
    {
        puts("ERROR: Failed to mount romfs image");
        return -1;
    }
    
    return 0;
}

/****************************************************************************
 * Name: iqa_fopen
 ****************************************************************************/

IQA_FILE* iqa_fopen(const char *filename, const char *mode)
{
    IQA_FILE                *pFile = new IQA_FILE; 
    iqa_mount_t             *pDriver = NULL;
    int                     ret;
    char                    file_first_seg[32];
    
    //parse out first segment of file name
    int i = 0;
    while(filename[i] != '/' || i == 0)
    {
        file_first_seg[i] = filename[i];
        i++;
    }
    file_first_seg[i] = '/';
    file_first_seg[++i] = '\0';
    
    //Find the driver based on mounted drivers
    for(auto &aMount : gMounts)
    {
        //if the mountpoint string matches the parsed string 
        //from the filename, stop searching
        if(strcmp(aMount.pMtPointStr, file_first_seg) == 0)
        {
            //initialize pDriver to the found mount
            pDriver = new iqa_mount_t;
            *pDriver = aMount;

            //initialize struct file in pFile
            pFile->theFile.f_inode = aMount.pinode;
            pFile->theFile.f_seekpos = 0;
            pFile->theFile.f_pos = 0;
            pFile->theFile.f_priv = NULL;
            
            //initialize buffer data
            pFile->bufPtr = 0;
            for(unsigned int x = 0; x < sizeof(pFile->buf); x++)
                pFile->buf[x] = ' ';
            break;
        }
    }
    
    //test for NULL
    if(pDriver == NULL)
    {
        puts("ERROR: Failed to find mounted driver");
        delete pFile;
        return NULL;
    }
    
    // Convert mode from char* to int (in f_oflags member variable)
    if((pFile->theFile.f_oflags = retrieve_filemode(mode)) < 0)
    {
        puts("ERROR: invalid file open mode");
        delete pFile;
        delete pDriver;
        return NULL;
    }

    //Use the driver pointer to call the driver's open file
    if ((ret = pDriver->pOps->open(&pFile->theFile, filename, pFile->theFile.f_oflags, 0)) < 0)
    {
        puts("ERROR: File not found");
        delete pFile;
        delete pDriver;
        return NULL;
        
    }
    
    delete pDriver;
    return pFile;
}

/****************************************************************************
 * Name: iqa_fgets
 * read file buffer up to \n or until end of buf array is reached
 ****************************************************************************/

char * iqa_fgets(char * buf, int maxLen, IQA_FILE *pFile)
{

    int c = 0;
    int end_of_file = 0;
    
    //sanity check
    if(maxLen < 2)
        return NULL;
    
    buf[0] = '\0';

    //read bytes from pFile->buf up until the next \n
    while (!end_of_file)
    {
        // Copy a char from pFile->buf to output buf
        buf[c] = pFile->buf[pFile->bufPtr++];
        
        // Test if out buffer is out of data
        if (pFile->bufPtr >= sizeof(pFile->buf))
        {
            //clear old buffer
            clear_file_buf(pFile);
            
            
            //Refill pFile->buf
            
            //NOTE: u.imops->read() returns # characters read
            //      or returns <0 if error
            end_of_file = pFile->theFile.f_inode->u.i_mops
                        ->read(&pFile->theFile, pFile->buf, sizeof(pFile->buf)) == 0;

            if (end_of_file)
                pFile->bufPtr = sizeof(pFile->buf);
                
            else
                pFile->bufPtr = 0;
        }

        //make sure buffer isnt overflowing
        if (c >= maxLen - 2)
        {
            buf[c+1] = '\0';
            return buf;
        }
        // check for "\n" in output buffer
        if (buf[c] == '\n')
        {
            buf[c+1] = '\0';
            return buf;
        }
        else if (!end_of_file)
            c++;
    }

    return NULL;
}

/****************************************************************************
 * Name: iqa_fclose.  Maybe nothing to do here.  Maybe.
 ****************************************************************************/

IQA_FILE* iqa_fclose(IQA_FILE *file)
{
    delete file;
    file = NULL;
    return file;
}

/***************************************************************************
 * Name: iqa_mount
****************************************************************************/
int iqa_romfs_mount(const char *fs_path, const char *blkdriver_path)
{
    struct inode    *blkdriver;
    void            *fsHandle;
    iqa_mount_t     *newMount;
    int             ret;
    
  
    // open blockdriver using function from nuttx_services.c
    ret = open_blockdriver(blkdriver_path, 0, &blkdriver);
    if(ret < 0)
    {
        puts("ERROR: Failed to open blockdriver");
        return -1;
    }

    // call romfs_operations.bind() passing in blkdriver and receive fsHandle
    ret = romfs_operations.bind(blkdriver, NULL, &fsHandle);
    if(ret < 0)
    {
        puts("ERROR: romfs_operations::bind()");
        return -1;
    }

    //dynamically allocate new mount w/ fsHandle, romfs_operations and fs_path
    newMount = new iqa_mount_t;
    newMount->pDrvData = fsHandle;
    newMount->pOps = &romfs_operations;
    newMount->pMtPointStr = fs_path;

    // Create an inode struct for the FS to app interaction
    newMount->pinode = new struct inode;
    newMount->pinode->i_crefs = 1;
    newMount->pinode->i_private = fsHandle;
    newMount->pinode->u.i_mops = newMount->pOps;
    
    //append new mount to linked list, free allocated memory
    gMounts.push_back(*newMount);
    delete newMount;

    return 0;
}

/***************************************************************************
 * derive integer based oflags based on char* mode such as "w", "r" etc  
 ***************************************************************************/
int retrieve_filemode(const char *mode)
{
    if(strcmp(mode, "r") == 0)
        return O_RDONLY;
    if(strcmp(mode, "w") == 0)
        return O_WRONLY;
    if(strcmp(mode, "r+") == 0 || strcmp(mode, "w+") == 0)
        return O_RDWR;
    if(strcmp(mode, "a") == 0)
        return O_APPEND;
    
    return -1;
}

/****************************************************************************
 *null terminate all elements in the file buffer
 ****************************************************************************/
void clear_file_buf(IQA_FILE *pFile)
{
    for(unsigned int x = 0; x < sizeof(pFile->buf); x++)
    {
        pFile->buf[x] = '\0';
    }
}



















// vim: et sw=4 ts=4 cindent
