#include <stdio.h>
#include <stdlib.h>		//atoi
#include <fcntl.h>		//open
#include <sys/stat.h>		//open
#include <sys/types.h>		//lseek
#include <sys/wait.h>		//wait
#include <unistd.h>		//read, write, close, lseek
#include <errno.h>		//r_함수


int main(int argc, char * argv[])
{
	int fd;
	long size;
	off_t offset;

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
		printf("parent started waiting\n");
		int e;
		while (e = wait(NULL) > 0) child++;
		if (e == -1) perror("wait failed");
		else printf("process[%ld](%ld bytes) confirms that %d process have completed their tasks\n", (long)getpid(), size, child);
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
	}

	return 0;
}

