#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "FaceDetectFilter.hh"
#include <sys/wait.h>

#define FRAME_WAIT_TIME	(5 * 1000)
#define SHM_SIZE	(0x1000)
#define SHM_MOD		(0666)
#define KILL_TIMES	(10)
//#define DISPLAY		(30 * 1)

volatile static int nofirst_trywait = 0;

static void draw(volatile frame_t *frame1, int min_x, int max_x, int min_y, int max_y, int color)
{
	int x, y;
	for(x = min_x * 2, y = min_y * 2; x <= max_x * 2; x++) {
		((uint8_t*)frame1->addr)[y * 1280 + x] = color;
	}
	for(x = min_x * 2, y = max_y * 2; x <= max_x * 2; x++) {
		((uint8_t*)frame1->addr)[y * 1280 + x] = color;
	}
	for(y = min_y * 2, x = min_x * 2; y <= max_y * 2; y++) {
		((uint8_t*)frame1->addr)[y * 1280 + x] = color;
	}
	for(y = min_y * 2, x = max_x * 2; y <= max_y * 2; y++) {
		((uint8_t*)frame1->addr)[y * 1280 + x] = color;
	}
}

void sig_handler(int signo)
{
	if (signo == SIGUSR2) {
		sigset_t zeromask;
		sigemptyset(&zeromask);
		sigsuspend(&zeromask);
	}

	if (signo == SIGUSR1) {
		/* nothing to be done */
	}
}

void child_handler(FaceDetect *facedetect, void *buf_detect,
		char *result, sem_t *sem, int *handle_frame)
{
	while(1) {
		if((*handle_frame) == 0) {
			//sum_frame++;
			//printf("sum_frame : %d\n", sum_frame);
			//printf("func:%s,line:%d\n", __func__, __LINE__);
			facedetect->process((unsigned char *)buf_detect, sem, result);
			//printf("func:%s,line:%d\n", __func__, __LINE__);
			(*handle_frame) = 1;
			//printf("func:%s,line:%d, *handle_frame = %d\n", __func__, __LINE__, *handle_frame);
		}
		usleep(FRAME_WAIT_TIME);
	}
}

FaceDetectFilter* FaceDetectFilter::createNew(int w, int h, int t)
{
	return new FaceDetectFilter(w > 640 ? 640 : w, h > 360 ? 360 : h, t);
}

FaceDetectFilter::FaceDetectFilter(int w, int h, int t)
	: Filter("FaceDetectFilter"), handle_time((t == -1) ? 5000 : (t * 1000)),
	pid(-1), kill_cnt(KILL_TIMES), count_frame(0), picture_size(w*h)
{
	facedetect = FaceDetect::createNew(w, h);

	if (sem_init(&sem, 1, 1) < 0) {
		printf("sem_init faild\n");
		return;
	}

	if ((shmid = shmget(IPC_PRIVATE, 4 + sizeof(face_detect_result_t) + (DISPLAY > 1024 ? DISPLAY : 1024) * sizeof(coordinates_t) + picture_size + 512, SHM_MOD)) < 0) {
		printf("shmget faild\n");
		return;
	}

	/*
	 * after test, the shmptr is the same in parent and child,
	 * so here I shmat is reasonable
	 */
	if ((shmptr = (char *)shmat(shmid, NULL, 0)) < 0) {
		printf("parent attach %d to memory faild\n", shmid);
		return;
	}
	handle_frame = (int *)shmptr;
	*handle_frame = 0;
	result = (face_detect_result_t *)(shmptr + sizeof(*handle_frame));
	result->buf = (coordinates_t *)((char *)result + sizeof(face_detect_result_t));
	buf_detect = (unsigned char *)((char *)result->buf + (DISPLAY > 1024 ? DISPLAY : 1024) * sizeof(coordinates_t) + 256);
	//printf("handle_frame=%p, result=%p, result->buf=%p, buf_detect=%p\n", handle_frame, result, result->buf, buf_detect);
}

