#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include "INC_INCLUDES.h"
#include "tdmb_t3700.h"

/*********************************************************************************/
/* Operating Chip set : T3700                                                    */
/* Software version   : version 1.31                                             */
/* Software Update    : 2009.07.30                                              */
/*********************************************************************************/



#define INC_INTERRUPT_LOCK()		
#define INC_INTERRUPT_FREE()

#define minSleeptime (MSEC_PER_SEC/HZ)

ST_SUBCH_INFO		g_stDmbInfo;
ST_SUBCH_INFO		g_stDabInfo;
ST_SUBCH_INFO		g_stDataInfo;
ENSEMBLE_BAND 	m_ucRfBand 						= KOREA_BAND_ENABLE;

/*********************************************************************************/
/*  RF Band Select                                                               */
/*                                                                               */
/*  INC_UINT8 m_ucRfBand = KOREA_BAND_ENABLE,                                    */
/*						   BANDIII_ENABLE,                                                 */
/*						   LBAND_ENABLE,                                                   */
/*						   CHINA_ENABLE,                                                   */
/*						   EXTERNAL_ENABLE,                                                */
/*********************************************************************************/
CTRL_MODE 				m_ucCommandMode 		= INC_I2C_CTRL;
ST_TRANSMISSION		m_ucTransMode				= TRANSMISSION_MODE1;
UPLOAD_MODE_INFO	m_ucUploadMode 			= STREAM_UPLOAD_TS;
CLOCK_SPEED				m_ucClockSpeed 			= INC_OUTPUT_CLOCK_4096;
//INC_ACTIVE_MODE		m_ucMPI_CS_Active 	= INC_ACTIVE_LOW;
INC_ACTIVE_MODE		m_ucMPI_CS_Active 	= INC_ACTIVE_HIGH;
//INC_ACTIVE_MODE		m_ucMPI_CLK_Active 	= INC_ACTIVE_LOW;
INC_ACTIVE_MODE		m_ucMPI_CLK_Active 	= INC_ACTIVE_HIGH;
INC_UINT16				m_unIntCtrl					= (INC_INTERRUPT_POLARITY_HIGH | \
										   INC_INTERRUPT_PULSE | \
										   INC_INTERRUPT_AUTOCLEAR_ENABLE | \
										   (INC_INTERRUPT_PULSE_COUNT & INC_INTERRUPT_PULSE_COUNT_MASK));


/*********************************************************************************/
/* PLL_MODE     m_ucPLL_Mode                                                     */
/*          T3700  Input Clock Setting                                           */
/*********************************************************************************/
PLL_MODE			m_ucPLL_Mode		= INPUT_CLOCK_24576KHZ;


/*********************************************************************************/
/* INC_DPD_MODE		m_ucDPD_Mode                                                   */
/*					T3700  Power Saving mode setting                                     */
/*********************************************************************************/
INC_DPD_MODE		m_ucDPD_Mode		= INC_DPD_ON;


/*********************************************************************************/
/*  MPI Chip Select and Clock Setup Part                                         */
/*                                                                               */
/*  INC_UINT8 m_ucCommandMode = INC_I2C_CTRL, INC_SPI_CTRL, INC_EBI_CTRL         */
/*                                                                               */
/*  INC_UINT8 m_ucUploadMode = STREAM_UPLOAD_MASTER_SERIAL,                      */
/*							   STREAM_UPLOAD_SLAVE_PARALLEL,                                 */
/*							   STREAM_UPLOAD_TS,                                             */
/*							   STREAM_UPLOAD_SPI,                                            */
/*                                                                               */
/*  INC_UINT8 m_ucClockSpeed = INC_OUTPUT_CLOCK_4096,                            */
/*							   INC_OUTPUT_CLOCK_2048,                                        */
/*							   INC_OUTPUT_CLOCK_1024,                                        */
/*********************************************************************************/


/*********************************************************************************/
/* �ݵ�� 1ms Delay�Լ��� �־�� �Ѵ�.                                           */
/*********************************************************************************/
void INC_DELAY(INC_UINT16 uiDelay)
{
	INC_UINT16 wSleep;
	INC_UINT16 wDelay;

	if (uiDelay >= 20)
	{
		wDelay = uiDelay%minSleeptime;
		wSleep = uiDelay - wDelay-10;
	
		if (wSleep > 0)
			msleep(wSleep);

		if (wDelay > 0)
			mdelay(wDelay);
	}
	else
	{
		mdelay(uiDelay);
	}
}

