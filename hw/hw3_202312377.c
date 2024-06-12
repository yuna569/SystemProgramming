#include <stdio.h>
#include <stdlib.h>        //atoi
#include <fcntl.h>        //open
#include <sys/stat.h>        //open
#include <sys/types.h>        //lseek
#include <sys/wait.h>        //wait
#include <unistd.h>        //read, write, close, lseek
#include <errno.h>        //r_함수


int main(int argc, char * argv[])
{
    int fd;
    long size;
    off_t offset;
    int pipefd[2];
    
    if(pipe(pipefd) < 0) {
        perror("pipe failed");
        return -1;
    }

    if (argc != 3) return -1;

    if ( (fd = open(argv[1], O_RDONLY)) == -1 ) {
        perror("failed to open");
        return -1;
    }
    offset = lseek(fd, 0, SEEK_END);
    size = offset;

    printf("file size: %ld bytes\n", size);

    close(fd);

    int n = atoi(argv[2]);
    long d = size / n;

    printf("d: %ld bytes\n", d);

    int i = 0;

    for (i = 0; i < n; i++) {
        if (fork()) continue;
        else {
            printf("child number %d opened\n", i+1);
            if ( (fd = open(argv[1], O_RDONLY)) == -1 ) {
                perror("failed to open");
                return -1;
            }
            offset = lseek(fd, d*i, SEEK_SET);
            break;
        }
    }

    int x;
    int child = 0;

    long childsize = 0;
    char buf[100];
    long count = 0;

    if (i == n) {
        long total_count = 0;
        
        printf("parent started waiting\n");
        
        int e;
        while (e = wait(NULL) > 0) child++;
        if (e == -1) perror("wait failed");
        else {
            for (int i = 0; i < n; i++) {
                long count;
                if (read(pipefd[0], &count, sizeof(count)) == -1) {
                    perror("read from pipe failed");
                    close(pipefd[0]);
                    exit(1);
                }
                total_count += count;
            }
            close(pipefd[0]);
            
            printf("process[%ld](%ld bytes) has found %ld alphabet letters in %s\n", (long)getpid(), size, total_count, argv[1]);
        }
    }
    else {
        printf("child number %d started reading\n", i+1);

        long next = d * (i+1);
        while ( (offset < next) && (x = read(fd, buf, 100)) ) {
            if (x == -1) perror("read failed");
            
            offset = lseek(fd, 0, SEEK_CUR);

            if ( (offset >= next) && (i != n-1) ) {
                x -= (offset-next);
                offset -= offset-next;
                lseek(fd, offset, SEEK_SET);
            }

            childsize += x;


            for (int j = 0; j < x; j++) {
                char a = buf[j];
                if ( ((a >= 65) && (a <= 90)) | ((a >= 97) && (a <= 122)) ) count++;
            }
        }
        printf("process[%ld](%ld bytes) has found %ld alphabet letters in (%ld ~ %ld)\n", (long)getpid(), childsize, count, d*i, offset-1);
        
        if (write(pipefd[1], &count, sizeof(count)) == -1) {
            perror("write to pipe failed");
            close(fd);
            close(pipefd[1]);
            exit(1);
        }
        close(fd);
        close(pipefd[1]);
        exit(0);
    }

    return 0;
}

