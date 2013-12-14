/*
 * Camera test application for Ingenic android 4.1
 *
 * Copyright 2012 Ingenic Semiconductor LTD.
 *
 * author: YeFei <feiye@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#ifdef HAVE_ANDROID_OS
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <pthread.h>
#include "v4l2_app_test.h"
#include <linux/ashmem.h>

v4l2_dev_info_t v4l2_dev_info;
fb_info_t fb_info;
mem_info_t mem_info;

u_int32_t outW, outH;
u_int32_t prev_x_off, prev_y_off;
u_int32_t screen_x_off, screen_y_off;
int fd_dmmu = -1;

static int prepare_framebuffer(fb_info_t *p) {
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	// Open the file for reading and writing
	p->fd = open("/dev/fb0", O_RDWR);
	if (p->fd < 0) {
		fprintf(stderr, "Error: cannot open framebuffer device.\n");
		return -1;
	}

	fprintf(stderr, "The framebuffer device was opened successfully.\n");

	// Get fixed screen information
	if (ioctl(p->fd, FBIOGET_FSCREENINFO, &finfo)) {
		fprintf(stderr, "Error reading fixed information.\n");
		return -1;
	}

	// Get variable screen information
	if (ioctl(p->fd, FBIOGET_VSCREENINFO, &vinfo)) {
		fprintf(stderr, "Error reading variable information.\n");
		return -1;
	}

	// Put variable screen information
	vinfo.activate |= FB_ACTIVATE_FORCE | FB_ACTIVATE_NOW;
	if (ioctl(p->fd, FBIOPUT_VSCREENINFO, &vinfo)) {
		fprintf(stderr, "Error writing variable information.\n");
		return -1;
	}
	// Figure out the size of the screen in bytes
	p->xres = vinfo.xres;
	p->yres = vinfo.yres;
	p->bpp = vinfo.bits_per_pixel;
	p->screensize = (vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8 );

	fprintf(stderr, "framebuffer info:\n xres = %d, yres = %d bpp = %d, screensize = %d\n",
			vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, p->screensize);

	// Map the device to memory
	p->fbp = (unsigned char *)mmap(0, p->screensize/*finfo.smem_len*/, PROT_READ | PROT_WRITE,
				MAP_SHARED, p->fd, 0);

	memset(p->fbp, 0, p->screensize);

	if (!p->fbp) {
		fprintf(stderr, "Error: failed to map framebuffer device to memory.\n");
		return -1;
	}

	fprintf(stderr, "framebuffer device was mapped to memory successfully. fbp = %p\n", p->fbp);
	return 0;
}


static void calc_display_dimension(u_int32_t *outW, u_int32_t *outH,
		u_int32_t *prev_x_offset, u_int32_t *prev_y_offset,
		u_int32_t *screen_x_offset, u_int32_t *screen_y_offset,
		fb_info_t *p1, v4l2_dev_info_t *p2) {

	if (p1->xres >= p2->size_w) {
		*outW = p2->size_w;
		*prev_x_offset = 0;
		if(p1->xres == p2->size_w)
			*screen_x_offset = 0;
		else
			*screen_x_offset = ((p1->xres - p2->size_w) / 2 - 1);
	} else {
		*screen_x_offset = 0;
		*prev_x_offset = ((p2->size_w - p1->xres) / 2  - 1);
		*outW = p1->xres;
	}

	if (p1->yres >= p2->size_h) {
		*outH = p2->size_h;
		*prev_y_offset = 0;
		if(p1->yres == p2->size_h)
			*screen_y_offset = 0;
		else
			*screen_y_offset = ((p1->yres - p2->size_h) / 2 - 1);
	} else {
		*outH = p1->yres;
		*screen_y_offset = 0;
		*prev_y_offset = ((p2->size_h - p1->yres) / 2 - 1);
	}
}


