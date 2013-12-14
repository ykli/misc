
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>

#include <media/videobuf2-core.h>
#include <media/videobuf2-memops.h>

#include "ovisp-video.h"
#include "ovisp-videobuf.h"

struct vb2_dc_conf {
	struct device			*dev;
	void				*vaddr;
	dma_addr_t			paddr;
	unsigned long			size;
	unsigned long			used;
};

struct vb2_dc_buf {
	struct vb2_dc_conf		*conf;
	void				*vaddr;
	dma_addr_t			paddr;
	unsigned long			size;
	struct vm_area_struct		*vma;
	atomic_t			refcount;
	struct vb2_vmarea_handler	handler;
};

static void ovisp_vb2_put(void *buf_priv)
{
	struct vb2_dc_buf *buf = buf_priv;
	struct vb2_dc_conf *conf = buf->conf;

	if (atomic_dec_and_test(&buf->refcount)) {
		conf->used = 0;
		kfree(buf);
	}
}

static void *ovisp_vb2_alloc(void *alloc_ctx, unsigned long size)
{
	struct vb2_dc_conf *conf = alloc_ctx;
	struct vb2_dc_buf *buf;

	buf = kzalloc(sizeof *buf, GFP_KERNEL);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	buf->vaddr = conf->vaddr + conf->used;
	buf->paddr = conf->paddr + conf->used;
	conf->used += size;

	buf->conf = conf;
	buf->size = size;
	buf->handler.refcount = &buf->refcount;
	buf->handler.put = ovisp_vb2_put;
	buf->handler.arg = buf;

	atomic_inc(&buf->refcount);

	return buf;
}

static void *ovisp_vb2_cookie(void *buf_priv)
{
	struct vb2_dc_buf *buf = buf_priv;

	return &buf->paddr;
}

static void *ovisp_vb2_vaddr(void *buf_priv)
{
	struct vb2_dc_buf *buf = buf_priv;
	if (!buf)
		return 0;

	return buf->vaddr;
}

static unsigned int ovisp_vb2_num_users(void *buf_priv)
{
	struct vb2_dc_buf *buf = buf_priv;

	return atomic_read(&buf->refcount);
}

static int ovisp_vb2_mmap(void *buf_priv, struct vm_area_struct *vma)
{
	struct vb2_dc_buf *buf = buf_priv;
	struct vb2_dc_conf *conf = buf->conf;

	if (!buf) {
		printk(KERN_ERR "No buffer to map\n");
		return -EINVAL;
	}

	return vb2_mmap_pfn_range(vma, conf->paddr, conf->size,
				  &vb2_common_vm_ops, &buf->handler);
}

static void *ovisp_vb2_get_userptr(void *alloc_ctx, unsigned long vaddr,
					unsigned long size, int write)
{
	struct vb2_dc_buf *buf;
	struct vm_area_struct *vma;
	dma_addr_t paddr = 0;
	int ret;

	buf = kzalloc(sizeof *buf, GFP_KERNEL);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	ret = vb2_get_contig_userptr(vaddr, size, &vma, &paddr);
	if (ret) {
		printk(KERN_ERR "Failed acquiring VMA for vaddr 0x%08lx\n",
				vaddr);
		kfree(buf);
		return ERR_PTR(ret);
	}

	buf->size = size;
	buf->paddr = paddr;
	buf->vma = vma;

	return buf;
}

static void ovisp_vb2_put_userptr(void *mem_priv)
{
	struct vb2_dc_buf *buf = mem_priv;

	if (!buf)
		return;

	vb2_put_vma(buf->vma);
	kfree(buf);
}

const struct vb2_mem_ops ovisp_vb2_memops = {
	.alloc		= ovisp_vb2_alloc,
	.put		= ovisp_vb2_put,
	.cookie		= ovisp_vb2_cookie,
	.vaddr		= ovisp_vb2_vaddr,
	.mmap		= ovisp_vb2_mmap,
	.get_userptr	= ovisp_vb2_get_userptr,
	.put_userptr	= ovisp_vb2_put_userptr,
	.num_users	= ovisp_vb2_num_users,
};
EXPORT_SYMBOL_GPL(ovisp_vb2_memops);

void *ovisp_vb2_init_ctx(struct device *dev)
{
	struct vb2_dc_conf *conf;

	conf = kzalloc(sizeof *conf, GFP_KERNEL);
	if (!conf)
		return ERR_PTR(-ENOMEM);

	conf->dev = dev;
	conf->used = 0;
	conf->size = ovisp_CAMERA_BUFFER_MAX;
#if !defined(CONFIG_VIDEO_ovisp_BUFFER_STATIC)
	conf->vaddr = dma_alloc_coherent(conf->dev, conf->size, &conf->paddr,
					GFP_KERNEL);
#else
	conf->paddr = ISP_MEMORY_BASE;
	conf->vaddr = ioremap(conf->paddr, conf->size);
#endif
	if (!conf->vaddr) {
		dev_err(conf->dev, "dma_alloc_coherent of size %ld failed\n",
			conf->size);
		kfree(conf);
		return ERR_PTR(-ENOMEM);
	}

	return conf;
}
EXPORT_SYMBOL_GPL(ovisp_vb2_init_ctx);

void ovisp_vb2_cleanup_ctx(void *alloc_ctx)
{
	kfree(alloc_ctx);
}
EXPORT_SYMBOL_GPL(ovisp_vb2_cleanup_ctx);

MODULE_DESCRIPTION("ovisp memory handling routines for videobuf2");
MODULE_LICENSE("GPL");

