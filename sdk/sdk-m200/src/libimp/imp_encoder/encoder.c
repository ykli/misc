/*
 * IMP venc func source file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#include <stdio.h>

int IMP_VENC_CreateGroup(VENC_GRP VeGroup)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_DestroyGroup(VENC_GRP VeGroup)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_DestroyChn(VENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_RegisterChn(VENC_GRP VeGroup, VENC_CHN VeChn )
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_UnRegisterChn(VENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_StartRecvPic(VENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_StartRecvPicEx(VENC_CHN VeChn, VENC_RECV_PIC_PARAM_S *pstRecvParam)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_StopRecvPic(VENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_Query(VENC_CHN VeChn, VENC_CHN_STAT_S *pstStat)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetChnAttr( VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetChnAttr( VENC_CHN VeChn, VENC_CHN_ATTR_S *pstAttr)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, bool bBlockFlag)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_InsertUserData(VENC_CHN VeChn, uint8_t *pu8Data, uint32_t u32Len)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SendFrame(VENC_GRP VeGroup, VIDEO_FRAME_INFO_S *pstFrame)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetMaxStreamCnt(VENC_CHN VeChn, uint32_t u32MaxStrmCnt)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetMaxStreamCnt(VENC_CHN VeChn, uint32_t *pu32MaxStrmCnt)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_RequestIDR(VENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetFd(VENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetRoiCfg(VENC_CHN VeChn, VENC_ROI_CFG_S *pstVencRoiCfg)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetRoiCfg(VENC_CHN VeChn, uint32_t u32Index, VENC_ROI_CFG_S *pstVencRoiCfg)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264SliceSplit(VENC_CHN VeChn, const VENC_PARAM_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264SliceSplit(VENC_CHN VeChn, VENC_PARAM_H264_SLICE_SPLIT_S *pstSliceSplit)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264InterPred(VENC_CHN VeChn, const VENC_PARAM_H264_INTER_PRED_S *pstH264InterPred)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264InterPred(VENC_CHN VeChn, VENC_PARAM_H264_INTER_PRED_S *pstH264InterPred)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264IntraPred(VENC_CHN VeChn, const VENC_PARAM_H264_INTRA_PRED_S *pstH264IntraPred)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264IntraPred(VENC_CHN VeChn, VENC_PARAM_H264_INTRA_PRED_S *pstH264IntraPred)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264Trans(VENC_CHN VeChn, const VENC_PARAM_H264_TRANS_S *pstH264Trans)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264Trans(VENC_CHN VeChn, VENC_PARAM_H264_TRANS_S *pstH264Trans)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264Entropy(VENC_CHN VeChn, const VENC_PARAM_H264_ENTROPY_S *pstH264EntropyEnc)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264Entropy(VENC_CHN VeChn, VENC_PARAM_H264_ENTROPY_S *pstH264EntropyEnc)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264Poc(VENC_CHN VeChn, const VENC_PARAM_H264_POC_S *pstH264Poc)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264Poc(VENC_CHN VeChn, VENC_PARAM_H264_POC_S *pstH264Poc)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264Dblk(VENC_CHN VeChn, const VENC_PARAM_H264_DBLK_S *pstH264Dblk)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264Dblk(VENC_CHN VeChn, VENC_PARAM_H264_DBLK_S *pstH264Dblk)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_PARAM_H264_VUI_S *pstH264Vui)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264Vui(VENC_CHN VeChn, VENC_PARAM_H264_VUI_S *pstH264Vui)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetJpegParam(VENC_CHN VeChn, const VENC_PARAM_JPEG_S *pstJpegParam)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetJpegParam(VENC_CHN VeChn, VENC_PARAM_JPEG_S *pstJpegParam)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetGrpFrmRate(VENC_GRP VeGroup, const GROUP_FRAME_RATE_S *pstGrpFrmRate)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetGrpFrmRate(VENC_GRP VeGroup, GROUP_FRAME_RATE_S *pstGrpFrmRate)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetRcPara(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcPara)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetRcPara(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcPara)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E enRefMode)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E *penRefMode)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetH264eRefParam(VENC_CHN VeChn, VENC_ATTR_H264_REF_PARAM_S* pstRefParam)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetH264eRefParam(VENC_CHN VeChn, VENC_ATTR_H264_REF_PARAM_S* pstRefParam)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_EnableIDR(VENC_CHN VeChn, bool bEnableIDR)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetColor2GreyConf(const GROUP_COLOR2GREY_CONF_S *pstGrpColor2GreyConf)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetColor2GreyConf(GROUP_COLOR2GREY_CONF_S *pstGrpColor2GreyConf)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetGrpColor2Grey(VENC_GRP VeGroup, const GROUP_COLOR2GREY_S *pstGrpColor2Grey)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetGrpColor2Grey(VENC_GRP VeGroup, GROUP_COLOR2GREY_S *pstGrpColor2Grey)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetGrpCrop(VENC_GRP VeGroup, const GROUP_CROP_CFG_S *pstGrpCropCfg)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetGrpCrop(VENC_GRP VeGroup, GROUP_CROP_CFG_S *pstGrpCropCfg)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_SetJpegSnapMode(VENC_CHN VeChn, VENC_JPEG_SNAP_MODE_E enJpegSnapMode)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_VENC_GetJpegSnapMode(VENC_CHN VeChn, VENC_JPEG_SNAP_MODE_E *penJpegSnapMode)
{
	printf("%s\n", __func__);

	return 0;
}
