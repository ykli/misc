#include "Options.hh"
#include "H264VideoEncoder.hh"
#include "Err.hh"

H264VideoEncoder*
H264VideoEncoder::createNew(UsageEnvironment& env,
							 IVSFramedFilter* inputSource) {
  return new H264VideoEncoder(env, inputSource);
}

H264VideoEncoder::H264VideoEncoder(UsageEnvironment& env, IVSFramedFilter* inputSource)
	: FramedFilter(env, inputSource),
	  x264EnCoder(NULL), pPicOut(NULL), nals(NULL), nnal(0), nalIndex(0),
	  i_pts(0), moveFlag(true), i_start(0), i_file(0) {
	inputSource->setOutBuffer(&framebuffer, &moveFlag);

	fd = fopen("/tmp/mediainfo", "w+");
	if (fd == NULL)
		fprintf(stderr, "open /tmp/mediainfo failed\n");
	x264_init(env);
}

H264VideoEncoder::~H264VideoEncoder() {
	x264_encoder_close(x264EnCoder);
	fclose(fd);
}

void H264VideoEncoder::doGetNextFrame() {
	fInputSource->getNextFrame((unsigned char *)framebuffer, fMaxSize,
							   afterGettingFrame, this,
							   FramedSource::handleClosure, this);
}

void H264VideoEncoder
::afterGettingFrame(void* clientData, unsigned frameSize,
		    unsigned numTruncatedBytes,
		    struct timeval presentationTime,
		    unsigned durationInMicroseconds) {
	H264VideoEncoder* source = (H264VideoEncoder*)clientData;
	source->afterGettingFrame1(frameSize, numTruncatedBytes,
                             presentationTime, durationInMicroseconds);
}

static inline int64_t x264_mdate(void)
{
	struct timeval tv_date;
    gettimeofday( &tv_date, NULL );
    return( (int64_t) tv_date.tv_sec * 1000000 + (int64_t) tv_date.tv_usec );
}

void H264VideoEncoder::Print_status(int i_frame, int64_t i_file)
{
    char    buf[200];
    int64_t i_elapsed = x264_mdate() - i_start;
    double fps = i_elapsed > 0 ? i_frame * 1000000. / i_elapsed : 0;
    double bitrate = (double) i_file * 8 * x264Param.i_fps_num / ( (double) x264Param.i_fps_den * i_frame * 1000 );

	if (showFPS) {
		sprintf( buf, "x264 %d frames: %.2f fps, %.2f kb/s", i_frame, fps, bitrate);
		fprintf( stderr, "%s  \r", buf+5 );
		fflush( stderr ); // needed in windows
	}

	sprintf( buf, "FPS:%.2f\nkbPS:%.2f", fps, bitrate);
    fseek( fd, 0, 0);
    fprintf( fd, "%s", buf);
}

void H264VideoEncoder
::afterGettingFrame1(unsigned frameSize,
		     unsigned numTruncatedBytes,
		     struct timeval presentationTime,
		     unsigned durationInMicroseconds) {
	int i = 0;
#ifdef SW_X264					//sw format convert here
	AVPicture pPictureSrc;
	SwsContext * pSwsCtx;

	pPictureSrc.data[0] = (unsigned char *)framebuffer;
	pPictureSrc.data[1] = pPictureSrc.data[2] = pPictureSrc.data[3] = NULL;
	pPictureSrc.linesize[0] = videoWidth * 2;  //YUV422 linesize = videoWidth * 2

	for (i = 1; i < 8; i++) {
		pPictureSrc.linesize[i] = 0;
	}

	pSwsCtx = sws_getContext(videoWidth, videoHeight, AV_PIX_FMT_YUYV422, videoWidth,
							 videoHeight, AV_PIX_FMT_YUV420P,
							 SWS_BICUBIC, 0, 0, 0);

	int rs = sws_scale(pSwsCtx, pPictureSrc.data, pPictureSrc.linesize, 0,
					   videoHeight, Picture.data, Picture.linesize);

	if (rs == -1) {
		printf("Can open to change to des image");
	}
	sws_freeContext(pSwsCtx);
#endif
	x264_encode();

	fFrameSize = 0;
	for (i = 0; i < nnal; i++) {
		memmove(fTo + fFrameSize, nals[i].p_payload,
				nals[i].i_payload);
		fFrameSize += nals[i].i_payload;
	}

	i_file += fFrameSize;
#define CALC_N_FRAME 20
	if ((i_pts % CALC_N_FRAME) == (CALC_N_FRAME / 2)) {
		if (i_start)
			Print_status(CALC_N_FRAME, i_file);
		i_start = x264_mdate();
		i_file = 0;
	}

	fPresentationTime = presentationTime;
	afterGetting(this);
}

