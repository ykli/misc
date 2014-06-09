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
//		do {
			pthread_mutex_lock(&mutex);
			threadState = THREAD_SLEEP;
			pthread_cond_wait(&cond, &mutex);
			threadState = THREAD_WAKED;
			pthread_mutex_unlock(&mutex);
		stream = streamList->getEmptyStream();
//		} while ((stream = streamList->getEmptyStream()) == NULL);
		LOCATION();
		usleep(400000);
		LOCATION();
#if 0
		memset(params, 0, sizeof(params));

		/* Get a new frame from Source */
		handlerList->getFrameSource(source);
		source->getFrame(frame);

		/* Filter works */
		for (int step = 1; step < handlerList->totalStep() - 1; step++) {
			Filter *handler;
			handler = handlerList->getNodeByStep(step);
			handler->doProcess(frame, params);
//			handler->afterProcess(frame, params);
		}

		/* Last step */
		final = handlerList->getLastNode(step);
		final->doProcess(frame, params, stream->addr);
		frameList->putFilledStream(stream);
#endif
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
