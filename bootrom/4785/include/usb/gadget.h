/*
 * gadget.h
 *
 * Internal BootROM c-code for JZ4780B.
 * Supports booting USB device.
 *
 * Copyright (C) 2005 - 2012 Ingenic Semiconductor Inc.
 *
 * Authors: cli <cli@ingenic.cn>
 */
#ifndef __GADGET_H__
#define __GADGET_H__
#include <usb/ch9.h>
#include <usb/cdc.h>

struct usb_request {
	u8 *buf;
	u32 length;
	u32 actual;
	u32 xfer_size;
	bool need_zpkt;
	bool is_used;
	void (*complete)(struct usb_ep *ep,
			struct usb_request *req);
	void *context;
};

struct usb_ep {
#define EP_NUM(ep)	((ep)->epaddr & USB_ENDPOINT_NUMBER_MASK)
#define EP_IS_IN(ep)	((ep)->epaddr & USB_ENDPOINT_DIR_MASK)
	u8 epaddr;
	u16 mps;		//max packet size
	u16 mps_limit;
	struct usb_ep	*next;
	struct usb_request *req;
#define EP_ACTIVE	(0x1 << 2)
#define EP_DATA_TRANS	(0x2 << 2)
#define EP_BUSY		(0x4 << 2)
#define EP_USED		(0x8 << 2)
#define EP_IS_BUSY(ep)		((ep)->flags & EP_BUSY)
#define EP_HAVE_DATA_TRANS(ep)	((ep)->flags & EP_DATA_TRANS)
#define EP_IS_USED(ep)		((ep)->flags & EP_USED)
#define EP_TYEP(ep)		((ep)->flags & USB_ENDPOINT_XFERTYPE_MASK)
	u32  flags;
};

struct usb_gadget {
	struct usb_ep		*ep0in;
	struct usb_ep		*ep0out;
	/*gadget provide to udc */
	int (*setup)(struct usb_ep *ep,
			struct usb_ctrlrequest *ctrl_req);
	/*udc provide to gadget */
	int (*ep_queue)(struct usb_ep *usb_ep,
			struct usb_request *req);

	int (*ep_enable)(struct usb_ep *usb_ep,
			struct usb_endpoint_descriptor *desc);

	struct usb_request* (*alloc_request)(void);
	struct usb_ep* (*get_ep)(struct usb_endpoint_descriptor *desc);
};

/*udc initializtion*/
int udc_init(int (*bind)(struct usb_gadget *gadget));

/*usb main interrupt handler*/
int usb_interrupt_handler(void);

#endif  /* __GADGET_H__ */