FaceDetectFilter::~FaceDetectFilter()
{
	while ((kill_cnt-- > 0) && (kill(pid, SIGKILL) != 0)) {
		//printf("func:%s, line:%d, test_cnt:%d, kill_cnt:%d\n", __func__, __LINE__, test_cnt, kill_cnt);
	}

	while ((pid >= 0) && (wait(&child_status) != pid)) {
		if (errno == EINTR) {
			printf("errno EINTR\n");
			continue;
		} else {
			printf("no existing unwaited-for child processes\n");
			break;
		}
	}

	if (sem_destroy(&sem) < 0) {
		printf("sem_init faild\n");
	}

	if ((shmptr > 0) && (shmdt(shmptr) < 0)) {
		printf("parent dattach %d from memory:%p faild\n", shmid, shmptr);
	}
	if ((shmid >= 0) && (shmctl(shmid, IPC_RMID, NULL) < 0)) {
		printf("parent rm shmid:%d failed\n", shmid);
	}
}

void FaceDetectFilter::doProcess(frame_t* frame_list, int nFrames, uint32_t *params)
{
	LOCATION("\n");
	int kill_times = 0;

	frame_t* frame2 = getFrameByType(frame_list, nFrames, FUNCTIONAL);/* For analysis */
	if (count_frame == 0) {
		//printf("func:%s,line:%d\n", __func__, __LINE__);
		memmove((void *)buf_detect, (void *)frame2->addr, picture_size);
		if ((pid = fork()) < 0) {
			printf("fork failed\n");
			return;
		} else if (pid == 0) { /*child*/
			sigset_t newmask, oldmask;
			struct sigaction act1, act2, oact;

			act1.sa_handler = sig_handler;
			sigemptyset(&act1.sa_mask);
			act1.sa_flags = 0;
			if (sigaction(SIGUSR1, &act1, &oact) < 0) {
				printf("sigaction SIGUSR1 new faild\n");
				return;
			}

			act2.sa_handler = sig_handler;
			sigemptyset(&act2.sa_mask);
			act2.sa_flags = 0;
			if (sigaction(SIGUSR2, &act2, NULL) < 0) {
				printf("sigaction SIGUSR2 new faild\n");
				return;
			}

			sigemptyset(&newmask);
			sigaddset(&newmask, SIGUSR1);
			if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0){
				printf("sigprocmask:SIG_BLOCK failed\n");
				return;
			}
			child_handler(facedetect, buf_detect, (char *)result, &sem, handle_frame);
			if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0){
				printf("sigprocmask:SIG_SETMASK failed\n");
				return;
			}
			if (sigaction(SIGUSR1, &oact, NULL) < 0) {
				printf("sigaction reseve faild\n");
				return;
			}
			return;
		}
		count_frame = 1;
	} else {
		if((*handle_frame) == 1) {
			//printf("func:%s,line:%d\n", __func__, __LINE__);
			memmove((void *)buf_detect, (void *)frame2->addr, picture_size);
			(*handle_frame) = 0;
		}
		kill_times = kill_cnt;
		while ((kill_times-- > 0) && (kill(pid, SIGUSR1) != 0));
	}

	//printf("func:%s,line:%d\n", __func__, __LINE__);
	usleep(handle_time);
	kill_times = kill_cnt;
	while ((kill_times-- > 0) && (kill(pid, SIGUSR2) != 0));

#if 1
	sem_wait(&sem);
	if (result->num != 0) {
		//printf("func:%s,line:%d, result->num = %d\n", __func__, __LINE__, result->num);
		int draw_frame = 0;
		frame_t* frame1 = getFrameByType(frame_list, nFrames, PRIMARY);	/* Primary stream */
		for(int i = 0; i < result->num; i++) {
			draw_frame++;
			if(draw_frame < DISPLAY) {
				draw(frame1, result->buf[i].minx, result->buf[i].maxx, result->buf[i].miny, result->buf[i].maxy, 0xff);
				draw(frame1, result->buf[i].minx - 1, result->buf[i].maxx + 1, result->buf[i].miny - 1, result->buf[i].maxy + 1, 0);
				draw(frame1, result->buf[i].minx + 1, result->buf[i].maxx - 1, result->buf[i].miny + 1, result->buf[i].maxy - 1, 0);
			}
		}
	}
	sem_post(&sem);
#endif
}
