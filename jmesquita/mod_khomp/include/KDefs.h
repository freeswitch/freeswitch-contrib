#if !defined KLDEFS_H
#define KLDEFS_H

#include "KTypeDefs.h"
#include "KH100Defs.h"
#include "KMixerDefs.h"
#include "KR2D.h"
#include "KErrorDefs.h"

/*
	Faixa de comandos:
	0x00 - 0x3F: Comandos de operacao do canal
	0x40 - 0x5F: Habilitacao de recursos
	0x60 - 0x7F: Mixer (KMixerDefs.h)
	0x80 - 0x8F: Especificos para R2 (KR2D.h)
	0x90 - 0x9F: H100
	0xA0 - 0xAF: Mais comandos de mixer
    0xB0 - 0xCF: Vagos
	0xD0 - 0xDF: Compatiblidade
	0xF0 - 0xFE: Erros
	0x8000 - 0xFF00: Configuracao
*/


/******************************************************************************
Comandos de controle de ligação
Call control commands
******************************************************************************/

#define CM_SEIZE					0x01
/*	Solicita uma ocupação no canal.
	Requests a seize in the channel.

	K3L_COMMAND.Object = ChannelNumber

	Events: EV_SEIZE_SUCCESS | EV_SEIZE_FAIL
  Params:
	[0..20] - ANI (ascii, null terminated)
*/     

#define CM_SYNC_SEIZE				0x02
/*	Ocupa um canal, e retorna imediatamente o resultado.
	Seizes a channel returning synchronously.

    K3L_COMMAND.Object = ChannelNumber

	Events: (synchronous command)
  Params:
	[0..20] - ANI (ascii, null terminated)
*/

#define CM_SIP_REGISTER             0x03
/*	Registra um usuario sip (somente para sinalizacao SIP)
	Register a sip user (SIP singnaling only)
	
    Events: EV_SIP_REGISTER_INFO
	Params:
	A null terminated ascii string with the registration parameters.
	The expected call parameters are:
	Parameter:          		Required:	Description:
	user                   		yes			User name
	password              		no			User password
	proxy                  		no			Proxy to register into
	unregister          		no			Signal to unregister a previously registered user
*/

#define CM_DIAL_DTMF				0x04
/*	Disca uma string terminada em 0 em DTMF.
	Dial a null terminated string, in DTMF.

    K3L_COMMAND.Object = ChannelNumber

	Events: EV_DTMF_SEND_FINISH

  Params:
	String with the number to dial (ascii)
*/

#define CM_DISCONNECT				0x05
/*	Desliga.
	Disconnects.

	K3L_COMMAND.Object = ChannelNumber

	Events: EV_CHANNEL_FREE
*/

#define CM_CONNECT					0x06
/*	Atende uma chamada recebida.
	Answers a incoming call.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_PRE_CONNECT				0x07
/*	Habilita audio antes de enviar o comando atendimento (mensagem sem tarifacao).
	Enable audio before send an answer command (no charged messages).

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_CAS_CHANGE_LINE_STT		0x08
/*	Para sinalizacao do tipo personalizada, muda o estado dos bits ABCD de saida
	For custom signaling changes forward ABCD bits state

	K3L_COMMAND.Object = ChannelNumber
	
	Params:
	[0] - New state
*/

#define CM_CAS_SEND_MFC				0x09
/*	Para sinalizacao do tipo personalizada, envia uma nova cifra MFC
	For custom signaling changes, sends a new MFC

	K3L_COMMAND.Object = ChannelNumber
	
	Params:
	[0] - New state
*/

#define CM_SET_FORWARD_CHANNEL		0x0A
/*	Programa um canal para ser prosseguir com as ligacoes de outro canal.
	Sets the a channel to forward call from the actual channel.

	K3L_COMMAND.Object = ChannelNumber

	Params:
	[0..3] - Pointer to an interger containing the forward channel number
*/

#define CM_CAS_SET_MFC_DETECT_MODE	0x0B
/*	Habilita e configura o detector de MFC para filtra sinais de entrada ou saida
	Enables and configures the MFC detectors to filters incoming ou outgoing signals

	K3L_COMMAND.Object = ChannelNumber
	Params:
	[0] - kscFree( 0 )/kcsIncoming( 1 )/kcsOutgoing( 2 )
*/

