#include "Handler.hh"

Handler::Handler()
{
	pthread_mutex_init(&mutex, NULL);
}

Handler::~Handler()
{
	pthread_mutex_destroy(&mutex);
}
