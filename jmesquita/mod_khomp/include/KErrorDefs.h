/******************************************************************************
Erros e falhas
Errors and fails
******************************************************************************/
// EV_CHANNEL_FAIL
   #define FC_REMOTE_FAIL			0x01
/*	Falha remota.
	Remote fail.
*/
   #define FC_LOCAL_FAIL			0x02
/*	Falha local.
	Local fail.
*/ 
   #define FC_REMOTE_LOCK			0x03
/*	Canal bloqueado remotamente.
	Channel remotely locked.
*/  
   #define FC_LINE_SIGNAL_FAIL		0x04
/*	Falha na sinalização de linha.
	Line signaling fail.
*/
   #define FC_ACOUSTIC_SIGNAL_FAIL	0x05
/*	Falha na sinalização de registro.
	Register signaling fail.
*/

enum KChannelFail
{
	kfcRemoteFail = FC_REMOTE_FAIL,
	kfcLocalFail = FC_LOCAL_FAIL,
	kfcRemoteLock = FC_REMOTE_LOCK,
	kfcLineSignalFail = FC_LINE_SIGNAL_FAIL,
	kfcAcousticSignalFail = FC_ACOUSTIC_SIGNAL_FAIL
};

// EV_INTERNAL_FAIL
   #define ER_INTERRUPT_CTRL		0x01
/*  Falha no controlador de interrupções
	Interrupt controller fail
*/
   #define ER_COMMUNICATION_FAIL	0x02
/*  Falha na comunicação com a interface
	Interface communication fail
*/
   #define ER_PROTOCOL_FAIL			0x03
/*	Falha no tratamento do protocolo de comunicação
	Protocol fail
*/
   #define ER_INTERNAL_BUFFER		0x04
/*  Erro interno na API
	API internal error
*/
	#define ER_MONITOR_BUFFER		0x05
/*	Erro no buffer de monitoração
	Monitor buffer error 
*/
	#define ER_INITIALIZATION		0x06
/*	Falha na inicialização do sistema
	Initialization fail
*/
	#define ER_INTERFACE_FAIL		0x07
/*	A interface nao responde
	Interface is dead
*/
	#define ER_CLIENT_COMM_FAIL		0x08
/*	Falha do cliente na comunicacao cliente/servidor
	Client fails in client/server communication
*/
	#define ER_POLL_CTRL			0x09
/*	Falha no controle de polling
	Polling control fail
*/
	#define ER_EVT_BUFFER_CTRL		0x0A
/*	Falha da API no controle de buffer circular de eventos
	Circular buffer event control fails
*/

	#define ER_INVALID_CONFIG_VALUE 0x0B
/*	Valor invalido em arquivo de configuracao
	Invalid configuration value
*/

	#define ER_INTERNAL_GENERIC_FAIL 0x0C
/*	Falha generica. Dados sobre o erro encontrados nos logs
	Generic fail. See logs for more information
*/

enum KInternalFail
{
	kifInterruptCtrl = ER_INTERRUPT_CTRL,
	kifCommunicationFail = ER_COMMUNICATION_FAIL,
	kifProtocolFail = ER_PROTOCOL_FAIL,
	kifInternalBuffer = ER_INTERNAL_BUFFER,
	kifMonitorBuffer = ER_MONITOR_BUFFER,
	kifInitialization = ER_INITIALIZATION,
	kifInterfaceFail = ER_INTERFACE_FAIL,
	kifClientCommFail = ER_CLIENT_COMM_FAIL
};	

// EV_SEIZE_FAIL
   #define FS_CHANNEL_LOCKED		0x01
/*	Ocupação negada por bloqueio de saída.
	The channel is locally locked for outgoing calls.
*/
   #define FS_INCOMING_CHANNEL		0x02
/*	Canal liberado somente para entrada.
	Channel is only for income calls.
*/
   #define FS_CHANNEL_NOT_FREE		0x03
/*	Canal ocupado ou com falha.
	Channel busy.
*/
   #define FS_DOUBLE_SEIZE			0x04
/*	Dupla ocupação (canal recebeu uma ocupação ao invés de uma confirmação).
	Double seize (channel receives a seize instead of a seize confirmation).
*/
   #define FS_LOCAL_CONGESTION		0x06
/*	Congestionamento local
	Local congestion
*/
   #define FS_NO_DIAL_TONE			0x07
/*	Sem tom de discagem (utilizado na analogica)
	No dial tone (analog lines)
*/
enum KSeizeFail
{
	ksfChannelLocked = FS_CHANNEL_LOCKED,
	ksfIncomingChannel = FS_INCOMING_CHANNEL,
	ksfChannelBusy = FS_CHANNEL_NOT_FREE,
	ksfDoubleSeizure = FS_DOUBLE_SEIZE,
	ksfCongestion = FS_LOCAL_CONGESTION,
	ksfNoDialTone = FS_NO_DIAL_TONE
};