#define CM_DROP_COLLECT_CALL		0x0C
/*	Derruba chamadas a cobrar, desligando a chamada e a atendendendo novamente após o
    tempo configurado.
	Drops collect calls by disconnecting and connecting again after the configured delay.
*/


#define CM_MAKE_CALL				0x0D
/*	Realiza uma chamada (disponivel para sinalizacao SIP)
	Make a call (available for SIP singnaling)
	
	K3L_COMMAND.Object = ChannelNumber

    Events: EV_CALL_SUCCESS | EV_CALL_FAIL
	Params:
	A null terminated ascii string with the call parameters.
	The expected call parameters are:
	Parameter:          				Required:	Protocol:   Description:
	dest_addr                           yes         ALL         Destiny's number (address)
	orig_addr           		        yes         ALL         Caller number (address)
	sip_to              				no          IP          Destiny's address/ID (same as dest_addr, can be used instead)
	network_dest_addr   				no          IP          Destiny's/SIP Proxy network address
	r2_categ_a                          no          R2          Caller Category
	isdn_orig_type_of_number            no          ISDN        Caller type of number (KQ931TypeOfNumber, KISDN.h)
	isdn_orig_numbering_plan            no          ISDN        Caller numbering plan (KQ931NumberingPlan, KISDN.h)
	isdn_orig_presentation              no          ISDN        Callers presentation indicator (KQ931PresentationIndicator, KISDN.h)
	isdn_dest_type_of_number            no          ISDN        Destiny's type of number
	isdn_dest_numbering_plan            no          ISDN        Destiny's numbering plan
*/

#define CM_RINGBACK 				0x0E
/*  Aceita ou rejeita a chamada entrante em qualquer sinalização, equivalente a CM_SEND_LINE_CONDITION para
	o R2 Digital.
	
    Accepts or reject an incomming call in any signaling, equivalent to CM_SEND_LINE_CONDITION in Digital R2.
   
    K3L_COMMAND.Object = ChannelNumber
    Params:
    A null terminated ascii string with the parameter to pass or NULL for default

	The expected parameters are:
	Parameter:			Protocol:	Description:
	r2_cond_b			R2			Called party category. Note that if CM_SET_LINE_CONDITION was called before it
									is not needed to call CM_RINGBACK, the category is sent automatically.
	sip_prov_resp		IP			Sip Response, if present, sends a DISCONNECT with given response (reason),
									default is to send a ringback to caller.
	isdn_cause			ISDN		ISDN cause, if present, sends DISCONNECT with given cause, default is to
									send alerting indication to caller.
*/

#define CM_USER_INFORMATION         0x0F
/*  Envia dados sobre o canal HDLC, disponível somente para RDSI.

    K3L_COMMAND.Object = ChannelNumber
    Params:
    A pointer to a KUserInformation structure which has the following members:

    ProtocolDescriptor - KQ931UserInfoProtocolDescriptor
    UserInfoLength - Length of UserInfo data array
    UserInfo - pointer to the data array to be transmitted



*/

#define CM_VOIP_SEIZE				0x23
/*  EM DESUSO: Usar CM_MAKE_CALL
    DEPRECATED: Use CM_MAKE_CALL instead
*/

/******************************************************************************
Bloqueio e desbloqueio de canais (E1)
Channel locking and unlocking	 (E1)
******************************************************************************/

#define	CM_LOCK_INCOMING			0x10
/*	Solicita à operadora o bloqueio de ligações entrantes.
	Requests the central office a lock of incoming calls.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_UNLOCK_INCOMING			0x11
/*	Solicita à operadora o desbloqueio de ligações entrantes.
	Requests the central office to unlock incoming calls.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_LOCK_OUTGOING			0x12
/*	Bloqueia ligações saintes..
	Locks outgoing calls.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_UNLOCK_OUTGOING			0x13
/*	Desbloqueia ligações saintes.
	Unlocks outgoing calls.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_START_SEND_FAIL			0x14
/*	Inicia envio de falha na linha.
	Starts to send a fail signal in the line.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_STOP_SEND_FAIL			0x15
/*	Para o envio de falha na linha.
	Stop send a fail signal in the line.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_END_OF_NUMBER            0x16
/*  Informa a sinalizacao que nao devem ser recebidos mais digitos de discagem
    Notify singaling to dont receive more digits

    K3L_COMMAND.Object = ChannelNumber
*/

