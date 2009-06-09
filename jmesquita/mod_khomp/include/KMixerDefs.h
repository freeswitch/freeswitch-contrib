/******************************************************************************
Comandos de controle do mixer
Mixer control commands
******************************************************************************/

#define CM_MIXER					0x60
/*	Envia um comando para o mixer associado ao canal.
	Sends a command to a channel associated mixer.
	
	K3L_COMMAND.Object = MixerNumber

  Params:
  	A pointer to a KMixerCommand strutcture which has the following members
  	
	Track - Track index
	Source - Audio source (KMixerSource, k3l.h)
	SourceIndex - Channel number or generator code

*/

#define CM_CLEAR_MIXER				0x61
/*	Coloca todas as trilhas do mixer em silêncio.
	Sets all mixer tracks to silent.
	
	K3L_COMMAND.Object = MixerNumber
*/

#define CM_PLAY_FROM_FILE			0x62
/*	Reproduz um arquivo de audio
	Play an audio file
	
	K3L_COMMAND.Object = PlayerNumber

    Events: EV_END_OF_STREAM

  Params: 
	FileName (null terminated)
*/

#define CM_RECORD_TO_FILE			0x63
/*	Inicia gravaçao de audio em arquivo
	Starts audio recording in file

	K3L_COMMAND.Object = PlayerNumber

  Params: 
	FileName (null terminated)
*/	

#define CM_PLAY_FROM_STREAM			0x64 
/*	Reproduz um buffer de audio
	Play an audio buffer
	
	K3L_COMMAND.Object = PlayerNumber

    Events: EV_END_OF_STREAM

  Params: 
    struct KPlayFromStreamCommand
	Buffer     - Pointer to the buffer
	BufferSize - Buffer size
*/

#define CM_INTERNAL_PLAY			0x65 
/*	Used internally
*/

#define CM_STOP_PLAY				0x66
/*	Para o reprodutor.
	Stops player.

	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_STOP_RECORD				0x67
/*	Para a gravção do canal.
	Stops channel recording.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_PAUSE_PLAY				0x68
/*	Pausa a reproducao de audio de um player
	Pauses a player

    K3L_COMMAND.Object = PlayerNumber
*/

#define CM_PAUSE_RECORD				0x69
/*	Pausa a gravacao de audio de um canal
	Pauses recording of a channel

    K3L_COMMAND.Object = ChannelNumber
*/

#define CM_RESUME_PLAY				0x6A
/*	Continua um play anteriormente pausado
	Resumes a paused play

	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_RESUME_RECORD			0x6B
/*	Continua um play anteriormente pausado
	Resumes a paused play

	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_INCREASE_VOLUME			0x6C
/*	Aumenta o volume na saida do mixer
	Increases volume on mixer out

	K3L_COMMAND.Object = MixerNumber
*/

#define CM_DECREASE_VOLUME			0x6D
/*	Diminui o volume na saida do mixer
	Decreases volume on mixer out

	K3L_COMMAND.Object = MixerNumber
*/

#define CM_LISTEN					0x6E
/*	Inicia o processo de transmissao de audio para reconhecimento de voz.
	Starts audio transmission process for voice recognition.

	K3L_COMMAND.Object = MixerNumber

  Params:
	[0..3] - Inteiro contendo a quantidade de milisegundos de audio em chamada da callback
*/

#define CM_STOP_LISTEN				0x6F
/*	Para o processo de transmissao de audio e para a gravacao do buffer de guarda.
	Stops audio tranmission process, and stops the buffer guard recoring..

	K3L_COMMAND.Object = MixerNumber
*/

#define CM_PREPARE_FOR_LISTEN		0x70
/*	Inicia a gravacao do buffer de guarda.
	Start guard buffer recording.

	K3L_COMMAND.Object = MixerNumber

  Params:
	[0..3] - Inteiro contendo o tamanho do buffer de guarda em milisegundos
*/

#define CM_PLAY_SOUND_CARD			0x71
/*	Inicia o play de canal na placa de som
	Starts the sound card channel player
	
	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_STOP_SOUND_CARD			0x72
/*	Para o play de canal na placa de som
	Stops the sound card channel player
	
	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_MIXER_CTBUS				0x73
/*	Envia um comando para o mixer associado ao de interconexao. Soh para placas com CTbus
	Sends a command to a inteconnect channel associated mixer. Only for boards with CTbus
	
	K3L_COMMAND.Object = CtbusMixerNumber

  Params:
	[0] - Mixer index
	[1] - Audio source
	[2] - Channel number or generator code

	kmsChannel - Channel
	kmsPlay - Play
	kmsGenerator - Signal
	kmsCTbus - another mixer channel
*/