static int dmmu_initial(dmmu_mem_info_t *p) {
	fd_dmmu = open("/dev/dmmu", O_RDWR);
	if (fd_dmmu < 0) {
		fprintf(stderr, "Error: cannot open dmmu device.\n");
		return -1;
	}
	return 0;
}

static int dmmu_uninit(dmmu_mem_info_t *p) {
	if (fd_dmmu < 0) {
		fprintf(stderr, "uninitial dmmu failed, dmmu_fd < 0\n");
		return -1;
	}
	close(fd_dmmu);
	fd_dmmu = -1;

	return 0;
}

static int get_camera_tlb_table(mem_info_t *p1, dmmu_mem_info_t *p2) {
	int ret = 0;
	if (&p1->tlb_base == NULL) {
		fprintf(stderr, "Error: tlb base is null.\n");
		return -1;
	}

	if (fd_dmmu < 0) {
		fprintf(stderr, "Error: dmmu_fd < 0\n");
		return -1;
	}

	ret = ioctl(fd_dmmu, DMMU_GET_PAGE_TABLE_BASE_PHYS, &p1->tlb_base);
	if (ret < 0) {
		fprintf(stderr, "dmmu_get_page_table_base_phys_addr ioctl failed, ret=%d\n", ret);
		return -1;
	}

	fprintf(stdout, "table_base phys_addr = 0x%08x\n", p1->tlb_base);
	return 0;
}

static void init_page_count(dmmu_mem_info_t *info)
{
	int page_count;
	u_int32_t start;			/* page start */
	u_int32_t end;			/* page end */

	start = ((u_int32_t)info->vaddr) & (~(DMMU_PAGE_SIZE-1));
	end = ((u_int32_t)info->vaddr + (info->size-1)) & (~(DMMU_PAGE_SIZE-1));
	page_count = (end - start)/(DMMU_PAGE_SIZE) + 1;

	info->page_count = page_count;
	info->start_offset = (u_int32_t)info->vaddr - start;
	info->end_offset = ((u_int32_t)info->vaddr + info->size) - end;
}

/* NOTE: page_start and page_end maybe used both by two buffer. */
static int dmmu_map_user_mem(dmmu_mem_info_t *p)
{
	int ret;

	p->paddr = 0;
	p->pages_phys_addr_table = NULL;

	/* page count && offset */
	init_page_count(p);
	ret = ioctl(fd_dmmu, DMMU_MAP_USER_MEM, p);
	if (ret < 0) {
		return -1;
	}
	return 0;
}

static int v4l2_dev_open(v4l2_dev_info_t *p) {

    p->fd = open (p->name, O_RDWR | O_NONBLOCK, 0);
    if (p->fd < 0) {
		fprintf (stderr, "Cannot open '%s': %d, %s\n", p->name, errno, strerror (errno));
		exit (EXIT_FAILURE);
    }
	return 0;
}

static int pmem_init(mem_info_t *p1, v4l2_dev_info_t *p2) {
	struct pmem_region region;
	int i, ret;

	if (p1->fd <= 0) {
		p1->fd = open(PMEM_DEV, O_RDWR);

		if (p1->fd <= 0) {
			fprintf(stderr, "open pmem device error, fd = %d\n", p1->fd);
			return -1;
		}

		p1->mem_size = p2->buf_size * p2->buffer_count;
		p1->start_vaddr = mmap(0, p1->mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, p1->fd, 0);

		if(!p1->start_vaddr) {
			fprintf(stderr, "Error: failed to map pmem\n");
			return -1;
		}

		printf("map pmem address is: %p\n", p1->start_vaddr);

		ret = ioctl(p1->fd, PMEM_GET_PHYS, &region);
		if (ret < 0) {
			fprintf(stderr, "pmem get phys address error, ret=%d\n", ret);
			return -1;
		}

		for (i = 0; i < p2->buffer_count; i++) {
			p2->buffer[i].start = (unsigned char *) (p1->start_vaddr) + p2->buf_size * i;
			p2->buffer[i].paddr = (unsigned char *) (region.offset) + p2->buf_size * i;
			p2->buffer[i].length = p2->buf_size;
		}

		printf("pmem phys address is: 0x%lx\n", region.offset);
	}
	return 0;
}