INC_UINT16 INC_I2C_READ(INC_UINT8 ucI2CID, INC_UINT16 uiAddr)
{
	INC_UINT16 wData;
	
	if (t3700_read_word_data(uiAddr, &wData) != -1)
	{
		return wData;
	}

	return INC_ERROR;
}

INC_UINT8 INC_I2C_WRITE(INC_UINT8 ucI2CID, INC_UINT16 uiAddr, INC_UINT16 uiData)
{

	if (t3700_write_word_data(uiAddr, uiData) != -1)
	    	return INC_SUCCESS;
	
	return INC_ERROR;
}

INC_UINT8 INC_I2C_READ_BURST(INC_UINT8 ucI2CID,  INC_UINT16 uiAddr, INC_UINT8* pData, INC_UINT16 nSize)
{
	if (t3700_read_block_data(uiAddr, pData, nSize) != -1)
		return INC_SUCCESS;

	return INC_ERROR; 
}

INC_UINT8 INC_EBI_WRITE(INC_UINT8 ucI2CID, INC_UINT16 uiAddr, INC_UINT16 uiData)
{
#if 0
	INC_UINT16 uiCMD = INC_REGISTER_CTRL(SPI_REGWRITE_CMD) | 1;
	INC_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	INC_INTERRUPT_LOCK();

	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiNewAddr >> 8;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiNewAddr & 0xff;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiCMD >> 8;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiCMD & 0xff;

	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = (uiData >> 8) & 0xff;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS =  uiData & 0xff;
	INC_INTERRUPT_FREE();
#endif	
	return INC_ERROR;
}

INC_UINT16 INC_EBI_READ(INC_UINT8 ucI2CID, INC_UINT16 uiAddr)
{
#if 0
	INC_UINT16 uiRcvData = 0;
	INC_UINT16 uiCMD = INC_REGISTER_CTRL(SPI_REGREAD_CMD) | 1;
	INC_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	INC_INTERRUPT_LOCK();
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiNewAddr >> 8;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiNewAddr & 0xff;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiCMD >> 8;
	*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiCMD & 0xff;
	
	uiRcvData  = (*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS & 0xff) << 8;
	uiRcvData |= (*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS & 0xff);
	
	INC_INTERRUPT_FREE();
	return uiRcvData;
#endif
	return 0;
}