#define CM_PLAY_FROM_STREAM_EX			0x74 
/*	Reproduz um buffer de audio
	Play an audio buffer
	
	K3L_COMMAND.Object = PlayerNumber

    Events: EV_END_OF_STREAM

  Params: 
    struct KPlayFromStreamCommand
	Buffer     - Pointer to the buffer
	BufferSize - Buffer size
	CodecIndex - 0 = 8Khz ALAW, 1 = 8Khz PCM, 2 = 11khz PCM
*/

#define CM_INTERNAL_PLAY_EX			0x75 
/*	Used internally
*/

#define CM_ENABLE_PLAYER_AGC		0x76
/*
	Habilita o controle de ganho automático do player.
	Enable player automatic gain contol.

	K3L_COMMAND.Object = ChannelNumber
*/

#define CM_DISABLE_PLAYER_AGC		0x77
/*
	Desabilita o controle de ganho automático do player.
	Disable player automatic gain contol.

	K3L_COMMAND.Object = ChannelNumber
*/


#define CM_START_STREAM_BUFFER		0x78
/*	Inicia um play de buffer contíuo
	Start continuous play buffer
	
	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_ADD_STREAM_BUFFER		0x79
/*	Adiciona dados a um stream de buffer contínuo
	Add data to a continuous play buffer
	
	K3L_COMMAND.Object = PlayerNumber

  Params: 
	[0..3] - Pointer to the buffer
	[4..7] - Buffer size
*/

#define CM_STOP_STREAM_BUFFER		0x7A
/*	Para um play de buffer contínuo
	Stop a continuous play buffer
	
	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_SEND_BEEP				0x7B
/*	Envia um bip no canal
	Sends a beep

	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_SEND_BEEP_CONF			0x7C
/*	Envia um bip na conferencia
	Sends a beep

	K3L_COMMAND.Object = PlayerNumber
*/

#define CM_ADD_TO_CONF				0x7D
/*	Adiciona um canal a uma conferencia
	Adds a channel to a specified conference

	K3L_COMMAND.Object = MixerNumber
  Params:
	[0] - Conference index
*/

#define CM_REMOVE_FROM_CONF			0x7E
/*	Retira um canal de uma conferencia
	Adds a channel to a specified conference

	K3L_COMMAND.Object = MixerNumber
*/

#define CM_RECORD_TO_FILE_EX		0x7F
/*	Inicia gravaçao de audio em arquivo, utilizando codec específico
	Starts audio recording in file, using specific codec

	K3L_COMMAND.Object = PlayerNumber

	Params:
	A null terminated ascii string with the parameters.
	The expected command parameters are:
	Parâmeter:    Description:
	file_name     File name for use in recording
	codec         Codec index(defined in KCodecIndex enum) for audio compression
                  if ommited ALaw 8khz will be used
*/

#define CM_SET_VOLUME				0xA0
/*	Configura o volume de saída do mixer para um valor na vaixa de -10 a +10. Zero reseta o volume.
	Sets output volume from mixer channel to a values between -10 and +10. Zero resets the volume.

	K3L_COMMAND.Object = MixerNumber

	Params:
	A null terminated ascii string with the parameters.
	The expected command parameters are:
	Parâmeter:    Description:
	volume        File name for use in recording
	type          Defines if is the "input" or "output" volume to be changed
*/

#define CM_START_CADENCE			0xA1
/*	Inicia a reprodução da cadência especificada com tom de 425Hz.
	Starts to play the specified 425Hz tone cadence.

	K3L_COMMAND.Object = MixerNumber

	Params:
	A null terminated ascii string with the parameters.
	The expected command parameters are:
	Parâmeter:			Description:
	cadence_times		A comma separated list of times (in milliseconds) for dialtone 
						and silence (in pairs), or "continuous" for continuous dialtone.
	mixer_track			The mixer track that sould be used to play the cadence. "0" (zero)
						is the default mixer track.
*/

#define CM_STOP_CADENCE				0xA2
/*	Pára a cadência iniciada com o comando CM_START_CADENCE.
	Stops the cadence started with CM_START_CADENCE.

	K3L_COMMAND.Object = MixerNumber
*/
