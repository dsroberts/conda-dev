/*

Intercept the very specific case where the 'read' function returns EIO
and retry

gcc -Wall -o read_intercept.so -fPIC -shared read_intercept.c -ldl

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *getFnFromFd(int fd) {
    char *fdPath = NULL;
    char *linkTarget = NULL;
    
    int rd;

    if ( ( rd = asprintf(&fdPath,"/proc/self/fd/%d",fd) == -1 ) ) {
        perror("Failed to write link name string");
        return NULL;
    }

    if ( ( linkTarget = realpath(fdPath,linkTarget) ) == NULL ) {
        perror("Failed to resolve symlink");
        return NULL;
    }

    free(fdPath);
    return linkTarget;

}

ssize_t (*old_read)(int fildes, void *buf, size_t nbyte) = NULL;

ssize_t read(int fildes, void *buf, size_t nbyte) {
    if ( old_read == NULL ){
        old_read = dlsym(RTLD_NEXT, "read");
    }

    ssize_t rd;
    char *fn = NULL;

    while ( ( rd = old_read(fildes,buf,nbyte) ) < 0 ) {
        if ( errno != EIO ) {
            // Got a real error, bail now
            break;
        }

        int ret;        
        off_t fpos;
        
        if ( ( fpos = lseek(fildes,0,SEEK_CUR) ) == -1 ) {
            perror("Failed to recover current file offset");
        }

        
        if ( fn == NULL ) {
            fn = getFnFromFd(fildes);
        }

        if ( ( ret = close(fildes) ) == -1 ) {
            perror("Failed to close existing fd");
        }
        // Maybe a long delay will help?
        sleep(1);
        // Can assume read-only
        int newfildes = openat(AT_FDCWD,fn,O_RDONLY|O_CLOEXEC);

        // Read the entire file into memory start to finish
        off_t fend;
        
        if ( ( fend = lseek(newfildes,0,SEEK_END) ) == -1 ) {
            perror("Failed to recover end of file position");
        }

        char *fileContent = (char *)malloc((size_t)fend+1);
        if ( fileContent == NULL ) {
            perror("Failed to allocate memory for file contents");
        }

        ssize_t bytesRead;
        ssize_t readTotal = 0;
        while ( readTotal < (size_t)fend ) {
            bytesRead = old_read(newfildes,fileContent,fend - bytesRead);
            if ( bytesRead == 0 ) {
                if ( fend == lseek(newfildes,0,SEEK_CUR) ) {
                    break;
                } else {
                    perror("Error reading file");
                }

            }
            readTotal += bytesRead;
        }
        fileContent[fend] = '\0';

        // Wait a bit
        sleep(1);

        // Nuke the buffer
        free(fileContent);

        if ( newfildes != fildes ) {
            if ( ( ret = dup2(newfildes,fildes) ) == -1 ) {
                perror("Error duplicating fd");
            }
            if ( ( ret = close(newfildes)  ) == -1 ) {
                perror("Failed to close new fd");
            }
        }
        lseek(fildes,fpos,SEEK_SET);
    }

    free(fn);
    return rd;

}