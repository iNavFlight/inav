/**
  ******************************************************************************
  * @file    usbd_ccid_smartcard_template.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_ccid_smartcard_template.c file.
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
#ifndef __USBD_CCID_SMARTCARD_TEMPLATE_H
#define __USBD_CCID_SMARTCARD_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#ifndef __USBD_CCID_IF_H
#include "usbd_ccid_if_template.h"
#endif /* __USBD_CCID_IF_H */

/* Exported constants --------------------------------------------------------*/
#define T0_PROTOCOL                    0x00U  /* T0 protocol */
#define T1_PROTOCOL                    0x01U  /* T1 protocol */
#define DIRECT                         0x3BU  /* Direct bit convention */
#define INDIRECT                       0x3FU  /* Indirect bit convention */
#define SETUP_LENGTH                     20U
#define HIST_LENGTH                      20U

#define SC_TRANSMIT_TIMEOUT             200U  /* Direction to transmit */
#define MAX_PROTOCOLLEVEL                 7U  /* Maximum levels of protocol */
#define MAX_INTERFACEBYTE                 4U  /* Maximum number of interface bytes per protocol */
#define LC_MAX                           24U
#define SC_RECEIVE_TIMEOUT           0x8000U  /* Direction to reader */

/* T=1 protocol constants */
#define T1_I_BLOCK                     0x00U  /* PCB (I-block: b8 = 0)  */
#define T1_R_BLOCK                     0x80U  /* PCB (R-block: b8 b7 = 10) */
#define T1_S_BLOCK                     0xC0U  /* PCB (S-block: b8 b7 = 11) */

/* I block */
#define T1_I_SEQ_SHIFT                    6U    /* N(S) position (bit 7) */

/* R block */
#define T1_IS_ERROR(pcb)      ((pcb) & 0x0FU)
#define T1_EDC_ERROR                   0x01U  /* [b6..b1] = 0-N(R)-0001 */
#define T1_OTHER_ERROR                 0x02U  /* [b6..b1] = 0-N(R)-0010 */
#define T1_R_SEQ_SHIFT                    4U  /* N(R) position (b5) */

/* S block  */
#define T1_S_RESPONSE                  0x20U   /* If response: set bit b6, if request reset b6 in PCB S-Block */
#define T1_S_RESYNC                    0x00U   /* RESYNCH: b6->b1: 000000 of PCB S-Block */
#define T1_S_IFS                       0x01U   /* IFS: b6->b1: 000001 of PCB S-Block */
#define T1_S_ABORT                     0x02U   /* ABORT: b6->b1: 000010 of PCB S-Block */
#define T1_S_WTX                       0x03U   /* WTX: b6->b1: 000011 of PCB S-Block */

#define NAD                               0U   /* NAD byte position in the block */
#define PCB                               1U   /* PCB byte position in the block */
#define LEN                               2U   /* LEN byte position in the block */
#define DATA                              3U   /* The position of the first byte of INF field in the block */

/* Modifiable parameters */
#define SAD                             0x0U   /* Source address: reader (allowed values 0 -> 7) */
#define DAD                             0x0U   /* Destination address: card (allowed values 0 -> 7) */
#define IFSD_VALUE                      254U   /* Max length of INF field Supported by the reader */
#define SC_FILE_SIZE                  0x100U   /* File size */
#define SC_FILE_ID                   0x0001U   /* File identifier */
#define SC_CLASS                       0x00U

/* Constant parameters */
#define INS_SELECT_FILE                0xA4U   /* Select file instruction */
#define INS_READ_FILE                  0xB0U   /* Read file instruction */
#define INS_WRITE_FILE                 0xD6U   /* Write file instruction */
#define TRAILER_LENGTH                    2U   /* Trailer length (SW1 and SW2: 2 bytes) */

#define SC_T1_RECEIVE_SUCCESS             0U
#define SC_T1_BWT_TIMEOUT                 1U
#define SC_T1_CWT_TIMEOUT                 2U

#define DEFAULT_FIDI_VALUE             0x11U
#define PPS_REQUEST                    0xFFU

/* SC Tree Structure -----------------------------------------------------------
                              MasterFile
                           ________|___________
                          |        |           |
                        System   UserData     Note
------------------------------------------------------------------------------*/

/* SC ADPU Command: Operation Code -------------------------------------------*/
#define SC_CLA_NAME                   0x00U

/*------------------------ Data Area Management Commands ---------------------*/
#define SC_SELECT_FILE                 0xA4U
#define SC_GET_RESPONCE                0xC0U
#define SC_STATUS                      0xF2U
#define SC_UPDATE_BINARY               0xD6U
#define SC_READ_BINARY                 0xB0U
#define SC_WRITE_BINARY                0xD0U
#define SC_UPDATE_RECORD               0xDCU
#define SC_READ_RECORD                 0xB2U

/*-------------------------- Administrative Commands -------------------------*/
#define SC_CREATE_FILE                 0xE0U

/*-------------------------- Safety Management Commands ----------------------*/
#define SC_VERIFY                      0x20U
#define SC_CHANGE                      0x24U
#define SC_DISABLE                     0x26U
#define SC_ENABLE                      0x28U
#define SC_UNBLOCK                     0x2CU
#define SC_EXTERNAL_AUTH               0x82U
#define SC_GET_CHALLENGE               0x84U

/*-------------------------- Smartcard Interface Byte-------------------------*/
#define SC_INTERFACEBYTE_TA               0U /* Interface byte TA(i) */
#define SC_INTERFACEBYTE_TB               1U /* Interface byte TB(i) */
#define SC_INTERFACEBYTE_TC               2U /* Interface byte TC(i) */
#define SC_INTERFACEBYTE_TD               3U /* Interface byte TD(i) */

