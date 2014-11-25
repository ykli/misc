#include "T05MultiInput.hh"

int T05MultiInput::videoWidth;
int T05MultiInput::videoHeight;
bool T05MultiInput::fHaveInitialized = false;
MultiFrameSource* T05MultiInput::mMultiFrameSource = NULL;
SingleFrameSource* T05MultiInput::mPrimarySource = NULL;
SingleFrameSource* T05MultiInput::mFunctionSource = NULL;

T05MultiInput* T05MultiInput::createNew(int w, int h, uint32_t fmt)
{
	videoWidth = w ? w : 1280;
	videoHeight = h ? h : 720;

	if (!fHaveInitialized) {
		if (!initialize()) return NULL;
		fHaveInitialized = true;
	}

	return new T05MultiInput();
}

T05MultiInput::T05MultiInput()
	: FrameSource("T05MultiInput")
{

}

bool T05MultiInput::initialize() {
	try{
		mMultiFrameSource = T05MultiFrameSource::getInstance(videoWidth, videoHeight, 0, 0);
	}catch(int& a){
		return false;
	}
	mPrimarySource = mMultiFrameSource->getFrameSourceByType(PRIMARY_SOURCE);
	mFunctionSource = mMultiFrameSource->getFrameSourceByType(FUNCTION_SOURCE);

    return true;
}

void T05MultiInput::doGetFrame(frame_t* frame_list, int& nFrames, uint32_t *params)
{
	LOCATION("\n");
	frame_t& frame1 = frame_list[0];
	mPrimarySource->getFrame(frame1.addr, frame1.timestamp, frame1.index);
	frame1.type = PRIMARY;
	frame1.size = videoWidth * videoHeight * 3 / 2;

	frame_t& frame2 = frame_list[1];
	mFunctionSource->getFrame(frame2.addr, frame2.timestamp, frame2.index);
	frame2.type = FUNCTIONAL;

#if 1
	frame2.size = 640 * 360;
#else /* For ISP debug */
	frame2.size = videoWidth * videoHeight * 2;
#endif

	nFrames = 2;
}

void T05MultiInput::doPutFrame(frame_t* frame_list, int nFrames)
{
	LOCATION("\n");
	frame_t& frame2 = frame_list[1];
	mFunctionSource->putFrame(frame2.addr);

	frame_t& frame1 = frame_list[0];;
	mPrimarySource->putFrame(frame1.addr);
}
