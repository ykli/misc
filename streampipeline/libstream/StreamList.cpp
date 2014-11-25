#include "StreamList.hh"
#include "H264Encoder.hh"
#include "V4L2Input.hh"

StreamList* StreamList::createNew(int nNodes)
{
	return new StreamList(nNodes);
}

StreamList::StreamList(int n)
	: nNodes(n), nodesMap(0)
{
	int ret;

	ret = pthread_mutex_init(&mutex, NULL);
	if (ret != 0)
		printf("pthread_mutex_init() error\n");

	ret = pthread_cond_init(&cond, NULL);
	if (ret != 0)
		printf("pthread_cond_init() error\n");

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
		sn->frame.addr = valloc(MAX_STREAM_SIZE);
	}
	curNode = streamNodes;
	curFilledNode = streamNodes;
	curPollingNode = streamNodes;
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

void StreamList::flushFilledStream()
{
	lock();
	stream_node_t *node_p = curNode;

	do {
		curNode->state = EMPTY;
		curNode = curNode->next;
	} while (curNode != node_p);

	curNode = streamNodes;
	curFilledNode = streamNodes;
	curPollingNode = streamNodes;
	wakeUpList();
	/* Ugly implement. Must be fixed later. */
	H264Encoder::ForceIDRFrame();

	unlock();
}

void StreamList::flushInputFrames()
{
	lock();
	V4L2Input::flushFramesAsync();
	unlock();
}

void StreamList::pollingStreamFilled(void)
{
	lock();
	stream_node_t *node_p = curPollingNode;

restart:
	do {
		if (curPollingNode->state == FILLED)
			break;
		else
			curPollingNode = curPollingNode->next;
	} while (curPollingNode != node_p);

	if (curPollingNode->state != FILLED) {
		pthread_cond_wait(&cond, &mutex);
		goto restart;
	}

	unlock();
}

void StreamList::updateStreamState(stream_node_t *node, node_state_t state)
{
	node->state = state;
#ifdef DEBUG
	int index = ((uint32_t)node - (uint32_t)streamNodes) / sizeof(stream_node_t);
	if (state == FILLED)
		nodesMap |= 1 << index;
	else
		nodesMap &= ~(1 << index);

	uint32_t map = nodesMap;
	int nEmpty = 0, nNotEmpty = 0, i = 0;
	for (i = 0; i < (int)sizeof(uint32_t); i++) {
		if (map & 0x1)
			nNotEmpty++;
		else
			nEmpty++;
		map >>= 1;
	}
	DBG("streamNode state:" LIGHT_RED "%d" COLOR_NONE ":" LIGHT_GREEN "%d" COLOR_NONE "\n", nEmpty, nNotEmpty);
#endif
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
