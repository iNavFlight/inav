/**
  ******************************************************************************
  * @file    usbd_mtp_opt.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_mtp_opt.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_MTP_OPT_H__
#define __USBD_MTP_OPT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#ifndef __USBD_MTP_IF_H
#include "usbd_mtp_if_template.h"
#endif /* __USBD_MTP_IF_H */
#include "usbd_mtp.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MTP_OPT
  * @brief This file is the header file for usbd_mtp_opt.c
  * @{
  */


/** @defgroup USBD_MTP_OPT_Exported_Defines
  * @{
  */

/*
 * MTP Class specification Revision 1.1
 * Appendix B. Object Properties
 */

/* MTP OBJECT PROPERTIES supported*/
#define    MTP_OB_PROP_STORAGE_ID                               0xDC01U
#define    MTP_OB_PROP_OBJECT_FORMAT                            0xDC02U
#define    MTP_OB_PROP_PROTECTION_STATUS                        0xDC03U
#define    MTP_OB_PROP_OBJECT_SIZE                              0xDC04U
#define    MTP_OB_PROP_ASSOC_TYPE                               0xDC05U
#define    MTP_OB_PROP_ASSOC_DESC                               0xDC06U
#define    MTP_OB_PROP_OBJ_FILE_NAME                            0xDC07U
#define    MTP_OB_PROP_DATE_CREATED                             0xDC08U
#define    MTP_OB_PROP_DATE_MODIFIED                            0xDC09U
#define    MTP_OB_PROP_KEYWORDS                                 0xDC0AU
#define    MTP_OB_PROP_PARENT_OBJECT                            0xDC0BU
#define    MTP_OB_PROP_ALLOWED_FOLD_CONTENTS                    0xDC0CU
#define    MTP_OB_PROP_HIDDEN                                   0xDC0DU
#define    MTP_OB_PROP_SYSTEM_OBJECT                            0xDC0EU
#define    MTP_OB_PROP_PERS_UNIQ_OBJ_IDEN                       0xDC41U
#define    MTP_OB_PROP_SYNCID                                   0xDC42U
#define    MTP_OB_PROP_PROPERTY_BAG                             0xDC43U
#define    MTP_OB_PROP_NAME                                     0xDC44U
#define    MTP_OB_PROP_CREATED_BY                               0xDC45U
#define    MTP_OB_PROP_ARTIST                                   0xDC46U
#define    MTP_OB_PROP_DATE_AUTHORED                            0xDC47U
#define    MTP_OB_PROP_DESCRIPTION                              0xDC48U
#define    MTP_OB_PROP_URL_REFERENCE                            0xDC49U
#define    MTP_OB_PROP_LANGUAGELOCALE                           0xDC4AU
#define    MTP_OB_PROP_COPYRIGHT_INFORMATION                    0xDC4BU
#define    MTP_OB_PROP_SOURCE                                   0xDC4CU
#define    MTP_OB_PROP_ORIGIN_LOCATION                          0xDC4DU
#define    MTP_OB_PROP_DATE_ADDED                               0xDC4EU
#define    MTP_OB_PROP_NON_CONSUMABLE                           0xDC4FU
#define    MTP_OB_PROP_CORRUPTUNPLAYABLE                        0xDC50U
#define    MTP_OB_PROP_PRODUCERSERIALNUMBER                     0xDC51U
#define    MTP_OB_PROP_REPRESENTATIVE_SAMPLE_FORMAT             0xDC81U
#define    MTP_OB_PROP_REPRESENTATIVE_SAMPLE_SIZE               0xDC82U
#define    MTP_OB_PROP_REPRESENTATIVE_SAMPLE_HEIGHT             0xDC83U
#define    MTP_OB_PROP_REPRESENTATIVE_SAMPLE_WIDTH              0xDC84U
#define    MTP_OB_PROP_REPRESENTATIVE_SAMPLE_DURATION           0xDC85U
#define    MTP_OB_PROP_REPRESENTATIVE_SAMPLE_DATA               0xDC86U
#define    MTP_OB_PROP_WIDTH                                    0xDC87U
#define    MTP_OB_PROP_HEIGHT                                   0xDC88U
#define    MTP_OB_PROP_DURATION                                 0xDC89U
#define    MTP_OB_PROP_RATING                                   0xDC8AU
#define    MTP_OB_PROP_TRACK                                    0xDC8BU
#define    MTP_OB_PROP_GENRE                                    0xDC8CU
#define    MTP_OB_PROP_CREDITS                                  0xDC8DU
#define    MTP_OB_PROP_LYRICS                                   0xDC8EU
#define    MTP_OB_PROP_SUBSCRIPTION_CONTENT_ID                  0xDC8FU
#define    MTP_OB_PROP_PRODUCED_BY                              0xDC90U
#define    MTP_OB_PROP_USE_COUNT                                0xDC91U
#define    MTP_OB_PROP_SKIP_COUNT                               0xDC92U
#define    MTP_OB_PROP_LAST_ACCESSED                            0xDC93U
#define    MTP_OB_PROP_PARENTAL_RATING                          0xDC94U
#define    MTP_OB_PROP_META_GENRE                               0xDC95U
#define    MTP_OB_PROP_COMPOSER                                 0xDC96U
#define    MTP_OB_PROP_EFFECTIVE_RATING                         0xDC97U
#define    MTP_OB_PROP_SUBTITLE                                 0xDC98U
#define    MTP_OB_PROP_ORIGINAL_RELEASE_DATE                    0xDC99U
#define    MTP_OB_PROP_ALBUM_NAME                               0xDC9AU
#define    MTP_OB_PROP_ALBUM_ARTIST                             0xDC9BU
#define    MTP_OB_PROP_MOOD                                     0xDC9CU
#define    MTP_OB_PROP_DRM_STATUS                               0xDC9DU
#define    MTP_OB_PROP_SUB_DESCRIPTION                          0xDC9EU
#define    MTP_OB_PROP_IS_CROPPED                               0xDCD1U
#define    MTP_OB_PROP_IS_COLOUR_CORRECTED                      0xDCD2U
#define    MTP_OB_PROP_IMAGE_BIT_DEPTH                          0xDCD3U
#define    MTP_OB_PROP_FNUMBER                                  0xDCD4U
#define    MTP_OB_PROP_EXPOSURE_TIME                            0xDCD5U
#define    MTP_OB_PROP_EXPOSURE_INDEX                           0xDCD6U
#define    MTP_OB_PROP_TOTAL_BITRATE                            0xDE91U
#define    MTP_OB_PROP_BITRATE_TYPE                             0xDE92U
#define    MTP_OB_PROP_SAMPLE_RATE                              0xDE93U
#define    MTP_OB_PROP_NUMBER_OF_CHANNELS                       0xDE94U
#define    MTP_OB_PROP_AUDIO_BITDEPTH                           0xDE95U
#define    MTP_OB_PROP_SCAN_TYPE                                0xDE97U
#define    MTP_OB_PROP_AUDIO_WAVE_CODEC                         0xDE99U
#define    MTP_OB_PROP_AUDIO_BITRATE                            0xDE9AU
#define    MTP_OB_PROP_VIDEO_FOURCC_CODEC                       0xDE9BU
#define    MTP_OB_PROP_VIDEO_BITRATE                            0xDE9CU
#define    MTP_OB_PROP_FRAMES_PER_THOUSAND_SECONDS              0xDE9DU
#define    MTP_OB_PROP_KEYFRAME_DISTANCE                        0xDE9EU
#define    MTP_OB_PROP_BUFFER_SIZE                              0xDE9FU
#define    MTP_OB_PROP_ENCODING_QUALITY                         0xDEA0U
#define    MTP_OB_PROP_ENCODING_PROFILE                         0xDEA1U
#define    MTP_OB_PROP_DISPLAY_NAME                             0xDCE0U
#define    MTP_OB_PROP_BODY_TEXT                                0xDCE1U
#define    MTP_OB_PROP_SUBJECT                                  0xDCE2U
#define    MTP_OB_PROP_PRIORITY                                 0xDCE3U
#define    MTP_OB_PROP_GIVEN_NAME                               0xDD00U
#define    MTP_OB_PROP_MIDDLE_NAMES                             0xDD01U
#define    MTP_OB_PROP_FAMILY_NAME                              0xDD02U
#define    MTP_OB_PROP_PREFIX                                   0xDD03U
#define    MTP_OB_PROP_SUFFIX                                   0xDD04U
#define    MTP_OB_PROP_PHONETIC_GIVEN_NAME                      0xDD05U
#define    MTP_OB_PROP_PHONETIC_FAMILY_NAME                     0xDD06U
#define    MTP_OB_PROP_EMAIL_PRIMARY                            0xDD07U
#define    MTP_OB_PROP_EMAIL_PERSONAL_1                         0xDD08U
#define    MTP_OB_PROP_EMAIL_PERSONAL_2                         0xDD09U
#define    MTP_OB_PROP_EMAIL_BUSINESS_1                         0xDD0AU
#define    MTP_OB_PROP_EMAIL_BUSINESS_2                         0xDD0BU
#define    MTP_OB_PROP_EMAIL_OTHERS                             0xDD0CU
#define    MTP_OB_PROP_PHONE_NUMBER_PRIMARY                     0xDD0DU
#define    MTP_OB_PROP_PHONE_NUMBER_PERSONAL                    0xDD0EU
#define    MTP_OB_PROP_PHONE_NUMBER_PERSONAL_2                  0xDD0FU
#define    MTP_OB_PROP_PHONE_NUMBER_BUSINESS                    0xDD10U
#define    MTP_OB_PROP_PHONE_NUMBER_BUSINESS_2                  0xDD11U
#define    MTP_OB_PROP_PHONE_NUMBER_MOBILE                      0xDD12U
#define    MTP_OB_PROP_PHONE_NUMBER_MOBILE_2                    0xDD13U
#define    MTP_OB_PROP_FAX_NUMBER_PRIMARY                       0xDD14U
#define    MTP_OB_PROP_FAX_NUMBER_PERSONAL                      0xDD15U
#define    MTP_OB_PROP_FAX_NUMBER_BUSINESS                      0xDD16U
#define    MTP_OB_PROP_PAGER_NUMBER                             0xDD17U
#define    MTP_OB_PROP_PHONE_NUMBER_OTHERS                      0xDD18U
#define    MTP_OB_PROP_PRIMARY_WEB_ADDRESS                      0xDD19U
#define    MTP_OB_PROP_PERSONAL_WEB_ADDRESS                     0xDD1AU
#define    MTP_OB_PROP_BUSINESS_WEB_ADDRESS                     0xDD1BU
#define    MTP_OB_PROP_INSTANT_MESSENGER_ADDRESS                0xDD1CU
#define    MTP_OB_PROP_INSTANT_MESSENGER_ADDRESS_2              0xDD1DU
#define    MTP_OB_PROP_INSTANT_MESSENGER_ADDRESS_3              0xDD1EU
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_FULL             0xDD1FU
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_LINE_1           0xDD20U
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_LINE_2           0xDD21U
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_CITY             0xDD22U
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_REGION           0xDD23U
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_POSTAL_CODE      0xDD24U
#define    MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_COUNTRY          0xDD25U
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_FULL             0xDD26U
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_LINE_1           0xDD27U
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_LINE_2           0xDD28U
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_CITY             0xDD29U
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_REGION           0xDD2AU
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_POSTAL_CODE      0xDD2BU
#define    MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_COUNTRY          0xDD2CU
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_FULL                0xDD2DU
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_LINE_1              0xDD2EU
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_LINE_2              0xDD2FU
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_CITY                0xDD30U
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_REGION              0xDD31U
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_POSTAL_CODE         0xDD32U
#define    MTP_OB_PROP_POSTAL_ADDRESS_OTHER_COUNTRY             0xDD33U
#define    MTP_OB_PROP_ORGANIZATION_NAME                        0xDD34U
#define    MTP_OB_PROP_PHONETIC_ORGANIZATION_NAME               0xDD35U
#define    MTP_OB_PROP_ROLE                                     0xDD36U
#define    MTP_OB_PROP_BIRTHDATE                                0xDD37U
#define    MTP_OB_PROP_MESSAGE_TO                               0xDD40U
#define    MTP_OB_PROP_MESSAGE_CC                               0xDD41U
#define    MTP_OB_PROP_MESSAGE_BCC                              0xDD42U
#define    MTP_OB_PROP_MESSAGE_READ                             0xDD43U
#define    MTP_OB_PROP_MESSAGE_RECEIVED_TIME                    0xDD44U
#define    MTP_OB_PROP_MESSAGE_SENDER                           0xDD45U
#define    MTP_OB_PROP_ACT_BEGIN_TIME                           0xDD50U
#define    MTP_OB_PROP_ACT_END_TIME                             0xDD51U
#define    MTP_OB_PROP_ACT_LOCATION                             0xDD52U
#define    MTP_OB_PROP_ACT_REQUIRED_ATTENDEES                   0xDD54U
#define    MTP_OB_PROP_ACT_OPTIONAL_ATTENDEES                   0xDD55U
#define    MTP_OB_PROP_ACT_RESOURCES                            0xDD56U
#define    MTP_OB_PROP_ACT_ACCEPTED                             0xDD57U
#define    MTP_OB_PROP_OWNER                                    0xDD5DU
#define    MTP_OB_PROP_EDITOR                                   0xDD5EU
#define    MTP_OB_PROP_WEBMASTER                                0xDD5FU
#define    MTP_OB_PROP_URL_SOURCE                               0xDD60U
#define    MTP_OB_PROP_URL_DESTINATION                          0xDD61U
#define    MTP_OB_PROP_TIME_BOOKMARK                            0xDD62U
#define    MTP_OB_PROP_OBJECT_BOOKMARK                          0xDD63U
#define    MTP_OB_PROP_BYTE_BOOKMARK                            0xDD64U
#define    MTP_OB_PROP_LAST_BUILD_DATE                          0xDD70U
#define    MTP_OB_PROP_TIME_TO_LIVE                             0xDD71U
#define    MTP_OB_PROP_MEDIA_GUID                               0xDD72U

/*  MTP event codes*/
#define      MTP_EVENT_UNDEFINED                                0x4000U
#define      MTP_EVENT_CANCELTRANSACTION                        0x4001U
#define      MTP_EVENT_OBJECTADDED                              0x4002U
#define      MTP_EVENT_OBJECTREMOVED                            0x4003U
#define      MTP_EVENT_STOREADDED                               0x4004U
#define      MTP_EVENT_STOREREMOVED                             0x4005U
#define      MTP_EVENT_DEVICEPROPCHANGED                        0x4006U
#define      MTP_EVENT_OBJECTINFOCHANGED                        0x4007U
#define      MTP_EVENT_DEVICEINFOCHANGED                        0x4008U
#define      MTP_EVENT_REQUESTOBJECTTRANSFER                    0x4009U
#define      MTP_EVENT_STOREFULL                                0x400AU
#define      MTP_EVENT_DEVICERESET                              0x400BU
#define      MTP_EVENT_STORAGEINFOCHANGED                       0x400CU
#define      MTP_EVENT_CAPTURECOMPLETE                          0x400DU
#define      MTP_EVENT_UNREPORTEDSTATUS                         0x400EU
#define      MTP_EVENT_OBJECTPROPCHANGED                        0xC801U
#define      MTP_EVENT_OBJECTPROPDESCCHANGED                    0xC802U
#define      MTP_EVENT_OBJECTREFERENCESCHANGED                  0xC803U

/*
 * MTP Class specification Revision 1.1
 * Appendix D. Operations
 */

/* Operations code */
#define      MTP_OP_GET_DEVICE_INFO                             0x1001U
#define      MTP_OP_OPEN_SESSION                                0x1002U
#define      MTP_OP_CLOSE_SESSION                               0x1003U
#define      MTP_OP_GET_STORAGE_IDS                             0x1004U
#define      MTP_OP_GET_STORAGE_INFO                            0x1005U
#define      MTP_OP_GET_NUM_OBJECTS                             0x1006U
#define      MTP_OP_GET_OBJECT_HANDLES                          0x1007U
#define      MTP_OP_GET_OBJECT_INFO                             0x1008U
#define      MTP_OP_GET_OBJECT                                  0x1009U
#define      MTP_OP_GET_THUMB                                   0x100AU
#define      MTP_OP_DELETE_OBJECT                               0x100BU
#define      MTP_OP_SEND_OBJECT_INFO                            0x100CU
#define      MTP_OP_SEND_OBJECT                                 0x100DU
#define      MTP_OP_FORMAT_STORE                                0x100FU
#define      MTP_OP_RESET_DEVICE                                0x1010U
#define      MTP_OP_GET_DEVICE_PROP_DESC                        0x1014U
#define      MTP_OP_GET_DEVICE_PROP_VALUE                       0x1015U
#define      MTP_OP_SET_DEVICE_PROP_VALUE                       0x1016U
#define      MTP_OP_RESET_DEVICE_PROP_VALUE                     0x1017U
#define      MTP_OP_TERMINATE_OPEN_CAPTURE                      0x1018U
#define      MTP_OP_MOVE_OBJECT                                 0x1019U
#define      MTP_OP_COPY_OBJECT                                 0x101AU
#define      MTP_OP_GET_PARTIAL_OBJECT                          0x101BU
#define      MTP_OP_INITIATE_OPEN_CAPTURE                       0x101CU
#define      MTP_OP_GET_OBJECT_PROPS_SUPPORTED                  0x9801U
#define      MTP_OP_GET_OBJECT_PROP_DESC                        0x9802U
#define      MTP_OP_GET_OBJECT_PROP_VALUE                       0x9803U
#define      MTP_OP_SET_OBJECT_PROP_VALUE                       0x9804U
#define      MTP_OP_GET_OBJECT_PROPLIST                         0x9805U
#define      MTP_OP_GET_OBJECT_PROP_REFERENCES                  0x9810U
#define      MTP_OP_GETSERVICEIDS                               0x9301U
#define      MTP_OP_GETSERVICEINFO                              0x9302U
#define      MTP_OP_GETSERVICECAPABILITIES                      0x9303U
#define      MTP_OP_GETSERVICEPROPDESC                          0x9304U

/*
 * MTP Class specification Revision 1.1
 * Appendix C. Device Properties
 */

/* MTP device properties code*/
#define    MTP_DEV_PROP_UNDEFINED                               0x5000U
#define    MTP_DEV_PROP_BATTERY_LEVEL                           0x5001U
#define    MTP_DEV_PROP_FUNCTIONAL_MODE                         0x5002U
#define    MTP_DEV_PROP_IMAGE_SIZE                              0x5003U
#define    MTP_DEV_PROP_COMPRESSION_SETTING                     0x5004U
#define    MTP_DEV_PROP_WHITE_BALANCE                           0x5005U
#define    MTP_DEV_PROP_RGB_GAIN                                0x5006U
#define    MTP_DEV_PROP_F_NUMBER                                0x5007U
#define    MTP_DEV_PROP_FOCAL_LENGTH                            0x5008U
#define    MTP_DEV_PROP_FOCUS_DISTANCE                          0x5009U
#define    MTP_DEV_PROP_FOCUS_MODE                              0x500AU
#define    MTP_DEV_PROP_EXPOSURE_METERING_MODE                  0x500BU
#define    MTP_DEV_PROP_FLASH_MODE                              0x500CU
#define    MTP_DEV_PROP_EXPOSURE_TIME                           0x500DU
#define    MTP_DEV_PROP_EXPOSURE_PROGRAM_MODE                   0x500EU
#define    MTP_DEV_PROP_EXPOSURE_INDEX                          0x500FU
#define    MTP_DEV_PROP_EXPOSURE_BIAS_COMPENSATION              0x5010U
#define    MTP_DEV_PROP_DATETIME                                0x5011U
#define    MTP_DEV_PROP_CAPTURE_DELAY                           0x5012U
#define    MTP_DEV_PROP_STILL_CAPTURE_MODE                      0x5013U
#define    MTP_DEV_PROP_CONTRAST                                0x5014U
#define    MTP_DEV_PROP_SHARPNESS                               0x5015U
#define    MTP_DEV_PROP_DIGITAL_ZOOM                            0x5016U
#define    MTP_DEV_PROP_EFFECT_MODE                             0x5017U
#define    MTP_DEV_PROP_BURST_NUMBER                            0x5018U
#define    MTP_DEV_PROP_BURST_INTERVAL                          0x5019U
#define    MTP_DEV_PROP_TIMELAPSE_NUMBER                        0x501AU
#define    MTP_DEV_PROP_TIMELAPSE_INTERVAL                      0x501BU
#define    MTP_DEV_PROP_FOCUS_METERING_MODE                     0x501CU
#define    MTP_DEV_PROP_UPLOAD_URL                              0x501DU
#define    MTP_DEV_PROP_ARTIST                                  0x501EU
#define    MTP_DEV_PROP_COPYRIGHT_INFO                          0x501FU
#define    MTP_DEV_PROP_SYNCHRONIZATION_PARTNER                 0xD401U
#define    MTP_DEV_PROP_DEVICE_FRIENDLY_NAME                    0xD402U
#define    MTP_DEV_PROP_VOLUME                                  0xD403U
#define    MTP_DEV_PROP_SUPPORTEDFORMATSORDERED                 0xD404U
#define    MTP_DEV_PROP_DEVICEICON                              0xD405U
#define    MTP_DEV_PROP_PLAYBACK_RATE                           0xD410U
#define    MTP_DEV_PROP_PLAYBACK_OBJECT                         0xD411U
#define    MTP_DEV_PROP_PLAYBACK_CONTAINER                      0xD412U
#define    MTP_DEV_PROP_SESSION_INITIATOR_VERSION_INFO          0xD406U
#define    MTP_DEV_PROP_PERCEIVED_DEVICE_TYPE                   0xD407U


/* Container Types */
#define    MTP_CONT_TYPE_UNDEFINED                              0U
#define    MTP_CONT_TYPE_COMMAND                                1U
#define    MTP_CONT_TYPE_DATA                                   2U
#define    MTP_CONT_TYPE_RESPONSE                               3U
#define    MTP_CONT_TYPE_EVENT                                  4U

#ifndef    MTP_STORAGE_ID
#define    MTP_STORAGE_ID                             0x00010001U   /* SD card is inserted*/
#endif  /* MTP_STORAGE_ID */

#define    MTP_NBR_STORAGE_ID                                   1U
#define    FREE_SPACE_IN_OBJ_NOT_USED                           0xFFFFFFFFU

/* MTP storage type */
#define    MTP_STORAGE_UNDEFINED                                0U
#define    MTP_STORAGE_FIXED_ROM                                0x0001U
#define    MTP_STORAGE_REMOVABLE_ROM                            0x0002U
#define    MTP_STORAGE_FIXED_RAM                                0x0003U
#define    MTP_STORAGE_REMOVABLE_RAM                            0x0004U

/* MTP file system type */
#define    MTP_FILESYSTEM_UNDEFINED                             0U
#define    MTP_FILESYSTEM_GENERIC_FLAT                          0x0001U
#define    MTP_FILESYSTEM_GENERIC_HIERARCH                      0x0002U
#define    MTP_FILESYSTEM_DCF                                   0x0003U

/* MTP access capability */
#define    MTP_ACCESS_CAP_RW                                    0U /* read write */
#define    MTP_ACCESS_CAP_RO_WITHOUT_DEL                        0x0001U
#define    MTP_ACCESS_CAP_RO_WITH_DEL                           0x0002U

/* MTP standard data types supported */
#define    MTP_DATATYPE_INT8                                    0x0001U
#define    MTP_DATATYPE_UINT8                                   0x0002U
#define    MTP_DATATYPE_INT16                                   0x0003U
#define    MTP_DATATYPE_UINT16                                  0x0004U
#define    MTP_DATATYPE_INT32                                   0x0005U
#define    MTP_DATATYPE_UINT32                                  0x0006U
#define    MTP_DATATYPE_INT64                                   0x0007U
#define    MTP_DATATYPE_UINT64                                  0x0008U
#define    MTP_DATATYPE_UINT128                                 0x000AU
#define    MTP_DATATYPE_STR                                     0xFFFFU

/* MTP reading only or reading/writing */
#define    MTP_PROP_GET                                         0x00U
#define    MTP_PROP_GET_SET                                     0x01U


/* MTP functional mode */
#define    STANDARD_MODE                                        0U
#define    SLEEP_STATE                                          1U
#define    FUNCTIONAL_MODE                                      STANDARD_MODE

/* MTP device info */
#define    STANDARD_VERSION                                     100U
#define    VEND_EXT_ID                                          0x06U
#define    VEND_EXT_VERSION                                     100U
#define    MANUF_LEN                                            (sizeof(Manuf) / 2U)
#define    MODEL_LEN                                            (sizeof(Model) / 2U)
#define    SUPP_OP_LEN                                          (sizeof(SuppOP) / 2U)
#define    SERIAL_NBR_LEN                                       (sizeof(SerialNbr) / 2U)
#define    DEVICE_VERSION_LEN                                   (sizeof(DeviceVers) / 2U)
#define    SUPP_IMG_FORMAT_LEN                                  (sizeof(SuppImgFormat) / 2U)
#define    SUPP_OBJ_PROP_LEN                                    (sizeof(ObjectPropSupp) / 2U)

#ifndef    MAX_FILE_NAME
#define    MAX_FILE_NAME                                        255U
#endif    /* MAX_FILE_NAME */

#ifndef    MAX_OBJECT_HANDLE_LEN
#define    MAX_OBJECT_HANDLE_LEN                                100U
#endif    /* MAX_OBJECT_HANDLE_LEN */

#ifndef    DEVICE_PROP_DESC_DEF_LEN
#define    DEVICE_PROP_DESC_DEF_LEN                            (uint8_t)(sizeof(DevicePropDefVal) / 2U)
#endif    /* DEVICE_PROP_DESC_DEF_LEN */

#ifndef   DEVICE_PROP_DESC_CUR_LEN
#define   DEVICE_PROP_DESC_CUR_LEN                             (uint8_t)(sizeof(DevicePropCurDefVal) / 2U)
#endif   /* DEVICE_PROP_DESC_CUR_LEN */

#ifndef   DEFAULT_FILE_NAME_LEN
#define   DEFAULT_FILE_NAME_LEN                                (uint8_t)(sizeof(DefaultFileName) / 2U)
#endif   /* DEFAULT_FILE_NAME_LEN */

/**
  * @}
  */


/** @defgroup USBD_MTP_OPT_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */

static const uint16_t SuppOP[] = { MTP_OP_GET_DEVICE_INFO, MTP_OP_OPEN_SESSION, MTP_OP_CLOSE_SESSION,
                                   MTP_OP_GET_STORAGE_IDS, MTP_OP_GET_STORAGE_INFO, MTP_OP_GET_NUM_OBJECTS,
                                   MTP_OP_GET_OBJECT_HANDLES, MTP_OP_GET_OBJECT_INFO, MTP_OP_GET_OBJECT,
                                   MTP_OP_DELETE_OBJECT, MTP_OP_SEND_OBJECT_INFO, MTP_OP_SEND_OBJECT,
                                   MTP_OP_GET_DEVICE_PROP_DESC, MTP_OP_GET_DEVICE_PROP_VALUE,
                                   MTP_OP_SET_OBJECT_PROP_VALUE, MTP_OP_GET_OBJECT_PROP_VALUE,
                                   MTP_OP_GET_OBJECT_PROPS_SUPPORTED, MTP_OP_GET_OBJECT_PROPLIST,
                                   MTP_OP_GET_OBJECT_PROP_DESC, MTP_OP_GET_OBJECT_PROP_REFERENCES
                                 };

static const uint16_t SuppEvents[] = {MTP_EVENT_OBJECTADDED};
static const uint16_t SuppImgFormat[] = {MTP_OBJ_FORMAT_UNDEFINED, MTP_OBJ_FORMAT_TEXT, MTP_OBJ_FORMAT_ASSOCIATION,
                                         MTP_OBJ_FORMAT_EXECUTABLE, MTP_OBJ_FORMAT_WAV, MTP_OBJ_FORMAT_MP3,
                                         MTP_OBJ_FORMAT_EXIF_JPEG, MTP_OBJ_FORMAT_MPEG, MTP_OBJ_FORMAT_MP4_CONTAINER,
                                         MTP_OBJ_FORMAT_WINDOWS_IMAGE_FORMAT, MTP_OBJ_FORMAT_PNG, MTP_OBJ_FORMAT_WMA,
                                         MTP_OBJ_FORMAT_WMV
                                        };

static const uint16_t SuppCaptFormat[] = {MTP_OBJ_FORMAT_UNDEFINED, MTP_OBJ_FORMAT_ASSOCIATION, MTP_OBJ_FORMAT_TEXT};

/* required for all object format : storageID, objectFormat, ObjectCompressedSize,
persistent unique object identifier, name*/
static const uint16_t ObjectPropSupp[] = {MTP_OB_PROP_STORAGE_ID, MTP_OB_PROP_OBJECT_FORMAT, MTP_OB_PROP_OBJECT_SIZE,
                                          MTP_OB_PROP_OBJ_FILE_NAME, MTP_OB_PROP_PARENT_OBJECT, MTP_OB_PROP_NAME,
                                          MTP_OB_PROP_PERS_UNIQ_OBJ_IDEN, MTP_OB_PROP_PROTECTION_STATUS
                                         };

static const uint16_t DevicePropSupp[] = {MTP_DEV_PROP_DEVICE_FRIENDLY_NAME, MTP_DEV_PROP_BATTERY_LEVEL};

/* for all mtp struct */
typedef struct
{
  uint32_t StorageIDS_len;
  uint32_t StorageIDS[MTP_NBR_STORAGE_ID];
} MTP_StorageIDSTypeDef;

#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint8_t FileName_len;
  uint16_t FileName[MAX_FILE_NAME];
} MTP_FileNameTypeDef;


