#include <getopt.h>

#include "H264Encoder.hh"

int H264Encoder::force_idr = 0;
int H264Encoder::showFPS = 0;
int H264Encoder::fps = 30;

H264Encoder* H264Encoder::createNew(int w, int h, int fmt)
{
	return new H264Encoder(w, h, fmt);
}

H264Encoder::H264Encoder(int w, int h, int fmt)
	: StreamSink("H264Encoder"),
	  x264EnCoder(NULL), pPicOut(NULL), nals(NULL), nnal(0), nalIndex(0),
	  i_pts(0), moveFlag(true), i_start(0), i_file(0),
	  videoWidth(w), videoHeight(h)
{
	fd = fopen("/tmp/mediainfo", "w+");
	x264_init(fmt);
}

H264Encoder* H264Encoder::createNew(int w, int h)
{
	return new H264Encoder(w, h);
}

H264Encoder::H264Encoder(int w, int h)
	: StreamSink("H264Encoder"),
	  x264EnCoder(NULL), pPicOut(NULL), nals(NULL), nnal(0), nalIndex(0),
	  i_pts(0), moveFlag(true), i_start(0), i_file(0),
	  videoWidth(w), videoHeight(h)
{
	fd = fopen("/tmp/mediainfo", "w+");
	x264_init(0);
}

H264Encoder::~H264Encoder()
{
	x264_encoder_close(x264EnCoder);
	fclose(fd);
}

static inline int64_t x264_mdate(void)
{
	struct timeval tv_date;
    gettimeofday( &tv_date, NULL );
    return( (int64_t) tv_date.tv_sec * 1000000 + (int64_t) tv_date.tv_usec );
}

static int tile_yuv_picture_alloc( x264_picture_t *pic, int i_csp, int i_width, int i_height )
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

int H264Encoder::set_x264_param(int argc, char **argv, x264_param_t *param)
{
	int c = 0;
	int optindex = 0;

	const struct option longopts[] = {
		/* {name, has_arg, flag, val} */
		{"i_rc_method",		1,	0,	0},
		{"f_rf_constant",	1,	0,	0},
		{"i_qp_min",		1,	0,	0},
		{"i_qp_max",		1,	0,	0},
		{"i_vbv_max_bitrate",	1,	0,	0},
		{"i_vbv_buffer_size",	1,	0,	0},
		{"i_qp_constant",	1,	0,	0},
		{"i_bitrate",		1,	0,	0},
		{"i_width",		1,	0,	0},
		{"i_height",		1,	0,	0},
		{"i_fps_num",		1,	0,	0},
		{"i_keyint_min",	1,	0,	0},
		{"i_keyint_max",	1,	0,	0},
		{"i_bframe",		1,	0,	0},
		{"b_annexb",		1,	0,	0},
		{"b_cabac",		1,	0,	0},
		{0, 0, 0, 0},
	};

	while ((c = getopt_long_only(argc, argv, "", longopts, &optindex)) != -1) {
		switch(c) {
		case '?' :
		case ':' :
			continue;
		case 0 : {
			 char const* optname = longopts[optindex].name;
			 if (!strcmp(optname, "i_rc_method")) {
				 if (!strcmp(optarg, "ABR")) {
					 param->rc.i_rc_method = X264_RC_ABR;
				 } else if (!strcmp(optarg, "CRF")) {
					 param->rc.i_rc_method = X264_RC_CRF;
				 } else if (!strcmp(optarg, "CQP")) {
					 param->rc.i_rc_method = X264_RC_CQP;
				 }
			 }
			 if (!strcmp(optname, "f_rf_constant")) {
				 param->rc.f_rf_constant = atof(optarg);
			 }
			 if (!strcmp(optname, "i_qp_min")) {
				 param->rc.i_qp_min = atof(optarg);
			 }
			 if (!strcmp(optname, "i_qp_max")) {
				 param->rc.i_qp_max = atof(optarg);
			 }
			 if (!strcmp(optname, "i_vbv_max_bitrate")) {
				 param->rc.i_vbv_max_bitrate = atof(optarg);
			 }
			 if (!strcmp(optname, "i_vbv_buffer_size")) {
				 param->rc.i_vbv_buffer_size = atof(optarg);
			 }
			 if (!strcmp(optname, "i_qp_constant")) {
				 param->rc.i_qp_constant = atof(optarg);
			 }
			 if (!strcmp(optname, "i_bitrate")) {
				 param->rc.i_bitrate = atof(optarg);
			 }
			 if (!strcmp(optname, "i_width")) {
				 param->i_width = atof(optarg);
			 }
			 if (!strcmp(optname, "i_height")) {
				 param->i_height = atof(optarg);
			 }
			 if (!strcmp(optname, "i_fps_num")) {
				 param->i_fps_num = atof(optarg);
			 }
			 if (!strcmp(optname, "i_keyint_min")) {
				 param->i_keyint_min = atof(optarg);
			 }
			 if (!strcmp(optname, "i_keyint_max")) {
				 param->i_keyint_max = atof(optarg);
			 }
			 if (!strcmp(optname, "i_bframe")) {
				 param->i_bframe = atof(optarg);
			 }
			 if (!strcmp(optname, "b_annexb")) {
				 param->b_annexb = atof(optarg);
			 }
			 if (!strcmp(optname, "b_cabac")) {
				 param->b_cabac = atof(optarg);
			 }
		}
		default:
			 break;
		}
	}

	return 0;
}