static int ashmem_init(mem_info_t *p1, v4l2_dev_info_t *p2) {

	int i;

	u_int32_t mem_size = p2->buf_size * p2->buffer_count;

	dmmu_initial(&p2->dmmu_para_info);
	get_camera_tlb_table(p1, &p2->dmmu_para_info);

	if (p1->fd <= 0)
		p1->fd = open("/dev/ashmem", O_RDWR);

	if (p1->fd <= 0) {
		fprintf(stderr, "open ashmem device error!\n");
		return -1;
	}

	if (ioctl(p1->fd, ASHMEM_SET_SIZE, mem_size)) {
		fprintf(stderr, "Error set ashmem size\n");
		return -1;
	}

	p1->start_vaddr = (unsigned char*)mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
										MAP_SHARED, p1->fd, 0);
	if(!(p1->start_vaddr)) {
		fprintf(stderr, "Error: failed to map ashmem\n");
		return -1;
	}
	printf("map ashmem address is: %p\n", p1->start_vaddr);

	p2->dmmu_para_info.vaddr = p1->start_vaddr;
	p2->dmmu_para_info.size = mem_size;
	memset(p1->start_vaddr, 1, mem_size);

	if(dmmu_map_user_mem(&p2->dmmu_para_info)) {
		fprintf(stderr, "Error: dmmu map user memory error\n");
		return -1;
	}

	for (i = 0; i < p2->buffer_count; i++) {
		p2->buffer[i].start = (unsigned char *) (p1->start_vaddr) + p2->buf_size * i;
		p2->buffer[i].length = p2->buf_size;
		printf("p2->buffer[%d].start = 0x%x\n", i, (unsigned int)(p2->buffer[i].start));
		printf("p2->buffer[%d].length = 0x%x\n", i, p2->buffer[i].length);
	}

	return 0;
}

static int v4l2_dev_mem_init(v4l2_dev_info_t *p) {

    struct v4l2_requestbuffers req;
    unsigned int buf_num;
    memset(&req, 0, sizeof(struct v4l2_requestbuffers));

    req.count = BUFFER_NR;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(p->is_use_usrptr == TRUE)
    	req.memory = V4L2_MEMORY_USERPTR;
    else
    	req.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl (p->fd, VIDIOC_REQBUFS, &req)) {
    	if(req.memory == V4L2_MEMORY_MMAP)
    		fprintf (stderr, "%s does not support memory mapping\n", p->name);
    	else
    		fprintf (stderr, "%s does not support user memory\n", p->name);
		exit (EXIT_FAILURE);
    }

    if (req.count < BUFFER_NR) {
        fprintf (stderr, "Insufficient buffer memory on %s\n",p->name);
        exit (EXIT_FAILURE);
    }

    p->buffer_count = req.count;
    p->buffer = calloc (req.count, sizeof (buffer_t));

    if (!p->buffer) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    if(req.memory == V4L2_MEMORY_MMAP) {
		for (buf_num = 0; buf_num < req.count; ++buf_num) {
			struct v4l2_buffer buf;
			memset(&buf, 0, sizeof(struct v4l2_buffer));

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = buf_num;

			if (-1 == ioctl (p->fd, VIDIOC_QUERYBUF, &buf)) {
				fprintf (stderr, "%s: VIDIOC_QUERYBUF\n", p->name);
				exit (EXIT_FAILURE);
			}

			p->buffer[buf_num].length = buf.length;
			p->buffer[buf_num].start = mmap (NULL, buf.length, PROT_READ | PROT_WRITE,
						MAP_SHARED, p->fd, buf.m.offset);

			if (MAP_FAILED == p->buffer[buf_num].start) {
				fprintf (stderr, "%s: mmap failed\n", p->name);
				exit (EXIT_FAILURE);
			}
		}
    } else {
    	if(p->tlb_flag != TRUE) {
    		pmem_init(&mem_info, p);
    	} else {
    		ashmem_init(&mem_info, p);
    		if ((-1) == ioctl(p->fd, VIDIOC_SET_TLB_BASE, &mem_info.tlb_base)) {
    			fprintf (stderr, "%s: VIDIOC_SET_TLB_BASE failed\n", __FUNCTION__);
    			exit (EXIT_FAILURE);
    		}
    	}

    	for (buf_num = 0; buf_num < req.count; ++buf_num) {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(struct v4l2_buffer));

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.index = buf_num;
			buf.m.userptr = (unsigned long)p->buffer[buf_num].start;
			buf.length = p->buffer[buf_num].length;

			if (-1 == ioctl (p->fd, VIDIOC_QBUF, &buf)) {
				fprintf (stderr, "%s: VIDIOC_QBUF failed\n", __FUNCTION__);
                exit (EXIT_FAILURE);
			}
		}
    }
	return 0;
}


