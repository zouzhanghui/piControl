//+=============================================================================================
//|
//!             \file ModGateComMain.h
//!             Communication layers of mGate
//|
//+---------------------------------------------------------------------------------------------
//|
//|             File-ID:                $Id: ModGateComMain.h 11009 2016-10-20 07:57:15Z mduckeck $
//|             Location:               $URL: http://srv-kunbus03.de.pilz.local/feldbus/software/trunk/platform/ModGateCom/sw/ModGateComMain.h $
//|             Company:                $Cpn: KUNBUS GmbH $
//|
//+=============================================================================================

#ifndef MODGATECOMMAIN_H_INC
#define MODGATECOMMAIN_H_INC

#if defined(__KUNBUSPI__) || defined(__KUNBUSPI_KERNEL__)
#define MODGATECOM_MAX_MODULES      2
#else
#define MODGATECOM_MAX_MODULES      1
#endif

#if defined (_MSC_VER)
#pragma warning (disable: 4200)
#endif

#include <kbUtilities.h>

typedef enum {
	FBSTATE_OFFLINE = 0x00,	// Physikalisch nicht verbunden
	FBSTATE_LINK = 0x01,	// Physikalisch verbunden, jedoch keine Kommunikation 
	FBSTATE_STANDBY = 0x02,	// Kommunikationsbereit (z.B. preoperational)
	FBSTATE_CYCLIC_IO = 0x03,	// Zyklischer Datenaustausch ist aktiv (z.B. operational)
} MODGATECOM_FieldbusStatus;

//**********************************************************************************************
// Link Layer
//**********************************************************************************************
typedef
#include <COMP_packBegin.h>
    struct {
	INT8U i8uDestination[6];
	INT8U i8uSource[6];
	INT16U i16uType;
	INT8U i8uACK;		//Acknowledge
	INT8U i8uCounter;
}
#include <COMP_packEnd.h>
MODGATECOM_LinkLayer;

//**********************************************************************************************
// Transport Layer
//**********************************************************************************************
typedef
#include <COMP_packBegin.h>
    struct {
	INT16U i16uCmd;
	INT16U i16uDataLength;
	INT32U i32uError;
	INT8U i8uVersion;
	INT8U i8uReserved;
}
#include <COMP_packEnd.h>
MODGATECOM_TransportLayer;

//**********************************************************************************************
// Application Layer
//**********************************************************************************************

typedef enum {
	MODGATE_ST_HW_CHECK = 0x00,
	MODGATE_ST_LINK_CHECK = 0x01,
	MODGATE_ST_ID_REQ = 0x02,
	MODGATE_ST_ID_RESP = 0x03,
	MODGATE_ST_RUN_NO_DATA = 0x04,
	MODGATE_ST_RUN = 0x05,
} MODGATE_AL_Status;

typedef enum {
	MODGATECOM_enPLS_INIT = 0x01,
	MODGATECOM_enPLS_LINK_MISSING = 0x02,
	MODGATECOM_enPLS_DATA_MISSING = 0x03,
	MODGATECOM_enPLS_RUN = 0x04,
	MODGATECOM_enPLS_ERROR = 0x05,
} MODGATECOM_EPowerLedState;

typedef enum {
	MODGATE_AL_CMD_ID_Req = 0x0001,
	MODGATE_AL_CMD_ID_Resp = 0x8001,
	MODGATE_AL_CMD_cyclicPD = 0x0002,
	MODGATE_AL_CMD_updatePD = 0x0003,	// not used yet
	MODGATE_AL_CMD_ST_Req = 0x0004,	// not used
	MODGATE_AL_CMD_ST_Resp = 0x8004,	// not used
} MODGATE_AL_Command;

// Feature descriptor bits
#define MODGATE_feature_IODataExchange          0x0001	// supports data-exchange using ethernet (e.g. mGate)
#define MODGATE_feature_RS485DataExchange       0x0002	// supports data exchange using RS485 (e.g. piDio)

#define MODGATE_LL_HEADER_LEN               (sizeof(MODGATECOM_LinkLayer))	// 16
#define MODGATE_TL_HEADER_LEN               (sizeof(MODGATECOM_LinkLayer) + sizeof(MODGATECOM_TransportLayer))	// 26
#define MODGATE_MAX_PD_DATALEN              512
#define MODGATE_AL_MAX_LEN                  (sizeof(MODGATECOM_CyclicPD) + MODGATE_MAX_PD_DATALEN)	// 543 size of the biggest AL packet
#define MODGATE_LL_MAX_LEN                  ((MODGATE_TL_HEADER_LEN + MODGATE_AL_MAX_LEN + 3) & 0xfffffffc)	// 544 bigger packet on the line, rounded up to the next multiple of 4

//**********************************************************************************************
typedef
#include <COMP_packBegin.h>
    struct {
	INT32U i32uSerialnumber;
	INT16U i16uModulType;
	INT16U i16uHW_Revision;
	INT16U i16uSW_Major;
	INT16U i16uSW_Minor;
	INT32U i32uSVN_Revision;
	INT16U i16uFBS_InputLength;
	INT16U i16uFBS_OutputLength;
	INT16U i16uFeatureDescriptor;
}
#include <COMP_packEnd.h>
MODGATECOM_IDResp;

