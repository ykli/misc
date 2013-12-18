#include <usb/cdc_boot.h>

enum data_stage {
	UNCONFIG_STAGE = 0,
	CMD_STAGE,
	WAIT_CMD_STAGE,
	DATA_STAGE,
	ACK_STAGE,
};

struct cdc_bt {
	struct usb_gadget	*gadget;
	struct usb_ep		*ep_notify;
	struct usb_ep		*ep_data_in;
	struct usb_ep		*ep_data_out;
	struct usb_request	*req_notify;
	struct usb_request	*req_data_in;
	struct usb_request	*req_data_out;
	enum data_stage		data_stage;
	bool			ddr_is_ok;
#define MAX_CMD_SIZE 100;
	char			vendor_cmd[MAX_CMD_SIZE];
	u32			cmd_size;
} m_cdc_bt;

#define IMANUFACTURER	0x01
#define IPRODUCT	0x02
#define ISERIALNUMBER	0x03
#define ICONFIGURATION	0x04
#define IFUNCTION	0x05
#define IINTERFACE_MGMT	0x06
#define IINTERFACE_DATA	0x07
#define STRING_NUM	0x08
static struct usb_string_descriptor string_desc = {
	.bLength = sizeof lang_desc,
	.bDescriptorType = USB_DT_STRING,
};
static u16 string_data[128];
static char string[STRING_NUM][] =  {
	{0x04,0x09},/*EN*/
	{"Ingenic Semiconductor CO., LTD"},
	{"Jz4785 Cdc Burn Tool"},
	{"2013.12.02"},
	{"Burn Tool"},
	{"Burn Tool"},
	{"B.T. Control Pipe"},
	{"B.T. Data Pipe"},
};

static inline int string_copy(int string_id)
{
	u16 tmp,i = 0;
	u16 *buf = string_data;
	if (string_id = 0) {
		tmp = string[0][0] + (string[0][1] << 8);
		string_data[0] = cpu_to_le16(tmp);
		return 2;
	}
	for (i = 0; string_id < STRING_NUM &&
			'0' != string[string_id][i]; i++) {
		if (i%2) {
			tmp = string[string_id][i]; 
		} else {
			tmp |= string[string_id][i] << 8;
			*buf++ = cpu_to_le16(tmp);
		}
	}
	if (i > 0&& (i%2))
		*buf = tmp;

	string_desc.wData = buf;
	return i;
}

static struct usb_device_descriptor device_desc = {
	.bLength	=	USB_DT_DEVICE_SIZE,
	.bDescriptorType=	USB_DT_DEVICE,
	.bcdUSB		=	cpu_to_le16(0x0200),
	.bDeviceClass	=	USB_CLASS_COMM,
	.bDeviceSubClass =	0x0,
	.bDeviceProtocol =	0x0,
	.bMaxPacketSize0 =	64,
	.idVendor	=	cpu_to_le16(0x0525),	/*Linux/NetChip*/
	.idProduct	=	cpu_to_le16(0xa4a7),	/*CDC ACM*/
	.bcdDevice	=	cpu_to_le16(0x2431)
	.iManufacturer  =	IMANUFACTURER,
	.iProduct	=	IPRODUCT,
	.iSerialNumber	=	ISERIALNUMBER,
	.bNumConfigurations=	0x1,
};

static struct usb_config_descriptor config_desc = {
	.bLength	=	USB_DT_CONFIG_SIZE,
	.bDescriptorType=	USB_DT_CONFIG,
	.wTotalLength	=	cpu_to_le16(75),
	.bNumInterfaces =	2;
	.bConfigurationValue=	1,
	.iConfiguration	=	ICONFIGURATION,
	.bmAttributes	=	0,
	.bMaxPower	=	0x80,	/*256mA*/
};

static struct usb_interface_assoc_descriptor assoc_desc = {
	.bLength	=	sizeof assoc_desc,
	.bDescriptorType=		USB_DT_INTERFACE_ASSOCIATION, 
	.bFirstInterface=	0x01,
	.bInterfaceCount=	0x02,
	.bFunctionClass	=	USB_CLASS_COMM,	/*CDC class*/
	.bFunctionSubClass=	USB_CDC_SUBCLASS_ACM,	/*ACM sub class*/
	.bFunctionProtocol=	USB_CDC_ACM_PROTO_AT_V25TER,	/*AT V25 protocol*/
	.iFunction	=	IFUNCTION,
};