typedef struct
{
  uint32_t ObjectHandle_len;
  uint32_t ObjectHandle[MAX_OBJECT_HANDLE_LEN];
} MTP_ObjectHandleTypeDef;

#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint32_t ObjectPropSupp_len;
  uint16_t ObjectPropSupp[SUPP_OBJ_PROP_LEN];
} MTP_ObjectPropSuppTypeDef;


#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint16_t StorageType;
  uint16_t FilesystemType;
  uint16_t AccessCapability;
  uint64_t MaxCapability;
  uint64_t FreeSpaceInBytes;
  uint32_t FreeSpaceInObjects;
  uint8_t  StorageDescription;
  uint8_t  VolumeLabel;
} MTP_StorageInfoTypedef;

typedef union
{
  uint16_t  str[100];
  uint8_t u8;
  int8_t i8;
  uint16_t u16;
  int16_t i16;
  uint32_t u32;
  int32_t i32;
  uint64_t u64;
  int64_t i64;
} MTP_PropertyValueTypedef;

#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint16_t    ObjectPropertyCode;
  uint16_t    DataType;
  uint8_t     GetSet;
  uint8_t    *DefValue;
  uint32_t    GroupCode;
  uint8_t     FormFlag;
} MTP_ObjectPropDescTypeDef;