INC_UINT8 INC_EBI_READ_BURST(INC_UINT8 ucI2CID,  INC_UINT16 uiAddr, INC_UINT8* pData, INC_UINT16 nSize)
{
#if 0	
	INC_UINT16 uiLoop, nIndex = 0, anLength[2], uiCMD, unDataCnt;
	INC_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;
	
	if(nSize > INC_MPI_MAX_BUFF) return INC_ERROR;
	memset((INC_INT8*)anLength, 0, sizeof(anLength));

	if(nSize > INC_TDMB_LENGTH_MASK) {
		anLength[nIndex++] = INC_TDMB_LENGTH_MASK;
		anLength[nIndex++] = nSize - INC_TDMB_LENGTH_MASK;
	}
	else anLength[nIndex++] = nSize;

	INC_INTERRUPT_LOCK();
	for(uiLoop = 0; uiLoop < nIndex; uiLoop++){

		uiCMD = INC_REGISTER_CTRL(SPI_MEMREAD_CMD) | (anLength[uiLoop] & INC_TDMB_LENGTH_MASK);
		
		*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiNewAddr >> 8;
		*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiNewAddr & 0xff;
		*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiCMD >> 8;
		*(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS = uiCMD & 0xff;
		
		for(unDataCnt = 0 ; unDataCnt < anLength[uiLoop]; unDataCnt++){
			*pData++ = *(volatile INC_UINT8*)STREAM_PARALLEL_ADDRESS & 0xff;
		}
	}
	INC_INTERRUPT_FREE();
#endif
	return INC_SUCCESS;
}

INC_UINT16 INC_SPI_REG_READ(INC_UINT8 ucI2CID, INC_UINT16 uiAddr)
{
#if 0	
	INC_UINT16 uiRcvData = 0;
	INC_UINT16 uiCMD = INC_REGISTER_CTRL(SPI_REGREAD_CMD) | 1;
	INC_UINT8 auiBuff[6];
	INC_UINT8 cCnt = 0;
	INC_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	auiBuff[cCnt++] = uiNewAddr >> 8;
	auiBuff[cCnt++] = uiNewAddr & 0xff;
	auiBuff[cCnt++] = uiCMD >> 8;
	auiBuff[cCnt++] = uiCMD & 0xff;
	INC_INTERRUPT_LOCK();
	//TODO SPI Write code here...

	//TODO SPI Read code here...
	INC_INTERRUPT_FREE();
	return uiRcvData;
#endif
	return 0;
}

INC_UINT8 INC_SPI_REG_WRITE(INC_UINT8 ucI2CID, INC_UINT16 uiAddr, INC_UINT16 uiData)
{
#if 0	
	INC_UINT16 uiCMD = INC_REGISTER_CTRL(SPI_REGWRITE_CMD) | 1;
	INC_UINT8 auiBuff[6];
	INC_UINT8 cCnt = 0;
	INC_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	auiBuff[cCnt++] = uiNewAddr >> 8;
	auiBuff[cCnt++] = uiNewAddr & 0xff;
	auiBuff[cCnt++] = uiCMD >> 8;
	auiBuff[cCnt++] = uiCMD & 0xff;
	auiBuff[cCnt++] = uiData >> 8;
	auiBuff[cCnt++] = uiData & 0xff;
	INC_INTERRUPT_LOCK();
	
	//TODO SPI SDO Send code here...

	INC_INTERRUPT_FREE();
#endif	
	return INC_SUCCESS;
}

INC_UINT8 INC_SPI_READ_BURST(INC_UINT8 ucI2CID, INC_UINT16 uiAddr, INC_UINT8* pBuff, INC_UINT16 wSize)
{
#if 0	
	INC_UINT16 uiLoop, nIndex = 0, anLength[2], uiCMD, unDataCnt;
	INC_UINT8 auiBuff[6];
	INC_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	if(wSize > INC_MPI_MAX_BUFF) return INC_ERROR;
	memset((INC_INT8*)anLength, 0, sizeof(anLength));

	if(wSize > INC_TDMB_LENGTH_MASK) {
		anLength[nIndex++] = INC_TDMB_LENGTH_MASK;
		anLength[nIndex++] = wSize - INC_TDMB_LENGTH_MASK;
	}
	else anLength[nIndex++] = wSize;

	INC_INTERRUPT_LOCK();
	for(uiLoop = 0; uiLoop < nIndex; uiLoop++){

		auiBuff[0] = uiNewAddr >> 8;
		auiBuff[1] = uiNewAddr & 0xff;
		uiCMD = INC_REGISTER_CTRL(SPI_MEMREAD_CMD) | (anLength[uiLoop] & INC_TDMB_LENGTH_MASK);
		auiBuff[2] = uiCMD >> 8;
		auiBuff[3] = uiCMD & 0xff;
		
		//TODO SPI[SDO] command Write code here..
		for(unDataCnt = 0 ; unDataCnt < anLength[uiLoop]; unDataCnt++){
			//TODO SPI[SDI] data Receive code here.. 
		}
	}
	INC_INTERRUPT_FREE();
#endif
	return INC_SUCCESS;
}

INC_UINT8 INTERFACE_DBINIT(void)
{
	memset(&g_stDmbInfo,	0, sizeof(ST_SUBCH_INFO));
	memset(&g_stDabInfo,	0, sizeof(ST_SUBCH_INFO));
	memset(&g_stDataInfo,	0, sizeof(ST_SUBCH_INFO));
	
	return INC_SUCCESS;
}

// �ʱ� ���� �Է½� ȣ��
INC_UINT8 INTERFACE_INIT(INC_UINT8 ucI2CID)
{
	return INC_INIT(ucI2CID);
}

// ���� �߻��� �����ڵ� �б�
INC_ERROR_INFO INTERFACE_ERROR_STATUS(INC_UINT8 ucI2CID)
{
	ST_BBPINFO* pInfo;
	pInfo = INC_GET_STRINFO(ucI2CID);
	return pInfo->nBbpStatus;
}

/*********************************************************************************/
/* ���� ä�� �����Ͽ� �����ϱ�....                                               */  
/* pChInfo->ucServiceType, pChInfo->ucSubChID, pChInfo->ulRFFreq ��              */
/* �ݵ�� �Ѱ��־�� �Ѵ�.                                                       */
/* DMBä�� ���ý� pChInfo->ucServiceType = 0x18                                  */
/* DAB, DATAä�� ���ý� pChInfo->ucServiceType = 0���� ������ �ؾ���.            */
/*********************************************************************************/
INC_UINT8 INTERFACE_START(INC_UINT8 ucI2CID, INC_CHANNEL_INFO* pChInfo)
{
	return INC_CHANNEL_START(ucI2CID, pChInfo);
}

/*********************************************************************************/
/* ���� ä�� �����Ͽ� �����ϱ�....                                               */  
/* pChInfo->ucServiceType, pChInfo->ucSubChID, pChInfo->ulRFFreq ��              */
/* �ݵ�� �Ѱ��־�� �Ѵ�.                                                       */
/* DMBä�� ���ý� pChInfo->ucServiceType = 0x18                                  */
/* DAB, DATAä�� ���ý� pChInfo->ucServiceType = 0���� ������ �ؾ���.            */
/* pMultiInfo->nSetCnt�� ������ ����ä�� ������.                                 */
/* FIC Data�� ����  ���ý� INC_MULTI_CHANNEL_FIC_UPLOAD ��ũ�θ� Ǯ��� �Ѵ�.    */
/* DMB  ä���� �ִ� 2��ä�� ������ �����ϴ�.                                     */
/* DAB  ä���� �ִ� 3��ä�� ������ �����ϴ�.                                     */
/* DATA ä���� �ִ� 3��ä�� ������ �����ϴ�.                                     */
/*********************************************************************************/
INC_UINT8 INTERFACE_MULTISUB_CHANNEL_START(INC_UINT8 ucI2CID, ST_SUBCH_INFO* pMultiInfo)
{
	return (INC_UINT8)INC_MULTI_START_CTRL(ucI2CID, pMultiInfo);
}

/*********************************************************************************/
/* ��ĵ��  ȣ���Ѵ�.                                                             */
/* ���ļ� ���� �޵�óѰ��־�� �Ѵ�.                                            */
/* Band�� �����Ͽ� ��ĵ�ô� m_ucRfBand�� �����Ͽ��� �Ѵ�.                        */
/* ���ļ� ���� �޵�óѰ��־�� �Ѵ�.                                            */
/*********************************************************************************/
INC_UINT8 INTERFACE_SCAN(INC_UINT8 ucI2CID, INC_UINT32 ulFreq)
{
	INC_INT16 nIndex;
	ST_FIC_DB* pstFicDb;
	INC_CHANNEL_INFO* pChInfo;
	pstFicDb = INC_GETFIC_DB(ucI2CID);
	
	INTERFACE_DBINIT();

	if(!INC_ENSEMBLE_SCAN(ucI2CID, ulFreq)) return INC_ERROR;
 	pstFicDb->ulRFFreq = ulFreq;
	
	for(nIndex = 0; nIndex < pstFicDb->ucSubChCnt; nIndex++){
		switch(pstFicDb->aucTmID[nIndex])
		{
		case 0x01 : pChInfo = &g_stDmbInfo.astSubChInfo[g_stDmbInfo.nSetCnt++];  break;
		case 0x00 : pChInfo = &g_stDabInfo.astSubChInfo[g_stDabInfo.nSetCnt++];	 break;
		default   : pChInfo = &g_stDataInfo.astSubChInfo[g_stDataInfo.nSetCnt++];break;
		}
		INC_UPDATE(pChInfo, pstFicDb, nIndex);
	}
	return INC_SUCCESS;
}

/*********************************************************************************/
/* ����ä�� ��ĵ�� �Ϸ�Ǹ� DMBä�� ������ �����Ѵ�.                             */
/*********************************************************************************/
INC_UINT16 INTERFACE_GETDMB_CNT(void)
{
	return (INC_UINT16)g_stDmbInfo.nSetCnt;
}

/*********************************************************************************/
/* ����ä�� ��ĵ�� �Ϸ�Ǹ� DABä�� ������ �����Ѵ�.                             */
/*********************************************************************************/
INC_UINT16 INTERFACE_GETDAB_CNT(void)
{
	return (INC_UINT16)g_stDabInfo.nSetCnt;
}

/*********************************************************************************/
/* ����ä�� ��ĵ�� �Ϸ�Ǹ� DATAä�� ������ �����Ѵ�.                            */
/*********************************************************************************/
INC_UINT16 INTERFACE_GETDATA_CNT(void)
{
	return (INC_UINT16)g_stDataInfo.nSetCnt;
}

/*********************************************************************************/
/* ����ä�� ��ĵ�� �Ϸ�Ǹ� Ensemble lable�� �����Ѵ�.                           */
/*********************************************************************************/
INC_UINT8* INTERFACE_GETENSEMBLE_LABEL(INC_UINT8 ucI2CID)
{
	ST_FIC_DB* pstFicDb;
	pstFicDb = INC_GETFIC_DB(ucI2CID);
	return pstFicDb->aucEnsembleLabel;
}

/*********************************************************************************/
/* DMB ä�� ������ �����Ѵ�.                                                     */
/*********************************************************************************/
INC_CHANNEL_INFO* INTERFACE_GETDB_DMB(INC_INT16 uiPos)
{
	if(uiPos >= MAX_SUBCH_SIZE) return INC_NULL;
	if(uiPos >= g_stDmbInfo.nSetCnt) return INC_NULL;
	return &g_stDmbInfo.astSubChInfo[uiPos];
}

/*********************************************************************************/
/* DAB ä�� ������ �����Ѵ�.                                                     */
/*********************************************************************************/
INC_CHANNEL_INFO* INTERFACE_GETDB_DAB(INC_INT16 uiPos)
{
	if(uiPos >= MAX_SUBCH_SIZE) return INC_NULL;
	if(uiPos >= g_stDabInfo.nSetCnt) return INC_NULL;
	return &g_stDabInfo.astSubChInfo[uiPos];
}

/*********************************************************************************/
/* DATA ä�� ������ �����Ѵ�.                                                    */
/*********************************************************************************/
INC_CHANNEL_INFO* INTERFACE_GETDB_DATA(INC_INT16 uiPos)
{
	if(uiPos >= MAX_SUBCH_SIZE) return INC_NULL;
	if(uiPos >= g_stDataInfo.nSetCnt) return INC_NULL;
	return &g_stDataInfo.astSubChInfo[uiPos];
}

// ��û �� FIC ���� ����Ǿ������� üũ
INC_UINT8 INTERFACE_RECONFIG(INC_UINT8 ucI2CID)
{
	return INC_FIC_RECONFIGURATION_HW_CHECK(ucI2CID);
}

INC_UINT8 INTERFACE_STATUS_CHECK(INC_UINT8 ucI2CID)
{
	return INC_STATUS_CHECK(ucI2CID);
}

INC_UINT16 INTERFACE_GET_CER(INC_UINT8 ucI2CID)
{
	return INC_GET_CER(ucI2CID);
}

INC_UINT8 INTERFACE_GET_SNR(INC_UINT8 ucI2CID)
{
	return INC_GET_SNR(ucI2CID);
}

INC_FIXED_FLOAT INTERFACE_GET_POSTBER(INC_UINT8 ucI2CID)
{
	return INC_GET_POSTBER(ucI2CID);
}

INC_FIXED_FLOAT INTERFACE_GET_PREBER(INC_UINT8 ucI2CID)
{
	return INC_GET_PREBER(ucI2CID);
}

/*********************************************************************************/
/* Scan, ä�� ���۽ÿ� ������ ������ ȣ���Ѵ�.                                   */
/*********************************************************************************/
void INTERFACE_USER_STOP(INC_UINT8 ucI2CID)
{
	ST_BBPINFO* pInfo;
	pInfo = INC_GET_STRINFO(ucI2CID);
	pInfo->ucStop = 1;
}

// ���ͷ�Ʈ �ο��̺�...
void INTERFACE_INT_ENABLE(INC_UINT8 ucI2CID, INC_UINT16 unSet)
{
	INC_SET_INTERRUPT(ucI2CID, unSet);
}

// ���ͷ��� Ŭ����
void INTERFACE_INT_CLEAR(INC_UINT8 ucI2CID, INC_UINT16 unClr)
{
	INC_CLEAR_INTERRUPT(ucI2CID, unClr);
}

// ���ͷ�Ʈ ���� ��ƾ... // SPI Slave Mode or MPI Slave Mode
INC_UINT8 INTERFACE_ISR(INC_UINT8 ucI2CID, INC_UINT8* pBuff)
{
	INC_UINT16 unLoop;
	INC_UINT32 ulAddrSelect;

	if(m_ucCommandMode != INC_EBI_CTRL){
		if(m_ucUploadMode == STREAM_UPLOAD_SPI){
			INC_SPI_READ_BURST(ucI2CID, APB_STREAM_BASE, pBuff, INC_INTERRUPT_SIZE);
		}
		
		else if(m_ucUploadMode == STREAM_UPLOAD_SLAVE_PARALLEL)
		{
			ulAddrSelect = (ucI2CID == TDMB_I2C_ID80) ? STREAM_PARALLEL_ADDRESS : STREAM_PARALLEL_ADDRESS_CS;
			for(unLoop = 0; unLoop < INC_INTERRUPT_SIZE; unLoop++){
				pBuff[unLoop] = *(volatile INC_UINT8*)ulAddrSelect & 0xff;
			}
		}
	}
	else
		INC_EBI_READ_BURST(ucI2CID, APB_STREAM_BASE, pBuff, INC_INTERRUPT_SIZE);

	if((m_unIntCtrl & INC_INTERRUPT_LEVEL) && (!(m_unIntCtrl & INC_INTERRUPT_AUTOCLEAR_ENABLE)))
		INTERFACE_INT_CLEAR(ucI2CID, INC_MPI_INTERRUPT_ENABLE);
	
	return INC_SUCCESS;
}

