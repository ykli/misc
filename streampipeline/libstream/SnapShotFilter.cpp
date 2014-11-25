#include "SnapShotFilter.hh"

void command_handler(char *buf);
static int flag = 0;

SnapShotFilter* SnapShotFilter::createNew(int w, int h, const char* path)
{
	return new SnapShotFilter(w, h, path);
}

SnapShotFilter::SnapShotFilter(int w, int h, const char* path)
	: Filter("SnapShotFilter"), width(w), height(h), picCnt(0)
{
	sprintf(savePath, "%s", path);
	signal_init(command_handler, "command");
}

void command_handler(char *buf)
{
	flag = 1;
	return ;
}

void SnapShotFilter::doProcess(frame_t* frame_list, int nFrames, uint32_t *params)
{
	LOCATION("\n");

	volatile frame_t* frame1 = getFrameByType(frame_list, nFrames, PRIMARY);
	volatile frame_t* frame2 = getFrameByType(frame_list, nFrames, FUNCTIONAL);

	if (flag == 1) {
		char path1[PATH_MAX];
		char path2[PATH_MAX];
		sprintf(path1, "stream1-%04d", picCnt);
		sprintf(path2, "stream2-%04d", picCnt);

		int fd1 = open(path1, O_CREAT | O_RDWR);
		if (fd1 < 0) {
			printf("open path1:%s Error\n", path1);
		}

		int fd2 = open(path2, O_CREAT | O_RDWR);
		if (fd2 < 0) {
			printf("open path2:%s Error\n", path2);
		}

		write(fd1, frame1->addr, frame1->size);
		write(fd2, frame2->addr, frame2->size);
		close(fd1);
		close(fd2);
		flag = 0;
		picCnt++;
	}
}