//**********************************************************************************************
typedef
#include <COMP_packBegin.h>
    struct {
	INT8U i8uFieldbusStatus;	// type MODGATECOM_FieldbusStatus
	INT16U i16uOffset;
	INT16U i16uDataLen;
	INT8U i8uData[0];	// dummy declaration for up to MODGATE_MAX_PD_DATALEN bytes
}
#include <COMP_packEnd.h>
MODGATECOM_CyclicPD;

typedef
#include <COMP_packBegin.h>
    struct {
	MODGATECOM_LinkLayer strLinkLayer;
	MODGATECOM_TransportLayer strTransportLayer;
	INT8U i8uData[MODGATE_AL_MAX_LEN];
}
#include <COMP_packEnd.h>
MODGATECOM_Packet;

//**********************************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

//**********************************************************************************************
// Link Layer
//**********************************************************************************************

	typedef struct _sLLData {
		MODGATECOM_LinkLayer Header;
		INT8U state;
		INT32U tick;	// tick counter
		INT8U retry;	// retry counter
		TBOOL timed_out;
		MODGATECOM_Packet *pLastData;
	} sLLData;

	typedef sLLData *LLHandle;

	INT32U MG_LL_init(LLHandle llHdl);
	INT32U MG_LL_send(LLHandle llHdl, MODGATECOM_Packet * pData_p);
	MODGATECOM_Packet *MG_LL_recv(LLHandle llHdl, INT32U * pi32uStatus_p, INT16U * pi16uLen_p);
	TBOOL MG_LL_pending(LLHandle llHdl);
	void MG_LL_abort(LLHandle llHdl);
	TBOOL MG_LL_timed_out(LLHandle llHdl);

//**********************************************************************************************
// Application Layer
//**********************************************************************************************
	typedef struct _sALData {
		sLLData llParas;

		MODGATECOM_IDResp OtherID;	//!< ID-Data of other mGate
		kbUT_Timer AL_Timeout;

		INT8U *pi8uInData;
		INT16U i16uInDataLen;
		INT16U i16uInDataLenActive;

		INT8U *pi8uOutData;
		INT16U i16uOutDataLen;
		INT16U i16uOutDataLenActive;

		INT8U i8uState;	//!< modular Gateway state machine state
		INT8U i8uOtherFieldbusState;	//!< Fieldbus State of other mGate
		MODGATECOM_EPowerLedState enLedStateAct;
		MODGATECOM_EPowerLedState enLedStateOld;
	} sALData;

	typedef sALData *ALHandle;

	extern sALData AL_Data_s[MODGATECOM_MAX_MODULES];

//**********************************************************************************************

	INT32U MODGATECOM_init(INT8U * pi8uInData_p, INT16U i16uInDataLen_p, INT8U * pi8uOutData_p,
			       INT16U i16uOutDataLen_p);
	void MODGATECOM_run(void);

	void MODGATECOM_SetOwnFieldbusState(INT8U i8uOwnFieldbusState_p);
	INT8U MODGATECOM_GetOwnFieldbusState(void);
	INT8U MODGATECOM_GetOtherFieldbusState(INT8U i8uInst_p);	// in modular Gateways always 0 must be passed
	MODGATECOM_EPowerLedState MODGATECOM_GetLedState(void);

	INT16U MODGATECOM_GetInputDataLengthActive(INT8U i8uInstance_p);	// in modular Gateways, always 0 must be passed
	INT16U MODGATECOM_GetOutputDataLengthActive(INT8U i8uInstance_p);	// ditto

//**********************************************************************************************
// internal
//**********************************************************************************************

	INT32U MODGATECOM_send_ID_Req(ALHandle);
	INT32U MODGATECOM_send_ID_Resp(ALHandle);
	INT32U MODGATECOM_send_cyclicPD(ALHandle);

	TBOOL MODGATECOM_recv_Id_Resp(ALHandle, MODGATECOM_Packet * pPacket_p);
	TBOOL MODGATECOM_recv_cyclicPD(ALHandle, MODGATECOM_Packet * pPacket_p);

	void MODGATECOM_managePowerLedRun(void);
	MODGATE_AL_Status MODGATECOM_GetState(INT8U i8uInst_p);

//**********************************************************************************************

	extern MODGATECOM_IDResp MODGATE_OwnID_g;	//!< ID-Data of this mGate

#define MODGATECOM_GetOtherFieldbusStatePtr(i)       (&AL_Data_s[i].i8uOtherFieldbusState)
#define MODGATECOM_GetOwnIdDataPtr()                 (&MODGATE_OwnID_g)
#define MODGATECOM_GetOtherIdDataPtr(i)              (&AL_Data_s[i].OtherID)

// internal functions
// for all functions: return value bTRUE is send/receive was successful
	TBOOL MODGATECOM_receive(void);
	TBOOL MODGATECOM_send(INT16U cmd);
	TBOOL MODGATECOM_send_ACK(void);

	void MODGATECOM_T1_Handler(void);
	void MODGATECOM_T2_Handler(void);

#ifdef  __cplusplus
}
#endif
#endif				//MODGATECOMMAIN_H_INC