#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint32_t   ObjectHandle;
  uint16_t   PropertyCode;
  uint16_t   Datatype;
  uint8_t   *propval;
} MTP_PropertiesTypedef;

#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint32_t MTP_Properties_len;
  MTP_PropertiesTypedef MTP_Properties[SUPP_OBJ_PROP_LEN];
} MTP_PropertiesListTypedef;


#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint32_t ref_len;
  uint32_t ref[1];
} MTP_RefTypeDef;

#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint16_t     DevicePropertyCode;
  uint16_t     DataType;
  uint8_t      GetSet;
  uint8_t      DefValue_len;
  uint16_t     DefValue[DEVICE_PROP_DESC_DEF_LEN];
  uint8_t      curDefValue_len;
  uint16_t     curDefValue[DEVICE_PROP_DESC_CUR_LEN];
  uint8_t      FormFlag;
} MTP_DevicePropDescTypeDef;

/* MTP device info structure */
#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint16_t       StandardVersion;
  uint32_t       VendorExtensionID;
  uint16_t       VendorExtensionVersion;
  uint8_t        VendorExtensionDesc_len;
#if USBD_MTP_VEND_EXT_DESC_SUPPORTED == 1
  uint16_t       VendorExtensionDesc[VEND_EXT_DESC_LEN];