void H264Encoder::x264_init(int fmt)
{
	int argc = 0;
	char **argv = NULL;
	x264_param_default_preset(&x264Param, "ultrafast", "zerolatency");
	//	x264_param_apply_profile(&x264Param, "baseline");

	x264Param.i_width = videoWidth;
	x264Param.i_height = videoHeight;
	x264Param.i_fps_num = fps;
	x264Param.i_fps_den = 1;
	x264Param.i_keyint_max = 25;
	x264Param.i_bframe = 0;
	if(fmt)
		x264Param.i_csp = fmt;
	else
		x264Param.i_csp = X264_CSP_NV12;
	x264Param.b_annexb = 1;
	x264Param.b_cabac = 1;
	//x264Param.rc.i_bitrate = 2048;
	//x264Param.rc.i_rc_method = X264_RC_ABR;
	x264Param.rc.i_rc_method = X264_RC_CRF;
	x264Param.rc.f_rf_constant = 28;
	x264Param.rc.i_qp_max = 34;
	x264Param.rc.i_qp_min = 26;
	x264Param.rc.i_vbv_max_bitrate = 1800;
	x264Param.rc.i_vbv_buffer_size = 900;

	if (parse_key_value_config(&argc, &argv, "/tmp/config") < 0) {
		fprintf(stderr, "parse_key_value_config failed\n");
	} else {
		//dump_argc_argv(argc, argv);
		if (set_x264_param(argc, argv, &x264Param) < 0) {
			fprintf(stderr, "set_x264_param failed\n");
		}
		destroy_argc_argv(argc, argv);
	}

	x264EnCoder = x264_encoder_open(&x264Param);

	if (!x264EnCoder) {
		printf("x264_encoder_open error\n");
		return;
	}

	/* Must memset, otherwise there will be Segmentation fault or bus error. */
	memset(&xPic, 0x0, sizeof(x264_picture_t));

	tile_yuv_picture_alloc(&xPic, x264Param.i_csp, videoWidth, videoHeight);
	xPic.img.plane[0] = (uint8_t*)framebuffer;
	xPic.img.plane[1] = (uint8_t*)xPic.img.plane[0] + videoWidth * videoHeight;
	xPic.img.plane[2] = (uint8_t*)xPic.img.plane[1] + videoWidth * videoHeight / 4;

	pPicOut = new x264_picture_t;
}

void H264Encoder::x264_encode()
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

	if (force_idr) {
		xPic.b_keyframe = 1;
		xPic.i_type = X264_TYPE_IDR;
		force_idr = 0;
		i_pts = 0;
		xPic.i_pts = i_pts++;
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

void H264Encoder::Print_status(int i_frame, int64_t i_file)
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
#if 1
	sprintf( buf, "FPS:%.2f\nkbPS:%.2f", fps, bitrate);
    fseek( fd, 0, 0);
    fprintf( fd, "%s", buf);
#endif
}

void H264Encoder::doProcess(frame_t* frame_list, int nFrames, uint32_t *params, frame_t& stream)
{
	LOCATION("\n");
	unsigned char* fTo = (unsigned char*)stream.addr;
	int fFrameSize = 0, i;
	frame_t* frame = getFrameByType(frame_list, nFrames, PRIMARY);
	framebuffer = frame->addr;

	x264_encode();

	stream.nal_num = nnal;
	for (i = 0; i < nnal; i++) {
		memmove(fTo + fFrameSize, nals[i].p_payload,
				nals[i].i_payload);
		stream.nal_size[i] = nals[i].i_payload;
		fFrameSize += nals[i].i_payload;
	}

	if (fFrameSize > MAX_STREAM_SIZE)
		printf("Warning: Stream size(%d) is too large, %d is expected\n", fFrameSize, MAX_STREAM_SIZE);
	i_file += fFrameSize;

#define CALC_N_FRAME 20
	if ((i_pts % CALC_N_FRAME) == (CALC_N_FRAME / 2)) {
		if (i_start)
			Print_status(CALC_N_FRAME, i_file);
		i_start = x264_mdate();
		i_file = 0;
	}


	stream.size = fFrameSize;
	stream.timestamp = frame->timestamp;
}
