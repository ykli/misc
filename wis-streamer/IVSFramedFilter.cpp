#include "Options.hh"
#include "IVSFramedFilter.hh"
#include "Err.hh"

static void* readFrame(void* clientData);

IVSFramedFilter* IVSFramedFilter::createNew(UsageEnvironment& env,
					   FramedSource* inputSource) {
	return new IVSFramedFilter(env, inputSource);
}

IVSFramedFilter::IVSFramedFilter(UsageEnvironment& env, FramedSource* inputSource)
	: FramedFilter(env, inputSource), fDoneFlag(0), threadFlag(1), isFirstFrame(1), frameCounter(0) {

	for (int i = 0; i < NR_FRAMEBUF; i++) {
		struct frame_list *framebuffer = &framebuffers[i];
		framebuffer->data = valloc(videoWidth * videoHeight * 2);

		if (i == 0)
			framebuffer->pre = &framebuffers[NR_FRAMEBUF - 1];
		else
			framebuffer->pre = framebuffer - 1;

		if (i == (NR_FRAMEBUF - 1))
			framebuffer->next = &framebuffers[0];
		else
			framebuffer->next = framebuffer + 1;
	}
	curframe = &framebuffers[0];

	struct ivs_buf_info info = {2.0, videoWidth, videoHeight, 6, ivsDiffThres < 0 ? 10 : ivsDiffThres};
	if (init_ivs_buf(info) < 0) {
		printf("init ivs buf failed\n");
	}
	detbuf = (uint8_t*)valloc(videoWidth * videoHeight);

	sem_init(&semStart, 0, 0);

	int error;
	error = pthread_create(&tid, NULL, readFrame, this);
	if (error) {
		printf("thread create error\n");
	}
}

IVSFramedFilter::~IVSFramedFilter() {
	threadFlag = 0;

	sem_post(&semStart);
	pthread_join(tid, NULL);
	sem_destroy(&semStart);
	setDoneFlag();

	deinit_ivs_buf();

	for (int i = 0; i < NR_FRAMEBUF; i++)
		free(framebuffers[i].data);

	free(detbuf);
}

static void* readFrame(void* clientData) {
	IVSFramedFilter* f = (IVSFramedFilter*)clientData;

	f->readFrame1();

	return NULL;
}

void IVSFramedFilter::readFrame1() {
	while (threadFlag) {
		sem_wait(&semStart);
		if (!threadFlag)
			break;
		fInputSource->getNextFrame((unsigned char *)curframe->data, fMaxSize,
								   afterGettingFrame, this,
								   FramedSource::handleClosure, this);
	}
}

void IVSFramedFilter::doGetNextFrame() {
	if (isFirstFrame) {
		sem_post(&semStart);
		isFirstFrame = 0;
	}
	envir().taskScheduler().doEventLoop(&fDoneFlag);
	clearDoneFlag();
	sem_post(&semStart);
	fPresentationTime = foutFrame.timestamp;
	afterGetting(this);
}

void IVSFramedFilter
::afterGettingFrame(void* clientData, unsigned frameSize,
		    unsigned numTruncatedBytes,
		    struct timeval presentationTime,
		    unsigned durationInMicroseconds) {
	IVSFramedFilter* source = (IVSFramedFilter*)clientData;
	source->afterGettingFrame1(frameSize, numTruncatedBytes,
                             presentationTime, durationInMicroseconds);
}

void IVSFramedFilter
::afterGettingFrame1(unsigned frameSize,
					 unsigned numTruncatedBytes,
					 struct timeval presentationTime,
					 unsigned durationInMicroseconds) {
	if (moveDetectEn) {
		bool MoveFlag = true;

		FormatConvertor::tile420_to_grey((uint8_t*)curframe->data, detbuf, videoWidth, videoHeight);
		MoveFlag = detect_ivs_buf_moving(detbuf);

		if ((++frameCounter % refreshMax) == 1)
			MoveFlag = true;

		if (MoveFlag == false)
			curframe = curframe->pre;
		*(foutFrame.moveFlagAddr) = MoveFlag;
	}

	*(foutFrame.data) = curframe->data;
	curframe = curframe->next;

	foutFrame.timestamp = presentationTime;
	setDoneFlag();
}