static void v4l2_dev_mem_free(mem_info_t *p) {
	munmap(p->start_vaddr, p->mem_size);
	close(p->fd);
	p->fd = -1;
}

static void v4l2_dev_close(v4l2_dev_info_t *p) {
	if(p->fd > 0) {
		close(p->fd);
	}
}

static int v4l2_dev_para_init(v4l2_dev_info_t *p) {

    struct v4l2_capability cap;
    struct v4l2_format fmt;
    //struct v4l2_streamparm* setfps;
    //unsigned int min;

	if (-1 == ioctl(p->fd, VIDIOC_QUERYCAP, &cap)) {
		fprintf (stderr, "%s is no V4L2 device\n", p->name);
		exit (EXIT_FAILURE);
	}

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf (stderr, "%s is no video capture device\n", p->name);
        exit (EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf (stderr, "%s does not support streaming i/o\n", p->name);
        exit (EXIT_FAILURE);
    }

    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = p->size_w;
    fmt.fmt.pix.height = p->size_h;
    fmt.fmt.pix.pixelformat = p->pix_format;//V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (-1 == ioctl (p->fd, VIDIOC_S_FMT, &fmt)) {
    	fprintf (stderr, "%s: VIDIOC_S_FMT error\n", p->name);
    	exit (EXIT_FAILURE);
    }

    if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
    	p->buf_size = p->size_h * p->size_w * 2;
    } else if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420 ||
    		fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_JZ420B) {
    	p->buf_size = p->size_h * p->size_w * 12 / 8;
    } else {
    	fprintf (stderr, "%s: should set buffer size according to pixel format!\n",
    				p->name);
    }

    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(p->fd, VIDIOC_G_FMT, &fmt) < 0) {
		printf("get format failed\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Width = %d\n",fmt.fmt.pix.width);
		printf("Height = %d\n",fmt.fmt.pix.height);
		printf("Image size = %d\n", fmt.fmt.pix.sizeimage);

		printf("pixelformat = %d\n",fmt.fmt.pix.pixelformat);
		printf("pix.field = %d\n",fmt.fmt.pix.field);
		if(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV
				|| fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420
				|| fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_JZ420B)
			printf("it is ok\n");
	}

	/* set framerate */
/*    setfps = (struct v4l2_streamparm *) calloc(1, sizeof(struct v4l2_streamparm));
    memset(setfps, 0, sizeof(struct v4l2_streamparm));

    setfps->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    setfps->parm.capture.timeperframe.numerator = 1;
    setfps->parm.capture.timeperframe.denominator = 25;

    if(ioctl(p->fd, VIDIOC_S_PARM, setfps) < 0) {
    	printf("VIDIOC_S_PARM failed!\n");
    	exit(EXIT_FAILURE);
    }

    p->fps = setfps->parm.capture.timeperframe.denominator;
  */
	p->fps = 20;
	return 0;
}


