#include <string.h>
#include "StreamSource.hh"

StreamSource* StreamSource::createNew(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps)
{
	return new StreamSource(nThread, nFrames, w, h, fmt, fps);
}

StreamSource::StreamSource(int nThread, int nFrames, int w, int h, uint32_t fmt, int fps) {
	pic.width = w;
	pic.height = h;
	pic.fmt = fmt;

	video.fps = fps;

	streamInit(nFrames);
}

StreamSource::~StreamSource() {
	
}

void StreamSource::getStream(void* to, int& len, struct timeval& timestamp)
{
	frame_t *stream = streamList->getFilledStream();

	memmove(to, stream->addr, stream->size);
	len = stream->size;
	timestamp = stream->timestamp;

	streamList->putUsedStream(stream);
}

void StreamSource::streamInit(int nFrames)
{
	streamList = StreamList::createNew(nFrames);
}

void StreamSource::streamOn(void)
{
	streamIsOn = 1;
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

void StreamSource::tickThread()
{
#if 0
	while (streamIsOn) {
		workthread = workthreadList->next();
		workthread->wakeUpThread();
		sleep(tpf);
	}
#endif
}
