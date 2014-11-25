#include <string.h>
#include "StreamSource.hh"

void* tick_thread(void *p);
int64_t b_timestamp;

StreamSource* StreamSource::createNew(int nThread, int nFrames, int fps, int w, int h, uint32_t fmt)
{
	return new StreamSource(nThread, nFrames, fps, w, h, fmt);
}

StreamSource::StreamSource(int nThread, int nFrames, int fps, int w, int h, uint32_t fmt)
	: streamIsOn(0), i_base(0), i_sleep(0), i_pts(0), i_elapsed(0) {
	nWorkThread = nThread;

#ifdef FREOPEN
	if (freopen("log", "w", stdout) == NULL)
		fprintf(stderr, "error freopen\n");
#endif

	b_timestamp = getTimer();

	pic.width = w;
	pic.height = h;
	pic.fmt = fmt;

	video.fps = fps;
	video.tpf = 1000000 / fps;

	streamInit(nFrames);
	handlerList = HandlerList::createNew();
	threadPollInit();
	pthread_mutex_init(&mutex, NULL);
}

StreamSource::~StreamSource() {
	pthread_mutex_destroy(&mutex);
//	delete workThread;
	for (int i = 0;  i < nWorkThread; i++) {
		thread_node_t *wt = &workThreadPoll[i];
		delete wt->workThread;
	}
	free(workThreadPoll);
}

void StreamSource::lock()
{
	pthread_mutex_lock(&mutex);
}

void StreamSource::unlock()
{
	pthread_mutex_unlock(&mutex);
}

void StreamSource::threadPollInit()
{
	workThreadPoll = (thread_node_t*)malloc(sizeof(thread_node_t) * nWorkThread);
	for (int i = 0;  i < nWorkThread; i++) {
		thread_node_t *wt = &workThreadPoll[i];
		wt->workThread = WorkThread::createNew(handlerList, NULL, streamList);

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
}

void StreamSource::getStream(void* to, int& len, struct timeval& timestamp)
{
	lock();
	frame_t *stream = streamList->getFilledStream();

	memmove(to, stream->addr, stream->size);
	len = stream->size;
	timestamp = stream->timestamp;

	streamList->putUsedStream(stream);
	unlock();
}

void StreamSource::getStream(void* to, int& len, int &nal_num, int *nal_size, struct timeval& timestamp)
{
	lock();
	frame_t *stream = streamList->getFilledStream();

	memmove(to, stream->addr, stream->size);
	len = stream->size;
	timestamp = stream->timestamp;
	nal_num = stream->nal_num;
	for(int i=0; i<nal_num; i++) {
		nal_size[i] = stream->nal_size[i];
	}

	streamList->putUsedStream(stream);
	unlock();
}


void StreamSource::pollingStream(void)
{
	lock();
	streamList->pollingStreamFilled();
	unlock();
}

void StreamSource::streamInit(int nFrames)
{
	streamList = StreamList::createNew(nFrames);
}

void StreamSource::streamOn(void)
{
	lock();
	LOCATION("\n");
	if (streamIsOn == 0) {
		streamIsOn = 1;
		int error = pthread_create(&tid, NULL, tick_thread, this);
		if (error) {
			DBG("thread create error\n");
		}
	}
	unlock();
}

void StreamSource::streamOff(void)
{
	lock();
	LOCATION("\n");
	if (streamIsOn == 1) {
		streamIsOn = 0;
		streamList->flushFilledStream();
		pthread_join(tid, NULL);

		/* Wait for all of the thread finish their work. */
		for (int i = 0; i < nWorkThread; i++) {
			int timeout = 100;
			WorkThread *wt = workThreadPoll[i].workThread;

			while (!wt->ThreadIsSlept() && --timeout)
				usleep(10000);

			if (!timeout)
				fprintf(stderr, "streamOff timeout\n");
		}
	}
	unlock();
}

void StreamSource::addHandler(FrameSource *fsource)
{
	handlerList->add(fsource);
}

void StreamSource::addHandler(StreamSink *ssink)
{
	handlerList->add(ssink);
}

void StreamSource::addHandler(Filter *filter)
{
	handlerList->add(filter);
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
	i_pts = 0;
	i_base = getTimer();
	streamList->flushInputFrames();
	streamList->flushFilledStream();
	while (streamIsOn) {
		i_pts++;
		WorkThread *wt = curThreadNode->workThread;
		wt->wakeUpThread();

		curThreadNode = curThreadNode->next;
		i_elapsed = getTimer() - i_base;
		i_sleep = video.tpf * i_pts - i_elapsed;
		usleep(i_sleep > 0 ? i_sleep : 0);
	}
}
