#if !defined KR2D_H
#define KR2D_H

/******************************************************************************
Comandos de controle de ligação especiais
Special call control commands
******************************************************************************/
#define CM_SET_LINE_CONDITION		0x80
/*	Programa a condição do ramal. Programar para kgbNone para enviar manualmente.
	Program called subscriber condition. Sets to kgbNone to sends it manually.

	K3L_COMMAND.Object = ChannelNumber

  Params:
	[0] - Subscriber Line Condition (Group B)
*/

#define CM_SEND_LINE_CONDITION		0x81
/*	Envia a condicao do ramal.
	Sends subscriber line condition.

	K3L_COMMAND.Object = ChannelNumber

  Params:
	[0] - Subscriber Line Condition (Group B)
*/


#define CM_SET_CALLER_CATEGORY		0x82
/*	Configura a categoria do assinante chamador enviada na ocupação do canal.
	Sets caller subscriber category.

	K3L_COMMAND.Object = ChannelNumber

  Params:
	[0] - Subscriber category, group II
*/      
	
#define CM_DIAL_MFC					0x83
/*	Disca uma string terminada em 0 em MFC. Máximo 20 dígitos.
	Dial a null terminated string, in MFC. Maximum 20 digits.

    K3L_COMMAND.Object = ChannelNumber

	Events: (syncronous command)
  Params:
	String with the number to dial (ascii)
*/

/* 
	A condicao do ramal em uma ligacao sainte vem nos eventos 
EV_CALL_SUCCESS  ou EV_CALL_FAIL, no campo AddInfo.

	A categoria do assinante chamador numa ligacao entrante vem no evento
EV_SEIZURE, no campo AddInfo.

*/

/******************************************************************************
Definições para sinalização (Brasil)
Signaling definitions (Brazil)
******************************************************************************/
/*
 *  Category of A    (MFC signaling, group II)
 */
enum KSignGroupII_Brazil
{
	kg2BrOrdinary		        = 0x01,     //	Subscriber without priority.
	kg2BrPriority		        = 0x02,     //	Subscriber with priority.
	kg2BrMaintenance	        = 0x03,     //	Maintenance equipment.
	kg2BrLocalPayPhone	        = 0x04,     //	Local pay phone.
	kg2BrTrunkOperator	        = 0x05,     //	Trunk operator.
	kg2BrDataTrans	            = 0x06,     //	Data transmission.
	kg2BrNonLocalPayPhone       = 0x07,     //	Non-local pay phone.
	kg2BrCollectCall	        = 0x08,     //	Collect call.
	kg2BrOrdinaryInter	        = 0x09,     //	International ordinary subscriber.
	kg2BrTransfered		        = 0x0B,     //	Transfered call.
};

/*
 *  Condition of B   (MFC signaling, group B)
 */
enum KSignGroupB_Brazil
{
	kgbBrLineFreeCharged		= 0x01,     //	Line free and charged.
	kgbBrBusy					= 0x02,     //	Line busy.
	kgbBrNumberChanged			= 0x03,     //	B number was changed.
	kgbBrCongestion				= 0x04,     //	Congestion.
	kgbBrLineFreeNotCharged		= 0x05,     //	Line free and not charged.
	kgbBrLineFreeChargedLPR		= 0x06,     //	Line free, charged, last party released.
	kgbBrInvalidNumber			= 0x07,     //	Unallocated number.
	kgbBrLineOutOfOrder			= 0x08,     //	Called line is out of order.
	kgbBrNone					= 0xFF      //	Used when a manual send of the condition is required.
};


/******************************************************************************
Definições para sinalização (Argentina)
Signaling definitions (Argentina)
******************************************************************************/
/*
 *  Category of A    (MFC signaling, group II)
 */
enum KSignGroupII_Argentina
{
	kg2ArOrdinary		        = 0x01,     //	Subscriber without priority.
	kg2ArPriority		        = 0x02,     //	Subscriber with priority.
	kg2ArMaintenance	        = 0x03,     //	Maintenance equipment.
	kg2ArLocalPayPhone	        = 0x04,     //	Local pay phone.
	kg2ArTrunkOperator	        = 0x05,     //	Trunk operator.
	kg2ArDataTrans	            = 0x06,     //	Data transmission.
    kg2ArCPTP                   = 0x0B,     //  C.P.T.P
    kg2ArSpecialLine            = 0x0C,     //  Special Subcriber Line
    kg2ArMobileUser             = 0x0D,     //  Mobile User
    kg2ArPrivateRedLine         = 0x0E,     //  Virtual Private Red Line
    kg2ArSpecialPayPhoneLine    = 0x0F,     //  Special Pay Phone Line
};

/*
 *  Condition of B   (MFC signaling, group B)
 */
