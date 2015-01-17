/*
 * IMP err header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_ERRNO_H__
#define __IMP_ERRNO_H__

/**
 * @file
 * common errno.
 */

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/**
 * 定义错误级别
 */
typedef enum ENUM_IMP_ERR_LEVEL
{
    EN_ERR_LEVEL_DEBUG = 0,  /**< debug-level */
    EN_ERR_LEVEL_INFO,       /**< informational */
    EN_ERR_LEVEL_NOTICE,     /**< normal but significant condition */
    EN_ERR_LEVEL_WARNING,    /**< warning conditions */
    EN_ERR_LEVEL_ERROR,      /**< error conditions */
    EN_ERR_LEVEL_CRIT,       /**< critical conditions */
    EN_ERR_LEVEL_ALERT,      /**< action must be taken immediately */
    EN_ERR_LEVEL_FATAL,      /**< just for compatibility with previous version */
    EN_ERR_LEVEL_BUTT
}ERR_LEVEL_E;


/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

/**
 * @def IMP_ERR_APPID
 * Ingenic media pattern sdk app id
 */
#define IMP_ERR_APPID  (0)

/**
 * @def IMP_DEF_ERR
 * 定义错误号，格式为：
 * |----------------------------------------------------------------|
 * | 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
 * |----------------------------------------------------------------|
 * |<--><--7bits----><----8bits---><--3bits---><------13bits------->|
 */
#define IMP_DEF_ERR( module, level, errid) \
    ((int)( (IMP_ERR_APPID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

/**
 * 定义通用错误码，所有模组必须保存0~63作为通用错误码。
*/
typedef enum ENUM_IMP_EN_ERR_CODE
{
    EN_ERR_INVALID_DEVID = 1, /**< invlalid device ID */
    EN_ERR_INVALID_CHNID = 2, /**< invlalid channel ID */
    EN_ERR_ILLEGAL_PARAM = 3, /**< at lease one parameter is illagal.
                               * eg, an illegal enumeration value */
    EN_ERR_EXIST         = 4, /**< resource exists */
    EN_ERR_UNEXIST       = 5, /**< resource unexists */

    EN_ERR_NULL_PTR      = 6, /**< using a NULL point */

    EN_ERR_NOT_CONFIG    = 7, /**< try to enable or initialize system, device
                               * or channel, before configing attribute */

    EN_ERR_NOT_SUPPORT   = 8, /**< operation or type is not supported by NOW */
    EN_ERR_NOT_PERM      = 9, /**< operation is not permitted.
                               * eg, try to change static attribute */

    EN_ERR_NOMEM         = 12,/**< failure caused by malloc memory */
    EN_ERR_NOBUF         = 13,/**< failure caused by malloc buffer */

    EN_ERR_BUF_EMPTY     = 14,/**< no data in buffer */
    EN_ERR_BUF_FULL      = 15,/**< no buffer for new data */

    EN_ERR_SYS_NOTREADY  = 16,/**< System is not ready,maybe not initialed or
                               * loaded. Returning the error code when opening
                               * a device file failed. */

    EN_ERR_BADADDR       = 17,/**< bad address.
                               * eg. used for copy_from_user & copy_to_user */

    EN_ERR_BUSY          = 18,/**< resource is busy.
                               * eg. destroy a venc chn without unregister it */

    EN_ERR_BUTT          = 63,/**< maxium code, private error code of all modules
                               * must be greater than it */
}EN_ERR_CODE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __IMP_ERRNO_H__ */
