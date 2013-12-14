
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-mediabus.h>
#include <media/front_camera.h>
#include <mach/ovisp-v4l2.h>

#define Front_camera_CHIP_ID	(0xc0)

#define VGA_WIDTH	640
#define VGA_HEIGHT	480
#define QVGA_WIDTH	320
#define QVGA_HEIGHT	240
#define CIF_WIDTH	352
#define CIF_HEIGHT	288
#define QCIF_WIDTH	176
#define	QCIF_HEIGHT	144
#define MAX_WIDTH	VGA_WIDTH
#define MAX_HEIGHT	VGA_HEIGHT

#define Front_camera_REG_END	0xff

struct front_camera_format_struct;
struct front_camera_info {
	struct v4l2_subdev sd;
	struct front_camera_format_struct *fmt;
	struct front_camera_win_size *win;
	int brightness;
	int contrast;
	int frame_rate;
};

struct regval_list {
	unsigned char addr;
	unsigned char value;
};

static struct regval_list front_camera_init_regs[] = {

	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_fmt_yuv422[] = {
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_win_vga[] = {

	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_win_cif[] = {
	0

	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_win_qvga[] = {
	

	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_win_qcif[] = {


	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_win_144x176[] = {


	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_l3[] = {
	{0xd5, 0xc0},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_l2[] = {
	{0xd5, 0xe0},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_l1[] = {
	{0xd5, 0xf0},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_h0[] = {
	{0xd5, 0x00},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_h1[] = {
	{0xd5, 0x10},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_h2[] = {
	{0xd5, 0x20},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_brightness_h3[] = {
	{0xd5, 0x30},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_l3[] = {
	{0xd3, 0x28},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_l2[] = {
	{0xd3, 0x30},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_l1[] = {
	{0xd3, 0x38},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_h0[] = {
	{0xd3, 0x40},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_h1[] = {
	{0xd3, 0x48},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_h2[] = {
	{0xd3, 0x50},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list front_camera_contrast_h3[] = {
	{0xd3, 0x58},	
	{Front_camera_REG_END, 0x00},	/* END MARKER */
};

static int front_camera_read(struct v4l2_subdev *sd, unsigned char reg,
		unsigned char *value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		return ret;

	*value = (unsigned char)ret;

	return 0;
}

static int front_camera_write(struct v4l2_subdev *sd, unsigned char reg,
		unsigned char value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);
	if (ret < 0)
		return ret;

	return 0;
}

static int front_camera_write_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	while (vals->addr!= Front_camera_REG_END) {
		int ret = front_camera_write(sd, vals->addr, vals->value);
		if (ret < 0)
			return ret;
		vals++;
	}
	return 0;
}

static int front_camera_reset(struct v4l2_subdev *sd, u32 val)
{
	return 0;
}

static int front_camera_init(struct v4l2_subdev *sd, u32 val)
{
	struct front_camera_info *info = container_of(sd, struct front_camera_info, sd);

	info->fmt = NULL;
	info->win = NULL;
	info->brightness = 0;
	info->contrast = 0;
	info->frame_rate = 0;

	return front_camera_write_array(sd, front_camera_init_regs);
}

static int front_camera_detect(struct v4l2_subdev *sd)
{
	unsigned char v;
	int ret;

	ret = front_camera_init(sd, 0);
	if (ret < 0)
		return ret;

	ret = front_camera_write(sd, 0xfc, 0x16);
	if (ret < 0)
		return ret;
	ret = front_camera_read(sd, 0xfc, &v);
	if (ret < 0)
		return ret;
	ret = front_camera_read(sd, 0x00, &v);
	if (ret < 0)
		return ret;
	if (v != Front_camera_CHIP_ID)
		return -ENODEV;
	return 0;
}

static struct front_camera_format_struct {
	enum v4l2_mbus_pixelcode mbus_code;
	enum v4l2_colorspace colorspace;
	struct regval_list *regs;
} front_camera_formats[] = {
	{
		.mbus_code	= V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace	= V4L2_COLORSPACE_JPEG,
		.regs 		= front_camera_fmt_yuv422,
	},
};
#define N_Front_camera_FMTS ARRAY_SIZE(front_camera_formats)

static struct front_camera_win_size {
	int	width;
	int	height;
	struct regval_list *regs;
} front_camera_win_sizes[] = {
	/* VGA */
	{
		.width		= VGA_WIDTH,
		.height		= VGA_HEIGHT,
		.regs 		= front_camera_win_vga,
	},
	/* CIF */
	{
		.width		= CIF_WIDTH,
		.height		= CIF_HEIGHT,
		.regs 		= front_camera_win_cif,
	},
	/* QVGA */
	{
		.width		= QVGA_WIDTH,
		.height		= QVGA_HEIGHT,
		.regs 		= front_camera_win_qvga,
	},
	/* QCIF */
	{
		.width		= QCIF_WIDTH,
		.height		= QCIF_HEIGHT,
		.regs 		= front_camera_win_qcif,
	},
	/* 144x176 */
	{
		.width		= QCIF_HEIGHT,
		.height		= QCIF_WIDTH,
		.regs 		= front_camera_win_144x176,
	},
};
#define N_WIN_SIZES (ARRAY_SIZE(front_camera_win_sizes))

static int front_camera_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned index,
					enum v4l2_mbus_pixelcode *code)
{
	if (index >= N_Front_camera_FMTS)
		return -EINVAL;

	*code = front_camera_formats[index].mbus_code;
	return 0;
}

static int front_camera_try_fmt_internal(struct v4l2_subdev *sd,
		struct v4l2_mbus_framefmt *fmt,
		struct front_camera_format_struct **ret_fmt,
		struct front_camera_win_size **ret_wsize)
{
	int index;
	struct front_camera_win_size *wsize;

	for (index = 0; index < N_Front_camera_FMTS; index++)
		if (front_camera_formats[index].mbus_code == fmt->code)
			break;
	if (index >= N_Front_camera_FMTS) {
		/* default to first format */
		index = 0;
		fmt->code = front_camera_formats[0].mbus_code;
	}
	if (ret_fmt != NULL)
		*ret_fmt = front_camera_formats + index;

	fmt->field = V4L2_FIELD_NONE;

	for (wsize = front_camera_win_sizes; wsize < front_camera_win_sizes + N_WIN_SIZES;
	     wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)
			break;
	if (wsize >= front_camera_win_sizes + N_WIN_SIZES)
		wsize--;   /* Take the smallest one */
	if (ret_wsize != NULL)
		*ret_wsize = wsize;

	fmt->width = wsize->width;
	fmt->height = wsize->height;
	fmt->colorspace = front_camera_formats[index].colorspace;
	return 0;
}

static int front_camera_try_mbus_fmt(struct v4l2_subdev *sd,
			    struct v4l2_mbus_framefmt *fmt)
{
	return front_camera_try_fmt_internal(sd, fmt, NULL, NULL);
}

static int front_camera_s_mbus_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *fmt)
{
	struct front_camera_info *info = container_of(sd, struct front_camera_info, sd);
	struct front_camera_format_struct *fmt_s;
	struct front_camera_win_size *wsize;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct v4l2_fmt_data *data = v4l2_get_fmt_data(fmt);
	int ret;
	int i;

	ret = front_camera_try_fmt_internal(sd, fmt, &fmt_s, &wsize);
	if (ret)
		return ret;

	if ((info->fmt != fmt_s) && fmt_s->regs) {
	//	ret = front_camera_write_array(sd, fmt_s->regs);
		if (ret)
			return ret;
	}

	if ((info->win != wsize) && wsize->regs) {
	//	ret = front_camera_write_array(sd, wsize->regs);
		if (ret)
			return ret;
	}

#if 1
		memset(data, 0, sizeof(*data));
		if ((wsize->width == VGA_WIDTH)
			&& (wsize->height == VGA_HEIGHT)) {
		//	data->flags = V4L2_I2C_ADDR_16BIT;
			data->slave_addr = client->addr;
			data->reg_num = ARRAY_SIZE(front_camera_win_vga)-1;
			for(i = 0;i<data->reg_num;i++) {
				data->reg[i].addr = front_camera_win_vga[i].addr;
				data->reg[i].data = front_camera_win_vga[i].value;
			}
		} else if ((wsize->width ==QVGA_WIDTH)
			&& (wsize->height == QVGA_HEIGHT)) {
		//	data->flags = V4L2_I2C_ADDR_16BIT;
			data->slave_addr = client->addr;
			data->reg_num = ARRAY_SIZE(front_camera_win_qvga)-1;
			for(i = 0;i<data->reg_num;i++) {
				data->reg[i].addr = front_camera_win_qvga[i].addr;
				data->reg[i].data = front_camera_win_qvga[i].value;
			}
		
		}
		else if ((wsize->width ==QCIF_WIDTH)
			&& (wsize->height == QCIF_HEIGHT)) {
		//	data->flags = V4L2_I2C_ADDR_16BIT;
			data->slave_addr = client->addr;
			data->reg_num = ARRAY_SIZE(front_camera_win_qcif)-1;
			for(i = 0;i<data->reg_num;i++) {
				data->reg[i].addr = front_camera_win_qcif[i].addr;
				data->reg[i].data = front_camera_win_qcif[i].value;
			}
		
		}
		else if ((wsize->width ==CIF_WIDTH)
			&& (wsize->height == CIF_HEIGHT)) {
		//	data->flags = V4L2_I2C_ADDR_16BIT;
			data->slave_addr = client->addr;
			data->reg_num = ARRAY_SIZE(front_camera_win_cif)-1;
			for(i = 0;i<data->reg_num;i++) {
				data->reg[i].addr = front_camera_win_cif[i].addr;
				data->reg[i].data = front_camera_win_cif[i].value;
			}
		
		}
	
#endif


	info->fmt = fmt_s;
	info->win = wsize;

	return 0;
}

static int front_camera_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	a->c.left	= 0;
	a->c.top	= 0;
	a->c.width	= MAX_WIDTH;
	a->c.height	= MAX_HEIGHT;
	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

static int front_camera_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	a->bounds.left			= 0;
	a->bounds.top			= 0;
	a->bounds.width			= MAX_WIDTH;
	a->bounds.height		= MAX_HEIGHT;
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	return 0;
}

static int front_camera_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	return 0;
}

static int front_camera_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	return 0;
}

static int front_camera_frame_rates[] = { 30, 15, 10, 5, 1 };

static int front_camera_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	if (interval->index >= ARRAY_SIZE(front_camera_frame_rates))
		return -EINVAL;
	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	interval->discrete.numerator = 1;
	interval->discrete.denominator = front_camera_frame_rates[interval->index];
	return 0;
}

static int front_camera_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;

	/*
	 * If a minimum width/height was requested, filter out the capture
	 * windows that fall outside that.
	 */
	for (i = 0; i < N_WIN_SIZES; i++) {
		struct front_camera_win_size *win = &front_camera_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int front_camera_queryctrl(struct v4l2_subdev *sd,
		struct v4l2_queryctrl *qc)
{
	return 0;
}

static int front_camera_set_brightness(struct v4l2_subdev *sd, int brightness)
{
	printk(KERN_DEBUG "[Front_camera]set brightness %d\n", brightness);

	switch (brightness) {
	case BRIGHTNESS_L3:
		front_camera_write_array(sd, front_camera_brightness_l3);
		break;
	case BRIGHTNESS_L2:
		front_camera_write_array(sd, front_camera_brightness_l2);
		break;
	case BRIGHTNESS_L1:
		front_camera_write_array(sd, front_camera_brightness_l1);
		break;
	case BRIGHTNESS_H0:
		front_camera_write_array(sd, front_camera_brightness_h0);
		break;
	case BRIGHTNESS_H1:
		front_camera_write_array(sd, front_camera_brightness_h1);
		break;
	case BRIGHTNESS_H2:
		front_camera_write_array(sd, front_camera_brightness_h2);
		break;
	case BRIGHTNESS_H3:
		front_camera_write_array(sd, front_camera_brightness_h3);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int front_camera_set_contrast(struct v4l2_subdev *sd, int contrast)
{
	printk(KERN_DEBUG "[Front_camera]set contrast %d\n", contrast);

	switch (contrast) {
	case CONTRAST_L3:
		front_camera_write_array(sd, front_camera_contrast_l3);
		break;
	case CONTRAST_L2:
		front_camera_write_array(sd, front_camera_contrast_l2);
		break;
	case CONTRAST_L1:
		front_camera_write_array(sd, front_camera_contrast_l1);
		break;
	case CONTRAST_H0:
		front_camera_write_array(sd, front_camera_contrast_h0);
		break;
	case CONTRAST_H1:
		front_camera_write_array(sd, front_camera_contrast_h1);
		break;
	case CONTRAST_H2:
		front_camera_write_array(sd, front_camera_contrast_h2);
		break;
	case CONTRAST_H3:
		front_camera_write_array(sd, front_camera_contrast_h3);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int front_camera_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct front_camera_info *info = container_of(sd, struct front_camera_info, sd);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = info->brightness;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = info->contrast;
		break;
	case V4L2_CID_FRAME_RATE:
		ctrl->value = info->frame_rate;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int front_camera_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct front_camera_info *info = container_of(sd, struct front_camera_info, sd);
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ret = front_camera_set_brightness(sd, ctrl->value);
		if (!ret)
			info->brightness = ctrl->value;
		break;
	case V4L2_CID_CONTRAST:
		ret = front_camera_set_contrast(sd, ctrl->value);
		if (!ret)
			info->contrast = ctrl->value;
		break;
	case V4L2_CID_FRAME_RATE:
		info->frame_rate = ctrl->value;
		break;
	case V4L2_CID_SET_AUTO_FOCUS:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int front_camera_g_chip_ident(struct v4l2_subdev *sd,
		struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_Front_camera, 0);
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int front_camera_g_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char val = 0;
	int ret;

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = front_camera_read(sd, reg->reg & 0xff, &val);
	reg->val = val;
	reg->size = 1;
	return ret;
}

static int front_camera_s_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	front_camera_write(sd, reg->reg & 0xff, reg->val & 0xff);
	return 0;
}
#endif

static const struct v4l2_subdev_core_ops front_camera_core_ops = {
	.g_chip_ident = front_camera_g_chip_ident,
	.g_ctrl = front_camera_g_ctrl,
	.s_ctrl = front_camera_s_ctrl,
	.queryctrl = front_camera_queryctrl,
	.reset = front_camera_reset,
	.init = front_camera_init,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = front_camera_g_register,
	.s_register = front_camera_s_register,
#endif
};

static const struct v4l2_subdev_video_ops front_camera_video_ops = {
	.enum_mbus_fmt = front_camera_enum_mbus_fmt,
	.try_mbus_fmt = front_camera_try_mbus_fmt,
	.s_mbus_fmt = front_camera_s_mbus_fmt,
	.cropcap = front_camera_cropcap,
	.g_crop	= front_camera_g_crop,
	.s_parm = front_camera_s_parm,
	.g_parm = front_camera_g_parm,
	.enum_frameintervals = front_camera_enum_frameintervals,
	.enum_framesizes = front_camera_enum_framesizes,
};

static const struct v4l2_subdev_ops front_camera_ops = {
	.core = &front_camera_core_ops,
	.video = &front_camera_video_ops,
};

static int front_camera_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct front_camera_info *info;
	int ret;

	info = kzalloc(sizeof(struct front_camera_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	v4l2_i2c_subdev_init(sd, client, &front_camera_ops);

	/* Make sure it's an front_camera */
	ret = front_camera_detect(sd);
	if (ret) {
		v4l_err(client,
			"chip found @ 0x%x (%s) is not an front_camera chip.\n",
			client->addr, client->adapter->name);
		kfree(info);
		return ret;
	}
	v4l_info(client, "front_camera chip found @ 0x%02x (%s)\n",
			client->addr, client->adapter->name);

	return 0;
}

static int front_camera_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct front_camera_info *info = container_of(sd, struct front_camera_info, sd);

	v4l2_device_unregister_subdev(sd);
	kfree(info);
	return 0;
}

static const struct i2c_device_id front_camera_id[] = {
	{ "front_camera", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, front_camera_id);

static struct i2c_driver front_camera_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "front_camera",
	},
	.probe		= front_camera_probe,
	.remove		= front_camera_remove,
	.id_table	= front_camera_id,
};

static __init int init_front_camera(void)
{
	return i2c_add_driver(&front_camera_driver);
}

static __exit void exit_front_camera(void)
{
	i2c_del_driver(&front_camera_driver);
}

module_init(init_front_camera);
module_exit(exit_front_camera);

MODULE_DESCRIPTION("A low-level driver for Omnivision front_camera sensors");
MODULE_LICENSE("GPL");