enum KSignGroupB_Argentina
{
	kgbArNumberChanged			= 0x02,     //	B number was changed.
	kgbArBusy					= 0x03,     //	Line busy.
	kgbArCongestion				= 0x04,     //	Congestion.
	kgbArInvalidNumber			= 0x05,     //	Unallocated number.
	kgbArLineFreeCharged		= 0x06,     //	Line free and charged.
	kgbArLineFreeNotCharged		= 0x07,     //	Line free and not charged.
	kgbArLineOutOfOrder			= 0x08,     //	Called line is out of order.
	kgbArNone					= 0xFF      //	Used when a manual send of the condition is required.
};


/******************************************************************************
Definições para sinalização (Chile)
Signaling definitions (Chile)
******************************************************************************/
/*
 *  Category of A    (MFC signaling, group II)
 */
enum KSignGroupII_Chile
{
    kg2ClOrdinary		        = 0x01,     //	Subscriber without priority.
	kg2ClPriority		        = 0x02,     //	Subscriber with priority.
	kg2ClMaintenance	        = 0x03,     //	Maintenance equipment.
	kg2ClTrunkOperator	        = 0x05,     //	Trunk operator.
	kg2ClDataTrans	            = 0x06,     //	Data transmission.
    kg2ClUnidentifiedSubscriber = 0x0B,     //  Unidentified subscriber.
};

/*
 *  Condition of B   (MFC signaling, group B)
 */
enum KSignGroupB_Chile
{
	kgbClNumberChanged			= 0x02,     //	B number was changed.
	kgbClBusy					= 0x03,     //	Line busy.
	kgbClCongestion				= 0x04,     //	Congestion.
	kgbClInvalidNumber			= 0x05,     //	Unallocated number.
	kgbClLineFreeCharged		= 0x06,     //	Line free and charged.
	kgbClLineFreeNotCharged		= 0x07,     //	Line free and not charged.
	kgbClLineOutOfOrder			= 0x08,     //	Called line is out of order.
	kgbClNone					= 0xFF      //	Used when a manual send of the condition is required.
};


/******************************************************************************
Definições para sinalização (Mexico)
Signaling definitions (Mexico)
******************************************************************************/
/*
 *  Category of A    (MFC signaling, group II)
 */
enum KSignGroupII_Mexico
{
	kg2MxTrunkOperator	        = 0x01,     //	Trunk operator.
	kg2MxOrdinary		        = 0x02,     //	Subscriber without priority.
	kg2MxMaintenance	        = 0x06,     //	Maintenance equipment.
};

/*
 *  Condition of B   (MFC signaling, group B)
 */
enum KSignGroupB_Mexico
{
	kgbMxLineFreeCharged		= 0x01,     //	Line free and charged.
	kgbMxBusy					= 0x02,     //	Line busy.
	kgbMxLineFreeNotCharged		= 0x05,     //	Line free and not charged.
	kgbMxNone				    = 0xFF      //	Used when a manual send of the condition is required.
};


/******************************************************************************
Definições para sinalização (Uruguai)
Signaling definitions (Uruguay)
******************************************************************************/
/*
 *  Category of A    (MFC signaling, group II)
 */
enum KSignGroupII_Uruguay
{
	kg2UyOrdinary		        = 0x01,     //	Subscriber without priority.
	kg2UyPriority		        = 0x02,     //	Subscriber with priority.
	kg2UyMaintenance	        = 0x03,     //	Maintenance equipment.
	kg2UyLocalPayPhone	        = 0x04,     //	Local pay phone.
	kg2UyTrunkOperator	        = 0x05,     //	Trunk operator.
	kg2UyDataTrans	            = 0x06,     //	Data transmission.
    kg2UyInternSubscriber       = 0x07,     //  Internacional subscriber.
};

/*
 *  Condition of B   (MFC signaling, group B)
 */
enum KSignGroupB_Uruguay
{
	kgbUyNumberChanged			= 0x02,     //	B number was changed.
	kgbUyBusy					= 0x03,     //	Line busy.
	kgbUyCongestion				= 0x04,     //	Congestion.
	kgbUyInvalidNumber			= 0x05,     //	Unallocated number.
	kgbUyLineFreeCharged		= 0x06,     //	Line free and charged.
	kgbUyLineFreeNotCharged		= 0x07,     //	Line free and not charged.
	kgbUyLineOutOfOrder			= 0x08,     //	Called line is out of order.
	kgbUyNone					= 0xFF      //	Used when a manual send of the condition is required.
};



/******************************************************************************
Definições para sinalização (Venezuela)
Signaling definitions (Venezuela)
******************************************************************************/
/*
 *  Category of A    (MFC signaling, group II)
 */
enum KSignGroupII_Venezuela
{
	kg2VeOrdinary		        = 0x01,     //	Subscriber without priority.
	kg2VePriority		        = 0x02,     //	Subscriber with priority.
	kg2VeMaintenance	        = 0x03,     //	Maintenance equipment.
	kg2VeLocalPayPhone	        = 0x04,     //	Local pay phone.
	kg2VeTrunkOperator	        = 0x05,     //	Trunk operator.
	kg2VeDataTrans	            = 0x06,     //	Data transmission.
    kg2VeNoTransferFacility     = 0x07,     //  Subscriber without transfer facility
};

