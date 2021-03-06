/****************************************************************************
 * main - A virtual filesystem for use in IqaGui or other apps
 *
 *   Copyright (C) 2021 IQ-Analog
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

#include <stdio.h>

#include "iqa_fs.h"


extern "C"
{
int main(int argc, char *argv[])
{
    IQA_FILE *fd;
    FILE* fp;
    FILE *pFile;
    char     buf[256];
    char buf2[256];

    // Initialize the romfs
    iqa_fs_init();
	
    // Try to open one of the diagram files
    fd = iqa_fopen("./diagrams/f1000_tx_block.txt", "r");
    pFile = fopen("IQA_FILE.txt", "w");

    // Read from the file until EOF
    while (iqa_fgets(buf, sizeof(buf), fd) != NULL)
    {
        printf("%s", buf);
        fprintf(pFile, "%s", buf);
    }

    iqa_fclose(fd);
    fclose(pFile);
    
    puts("\n\n\n\n\n\n\n\n");
    
    fp = fopen("./diagrams/f1000_tx_block.txt", "r");
    pFile = fopen("FILE.txt", "w");
    while (fgets(buf2, sizeof(buf2), fp) != NULL)
    {
        printf("%s", buf2);
        fprintf(pFile, "%s", buf2);
    }

    fclose(fp);
    fclose(pFile);
    
    return 0;
}

} /* extern C */