int tile_yuv_picture_alloc( x264_picture_t *pic, int i_csp, int i_width, int i_height )
{
    pic->i_type = X264_TYPE_AUTO;
    pic->i_qpplus1 = 0;
    pic->img.i_csp = i_csp;
    pic->img.i_plane = 2;

    pic->img.i_stride[0] = i_width;
    pic->img.i_stride[1] = i_width;
    pic->param = NULL;
    return 0;
}

void H264VideoEncoder::x264_init(UsageEnvironment& env)
{
	char msg[4];
	char buf[5];
	FILE *conf_fd;
	char * str_1 ="ABR";
	char * str_2 ="CQP";
	char * str_3 ="CRF";

    x264_param_default_preset(&x264Param, "ultrafast", "zerolatency");
//	x264_param_apply_profile(&x264Param, "baseline");

    x264Param.i_width = videoWidth;
    x264Param.i_height = videoHeight;
    x264Param.i_fps_num = 25;
    x264Param.i_fps_den = 1;
    x264Param.i_keyint_max = 25;
	x264Param.i_bframe = 0;
#ifndef SW_X264
    x264Param.i_csp           = X264_CSP_YUYV;
#endif
	x264Param.b_annexb = 1;
	x264Param.b_cabac = 1;
	//x264Param.rc.i_bitrate = 2048;
	//x264Param.rc.i_rc_method = X264_RC_ABR;
	x264Param.rc.i_rc_method = X264_RC_CRF;
	x264Param.rc.f_rf_constant = 28;
	x264Param.rc.i_qp_max = 30;
	x264Param.rc.i_qp_min = 26;
	conf_fd = fopen("/tmp/config", "r");
	if (conf_fd == NULL)
		fprintf(stderr, "open /tmp/config failed\n");
	else{
	fgets(msg,4,conf_fd);
	fgets(buf,5,conf_fd);
	}
	if(strcmp(msg,str_1)==0){
		x264Param.rc.i_bitrate = atoi(buf);
		x264Param.rc.i_rc_method = X264_RC_ABR;
	}
	if(strcmp(msg,str_2)==0){
		x264Param.rc.i_qp_constant = atoi(buf);
		x264Param.rc.i_rc_method = X264_RC_CQP;
	}
	if(strcmp(msg,str_3)==0){
		x264Param.rc.f_rf_constant = atoi(buf)*1.0f;
		x264Param.rc.i_rc_method = X264_RC_CRF;
	}
	if (conf_fd != NULL){
	fclose(conf_fd);
	}
    x264EnCoder = x264_encoder_open(&x264Param);

	if (!x264EnCoder) {
		err(env) << "x264_encoder_open error.\n";
		return;
	}

	/* Must memset, otherwise there will be Segmentation fault or bus error. */
	memset(&xPic, 0x0, sizeof(x264_picture_t));
#ifdef SW_X264
    avpicture_alloc(&Picture, PIX_FMT_YUV420P, videoWidth,
					videoHeight);

    x264_picture_alloc(&xPic, X264_CSP_I420, videoWidth, videoHeight);

    xPic.img.plane[0] = Picture.data[0];
    xPic.img.plane[1] = Picture.data[1];
    xPic.img.plane[2] = Picture.data[2];
#else
	tile_yuv_picture_alloc(&xPic, X264_CSP_YUYV, videoWidth, videoHeight);
    xPic.img.plane[0] = (uint8_t*)framebuffer;
    xPic.img.plane[1] = (uint8_t*)xPic.img.plane[0] + videoWidth * videoHeight;
    xPic.img.plane[2] = (uint8_t*)xPic.img.plane[1] + videoWidth * videoHeight / 4;
#endif
    pPicOut = new x264_picture_t;
}

void H264VideoEncoder::x264_encode()
{
    xPic.i_pts = i_pts++;

#ifdef PRINT_ENCODE_TIME
	int64_t t1,delta_t;
	t1 = x264_mdate();
#endif
#ifndef SW_X264
	xPic.img.plane[0] = (uint8_t*)framebuffer;
	xPic.img.plane[1] = (uint8_t*)xPic.img.plane[0] + videoWidth * videoHeight;
#endif
	if (moveFlag == false) {
		xPic.i_qpplus1 = 50;
		xPic.i_type = X264_TYPE_P;
	} else {
		xPic.i_type = X264_TYPE_AUTO;
		xPic.i_qpplus1 = 0;
	}
	x264_encoder_encode(x264EnCoder, &nals, &nnal, &xPic, pPicOut);

#ifdef PRINT_ENCODE_TIME
	delta_t = x264_mdate() - t1;
#endif

#ifdef PRINT_ENCODE_TIME
#define CNT 30
	static int64_t time = 0;
	static int cnt = 0;

	if (cnt < CNT) {
		time += delta_t;
		cnt++;
	} else {
		fprintf(stderr, "encode time:%lld(usec)\n", time / CNT);
		cnt = 0;
		time = 0;
	}
#endif
}
