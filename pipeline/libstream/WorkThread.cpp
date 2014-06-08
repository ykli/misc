#include "WorkThread.hh"

WorkThread::WorkThread(HandlerList *h, FrameList *f, StreamList *s)
	: handlerList(h), frameList(f), streamList(s),
{
	int error;

	error = pthread_create(&tid, NULL, work_thread, this);
	if (error) {
		printf("thread create error\n");
	}
}

WorkThread::~WorkThread()
{
	pthread_join(tid, NULL);
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
		frame_t& frame;

		/* Wait for wake up signal */
		do {
			pthread_mutex_lock(&mutex);
//			threadState = THREAD_SLEEP;
			pthread_cond_wait(&cond, &mutex);
//			threadState = THREAD_WAKED;
			pthread_mutex_unlock(&mutex);
		} while ((stream = streamList->get()) == NULL);

		memset(params, 0, sizeof(params));

		/* Get a new frame from Source */
		handlerList->getFrameSource(source);
		source->getFrame(frame);

		/* Filter works */
		for (int step = 1; step < handlerList->totalStep() - 1; step++) {
			Filter *handler;
			handler = handlerList->getNodeByStep(step);
			handler->doProcess(frame, params);
			handler->afterProcess(frame, params);
		}

		/* Last step */
		final = handlerList->getLastNode(step);
		final->doProcess(frame, params, stream->addr);
		frameList->put(frame);
	}
}

void WorkThread::wakeUpThread()
{
	/* This may be called from another thread */
	pthread_cond_signal(&cond);
}