static void v4l2_start_capturing(v4l2_dev_info_t *p) {

    int i;
    enum v4l2_buf_type type;

    if(p->is_use_usrptr == FALSE) {
		for (i = 0; i < p->buffer_count; ++i) {
			struct v4l2_buffer buf;
			memset(&buf, 0, sizeof(struct v4l2_buffer));

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == ioctl (p->fd, VIDIOC_QBUF, &buf)) {
				fprintf (stderr, "%s: VIDIOC_QBUF failed\n", __FUNCTION__);
				exit (EXIT_FAILURE);
			}
		}
	}

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl (p->fd, VIDIOC_STREAMON, &type)) {
    	fprintf (stderr, "%s: VIDIOC_STREAMON failed\n", p->name);
        exit (EXIT_FAILURE);
    }
}

static void stop_capturing (v4l2_dev_info_t *p) {
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl (p->fd, VIDIOC_STREAMOFF, &type))
        exit (EXIT_FAILURE);

    if (fd_dmmu != -1) {
	    dmmu_uninit(&p->dmmu_para_info);
	    fd_dmmu = -1;
    }
}


static void destory_framebuffer(fb_info_t *p) {

    if (-1 == munmap(p->fbp, p->screensize)) {
          printf(" Error: framebuffer device munmap() failed.\n");
          exit (EXIT_FAILURE) ;
    }
    if(p->fd > 0) {
    	if(close(p->fd)) {
			fprintf(stderr, "Error: failed to close fb.\n");
		}
    }
}


static void yuv420b_to_yuv420p(unsigned char *preview_frame, v4l2_dev_info_t *p)
{
	u_int32_t i;
	unsigned char *yuv420p_y, *yuv420p_u, *yuv420p_v;

	unsigned char *uv_temp, *uv_temp_bak;
	int uv_stride = 128;

	yuv420p_y = preview_frame;
	yuv420p_u = yuv420p_y + (p->size_w * p->size_h + (p->size_w * p->size_h / 4));
	yuv420p_v = yuv420p_u + 64;

	printf("------------->yuv420b_to_yuv420p\n");

	uv_temp = malloc(p->size_w * p->size_h >> 1);
	uv_temp_bak = uv_temp;

	for(i = 0; i < (p->size_w * p->size_h >> 1); i += uv_stride) {
		memcpy(uv_temp + i / 2, yuv420p_u + uv_stride, 64);
	}

	uv_temp = uv_temp_bak;
	uv_temp += p->size_w * p->size_h >> 2;

	for(i = 0; i < (p->size_w * p->size_h >> 1); i += uv_stride) {
		memcpy(uv_temp + i / 2, yuv420p_v + uv_stride, 64);
	}

	memcpy(yuv420p_y + (p->size_w * p->size_h), uv_temp_bak, (p->size_w * p->size_h >> 1));
}


static void yuv420p_to_444(unsigned char *preview_frame, v4l2_dev_info_t *p)
{
	u_int32_t line, col;
	unsigned char *yuv420p_y, *yuv420p_u, *yuv420p_v;

	yuv420p_y = preview_frame;
	yuv420p_u = yuv420p_y + (p->size_w * p->size_h);
	yuv420p_v = yuv420p_u + (p->size_w * p->size_h >> 2);

	printf("------------->yuv420p_to_444\n");

	for (line = 0; line < p->size_h; line++) {

		for (col = 0; col < p->size_w; col += 4) {
			p->ybuf[line * p->size_w + col] = *(yuv420p_y + 0); /* Y0 */
			p->ubuf[line * p->size_w + col] = *(yuv420p_u); /* U0 */
			p->vbuf[line * p->size_w + col] = *(yuv420p_v); /* V1 */

			p->ybuf[line * p->size_w + col + 1] = *(yuv420p_y + 1); /* Y1 */
			p->ubuf[line * p->size_w + col + 1] = *(yuv420p_u); /* U0 */
			p->vbuf[line * p->size_w + col + 1] = *(yuv420p_v); /* V1 */

			p->ybuf[line * p->size_w + col + 2] = *(yuv420p_y + 2); /* Y0 */
			p->ubuf[line * p->size_w + col + 2] = *(yuv420p_u); /* U0 */
			p->vbuf[line * p->size_w + col + 2] = *(yuv420p_v); /* V1 */

			p->ybuf[line * p->size_w + col + 3] = *(yuv420p_y + 3); /* Y1 */
			p->ubuf[line * p->size_w + col + 3] = *(yuv420p_u); /* U0 */
			p->vbuf[line * p->size_w + col + 3] = *(yuv420p_v); /* V1 */

			yuv420p_y += 4;
			yuv420p_u += 1;
			yuv420p_v += 1;
		}
	}
}