/*-------------------------- Answer to reset Commands ------------------------*/
#define SC_GET_A2R                     0x00U

/* SC STATUS: Status Code ----------------------------------------------------*/
#define SC_EF_SELECTED                 0x9FU
#define SC_DF_SELECTED                 0x9FU
#define SC_OP_TERMINATED             0x9000U

/* Smartcard Voltage */
#define SC_VOLTAGE_5V                  0x00U
#define SC_VOLTAGE_3V                  0x01U
#define SC_VOLTAGE_NOINIT              0xFFU
/*----------------- ATR Protocole supported ----------------------------------*/
#define ATR_T01                        0x00U


/* Exported types ------------------------------------------------------------*/
typedef enum
{
  SC_POWER_ON =     0x00,
  SC_RESET_LOW =    0x01,
  SC_RESET_HIGH =   0x02,
  SC_ACTIVE =       0x03,
  SC_ACTIVE_ON_T0 = 0x04,
  SC_ACTIVE_ON_T1 = 0x05,
  SC_POWER_OFF =    0x06,
  SC_NO_INIT =      0x07

} SC_State;

/* Interface Byte structure - TA(i), TB(i), TC(i) and TD(i) ------------------*/
typedef struct
{
  uint8_t Status;               /* The Presence of the Interface byte */
  uint8_t Value;                /* The Value of the Interface byte */
} SC_InterfaceByteTypeDef;

/* Protocol Level structure - ------------------------------------------------*/
typedef struct
{
  SC_InterfaceByteTypeDef InterfaceByte[MAX_INTERFACEBYTE];     /* The Values of the Interface byte
                                                                   TA(i), TB(i), TC(i)and TD(i) */
} SC_ProtocolLevelTypeDef;

/* ATR structure - Answer To Reset -------------------------------------------*/
typedef struct
{
  uint8_t TS;                                   /* Bit Convention Direct/Indirect */
  uint8_t T0;                                   /* Each bit in the high nibble = Presence of the further interface byte;
                                                   Low nibble = Number of historical byte */
  SC_ProtocolLevelTypeDef T[MAX_PROTOCOLLEVEL]; /* Setup array */
  uint8_t Historical[HIST_LENGTH];              /* Historical array */
  uint8_t Tlength;                              /* Setup array dimension */
  uint8_t Hlength;                              /* Historical array dimension */
  uint8_t TCK;
} SC_ATRTypeDef;

/* ADPU-Header command structure ---------------------------------------------*/
typedef struct
{
  uint8_t CLA;  /* Command class */
  uint8_t INS;  /* Operation code */
  uint8_t P1;   /* Selection Mode */
  uint8_t P2;   /* Selection Option */
} SC_HeaderTypeDef;

/* ADPU-Body command structure -----------------------------------------------*/
typedef struct
{
  uint8_t LC;           /* Data field length */
  uint8_t Data[LC_MAX]; /* Command parameters */
  uint8_t LE;           /* Expected length of data to be returned */
} SC_BodyTypeDef;

/* ADPU Command structure ----------------------------------------------------*/
typedef struct
{
  SC_HeaderTypeDef Header;
  SC_BodyTypeDef Body;
} SC_ADPU_CommandsTypeDef;

/* SC response structure -----------------------------------------------------*/
typedef struct
{
  uint8_t Data[LC_MAX];  /* Data returned from the card */
  uint8_t SW1;           /* Command Processing status */
  uint8_t SW2;           /* Command Processing qualification */
} SC_ADPU_ResponseTypeDef;

/* SC Command Status -----------------------------------------------------*/
typedef enum
{
  SC_CS_FAILED          = 0x00,
  SC_CS_PIN_ENABLED     = 0x01,
  SC_CS_PIN_VERIFIED    = 0x02,
  SC_CS_READ            = 0x03,
  SC_CS_PIN_CHANGED     = 0x04

} SC_Command_State;
/* SC Response Status -----------------------------------------------------*/
typedef enum
{
  REP_OK          = 0x00,
  REP_NOT_OK      = 0x01,
  REP_NOT_SUPP    = 0x02,
  REP_ENABLED     = 0x03,
  REP_CHANGE      = 0x04

} REP_Command_t;
/* Conforming of Command with ICC APP -----------------------------------------------------*/
typedef enum
{
  Command_OK          = 0x00,
  Command_NOT_OK      = 0x01,

} Command_State_t;

typedef enum
{
  SC_DISABLED = 0U,
  SC_ENABLED =  !SC_DISABLED
} SCPowerState;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* APPLICATION LAYER ---------------------------------------------------------*/
void SC_Handler(SC_State *SCState, SC_ADPU_CommandsTypeDef *SC_ADPU, SC_ADPU_ResponseTypeDef *SC_Response);
void SC_PowerCmd(SCPowerState NewState);
void SC_ParityErrorHandler(void);
void SC_PTSConfig(void);
uint8_t SC_Detect(void);
uint32_t SC_GetDTableValue(uint32_t idx);
void SC_VoltageConfig(uint32_t SC_Voltage);
void SC_SetState(SC_State scState);
void SC_IOConfig(void);

extern uint8_t SC_ATR_Table[40];
extern SC_ATRTypeDef SC_A2R;
extern SC_ADPU_ResponseTypeDef SC_Response;

extern uint8_t ProtocolNUM_OUT;
extern SC_ADPU_CommandsTypeDef SC_ADPU;

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CCID_SMARTCARD_TEMPLATE_H */
