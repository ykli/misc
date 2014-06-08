#include "StreamList.hh"

StreamList* StreamList::createNew(int nNodes)
{
	return new StreamList(nNodes);
}

StreamList::StreamList(int nNodes)
{

}

StreamList::~StreamList()
{

}

frame_t* StreamList::getFilledStream()
{
	frame_t* f = NULL;
	stream_node_t node = findNodeByFrame(f);

	updateStreamState(node, USED);

	return f;
}

void StreamList::putUsedStream(frame_t* f)
{

}

frame_t* StreamList::getEmptyStream()
{
	frame_t* f = NULL;

	return f;
}

void StreamList::putFilledStream(frame_t* f)
{

}

void StreamList::updateStreamState(stream_node_t node, node_state_t state)
{
	node->state = state;
}

stream_node_t StreamList::findNodeByFrame(frame_t *frame)
{
	return container_of(frame, struct stream_node, frame);
}
