#include "WorkThread.hh"

WorkThread::WorkThread(HandlerList *h, FrameList *f, StreamList *s)
	: handlerList(h), frameList(f), streamList(s),
{

}

WorkThread::~WorkThread()
{
}

void* work_thread(void *p) {
	WorkThread *wt = (WorkThread *)p;

	wt->Thread();

	return NULL;
}

void WorkThread::Thread()
{
	while (1) {
		do {
			pthread_wait_cond();
		} while ((stream = streamList->get()) == NULL);

		memset(params, 0, sizeof(params));

		for (int step = 0; step < handlerList->totalStep(); step++) {
			Handler *handler;
			handler = handlerList->getNodeByStep(step);
			handler->doProcess(frame, params);
		}
 /
		frameList->put(frame);
	}
}

