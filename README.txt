
                                                       
     +-------------------------------------------------------------------------+
     |  init()                                                                 |
     |  {                                                                      |
     |     romfs_ptr = romfs_init(g_romfs_image);                              |
     |     iqa_mount("/rom",   romfs_ptr);                                     |
     |     //iqa_mount("/f1000", squishfs_ptr);                                |
     |     //iqa_mount("/spi",   spiffs_ptr);                                  |
     |  }                                                                      |
     |                                                                         |
     |  iqa_fopen("/rom/diagrams/diag1_txt", "r");                             |
     |                                                                         |
     |                                                                         |
     |  iqa_mount(char *path, struct_ptr *);                                   |
     |                                                                         |
     |                                                                         |
     |                                                                         |
     |  IQA_FILE * iqa_fopen(char * path, const char *mode)                    |
     |  {                                                                      |
     |     IQA_FILE  *pfile = new IQA_FILE;                                    |
     |     fs_driver *pDriver;                                                 |
     |     char       file_first_segment[32];                                  |
     |                                                                         |
     |     // Parse out the file_first_segment up to the second '/'            |
     |     // TODO:  Do this                                                   |
     |                                                                         |
     |     // Find the driver based on mounted drivers                         |
     |     pDriver = gMounts->pFirstMountedDriver;                             |
     |     while (pDriver && strcmp(pDriver->mount_name != file_first_segment))|
     |        pDriver = pDriver->pNext;                                        |
     |                                                                         |
     |     pFile->pDriver = pDriver;                                           |
     |     // TODO test for null                                               |
     |                                                                         |
     |     // TODO:  derive integer based oflags based on char * mode such     |
     |               as "w", "w+", "r", "a", etc.                              |
     |               int version is like O_RDWR, O_RDONLY, etc.                |
     |                                                                         |
     |     // Use the driver pointer to call the driver's open file            |
     |     if ((ret = pDriver->u.i_mops->open(filep, path,                     |
     |         filep->f_oflags, mode)) != OK)                                  |
     |  }                                                                      |
     |                                                                         |
     |  iqa_fgets(IQA_FILE *, ...)                                             |
     |  iqa_fclose(IQA_FILE *, ...)                                            |
     |                                                                         |
     +---------+------------------------+--------------------------+-----------+
               |                        |                          |
               v                        v                          v          
     +------------------+      +--------+---------+      +------------------+ 
     |  romfs Driver    |      |  squishfs Driver |      |  spiffs Driver   | 
     |                  |      |                  |      |                  | 
     |  struct {        |      |  struct {        |      |  struct {        | 
     |   .open          |      |   .open          |      |   .open          | 
     |   .close         |      |   .close         |      |   .close         | 
     |   .read          |      |   .read          |      |   .read          | 
     |  }               |      |  }               |      |  }               | 
     |                  |      |                  |      |                  | 
     | g_romfs_image    |      |                  |      |                  | 
     +------------------+      +------------------+      +------------------+ 
                                                                              
                        
     read(fd, buff, how_many_bytes);
                        
                        
                        
                        
                        
                         
     