#define CM_SEND_SIP_DATA			0x17
/*
	Envia dados raw através de uma ligação SIP(só funciona com ligações K3L para K3L)
	Send raw data throught a SIP call(works only with K3L to K3L calls)

	K3L_COMMAND.Object = ChannelNumber
    Params:
    A pointer to a KSipData structure which has the following members:

    DataLength - Length of data array
    Data       - pointer to the data array to be transmitted
*/

#define CM_SS_TRANSFER              0x18
/*  single step transfer, QSig signaling only!

    Params:
    Events: EV_DISCONNECT | EV_SS_TRANSFER_FAIL
	Params:
	A null terminated ascii string with the call parameters.
	The expected call parameters are:
	Parameter:          		Required:	Type:       Description:
    transferred_to              yes         int string  The number to be transferred to
    await_connect               no          boolean     Await to be connected to the third party, default 0 (false)
*/

#define CM_GET_SMS                  0x19
/* Verifica novas mensagens SMS
   Check for new SMS messages

   K3L_COMMAND.Object = ChannelNumber

   Events: EV_SMS_INFO, EV_SMS_DATA
*/

#define CM_PREPARE_SMS              0x1A
/* Armazena o texto a ser enviado via SMS
   Store SMS text to send

   K3L_COMMAND.Object = ChannelNumber
   K3L_COMMAND.Params = A null terminated ascii string with the text
*/

#define CM_SEND_SMS                 0x1B
/* Envia o ultimo SMS armazenado com CM_PREPARE_SMS
   Send the last SMS stored with CM_PREPARE_SMS

   K3L_COMMAND.Object = ChannelNumber
   K3L_COMMAND.Params = A null terminated ascii string with the SMS params
   Parameter:       Type:           Description:
   sms_to           int             The number to be send
   sms_coding       enum:           The message coding (Optional)
                     iso88591        iso88591 converted to 7-bits GSM alphabet (Default)
                     user            8-bits user-defined alphabet

   Events: EV_SMS_SEND_RESULT
*/

#define CM_SEND_TO_MODEM            0x1C
/* Envia o comando para o Modem
   Send command to Modem

   K3L_COMMAND.Object = ChannelNumber
   K3L_COMMAND.Params = A null terminated ascii string with the AT modem command
*/

#define CM_CHECK_NEW_SMS            0x1D
/* Consulta sobre novos SMSs
   Check for new SMSs arrived

   K3L_COMMAND.Object = ChannelNumber

   Events: EV_NEW_SMS
*/

#define CM_ISDN_SEND_SUBADDRESSES   0x1E
/* Envia os parâmetros de Calling/Called Party Subaddress no SETUP da próxima ligação
   Sends Calling/Called Party Subaddress information elements in next call's SETUP.

   K3L_COMMAND.Object = ChannelNumber
   K3L_COMMAND.Params = A pointer to a KISDNSubaddresses structure 
        containing both Calling and Called Party Subaddress, in the 
        case of one of them is to be missing in the SETUP message, 
        Subaddress Information Length should be set to 0 (zero).

   See:
        CM_MAKE_CALL
        EV_NEW_CALL
        EV_ISDN_SUBADDRESSES

*/

/******************************************************************************
Ativacao de recursos
Resource enabling
******************************************************************************/
#define CM_ENABLE_DTMF_SUPPRESSION	0x30
/*
	Habilita a supressao de DTMF
	Enables dtmf suppression

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_DTMF_SUPPRESSION	0x31
/*	Desabilita a supressao de DTMF
	Disables dtmf suppression

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_AUDIO_EVENTS		0x32
/*	Habilita os eventos de audio.
	Enable audio events.

    K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_AUDIO_EVENTS		0x33
/*	Desabilita os eventos de audio.
	Disable audio events.

    K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_CALL_PROGRESS		0x34
/*	Habilita o call progress automatico.
	Enable automatic call progress.

    K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_CALL_PROGRESS	0x35
/*	Desabilita o call progress automatico.
	Disable automatic call progress.

    K3L_COMMAND.Object = ChannelNumber
*/

