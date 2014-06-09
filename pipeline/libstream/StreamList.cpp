#include "StreamList.hh"

StreamList* StreamList::createNew(int nNodes)
{
	return new StreamList(nNodes);
}

StreamList::StreamList(int nNodes)
{
	pthread_mutex_init(&mutex, NULL);
	streamNodes = (stream_node_t *)calloc(nNodes, sizeof(stream_node_t));

	for (int i = 0;  i < nNodes; i++) {
		stream_node_t *sn = &streamNodes[i];

		if (i == 0)
			sn->pre = &streamNodes[nNodes - 1];
		else
			sn->pre = sn - 1;

		if (i == (nNodes - 1))
			sn->next = &streamNodes[0];
		else
			sn->next = sn + 1;

		sn->frame.index = i;
	}
	curNode = streamNodes;
	curFilledNode = streamNodes;
}

StreamList::~StreamList()
{

}

frame_t* StreamList::getFilledStream()
{
	lock();
	frame_t* f = NULL;
	stream_node_t *node_p = curFilledNode;

restart:
	do {
		if (curFilledNode->state == FILLED)
			break;
		else
			curFilledNode = curFilledNode->next;
	} while (curFilledNode != node_p);

	if (curFilledNode->state == FILLED) {
		f = &curFilledNode->frame;
		updateStreamState(curFilledNode, USED);
	} else {
//		DBG("No Filled Stream found\n");
		pthread_cond_wait(&cond, &mutex);
//		LOCATION();
		goto restart;
	}
//	DBG("getFilledStream:%d\n", f->index);
	unlock();

	return f;
}

void StreamList::putUsedStream(frame_t* f)
{
	lock();
	stream_node_t *node = findNodeByFrame(f);
	updateStreamState(node, EMPTY);
	wakeUpList();
//	LOCATION();
//	DBG("putUsedStream:%d\n", f->index);
	unlock();
}

frame_t* StreamList::getEmptyStream()
{
	lock();
	frame_t* f = NULL;
	stream_node_t *node_p = curNode;

restart:
	do {
		if (curNode->state == EMPTY) {
			break;
		}else
			curNode = curNode->next;
	} while (curNode != node_p);

	if (curNode->state == EMPTY) {
		f = &curNode->frame;
		updateStreamState(curNode, USED);
	} else {
//		DBG("No Empty Stream found\n");
		pthread_cond_wait(&cond, &mutex);
		goto restart;
	}
//	DBG("getEmptyStream:%d\n", f->index);
	unlock();

	return f;
}

void StreamList::putFilledStream(frame_t* f)
{
	lock();
//	LOCATION();
	stream_node_t *node = findNodeByFrame(f);
	updateStreamState(node, FILLED);
	wakeUpList();
//	LOCATION();
//	DBG("putFilledStream:%d\n", f->index);
	unlock();
}

void StreamList::updateStreamState(stream_node_t *node, node_state_t state)
{
	node->state = state;
}

stream_node_t* StreamList::findNodeByFrame(frame_t *frame)
{
	return container_of(frame, struct stream_node, frame);
}

void StreamList::wakeUpList()
{
	/* This may be called from another thread */
//	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
//	pthread_mutex_unlock(&mutex);
}
