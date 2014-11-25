#ifndef __LINEDETECTFILTER_HH_MY__
#define __LINEDETECTFILTER_HH_MY__

#include "FaceDetect.hh"
#include "Filter.hh"
#include <semaphore.h>
#include <unistd.h>

class FaceDetectFilter: public Filter {
	public:
		static FaceDetectFilter* createNew(int w, int h, int t);
		void doProcess(frame_t* frame_list, int nFrames, uint32_t *params);

	private:
		FaceDetectFilter(int w, int h, int t);
		~FaceDetectFilter();

		FaceDetect *facedetect;
		int handle_time;
		unsigned char *buf_detect;
		int shmid;
		char *shmptr;
		pid_t pid;
		int kill_cnt;
		int *handle_frame;
		int child_status;
		int count_frame;
		int picture_size;
		sem_t sem;
		face_detect_result_t *result;
};

#endif