static void yuv422_to_444(unsigned char *preview_frame, v4l2_dev_info_t *p)
{
	u_int32_t line, col;
	unsigned char *yuyv = preview_frame;

	for (line = 0; line < p->size_h; line++) {
		for (col = 0; col < p->size_w; col += 2) {
			p->ybuf[line * p->size_w + col] = *(yuyv + 0); /* Y0 */
			p->ubuf[line * p->size_w + col] = *(yuyv + 1); /* U0 */
			p->vbuf[line * p->size_w + col] = *(yuyv + 3); /* V1 */

			p->ybuf[line * p->size_w + col + 1] = *(yuyv + 2); /* Y1 */
			p->ubuf[line * p->size_w + col + 1] = *(yuyv + 1); /* U0 */
			p->vbuf[line * p->size_w + col + 1] = *(yuyv + 3); /* V1 */

			yuyv += 4;
		}
	}
}

static void preview_display_vbuffer(unsigned char *preview_frame, fb_info_t *p1,
						v4l2_dev_info_t *p2) {
	u_int32_t line, col;
	int fb_line = screen_y_off;
	int fb_col = screen_x_off;

	int prev_index = 0;
	int fb_index = 0;

	int colorR, colorG, colorB;
	//int Cr, Cb;

	if(p2->pix_format == V4L2_PIX_FMT_YUYV)
		yuv422_to_444(preview_frame, p2);
	else if(p2->pix_format == V4L2_PIX_FMT_YUV420) {
		//unsigned char *yuyv = malloc(p2->size_h * p2->size_w << 1);
		yuv420p_to_444(preview_frame, p2);
	}
	else if (p2->pix_format == V4L2_PIX_FMT_JZ420B) {
		yuv420b_to_yuv420p(preview_frame, p2);
		yuv420p_to_444(preview_frame, p2);
	}
	if (p1->bpp == 32) {
		struct bpp24_pixel *pixel = (struct bpp24_pixel *)p1->fbp;

		for (line = prev_y_off; line < (prev_y_off + outH); line++, fb_line++) {
			fb_col = screen_x_off;
			for (col = prev_x_off; col < (prev_x_off + outW); col++, fb_col++) {
				prev_index = line * p2->size_w + col;
				fb_index = fb_line * p1->xres + fb_col;

				colorR = p2->ybuf[prev_index] + ((359 * (p2->vbuf[prev_index] - 128)) >> 8);
				colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

				colorG = p2->ybuf[prev_index] - ((183 * (p2->vbuf[prev_index] - 128) + 88 *
							(p2->ubuf[prev_index] - 128)) >> 8);
				colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);

				colorB = p2->ybuf[prev_index] + ((454 * (p2->ubuf[prev_index]  - 128)) >> 8);
				colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);

				pixel[fb_index].red = (colorR & 0xff);
				pixel[fb_index].green = (colorG & 0xff);
				pixel[fb_index].blue = (colorB & 0xff);

			}
		}
	} else {
		struct bpp16_pixel *pixel = (struct bpp16_pixel *)p1->fbp;
		for (line = prev_y_off; line < (prev_y_off + outH); line++, fb_line++) {
			fb_col = screen_x_off;
			for (col = prev_x_off; col < (prev_x_off + outW); col++, fb_col++) {
				prev_index = line * p2->size_w + col;
				fb_index = fb_line * p1->xres + fb_col;

				colorR = p2->ybuf[prev_index] + ((359 * (p2->vbuf[prev_index] - 128)) >> 8);
				colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

				colorG = p2->ybuf[prev_index] - ((183 * (p2->vbuf[prev_index] - 128) + 88 *
							(p2->ubuf[prev_index] - 128)) >> 8);
				colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);

				colorB = p2->ybuf[prev_index] + ((454 * (p2->ubuf[prev_index]  - 128)) >> 8);
				colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);

				pixel[fb_index].red = ((colorR & 0xff) >> 3);
				pixel[fb_index].green = ((colorG & 0xff) >> 2);
				pixel[fb_index].blue = ((colorB & 0xff) >> 3);
			}
		}
	}
}