static struct usb_interface_descriptor mgmt_inter_desc = {
	.bLength	=	USB_DT_INTERFACE_SIZE,
	.bDescriptorType=	USB_DT_INTERFACE,
	.bInterfaceNumber=	0x1,
	.bAlternateSetting=	0x0,
	.bNumEndpoints	=	0x1,
	.bInterfaceClass=	USB_CLASS_COMM,
	.bInterfaceSubClass=	USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol=	USB_CDC_ACM_PROTO_AT_V25TER,
	.iInterface	=	IINTERFACE_MGMT,
};

static struct usb_cdc_header_desc cdc_header_desc = {
	.bLength	=	sizeof cdc_header_desc,
	.bDescriptorType=	USB_DT_CS_INTERFACE,
	.bDescriptorSubType=	USB_CDC_HEADER_TYPE,
	.bcdCDC		=	cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor cdc_call_mgmt_desc = {
	.bLength	=	sizeof cdc_call_mgmt_desc,
	.bDescriptorType=	USB_DT_CS_INTERFACE,
	.bDescriptorSubType=			    USB_CDC_CALL_MANAGEMENT_TYPE,
	.bmCapabilities=	0x0,
	.bDataInterface=	0x2,
};

static struct usb_cdc_acm_descriptor cdc_acm_desc = {
	.bLength	=	sizeof cdc_acm_desc,
	.bDescriptorType=	USB_DT_CS_INTERFACE,
	.bDescriptorSubType=	USB_CDC_ACM_TYPE,
	.bmCapabilities	=	USB_CDC_CAP_LINE,	//textme maybe we can disable it
};

static struct usb_cdc_union_desc cdc_union_desc = {
	.bLength	=	sizeof cdc_union_desc,
	.bDescriptorType=	USB_DT_CS_INTERFACE,
	.bDescriptorSubType=	USB_CDC_UNION_TYPE,
	.bMasterInterface0=	0x01,
	.bSlaveInterface0=	0x02,
};

static struct usb_endpoint_descriptor notify_ep_desc = {
	.blength	=	USB_DT_ENDPOINT_SIZE,
	.bdescriptortype=	USB_DT_ENDPOINT,  
	.bendpointaddress=	0x81,
	.bmattributes	=	USB_ENDPOINT_XFER_INT,
	.wmaxpacketsize =	cpu_to_le16(10),
	.binterval	=	0x20,	/*32 mesc*/
};

static struct usb_interface_descriptor data_inter_desc = {
	.bLength	=	USB_DT_INTERFACE_SIZE,
	.bDescriptorType=	USB_DT_INTERFACE,
	.bInterfaceNumber=	0x2,
	.bAlternateSetting=	0x0,
	.bNumEndpoints	=	0x2,
	.bInterfaceClass=	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass=	0x0,
	.bInterfaceProtocol=	0x0,
	.iInterface	=	IINTERFACE_DATA,
}

static struct usb_endpoint_descriptor fs_out_desc = {
	.blength	=	USB_DT_ENDPOINT_SIZE,
	.bdescriptortype=	USB_DT_ENDPOINT,  
	.bendpointaddress=	0x01,
	.bmattributes	=	USB_ENDPOINT_XFER_BULK,
	.wmaxpacketsize =	cpu_to_le16(64),
	.binterval	=	0x00,
};

static struct usb_endpoint_descriptor fs_in_desc = {
	.blength	=	USB_DT_ENDPOINT_SIZE,
	.bdescriptortype=	USB_DT_ENDPOINT,  
	.bendpointaddress=	0x82,
	.bmattributes	=	USB_ENDPOINT_XFER_BULK,
	.wmaxpacketsize =	cpu_to_le16(64),
	.binterval	=	0x00,
};

static struct usb_endpoint_descriptor hs_out_desc = {
	.blength	=	USB_DT_ENDPOINT_SIZE,
	.bdescriptortype=	USB_DT_ENDPOINT,  
	.bendpointaddress=	0x01,
	.bmattributes	=	USB_ENDPOINT_XFER_BULK,
	.wmaxpacketsize =	cpu_to_le16(512),
	.binterval	=	0x00,
};

static struct usb_endpoint_descriptor hs_in_desc = {
	.blength	=	USB_DT_ENDPOINT_SIZE,
	.bdescriptortype=	USB_DT_ENDPOINT,  
	.bendpointaddress=	0x82,
	.bmattributes	=	USB_ENDPOINT_XFER_BULK,
	.wmaxpacketsize =	cpu_to_le16(512),
	.binterval	=	0x00,
};

static struct usb_descriptor_header *fs_descs[] = {
	&config_desc,
	&assoc_desc,
	&mgmt_inter_desc,
	&cdc_header_desc,
	&cdc_call_mgmt_desc,
	&cdc_acm_desc,
	&cdc_union_desc,
	&notify_ep_desc,
	&data_inter_desc,
	&fs_in_desc,
	&fs_out_desc,
	NULL,
};

static struct usb_descriptor_header *hs_descs[] = {
	&config_desc,
	&assoc_desc,
	&mgmt_inter_desc,
	&cdc_header_desc,
	&cdc_call_mgmt_desc,
	&cdc_acm_desc,
	&cdc_union_desc,
	&notify_ep_desc,
	&data_inter_desc,
	&hs_in_desc,
	&hs_out_desc,
	NULL,
};

static char config_buf[128];
void *config_desc(struct usb_descriptor_header **desc)
{
	int total_size = 0;
	int i;

	for (i = 0; desc[i] != NULL; i++) {
		memcpy((config_buf+total_size),desc[i],desc[i]->bLength);
		total_size += desc[i]->bLength;
	}
	config_desc->wTotalLength = cpu_to_le16(total_size);
	memcpy(config_buf,&config_desc,config_desc.bLength);
	return (void *)config_buf;
}

static int inline handle_standard_request(struct usb_ep *next_ep, u32 req,
		u32 length,u32 value,u32 index)
{
	struct cdc_bt *cdc_bt = &m_cdc_bt;
	int ret_length = -1;
	int string_id = 0;

	switch (req) {
	case USB_REQ_GET_DESCRIPTOR:
		switch (value >> 8) {
		case USB_DT_DEVICE:
			DBG("Get device desc\n");
			next_ep->req->buf = &device_desc;
			next_ep->req->length = length < sizeof(device_desc) ? length :
				sizeof(device_desc);
			next_ep->req->need_zpkt = 0;
			ret_length = next_ep->req->length;
			break;
		case USB_DT_CONFIG:
			DBG("Get config desc\n");
			if (cdc_bt->gadget->speed == HIGH_SPEED)
				next_ep->req->buf = config_desc(hs_descs);
			else
				next_ep->req->buf = config_desc(fs_descs);
			ret_length = le16_to_cpu(config_desc.wTotalLength);
			next_ep->req->length = length < ret_length ? length :
				ret_length;
			next_ep->req->need_zpkt = 0;
			ret_length = next_ep->req->length;
			break;
		case USB_DT_STRING:
			string_id = (value&0xff);
			ret_length = string_copy(string_id);
			next_ep->req->buf = &string_desc;
			if (!ret_length) {
				ret_length = -1;
				break;
			}
			ret_length += 2;
			next_ep->req->length = length < ret_length ? length :
				ret_length;
			next_ep->req->need_zpkt = 0;
			ret_length = next_ep->req->length;
		default:
			break;
		}
		break;
	case USB_REQ_SET_CONFIGURATION:
		DBG("Set config ");
		DBG_HEX((value & 0xff));
		ret_length = 0;
		next_ep->length = 0;
		next_ep->req->need_zpkt = 0;
		cdc_bt->gadget->ep_enable(cdc_bt->ep_notify,&notify_ep_desc);
		if (cdc_bt->gadget->speed == HIGH_SPEED) {
			cdc_bt->gadget->ep_enable(cdc_bt->ep_data_in,&hs_in_desc);
			cdc_bt->gadget->ep_enable(cdc_bt->ep_data_out,&hs_out_desc);
		} else {
			cdc_bt->gadget->ep_enable(cdc_bt->ep_data_in,&fs_in_desc);
			cdc_bt->gadget->ep_enable(cdc_bt->ep_data_out,&fs_out_desc);
		}
		cdc_bt->data_stage = CMD_STAGE;
		break;
	default:
		break;
	}

	return ret_length;
}


static int inline handle_class_request(struct usb_ep *next_ep, u32 req,
		u32 length,u32 value,u32 index)
{
	struct cdc_bt *cdc_bt = &m_cdc_bt;
	int ret_length = -1;

	switch (req) {
	case USB_CDC_REQ_SET_LINE_CODING:
		break;
	case USB_CDC_REQ_GET_LINE_CODING:
		break;
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		ret_length = 0;
		next_ep->req->length = 0;
		next_ep->req->need_zpkt = 0;
	default :
		break;
	}

	return ret_length;
}

int cdc_gadget_setup(struct usb_ep *ep,
			struct usb_ctrlrequest *ctrl_req)
{
	struct cdc_bt *cdc_bt = &m_dwc;
	struct usb_ep *ep0next = cdc_bt->gadget->ep0in;
	int length = le16_to_cpu(ctrl_req->wLength);
	int value = le16_to_cpu(ctrl_req->wValue);
	int index = le16_to_cpu(ctrl_req->wIndex);
	