#define CM_FLASH					0x36
/*	Flash
	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_PULSE_DETECTION	0x37
/*
	Habilita a detecção de Pulso(decádica)
	Enables pulse detection

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_PULSE_DETECTION	0x38
/*
	Desabilita a detecção de Pulso(decádica)
	Disables pulse detection

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_ECHO_CANCELLER	0x39
/*
	Habilita o cancelamento de eco.
	Enables echo cancelation

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_ECHO_CANCELLER	0x3A
/*
	Desabilita o cancelador de eco.
	Disables echo cancelation.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_AGC				0x3B
/*
	Habilita o controle de ganho automático do canal.
	Enable auto gain contol.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_AGC				0x3C
/*
	Desabilita o controle de ganho automático do canal.
	Disable auto gain contol.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_HIGH_IMP_EVENTS	0x3D
/*
	Habilita eventos e gravação de audio com a linha em alta impedância.
	Enable audio events and record with high impedance line.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_HIGH_IMP_EVENTS	0x3E
/*
	Desabilita eventos e gravação de audio com a linha em alta impedância.
	Disable audio events and record with high impedance line.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_ENABLE_CALL_ANSWER_INFO  0x40
/*
	Desabilita evento de informação de atendimento.
	Disable call answer info event.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_CALL_ANSWER_INFO 0x41
/*
	Habilita evento de informação de atendimento.
	Enable call answer info event.

	K3L_COMMAND.Object = ChannelNumber
*/


/******************************************************************************
Comandos de link e da placa (E1)
Link and board commands (E1)
******************************************************************************/
#define CM_RESET_LINK				0xF1
/*	Reinicia o link inteiro.
	Resets the entire link.

	K3L_COMMAND.Object = LinkNumber
*/

#define CM_CLEAR_LINK_ERROR_COUNTER 0xF2
/*  Clears the error counters for the specified link.
    Zera os contadores de erro para o link indicado.

    K3L_COMMAND.Object = LinkNumber
*/

#define CM_SEND_DEVICE_SECURITY_KEY 0xF3
/*  Usado para liberar placas que estejam utilizando a proteção de integradores
    Used to unlock boards that are using integrator protection.
    
    Params:
    	Pointer to an unsigned integer containing the security key for the board.
    	
    See:
    	EV_REQUEST_DEVICE_SECURITY_KEY
*/

/******************************************************************************
 Eventos enviados da placa ao host
 Device to host sent events
******************************************************************************/

#define EV_CHANNEL_FREE				0x01
/*	O canal está livre para uso
	Channel ready for use
*/

#define EV_CONNECT					0x03
/*	A chamada esta completa e os recursos de audio estao ativos.
	Call completed. Audio resources ready.
*/

#define EV_DISCONNECT				0x04
/*	O assinante B desligou.
	Called part dropped the call.
	
	K3L_EVENT.AddInfo = Disconnect Cause (KQ931Cause if signaling is ISDN, zero otherwise)
*/

#define EV_CALL_SUCCESS				0x05
/*	A chamada terá proceguimento. Na sinalização R2, o campo AddInfo informa a condição de B (Grupo B).
	Call will proceed. In R2 signaling, field AddInfo contains B's condition (Group B).

	K3L_EVENT.AddInfo = Called line condition (if in R2D)

	Obs: Eventos consecutivos: EV_CONNECT (atendimento) ou EV_NO_ANSWER (ninguém atende).
		 Following events: EV_CONNECT (answering) or EV_NO_ANSWER (nobody answers).
*/

#define EV_CALL_FAIL				0x06
/*	A chamada não procederá. O campo AddInfo informa o motivo.
	Call fails and will not proceed. Field AddInfo indicates the given reason.

	K3L_EVENT.AddInfo = Reason of disconnection.

		R2		- Group B (KSignGroupB, KR2D.h)
		ISDN	- ISDN cause (KQ931Cause, KISDN.h)
		Analog 	- Cadence name that has been detected ('A'-'Z')

    Obs: Eventos consecutivos: EV_CHANNEL_FREE
		 Following events:     EV_CHANNEL_FREE
*/

#define EV_NO_ANSWER				0x07
/*	Destino não atende.
	Called does not answer.
*/

#define EV_BILLING_PULSE			0x08
/*	Recebido um pulso de cobrança.
	A billing pulse was received.
*/

#define EV_SEIZE_SUCCESS			0x09
/*	A solicitação de ocupação assincrona foi bem sucedida.
	The async seizure command was successful.
*/

#define EV_SEIZE_FAIL				0x0A
/*	Falhas na ocupação assincrona.
	Async seize fail.

	K3L_EVENT.AddInfo = Fail cause. (enum KSeizeFail)
*/

#define EV_SEIZURE_START			0x0B
/*	Notificao de ocupacao do canal, inicio da sinalizacao.
	Channel seizure notification, begin of signaling.
*/

