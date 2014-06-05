

void Handler::process(frame_t frame, uint32& params, uint32_t* to)
{
	doProcess(frame, params);
	afterProcess();
}