/*
 *  Condition of B   (MFC signaling, group B)
 */
enum KSignGroupB_Venezuela
{
	kgbVeLineFreeChargedLPR		= 0x01,     //	Line free, charged, last party released.
	kgbVeNumberChanged			= 0x02,     //	B number was changed.
	kgbVeBusy					= 0x03,     //	Line busy.
	kgbVeCongestion				= 0x04,     //	Congestion.
	kgbVeInformationTone		= 0x05,     //	Special Information Tone.
	kgbVeLineFreeCharged		= 0x06,     //	Line free and charged.
	kgbVeLineFreeNotCharged		= 0x07,     //	Line free and not charged.
    kgbVeLineBlocked            = 0x08,     //  Blocked for input trafic.
    kgbVeIntercepted            = 0x09,     //  Intercepted.
    kgbVeDataTrans              = 0x0A,     //  Data Transmission Equipment
	kgbVeNone					= 0xFF      //	Used when a manual send of the condition is required.
};






/******************************************************************************
OBSOLETO
DEPRECATED
******************************************************************************/

// Grupo B
// Group B
enum KSignGroupB
{
	kgbLineFreeCharged			= 0x01,
/*	Livre com tarifação.
	Line free and charged.
*/
	kgbLineFreeNotCharged		= 0x05,
/*	Livre sem tarifação.
	Line free with no charge.
*/
	kgbLineFreeChargedLPR		= 0x06,
/*	Livre com tarifaçao. Retenção sob controle de assinante chamado.
	Line free, charged, last party released.
*/
	kgbBusy						= 0x02,
/*	Linha assinante B ocupado.
	Line busy.
*/
	kgbNumberChanged			= 0x03,
/*	Número de B foi mudado.
	B number was changed.
*/
	kgbCongestion				= 0x04,
/*	Congestionamento.
	Congestion.
*/
	kgbInvalidNumber			= 0x07,
/*	Nível ou número vago.
	Unallocated number.
*/
	kgbLineOutOfOrder			= 0x08,
/*	Assinante B fora de serviço.
	Called line is out of order.
*/
	kgbNone						= 0xFF
/*  Este valor deve ser atribuido quando a for necessario enviar a condicao manualmente
	This value is used when a manual send of the condition is required
*/
};

#define STT_GB_LINEFREE_CHARGED				0x01
#define STT_GB_LINEFREE_NOTCHARGED			0x05
#define STT_GB_LINEFREE_CHARGED_LPR			0x06
#define	STT_GB_BUSY							0x02
#define STT_GB_NUMBER_CHANGED				0x03
#define STT_GB_CONGESTION					0x04
#define STT_GB_UNALLOCATED_NUMBER			0x07
#define STT_GB_LINE_OUT_OF_ORDER			0x08
#define STT_GB_NONE							0xFF

// Grupo II
// Group II
enum KSignGroupII
{
	kg2Ordinary			= 0x01,
/*	Assinante comum.
	Ordinalry subscriber.
*/
	kg2Priority			= 0x02,
/*	Assinante com tarifação especial.
	Subscriber with priority.
*/
	kg2Maintenance		= 0x03,
/*	Equipamento de manutenção.
	Maintenance equipment.
*/
	kg2LocalPayPhone	= 0x04,
/*	Telefone público local.
	Local pay phone.
*/
	kg2TrunkOperator	= 0x05,
/*	Telefonista.
	Trunk operator.
*/
	kg2DataTrans		= 0x06,
/*	Comunicação de dados.
	Data transmission.
*/
	kg2NonLocalPayPhone = 0x07,
/*	Telefone público interurbando.
	Non-local pay phone.
*/
	kg2CollectCall		= 0x08,
/*	Chamada a cobrar.
	Collect call.
*/
	kg2OrdinaryInter	= 0x09,
/*	Assinante comum, entrante internacional.
	International ordinary subscriber.
*/
	kg2Transfered		= 0x0B,
/*	Chamada transferida.
	Transfered call.
*/
};

#define STT_GII_ORDINARY					0x01
#define STT_GII_PRIORITY					0x02
#define STT_GII_MAINTENANCE					0x03
#define STT_GII_LOCAL_PAY_PHONE				0x04
#define STT_GII_TRUNK_OPERATOR				0x05
#define STT_GII_DATA_TRANS					0x06
#define STT_GII_NON_LOCAL_PAY_PHONE			0x07		
#define STT_GII_COLLECT_CALL				0x08
#define STT_GII_ORDINARY_INTERNATIONAL		0x09
#define STT_GII_TRANSFERED					0x0B

#endif
