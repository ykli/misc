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

	threadState = THREAD_WAKED;
	error = pthread_create(&tid, NULL, work_thread, this);
	if (error) {
		DBG("thread create error\n");
	}
}

WorkThread::~WorkThread()
{
	pthread_mutex_lock(&mutex);
	if(threadState == THREAD_SLEEP) {
		threadState = THREAD_STOP;
		pthread_cond_signal(&cond);
	} else {
		threadState = THREAD_STOP;
	}
	pthread_mutex_unlock(&mutex);

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
	pthread_mutex_lock(&mutex);
	while (threadState != THREAD_STOP) {
		frame_t frame_list[MAX_FRAMES];
		int nFrames = 0;
		frame_t *stream;

		/* Wait for wake up signal */
		threadState = THREAD_SLEEP;
		pthread_cond_wait(&cond, &mutex);
		if(threadState == THREAD_STOP)
			break;
		threadState = THREAD_WAKED;

		stream = streamList->getEmptyStream();
		memset(params, 0, sizeof(params));

		/* Get a new frame from Source */
		FrameSource *source = handlerList->getFrameSource();

		source->getFrame(frame_list, nFrames, params);

		/* Filter works */
		for (int step = 0; step < handlerList->getNumOfFilters(); step++) {
			Filter *filter = handlerList->getFilterByStep(step);
			filter->process(frame_list, nFrames, params);
		}

		/* Final step */
		StreamSink *sink = handlerList->getStreamSink();

		sink->process(frame_list, nFrames, params, *stream);

		source->putFrame(frame_list, nFrames);

		streamList->putFilledStream(stream);
	}
	pthread_mutex_unlock(&mutex);
}

void WorkThread::wakeUpThread()
{
	/* This may be called from another thread */
	pthread_mutex_lock(&mutex);
	if (threadState == THREAD_WAKED)
		printf("Warning: Thread work is not finished\n");
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}