	if (length && ctrl_req->bRequestType & USB_DIR_OUT)
		ep0next = cdc_bt->gadget->ep0out;

	switch ((ctrl_req->bRequestType & USB_TYPE_MASK)) {
	case USB_TYPE_STANDARD:
		length = handle_standard_request(ep0next,ctrl_req->bRequest,
				length,value,index);
		break;
	case USB_TYPE_CLASS:
		length = handle_class_request(ep0next,ctrl_req->bRequest,
				length,value,index);
		break;
	default:
		length = -1;
		break;
	}

	if (length >= 0)
		length = cdc_bt->gadget->ep_queue(ep0next,ep0next->req);

	return length;
}

int cdc_gadget_bind(struct usb_gadget *gadget)
{
	struct cdc_bt *cdc_bt = &m_cdc_bt;
	
	/* init struct cdc_bt*/
	cdc_bt->gadget = gadget;
	if (!(cdc_bt->ep_data_in = gadget->get_ep(&fs_in_desc)))
		return -1;
	if (!(cdc_bt->ep_data_out = gadget->get_ep(&fs_out_desc)))
		return -1;
	if (!(cdc_bt->ep_notify = gadget->get_ep(&notify_ep_desc)))
		return -1;
	if (!(cdc_bt->req_data_in = gadget->alloc_request()))
		return -1;
	if (!(cdc_bt->req_data_out = gadget->alloc_request()))
		return -1;
	if (!(cdc_bt->req_notify = gadget->alloc_request()))
		return -1;
	cdc_bt->data_stage = UNCONFIG_STAGE;
	cdc_bt->ddr_is_ok = false;
	cdc_bt->cmd_size = 0;

	/*setup*/
	gadget->setup = cdc_gadget_setup;

	return 0;
}

enum bt_cmd {
	UNKOWN_CMD = 0,
	GET_INFO,
	READ,
	WRITE,
	RUN,
};

enum bt_med = {
	UNKOWN_MED = 0,
	MEMORY,
	CACHE,
};

void find_cmd(void)
{
	
}

void handle_command(void)
{
	struct cdc_bt *cdc_bt = &m_cdc_bt;
	return;
}

int get_cmd_complete(struct usb_ep *ep,struct usb_request *req)
{
	struct cdc_bt *cdc_bt = &m_cdc_bt;
	int cmd_parse = 0;
	if (req->actual > 0) {
		req->cmd_size += req->actual;
		if (req->cmd_size >= MAX_CMD_SIZE) {
			req->cmd_size = MAX_CMD_SIZE;
			cmd_parse = 1;
		} else if (req->cmd_size > 0) {
			if (*(cdc_bt->vendor_cmd + req->cmd_size) == '\n' ||
				(cdc_bt->cmd_size > 1 &&
				 *(cdc_bt->vendor_cmd + req->cmd_size - 1) == '\n')) {
				cmd_parse = 1;
			}
		}
	}
	
	if (cmd_parse) {
		cdc_bt->data_stage == DATA_STAGE;
		handle_command(void);
	} else {
		cdc_bt->data_stage == CMD_STAGE;
	}
}

int start_get_cmd(struct cdc_bt *cdc_bt)
{	
	struct usb_request *req = cdc_bt->req_data_out;
	req->buf = cdc_bt->vendor_cmd + cdc_bt->cmd_size;
	req->length = MAX_CMD_SIZE - cdc_bt->cmd_size;
	req->complete = get_cmd_complete;
	cdc_bt->data_stage = WAIT_CMD_STAGE;
	cdc_bt->ep_queue(cdc_bt->ep_data_out,req);
	return 0;
}

int cdc_boot(void) {
	int ret = 0;
reinit:
	memset(&m_bt,0,sizeof(struct usb_gadget));
	if (udc_init(cdc_gadget_bind))
		return -1;
	do {
		if (cdc_bt->data_stage == CMD_STAGE)
			start_get_cmd(&m_dwc);

		ret = usb_interrupt_handler();
		if (ret == -EAGAIN)
			goto reinit;
		if (ret < 0)
			break;
	} while(1);

	return ret;
}