unsigned int buf_index;

static void *thread_preview_display_vbuffer(void *arg) {
	v4l2_dev_info_t *p = (v4l2_dev_info_t *) arg;
	preview_display_vbuffer((unsigned char *) p->buffer[buf_index].start, &fb_info, p);
	return NULL;
}


void dump_sensor_data(void *frame_buffer)
{
	FILE * fp =NULL;
	char * buf =NULL;
	char file_name[64];
	static int i = 0;

	buf = ((char *)frame_buffer);

	snprintf(file_name, sizeof(file_name), "/data/cameraHal.yuv%d", i++);
	if((fp = fopen(file_name, "w+")) == NULL){
  		printf("open sensor dump file error !\n");
		return;
	}
	fwrite(buf, v4l2_dev_info.buf_size, 1, fp);
	fclose(fp);
}


static int v4l2_read_frame (v4l2_dev_info_t *p1, fb_info_t *p2)
{
    struct v4l2_buffer buf;
    //unsigned int i;
    int ret;
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(p1->is_use_usrptr == TRUE)
    	buf.memory = V4L2_MEMORY_USERPTR;
    else
    	buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl (p1->fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
        case EAGAIN:
        	return 0;
        case EIO:
        	return 0;
        default:
            exit (EXIT_FAILURE);
        }
    }

    assert (buf.index < p1->buffer_count);
    buf_index = buf.index;

    ret = pthread_create(&p1->display_thread, NULL, thread_preview_display_vbuffer, p1);
    if (ret) {
    	printf("Create pthread error!/n");
        return 1;
    }

    //dump_sensor_data((void *) p1->buffer[buf_index].start);
    if (-1 == ioctl (p1->fd, VIDIOC_QBUF, &buf)) {
    	fprintf (stderr, "%s: VIDIOC_QBUF failed\n", __FUNCTION__);
        exit (EXIT_FAILURE);
    }

    return 1;
}


static void v4l2_preview_run(v4l2_dev_info_t *p1, fb_info_t *p2)
{
    int frames;
    frames = (p1->fps) * (p1->preview_seconds);

    while (frames-- > 0) {
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;
            FD_ZERO (&fds);
            FD_SET (p1->fd, &fds);

            tv.tv_sec = 20;
            tv.tv_usec = 0;

            r = select (p1->fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;
                exit(1);
            }

            if (0 == r) {
                fprintf (stderr, "select timeout\n");
                exit(1);
            }

            if(v4l2_read_frame (p1, p2)) {
				//printf("v4l2_read_frame %d\n", frames);
				break;
            }
        }
    }
}

void display_usage() {
	fprintf(stderr,
		"\n"
		"-d | --device     select video dev node\n"
		"-s | --size       set frame size, for example, 640*480\n"
		"-t | --time       how many seconds to preview\n"
		"-u | --usrptr     use user point or not\n"
		"-f | --format     set yuv format to cim controller, for example:\n"
		"                  yuv422, yuv420p, yuv420b\n"
		"-v | --tlb        whether use tlb or not\n"
		"-h | --help       print this help information\n"
		);
}

