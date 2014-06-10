#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <StreamSource.hh>

#include "V4L2Input.hh"
#include "H264Encoder.hh"
#include "AAAFilter.hh"
#include "BBBFilter.hh"

#define NUM_TEST_FRAMES		30
#define MAX_FRAME_SIZE		(10 * 1024 * 1024)

int main(int argc, char** argv)
{
	int iframes = 0;
	void *tmp_dst = valloc(MAX_FRAME_SIZE);

	printf("Test StreamSource\n");
	StreamSource *streamsource = StreamSource::createNew(2, 3, 640, 480, 0, 25);

	V4L2Input* input = V4L2Input::createNew();
	H264Encoder* encoder = H264Encoder::createNew();
	AAAFilter* aaaFilter = AAAFilter::createNew();
	BBBFilter* bbbFilter = BBBFilter::createNew();

	streamsource->addHandler(input);
	streamsource->addHandler(encoder);
	streamsource->addHandler(aaaFilter);
	streamsource->addHandler(bbbFilter);
	
	streamsource->streamOn();
	
	for (iframes = 0; iframes < NUM_TEST_FRAMES; iframes++) {
		struct timeval timestamp;
		int frame_size;
	
		streamsource->getStream(tmp_dst, frame_size, timestamp);
		printf("Get one frame, size=%d timestamp=%ld.%ld\n", frame_size, timestamp.tv_sec, timestamp.tv_usec / 1000);
		usleep(40000);
	}

	streamsource->streamOff();
	delete streamsource;

	return 0;
}
