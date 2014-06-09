#include <string.h>
#include "StreamSource.hh"

void* tick_thread(void *p);

StreamSource* StreamSource::createNew(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps)
{
	return new StreamSource(nThread, nFrames, w, h, fmt, fps);
}

StreamSource::StreamSource(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps) {
	nWorkThread = nThread;

	pic.width = w;
	pic.height = h;
	pic.fmt = fmt;

	video.fps = fps;
	video.tpf = 1000000 / fps;

	streamInit(nFrames);
	threadPollInit();
}

StreamSource::~StreamSource() {
//	delete workThread;
}

void StreamSource::threadPollInit()
{
	workThreadPoll = (thread_node_t*)malloc(sizeof(thread_node_t) * nWorkThread);
	for (int i = 0;  i < nWorkThread; i++) {
		thread_node_t *wt = &workThreadPoll[i];
		wt->workThread = WorkThread::createNew(NULL, NULL, streamList);

		if (i == 0)
			wt->pre = &workThreadPoll[nWorkThread - 1];
		else
			wt->pre = wt - 1;

		if (i == (nWorkThread - 1))
			wt->next = &workThreadPoll[0];
		else
			wt->next = wt + 1;
	}
	curThreadNode = workThreadPoll;

	int error = pthread_create(&tid, NULL, tick_thread, this);
	if (error) {
		DBG("thread create error\n");
	}
}

void StreamSource::getStream(void* to, int& len, struct timeval& timestamp)
{
//	sleep(10);

#if 0
	while (1) {
		usleep(100000);
		DBG("wake\n");
		workThread->wakeUpThread();
	}
#endif
#if 1
//	LOCATION();
	frame_t *stream = streamList->getFilledStream();

	memmove(to, stream->addr, stream->size);
	len = stream->size;
	timestamp = stream->timestamp;
//	LOCATION();
	streamList->putUsedStream(stream);
	DBG("get:%d\n", stream->index);
#endif
}
void StreamSource::streamInit(int nFrames)
{
	streamList = StreamList::createNew(nFrames);
}

void StreamSource::streamOn(void)
{
	streamIsOn = 1;
//	workThread->
}

void StreamSource::streamOff(void)
{
	streamIsOn = 0;
//	pthread_join(tid, NULL);
}

void StreamSource::addHandler()
{
}

void StreamSource::delHandler()
{
	
}

void* tick_thread(void *p)
{
	StreamSource *ss = (StreamSource *)p;

	ss->tickThread();

	return NULL;
}

void StreamSource::tickThread()
{
/*
 * Demon thread.
 */
	while (streamIsOn) {
		WorkThread *wt = curThreadNode->workThread;
		wt->wakeUpThread();
		curThreadNode = curThreadNode->next;
		usleep(video.tpf);
	}
}
