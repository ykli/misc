#include "Handler.hh"

//#define TRACING_HANDLER_TIME
#define CALC_TIMES 1

Handler::Handler(const char *n)
	: elapsed(0), times(0)
{
	sprintf(name, "%s", n);
	pthread_mutex_init(&mutex, NULL);
}

Handler::~Handler()
{
	pthread_mutex_destroy(&mutex);
}

void Handler::lock()
{
	pthread_mutex_lock(&mutex);
}

void Handler::unlock()
{
	pthread_mutex_unlock(&mutex);
}

void Handler::preStamp()
{
#ifdef TRACING_HANDLER_TIME
	t1 = getTimer();
#endif
}

void Handler::afterStamp()
{
#ifdef TRACING_HANDLER_TIME
	elapsed += getTimer() - t1;
	if (++times == CALC_TIMES) {
		int64_t aver = elapsed / times;
		DBG("%s: %lldmsec\n", name, aver / 1000);
		elapsed = 0;
		times = 0;
	}
#endif
}
