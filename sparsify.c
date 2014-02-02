    #define _FILE_OFFSET_BITS 64
    #define _GNU_SOURCE             /* See feature_test_macros(7) */
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <stdio.h>
    #include <string.h>
    #include <linux/falloc.h>
    #include <unistd.h>
     
    #define BLOCKSIZE 4096
     
//LICENSE
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.


//supplied by remcohn from #btfs on Freenode
//originally punch.c
//but sparsify is easier to remember
//  -- cheater

    int main(int argc, char** argv) {
        ssize_t rsize;
        int fh, result;
        char buff[BLOCKSIZE], ebuff[BLOCKSIZE];
        off_t curpos, pstart, psize;
        long int freed;
     
        if (argc < 2) {
            printf("%s [file...]\n", argv[0]);
            return 0;
        }
     
        memset(ebuff, 0, BLOCKSIZE);        // prepare a block of 0's
        int i;
        for (i = 1; i < argc; i++) {
            char* file=argv[i];
            curpos = 0;
            pstart = 0;
            psize = 0;
            freed = 0;
     
            fh = open(file, O_RDWR, O_NOATIME);
            if (fh == -1) {
                perror("open()");
                return -1;
            }
     
            printf("sparsifying %s ", file);
            fflush(stdout);
            while ((rsize = read(fh, buff, BLOCKSIZE)) > 0) {
                result = memcmp(buff, ebuff,rsize);
                if (result == 0) { // block is empty
                    if (pstart == 0) { // previous block as not empty?
                        pstart = curpos;
                        psize = rsize; // save for later punching
                    } else {
                        psize += rsize; // previous block was empty too, add size
                    }
     
                    freed += rsize;
                } else if (pstart) { // block is not empty and we have a block that we still need to punch
                    result = fallocate(fh, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, pstart, psize);
                    if (result == -1) {
                        perror("fallocate()");
                        return -1;
                    }
                    pstart = 0;
                    psize = 0;
                }
                curpos += rsize;
            }
     
            if (pstart) { // still a block to do ?
                result = fallocate(fh, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, pstart, psize);
                if (result == -1) {
                    perror("fallocate()");
                    return -1;
                }
            }
     
            if (rsize == 0) {
                printf("done. freed %ld bytes\n", freed);
                continue;
            }
            if (rsize == -1) {
                perror("read()");
                return -1;
            }
        }
        return 0;
    }