#endif /* USBD_MTP_VEND_EXT_DESC_SUPPORTED */
  uint16_t       FunctionalMode;
  uint32_t       OperationsSupported_len;
  uint16_t       OperationsSupported[SUPP_OP_LEN];
  uint32_t       EventsSupported_len;
#if USBD_MTP_EVENTS_SUPPORTED == 1
  uint16_t       EventsSupported[SUPP_EVENTS_LEN];
#endif /* USBD_MTP_EVENTS_SUPPORTED */
  uint32_t       DevicePropertiesSupported_len;
#if USBD_MTP_DEVICE_PROP_SUPPORTED == 1
  uint16_t       DevicePropertiesSupported[SUPP_DEVICE_PROP_LEN];
#endif /* USBD_MTP_DEVICE_PROP_SUPPORTED */
  uint32_t       CaptureFormats_len;
#if USBD_MTP_CAPTURE_FORMAT_SUPPORTED == 1
  uint16_t       CaptureFormats[SUPP_CAPT_FORMAT_LEN];
#endif /* USBD_MTP_CAPTURE_FORMAT_SUPPORTED */
  uint32_t       ImageFormats_len;
  uint16_t       ImageFormats[SUPP_IMG_FORMAT_LEN];
  uint8_t        Manufacturer_len;
  uint16_t       Manufacturer[MANUF_LEN];
  uint8_t        Model_len;
  uint16_t       Model[MODEL_LEN];
  uint8_t        DeviceVersion_len;
  uint16_t       DeviceVersion[DEVICE_VERSION_LEN];
  uint8_t        SerialNumber_len;
  uint16_t       SerialNumber[SERIAL_NBR_LEN];
} MTP_DeviceInfoTypedef;

/** @defgroup USBD_MTP_OPT_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_OPT_Exported_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_OPT_Exported_Functions
  * @{
  */

void USBD_MTP_OPT_CreateObjectHandle(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetDeviceInfo(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetStorageIDS(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetStorageInfo(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectHandle(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectInfo(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectReferences(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectPropSupp(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectPropDesc(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectPropValue(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetObjectPropList(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_GetDevicePropDesc(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_SendObjectInfo(USBD_HandleTypeDef  *pdev, uint8_t *buff, uint32_t len);
void USBD_MTP_OPT_SendObject(USBD_HandleTypeDef  *pdev, uint8_t *buff, uint32_t len);
void USBD_MTP_OPT_GetObject(USBD_HandleTypeDef  *pdev);
void USBD_MTP_OPT_DeleteObject(USBD_HandleTypeDef  *pdev);


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_MTP_OPT_H__ */
/**
  * @}
  */

/**
  * @}
  */
