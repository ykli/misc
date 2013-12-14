#ifndef __V4L2_APP_TEST_H__
#define __V4L2_APP_TEST_H__

#define BASE_VIDIOC_PRIVATE 192
#define VIDIOC_SET_TLB_BASE _IOW('V', BASE_VIDIOC_PRIVATE, unsigned int)
#define V4L2_PIX_FMT_JZ420B  v4l2_fourcc('J', 'Z', '1', '2') /* 12  YUV 4:2:0 B*/

#define TRUE	1
#define FALSE	0

#define PMEM_IOCTL_MAGIC 'p'
#define PMEM_GET_PHYS		_IOW(PMEM_IOCTL_MAGIC, 1, u_int32_t)
#define PMEM_UNMAP			_IOW(PMEM_IOCTL_MAGIC, 4, u_int32_t)

#define DMMU_PAGE_SIZE 4096
#define DMMU_IOCTL_MAGIC 'd'

#define DMMU_GET_PAGE_TABLE_BASE_PHYS		_IOW(DMMU_IOCTL_MAGIC, 0x01, u_int32_t)
#define DMMU_GET_BASE_PHYS                  _IOR(DMMU_IOCTL_MAGIC, 0x02, u_int32_t)
#define DMMU_MAP_USER_MEM		            _IOWR(DMMU_IOCTL_MAGIC, 0x11, struct dmmu_mem_info)
#define DMMU_UNMAP_USER_MEM		            _IOW(DMMU_IOCTL_MAGIC, 0x12, struct dmmu_mem_info)
#define DMMU_GET_TLB_PHYS                   _IOWR(DMMU_IOCTL_MAGIC, 0x13, struct dmmu_mem_info)

#define PMEM_DEV	"/dev/pmem_camera"
#define BUFFER_NR	3

typedef struct buffer {
	unsigned char *start;
    size_t length;
    unsigned char *paddr;
} buffer_t;

typedef struct dmmu_mem_info{
	int size;
	int page_count;

	u_int32_t paddr;
	void *vaddr;
	void *pages_phys_addr_table;

	u_int32_t start_offset;
	u_int32_t end_offset;

} dmmu_mem_info_t;

typedef struct v4l2_dev_info {
	char name[16];
	int fd;
	u_int32_t size_w;
	u_int32_t size_h;
	u_int32_t buf_size;
	u_int32_t preview_seconds;
	buffer_t *buffer;
	int buffer_count;
	u_int32_t fps;
	int is_use_usrptr;
	unsigned char *ybuf;
	unsigned char *ubuf;
	unsigned char *vbuf;
	pthread_t display_thread;
	__u32 pix_format;
	int tlb_flag;
	dmmu_mem_info_t dmmu_para_info;
} v4l2_dev_info_t;


typedef struct fb_info {
	u_int32_t xres;
	u_int32_t yres;
	u_int32_t bpp;
	u_int32_t screensize;
	int fd;
	unsigned char *fbp;
} fb_info_t;

typedef struct mem_info {
	void *start_vaddr;
	int fd;
	int mem_size;
	u_int32_t tlb_base;
} mem_info_t;


struct pmem_region {
	unsigned long offset;
	unsigned long len;
};

struct bpp24_pixel {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;

};

struct bpp16_pixel {
	unsigned short blue:5;
	unsigned short green:6;
	unsigned short red:5;
};

#endif