static const char short_options[] = "d:s:t:f:uvh";

static const struct option long_options[] = {
		{"device", required_argument, NULL, 'd'},
		{"size", required_argument, NULL, 's'},
		{"time", required_argument, NULL, 't'},
		{"format", required_argument, NULL, 'f'},
		{"tlb", no_argument, NULL, 'v'},
		{"usrptr", no_argument, NULL, 'u'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
};


static int command_line(int argc, char **argv, v4l2_dev_info_t *p) {
	int opt = 0, Index = 0;
	int width = 0, height = 0, seconds = 10;
	int tlb_flag = 0;
	char pix_format[16];

	if(argc <= 1) {
		display_usage();
		exit(EXIT_SUCCESS);
	}
	opt = getopt_long(argc, argv, short_options, long_options, &Index);
	while(opt != -1) {
		switch(opt) {
		case 'u':
			p->is_use_usrptr = TRUE;
			break;
		case 't':
			seconds = atoi(optarg);
			break;
		case 's':
			if(sscanf(optarg, "%d*%d", &width, &height) != 2) {
				fprintf(stdout, "No geometry information provided by -s parameter.\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'd':
			if(sscanf(optarg, "%s", p->name) != 1) {
				fprintf(stdout, "Invalid format provided by -d parameter.\n");
				exit(EXIT_FAILURE);
			}
			break;

		case 'f':
			if(sscanf(optarg, "%s", pix_format) != 1) {
				fprintf(stdout, "Invalid format provided by -f parameter.\n");
				exit(EXIT_FAILURE);
			}
			break;

		case 'v':
			tlb_flag = TRUE;
			break;

		case 'h':	/* fall-through is intentional */
			display_usage();
			exit(EXIT_SUCCESS);
		default:
			display_usage();
			exit(EXIT_FAILURE);
		}
		opt = getopt_long(argc, argv, short_options, long_options, &Index);
	}

	p->size_w = width;
	p->size_h = height;
	p->preview_seconds = seconds;
	p->tlb_flag = tlb_flag;

	if(strcmp(pix_format, "yuv422") == 0) {
		p->pix_format = V4L2_PIX_FMT_YUYV;
	} else if(strcmp(pix_format, "yuv420p") == 0) {
		p->pix_format = V4L2_PIX_FMT_YUV420;
	} else if(strcmp(pix_format, "yuv420b") == 0) {
		p->pix_format = V4L2_PIX_FMT_JZ420B;
	}
	return 0;
}

int main(int argc, char **argv) {
	command_line(argc, argv, &v4l2_dev_info);

	prepare_framebuffer(&fb_info);

	calc_display_dimension(&outW, &outH, &prev_x_off, &prev_y_off, &screen_x_off,
							&screen_y_off, &fb_info, &v4l2_dev_info);

	v4l2_dev_open(&v4l2_dev_info);
	v4l2_dev_para_init(&v4l2_dev_info);

	v4l2_dev_info.ybuf = malloc(v4l2_dev_info.buf_size >> 1);
	v4l2_dev_info.ubuf = malloc(v4l2_dev_info.buf_size >> 1);
	v4l2_dev_info.vbuf = malloc(v4l2_dev_info.buf_size >> 1);

	v4l2_dev_mem_init(&v4l2_dev_info);
	v4l2_start_capturing(&v4l2_dev_info);
	v4l2_preview_run(&v4l2_dev_info, &fb_info);

	stop_capturing(&v4l2_dev_info);
	destory_framebuffer(&fb_info);
	v4l2_dev_mem_free(&mem_info);
	v4l2_dev_close(&v4l2_dev_info);

	free(v4l2_dev_info.ybuf);
	free(v4l2_dev_info.ubuf);
	free(v4l2_dev_info.vbuf);

	return 0;
}
