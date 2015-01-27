/*
 * IMP venc func source file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#include <stdio.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>

int IMP_Encoder_CreateGroup(ENC_GRP VeGroup)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_DestroyGroup(ENC_GRP VeGroup)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_CreateChn(ENC_CHN VeChn, const IMPEncoderCHNAttrSt *pstAttr)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_DestroyChn(ENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_RegisterChn(ENC_GRP VeGroup, ENC_CHN VeChn )
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_UnRegisterChn(ENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_StartRecvPicEx(ENC_CHN VeChn, IMPEncoderRecvPicParamSt *pstRecvParam)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_StopRecvPic(ENC_CHN VeChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_GetStream(ENC_CHN VeChn, IMPEncoderStreamSt *pstStream, bool bBlockFlag)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_ReleaseStream(ENC_CHN VeChn, IMPEncoderStreamSt *pstStream)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_EnableIDR(ENC_CHN VeChn, bool bEnableIDR)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_SetJpegSnapMode(ENC_CHN VeChn, IMPEncoderJpegSnapModeEn enJpegSnapMode)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_GetJpegSnapMode(ENC_CHN VeChn, IMPEncoderJpegSnapModeEn *penJpegSnapMode)
{
	printf("%s\n", __func__);

	return 0;
}
