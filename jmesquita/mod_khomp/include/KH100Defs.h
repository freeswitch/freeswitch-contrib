#if !defined KH100DEFS_H
#define KH100DEFS_H

/******************************************************************************
Comandos do H100
H100 commands
******************************************************************************/

#define CM_SEND_TO_CTBUS			0x90
/*
	Liga o TX do canal especificado a um determinado Stream/Timeslot do ctbus.
	Link the TX of the specified channel to the specified ctbus stream/timeslot
	K3L_COMMAND.Object = ChannelNumber
  Params:
	[0] - Ctbus stream
	[1] - Ctbus timeslot
	[2] - Start/Stop
*/

#define CM_RECV_FROM_CTBUS			0x91
/*	Liga o RX do canal especificado a um determinado Stream/Timeslot do ctbus.
	Link the RX of the specified channel to the specified ctbus stream/timeslot
	K3L_COMMAND.Object = ChannelNumber
	Params:
	[0] - Ctbus stream
	[1] - Ctbus timeslot
	[2] - Start/Stop
*/

#define CM_SEND_RANGE_TO_CTBUS		0x92
/*
	Liga o TX dos canais especificados aos determinados Stream/Timeslots do ctbus.
	Link the TX of the specified channels to the specified ctbus stream/timeslots
	
    K3L_COMMAND.Object = First channel to be used in the block

  Params:
	A null terminated ascii string with the command parameters.
	The expected call parameters are:
	Parameter:          		Required:	Description:
	range_count           		yes			Number of channels in the block
	stream              		yes			Stream of the H100 BUS
	first_timeslot     		    yes		    First timeslot, it´s linked with the first channel
    enabled                     no          if true enable the transmission, else disable it. Default value is true.
*/


#define CM_SETUP_H100				0x93
/*	Este comando está obsoleto e não é mais processado pela API, a configuração 
    deve ser feita utilizando o aplicativo KConfig.

	This command is deprecated and is no longer supported, this configuration 
	should be done using KConfig.

	------
	Configura a iteração entre a placa e o ctbus
	Sets up the iteration between the board and the ctbus
	
	K3L_COMMAND.Object = Command*
  Params:
    [0] - Params*

  * See documentation 
	Command definitions will be finded in KH100Defs.h
*/


enum KH100ConfigIndex
{
	khciDeviceMode = 0,
	khciMasterGenClock = 1,
	khciCTNetRefEnable = 4,
	khciSCbusEnable = 6,
	khciHMVipEnable = 7,
	khciMVip90Enable = 8,
	khciCTbusDataEnable = 9,
	khciCTbusFreq03_00 = 10,
	khciCTbusFreq07_04 = 11,
	khciCTbusFreq11_08 = 12,
	khciCTbusFreq15_12 = 13,
	khciMax = 14,
	khciMasterDevId = 20,
	khciSecMasterDevId = 21,
	khciCtNetrefDevId = 22,
    khciMaxH100ConfigIndex
};	

// enumerados usados juntamente a estrutura K3L_H100_STATUS
enum KMasterPLLClockReference
{
    h100_Ref_FreeRun = 0,
    h100_Ref_holdover = 1,
	h100_Automatic = 2,
    h100_Ref_ctnetref = 7,
    h100_Ref_link0 = 8,
    h100_Ref_link1 = 9
};

enum KSlavePLLClockReference
{
    h100_PllLoc_ClkA = 0,
    h100_PllLoc_ClkB = 1,
    h100_PllLoc_SCBus = 2,
    h100_PllLoc_MVIP90 = 3,
    h100_PllLoc_Link0 = 4,
    h100_PllLoc_Link1 = 5,
    h100_PllLoc_Error = 6
};


/******************************************************************************
CM_SETUP_H100
Configura o H100. Este comando é na verdade um grupo de comandos de configuração.
*******************************************************************************/
#define H100_DEVICE_MODE				khciDeviceMode
// Define o modo de operação da placa.
	enum KH100Mode
	{
		h100_Slave,
		h100_Master,
		h100_StandbyMaster,
		h100_Diagnostic,
		h100_NotConnected
	};

#define H100_MASTER_GEN_CLOCK			khciMasterGenClock
// Define qual o clock a ser gerado (A ou B) no CTbus caso a placa seja ou venha a ser a master
	enum KH100SelectCtbusClock
	{
		h100_scClockA,
		h100_scClockB
	};

// trier - changes... :)
#define H100_CT_NETREF_ENABLE			khciCTNetRefEnable
// Habilita ou desabilita e programa a freq. de geração do CT_NETREF. 
	enum KH100CtNetref
	{
		h100_nrOff,
		h100_nrEnable
	};

#define	H100_SCBUS_ENABLE				khciSCbusEnable
// Habilita ou desabilita e configura o clock da geração da referência do scbus.
	enum KH100ScbusEnable
	{
		h100_seOff,
		h100_seOn2Mhz,
		h100_seOn4Mhz,
		h100_seOn8Mhz
	};

#define H100_HMVIP_ENABLE				khciHMVipEnable
// Habilita ou desabilita a geração da referência do HMVIP.
// 1 - On, 0 - Off KH100Enable

#define H100_MVIP90_ENABLE				khciMVip90Enable
// Habilita ou desabilita a geração da referência do MVIP90.
// 1 - On, 0 - Off KH100Enable

#define H100_CTBUS_DATA_ENABLE			khciCTbusDataEnable
// Habilita ou desabilita a transmissao de dados no CTbus.
// 1 - On, 0 - Off KH100Enable

enum KH100Enable
{
	h100_On = 0x01,
	h100_Off = 0x00
};
	
#define H100_CTBUS_FREQ_03_00			khciCTbusFreq03_00
#define H100_CTBUS_FREQ_07_04			khciCTbusFreq07_04
#define H100_CTBUS_FREQ_11_08			khciCTbusFreq11_08
#define H100_CTBUS_FREQ_15_12			khciCTbusFreq15_12
// Configura a frequência de geração dos clocks do ctbus em blocos de 4 streams conforme a
// especificação do H100. Bloco 0, streams de 0 a 3, bloco 1, de 4 a 7, etc..
// As stream de 16 a 31 estarão sempre em 8Mhz conforme especificação.
enum KH100CtbusFreq
{
	h100_cf_2Mhz	=	0,
	h100_cf_4Mhz	=	1,
	h100_cf_8Mhz	=	2 
};


#endif