#define EV_CAS_LINE_STT_CHANGED		0x0C
/*	Notificacao de mudanca no estado da linha (ABCD)
	Line state change notification (ABCD)
*/

#define EV_CAS_MFC_RECV				0x0D
/*	Recebimento de cifra MFC
	MFC digit received
*/

#define EV_NEW_CALL                 0x0E
/*  Notificação de recebimento de chamada
    Call arriving notification

  Params:
		A null terminated ascii string with the call parameters. To easily get the parameters, use
	the k3lGetEventParam function.
	The parameters that could be present are:
	Parameter:					        Protocol:   Description:
	dest_addr		                    ALL         Destiny's number (address)
	orig_addr                           ALL         Caller number (address)
	network_orig_addr                   IP          Caller SIP network address
	r2_categ_a                          R2          Caller Category
	isdn_orig_type_of_number            ISDN        Caller type of number (KQ931TypeOfNumber, KISDN.h)
	isdn_orig_numbering_plan            ISDN        Caller numbering plan (KQ931NumberingPlan, KISDN.h)
	isdn_orig_presentation              ISDN        Caller presentation indicator (KQ931PresentationIndicator, KISDN.h)
    isdn_orig_screening                 ISDN        Caller screening indicator (KQ931ScreeningIndicator, KISDN.h)
	isdn_dest_type_of_number            ISDN        Destiny's type of number
	isdn_dest_numbering_plan            ISDN        Destiny's numbering plan
	isdn_redirecting_number             ISDN        "TRUE" if call is being redirected
	isdn_reverse_charge                 ISDN        "TRUE" if call is reverse charge (collect call)
    isdn_has_subaddresses               ISDN        "TRUE" if call has subaddresses information (will be
                                                    sent in EV_ISDN_SUBADDRESSES)
*/

#define EV_USER_INFORMATION         0x0F
/*  Notificação de dados de usuário recebidos pelo HDLC em uma ligação RDSI.
    User information received by HDLC in ISDN call.

    Params:
        A pointer to a KUserInformation structure, containing information and the data itself.

*/

#define EV_DIALED_DIGIT             0x10
/*  Recebido digito de discagem
    Dialing digit received
   
    K3L_EVENT.AddInfo = Digit (ascii)
*/

#define EV_SIP_REGISTER_INFO        0x11
/*  Notificação de conclusao de registro
    Register conclusion notification

  Params:
        A null terminated ascii string with the registration parameters.
    The expected call parameters are:
    Parameter:          		Description:
    user                   		User name of registration
    proxy                  		Registration proxy
	unregister          		Signal that this the event represents a response to an user unregistration

    K3L_EVENT.AddInfo = SIP Response. ( enum KSIP_Failures )
*/

#define EV_RING_DETECTED            0x12
/*  Recebido ring em ramal analógico
    Ringing received on analog branch

    Only Available when High Impedance Events are disabled.

    See:
        CM_ENABLE_HIGH_IMP_EVENTS
        CM_DISABLE_HIGH_IMP_EVENTS
*/

#define EV_ISDN_SUBADDRESSES        0x13
/*  Chamada RDSI possuia informação de Subaddress na mensagem  SETUP
    ISDN Call has subadress information in SETUP message

   Params:
        A pointer to a KISDNSubaddresses structure containing both
        Calling and Called Party Subaddress, in the case of one of
        them is missing in the SETUP message, Subaddress Information
        Length will be set to 0 (zero).

   See:
        EV_NEW_CALL

*/

#define EV_CALL_HOLD_START			0x16
/*  Indica se o canal entrou em espera
    Indicates if a channel is being held
*/

#define EV_CALL_HOLD_STOP			0x17
/*  Indica se o canal saiu da espera
    Indicates if a channel left hold state
*/

#define EV_SS_TRANSFER_FAIL         0x18
/*  Falha na transferencia do QSig!
    QSig Single Step Transfer Fail   
*/

#define EV_FLASH                    0x19
/*  Flash detectado
    Flash detected
*/

#define EV_ISDN_PROGRESS_INDICATOR	0x1A
/*  Recebimento do elemento de informação Progress Indicator
	em canal RDSI.
	Progress Indicator information element received in a
	ISDN channel.

	K3L_EVENT.AddInfo = Progress Description (KQ931ProgressIndication, KISDN.h)

*/

