#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "WriteResult.hh"

WriteResult::WriteResult(char *ret_store_path): fd(-1), path(ret_store_path)
{
	if ((NULL == path) || ((fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0)) {
		printf("open %s failed! \n", path);
	}
}

WriteResult::~WriteResult()
{
	if (fd < 0) return;
	close(fd);
}
void WriteResult::WriteFd(char *ibuf)
{
	int write_cnt = 0;
	int write_total = strlen(ibuf);
	int write_left = write_total;

	if (fd < 0) return;

	if (lseek(fd, 0, SEEK_SET) < 0) {
		printf("seek %s error\n", path);
	}
rewrite:
	while ((write_cnt = write(fd, ibuf + (write_total - write_left), write_left)) > 0) {
		write_left -= write_cnt;
	}
	if (write_cnt < 0 && write_left > 0) {
		write_cnt = 0;
		if (errno == EAGAIN) {
			goto rewrite;
		} else {
			printf("write %s failed\n", path);
		}
	}
}
