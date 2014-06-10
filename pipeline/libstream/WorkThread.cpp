#include <sys/types.h>
#include "WorkThread.hh"

void* work_thread(void *p);

WorkThread* WorkThread::createNew(HandlerList *h, FrameList *f, StreamList *s)
{
	return new WorkThread(h, f, s);
}

WorkThread::WorkThread(HandlerList *h, FrameList *f, StreamList *s)
	: handlerList(h), frameList(f), streamList(s)
{
	int error;

	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);

	error = pthread_create(&tid, NULL, work_thread, this);
	if (error) {
		DBG("thread create error\n");
	}
}

WorkThread::~WorkThread()
{
	pthread_join(tid, NULL);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}

void* work_thread(void *p)
{
	WorkThread *wt = (WorkThread *)p;

	wt->Thread();

	return NULL;
}

void WorkThread::Thread()
{
	while (1) {
		frame_t frame;
		frame_t *stream;

		/* Wait for wake up signal */
		pthread_mutex_lock(&mutex);
		threadState = THREAD_SLEEP;

		pthread_cond_wait(&cond, &mutex);

		threadState = THREAD_WAKED;
		pthread_mutex_unlock(&mutex);

		stream = streamList->getEmptyStream();

		LOCATION();
		usleep(60000);
		LOCATION();
//		memset(params, 0, sizeof(uint32_t));//This is wrong!!!

		/* Get a new frame from Source */
		FrameSource *source = handlerList->getFrameSource();
		source->getFrame(frame, params);

		/* Filter works */
		for (int step = 0; step < handlerList->getNumOfFilters(); step++) {
			Filter *filter = handlerList->getFilterByStep(step);;
			filter->process(frame, params);
		}

		/* Final step */
		FinalStep *final = handlerList->getFinalStep();
		final->process(frame, params, stream->addr);

		source->putFrame(frame);

		getTimeStamp(stream->timestamp);
		streamList->putFilledStream(stream);
	}
}

void WorkThread::wakeUpThread()
{
	/* This may be called from another thread */
	pthread_mutex_lock(&mutex);
	if (threadState == THREAD_WAKED)
		DBG("Thread work is not finished\n");
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}