#define EV_DTMF_DETECTED			0x20
/*	Detectado dígito DTMF.
	DTMF digit detected.

	K3L_EVENT.AddInfo = Detected digit (ascii)
*/

#define EV_DTMF_SEND_FINISH			0x21
/*	Fim da discagem DTMF
	DTMF dial complete
*/

#define EV_AUDIO_STATUS				0x22
/*	Mudança no estado do audio do canal.
	Audio status changed.

	K3L_EVENT.AddInfo = Audio detected. (enum KMixerTone)
*/

#define EV_CADENCE_RECOGNIZED		0x23
#define EV_CALL_PROGRESS			EV_CADENCE_RECOGNIZED
/*	Deteccao de uma cadencia no call progress.
	Cadenced detected by automatic call progress.

	K3L_EVENT.AddInfo = Cadence index defined in config. file
*/

#define EV_END_OF_STREAM			0x24
/*	Encontrado fim de arquivo ou de buffer na reprodução de áudio.
	End of file or end of buffer reached during a play.
*/

#define EV_PULSE_DETECTED			0x25
/*	Detectado dígito decádico.
	PULSE digit detected.

	K3L_EVENT.AddInfo = Detected digit (ascii)
*/

#define EV_POLARITY_REVERSAL		0x26
/*  Detectada inversao de polaridade
	Polarity reversal detected
*/

#define EV_CALL_ANSWER_INFO     	0x27
/*  Indicates information about the call after connection
	Indica o informações sobre a chamada quando ela é completada
    Obs.: the indications are not precise in some cases
          as indicações não são precisas em alguns casos

    K3L_EVENT.AddInfo = Call Info ( enum KCallStartInfo );
*/

#define EV_COLLECT_CALL          	0x28
/*  Indicates an incoming collect call
	Indica um ligação a cobrar entrante
*/

#define EV_SIP_DTMF_DETECTED        0x29
/*	Detectado dígito DTMF out band em canais SIP(RFC2833/SIP EVENT).
	Out  band DTMF digit detected in SIP channel(RFC2833/SIP EVENT).

	K3L_EVENT.AddInfo = Detected digit (ascii)
*/

#define EV_SIP_DATA			        0x2A
/*	Indica recepção de pacotes de dados enviado com o comando CM_SEND_SIP_DATA.
	Indicates reception of data sent by CM_SEND_SIP_DATA command.

    Params:
    A pointer to a KSipData structure which has the following members:

    DataLength - Length of data array
    Data       - pointer to the data array
*/


#define EV_RECV_FROM_MODEM			0x42
/* Eventos recebidos do modem
   Events received from modem

   K3L_EVENT.Params = A null terminated string with the events from modem
*/

#define EV_NEW_SMS                  0x43
/* Indica a chegada de um novo SMS
   Indicates a new SMS arrived
*/

#define EV_SMS_INFO                 0x44
/* Recebido mensagem SMS
   Received SMS message

   K3L_EVENT.Params = A null terminated string with the sms parameters
    Parameter:      Type    		Description:
     sms_from       string            The number of sender
     sms_date       string            Date that the message has been send
     sms_size       int            Message size
     sms_coding     enum:           The message alphabet
                     iso88591        7-bits GSM alphabet converted to iso88591
                     user            8-bits user-defined alphabet
*/

#define EV_SMS_DATA                 0x45
/* Recebido mensagem SMS
   Received SMS message

   K3L_EVENT.Param = A null terminated string with the sms body
*/

#define EV_SMS_SEND_RESULT          0x46
/* Indica o resultado do envio de SMS
   Indicates de result of SMS send

   K3L_EVENT.AddInfo = Result code (KGsmCallCause)
*/

/******************************************************************************
Erros
Errors
******************************************************************************/
#define EV_CHANNEL_FAIL				0x30
/*	Falhas relacionadas ao sistema.
	System related fails.

	K3L_EVENT.AddInfo = Fail cause
		R2		- KChannelFailCause, KErrorDefs.h
		ISDN	- KQ931Cause, KISDN.h
*/

#define EV_REFERENCE_FAIL			0x31
/*	Referncia de rede da mestre secundaria perdida.
	Secundary master network reference lost.
*/

#define EV_INTERNAL_FAIL			0x32
/*	Falha do sistema.
	System internal fail.

	K3L_EVENT.AddInfo = Fail cause. (enum KInternalFail)
*/

