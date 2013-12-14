#ifndef __OVISP_VIDEOBUF_H__
#define __OVISP_VIDEOBUF_H__

#include <media/videobuf2-core.h>
#include <linux/dma-mapping.h>

static inline dma_addr_t
ovisp_vb2_plane_paddr(struct vb2_buffer *vb, unsigned int plane_no)
{
	dma_addr_t *paddr = vb2_plane_cookie(vb, plane_no);

	return *paddr;
}

void *ovisp_vb2_init_ctx(struct device *dev);
void ovisp_vb2_cleanup_ctx(void *alloc_ctx);

extern const struct vb2_mem_ops ovisp_vb2_memops;

#endif/* __OVISP_VIDEOBUF_H__ */