#define EV_HARDWARE_FAIL			0x33
/*	Usado apenas internamente.
	Internal use only.
*/

#define EV_LINK_STATUS				0x34
/*	Indica periodicamente variação nos contadores de erros dos links, se ocorrerem.
    Indicates periodically link errors counters variations, if it did happens.

	K3L_EVENT.AddInfo = Cause (enum KE1Status)
    K3L_EVENT.ObjectId = koiLink
*/

#define EV_PHYSICAL_LINK_UP			0x35
/*	Estabelecimento do link E1
	Link up

    K3L_EVENT.AddInfo = Cause (enum KE1Status)
    K3L_EVENT.ObjectId = koiLink
*/

#define EV_PHYSICAL_LINK_DOWN		0x36
/*	Queda do link
	Link down

	K3L_EVENT.AddInfo = Cause (enum KE1Status)
    K3L_EVENT.ObjectId = koiLink
*/


#define EV_CLIENT_RECONNECT		0xF0
/* Indica que a api cliente reconectou automaticamente com sucesso na api servidora
   após uma falha de comunicação.

   Signs that client api successfully connected to the server api after a communication
   failure.
*/

#define EV_CLIENT_AUDIOLISTENER_TIMEOUT     0xF1
/* Indica que a callback registrada para a audio listener foi desativada automaticamente após ficar 3 segundos sem resposta.
   (somente quando a interface de buffers assincronos não está sendo utilizada)

   Signs that a registered audio listener callback was automaticaly deactivated due to wait 3 seconds without response.
   (only when assynchronous buffers interface is not used)
*/

#define EV_CLIENT_BUFFERED_AUDIOLISTENER_OVERFLOW   0xF2
/* Indica que o buffer da callback de áudio chegou ao limite e vai descartar alguns dados de áudio.
   (somente para interface de buffers assincrona)

   Signs that the audio callback buffer overflowed and will discard some audio samples.
   (assynchronous buffers interface only)
*/

#define EV_REQUEST_DEVICE_SECURITY_KEY 0xF3
/* Indica que a placa está usando a proteção dos integradores
   Indicates that the board is using the integrator protection.
   
   Params:
   		A NULL terminated string the should be use to send the device security key.
   		
   See:
   		CM_SEND_DEVICE_SECURITY_KEY

*/

/******************************************************************************
Compatibilidade
Compatibility
******************************************************************************/
#define CM_SEND_DTMF				0xD1
/*	EM DESUSO: Usar CM_DIAL_DTMF
    Envia um dígito de discagem (DTMF).
    DEPRECATED: Use CM_DIAL_DTMF instead
	Sends a dial digit (DTMF).

	K3L_COMMAND.Object = ChannelNumber

    Events: EV_DTMF_SEND_FINISH

  Params:
	[0] - Ascii of the digit
*/

#define CM_STOP_AUDIO 				0xD2
/*  EM DESUSO: Usar CM_STOP_PLAY ou CM_STOP_RECORD
	Para a gravção do canal.
	DEPRECATED: Use CM_STOP_PLAY or CM_STOP_RECORD instead
	Stops channel recording.

	K3L_COMMAND.Object = PlayerNumber/ChannelNumber

  Params:
	[0] - 1 StopPlay (kTx)| 2 StopRx (kRx) | 3 StopBoth (kBoth)
*/

#define CM_HARD_RESET				0xF0
/*	Reinicia uma placa.
	Restarts a board.

	K3L_COMMAND.Object = none
*/

#define EV_VOIP_SEIZURE					0x40
/*  EM DESUSO: Usar EV_NEW_CALL
    Foi solicitada uma ocupação de entrada.
    DEPRECATED: Use EV_NEW_CALL instead
	There is a incoming seize in a network resource.

   
  Params:
	[0..60] - DNIS (dialed number, ascii, null terminated)
	[61..122] - ANI (dialer number, ascii, null terminated)
*/

#define EV_SEIZURE					0x41
/*  EM DESUSO: Usar EV_NEW_CALL  
	Foi solicitada uma ocupação de entrada.
	DEPRECATED: Use EV_NEW_CALL instead
	There is a incoming seize in a network resource.

    K3L_EVENT.AddInfo = Category of A (R2)

  Params:
	[0..20] - DNIS (dialed number, ascii, null terminated)
	[21..42] - ANI (dialer number, ascii, null terminated)
*/

#endif

