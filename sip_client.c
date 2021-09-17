/*
 * sip_client.c
 *
 *  Created on: 14 янв. 2021 г.
 *      Author: lenovo
 */
#include "sip_client.h"

#include "uip.h"
#include "myulog.h"
#include "mymore.h"
#include "settings.h"

#include <string.h>
#include <stdlib.h>

#include "resolv.h"
#include "xprintf.h"
#include "g711.h"

#include "speaker.h"
#include "uip_arp.h"

#include <string.h>

#include "main.h"

#include "lwrb.h"
#include "audio.h"
#include "mygoertzel.h"

struct uip_udp_conn *sip_conn = NULL; //соединение по sip
struct uip_udp_conn *sip_rtp_conn = NULL; //соединение по rtp
uint32_t sip_rtp_session_id = 0; //номер RTP сессии
unsigned short sip_rtp_wSeqNum = 0; //номер пакета в RTP сессии
uint32_t sip_regiser_tick = 0; //таймер для регистрации
uint32_t sip_time_register = 45000; //врмемя перерегистрации по умолчанию
char sip_invite_uri[MAX_SIP_URI_SIZE] = {0}; //здесь находиться пользователь которого мы вызыаем (44259@df.feelinhome.ru)
bool SendAutRegisterRequest = FALSE; //от зацикливания
bool SendAutInviteRequest = FALSE; //от зацикливания
uint32_t sip_rtp_session_tick = 0; //таймер на таймоут RTP сессии
bool sip_rtp_marker = FALSE; //маркер начала RTP пакетов
uint32_t stp_rtp_time_stamp_begin = 0; //таймстамп RTP пакета
unsigned char rtp_codec = 0; //текущий кодек RTP
bool was_registration = FALSE; //была регистрация


SIPCALL Call = {0}; //данные для вызова


static char sip_buf[MAX_SIP_BUF_SZ]; //буфер где формируется сообщения SIP

//приемный буфер RTP пакетов
static char sip_rtp_dac_buf_data[AUDIO_BUF_SZ + MAX_RTP_DATA_PAYLOAD_SZ];
lwrb_t sip_rtp_dac_buf;


///буфер АЦП
uint16_t sip_rtp_adc_buf[2][MAX_RTP_DATA_PAYLOAD_SZ];
//проигранный буфер АЦП
int sip_rtp_adc_buf_fill = -1;
//аборт вызова
bool sip_invite_abort = FALSE;
//идет сип вызов
uint8_t sip_state = SIP_STATE_DEFAULT;

//GOERTZEL_COEFF goertzel_coeff_941, goertzel_coeff_1336;
//коэффицент DTMF
//uint16_t sip_dtmf = 7;
//кол-во точек для герцеля AUDIO_BUF_SZ / GOERTZEL_SPLIT
//#define GOERTZEL_SPLIT	5

int tmp_rcv_rtp_counter = 0;
int tmp_snd_rtp_counter = 0;
//int tmp_magnitude_941 = 0;
//int tmp_magnitude_1336 = 0;

//-------------------------------------------------------------------------------------
// -----------------------------------  Ф-ции -----------------------------------------
//-------------------------------------------------------------------------------------

//посылка UDP пакета через UDP соединение c
void uip_udp_send_my(struct uip_udp_conn *c, const void *data, int len)
{
	int max_size = UIP_BUFSIZE - (UIP_LLH_LEN + UIP_IPUDPH_LEN) - 2;
	if (len > max_size) len = max_size;
	uip_udp_conn = c; // set your connection
	uip_slen = len; // set the length of data to send
	memcpy(&uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN], data, len); // copy to the buffer
	uip_process(UIP_UDP_SEND_CONN); // tell uip to construct the package
	uip_arp_out(); // attack Ethernet header
	enc28j60PacketSendMy(UIP_LLH_LEN + UIP_IPUDPH_LEN + len, uip_buf);
	uip_len = 0;
}

//преобразование массива АЦП 16 бит в LAW 8 данные
uint16_t Law_Encode16(int8_t * dst, uint16_t dst_size, uint16_t *src, uint16_t src_size, unsigned char codec)
{
	uint16_t i;
	uint16_t size = MIN(dst_size, src_size);
	float volume_sip_adc = (float)settings.volume_sip_adc / 10.0; //от 0.0 до 2.5

	if (codec == 8) {//a-Law
		for (i = 0; i < size; ++i) {
			//dst[i] = ALaw_Encode((int16_t)src[i]  - 3102); //надо подобрать
			dst[i] = ALaw_Encode( (int16_t)(volume_sip_adc * ((float)src[i]  - 3102.0)) );
		}
	} else { //m-Law (0)
		for (i = 0; i < size; ++i)  {
			//dst[i] = MuLaw_Encode((int16_t)src[i]  - 3102);
			dst[i] = MuLaw_Encode( (int16_t)(volume_sip_adc * ((float)src[i]  - 3102.0)) );
		}
	}
	return size;
}


//преобразование массива Law 8 бит в 16 битные АЦП данные
uint16_t Law_Decodes16(uint16_t * dst, uint16_t dst_size, char *src, uint16_t src_size, unsigned char codec)
{
	uint16_t i;
	uint16_t size = MIN(dst_size, src_size);
	float volume_sip_dac = (float)settings.volume_sip_dac / 10.0; //от 0.0 до 2.5

	if (codec == 8) {//a-Law
		for (i = 0; i < size; ++i) {
			//dst[i] = 2048 + ALaw_Decode(src[i]) / 4; //ALaw_Decode от -4032 до +4032
			dst[i] =  (int16_t)(volume_sip_dac * (float)(2048 + ALaw_Decode(src[i]) / 4)); //ALaw_Decode от -4032 до +4032
		}
	} else { //m-Law (0)
		for (i = 0; i < size; ++i) {
			//dst[i] = 2048 + MuLaw_Decode(src[i]) / 8; //MuLaw_Decode от −8159 до +8158
			dst[i] =  (int16_t)(volume_sip_dac * (float)(2048 + MuLaw_Decode(src[i]) / 8)); //MuLaw_Decode от −8159 до +8158;
		}
	}
	return size;
}

//-------------------------------------------------------------------------------------
// ----------------  СИП события RTP на АЦП -----------------------------------------
//-------------------------------------------------------------------------------------

//заполняем хидер rtp
void sip_rtp_data_fill_header(PRTPHEADER prtpheader)
{
	prtpheader->cVersion = 128;
	if (sip_rtp_marker) {
		prtpheader->cPayloadType = 0x80 | rtp_codec ;
		sip_rtp_marker = FALSE;
	}
	else prtpheader->cPayloadType = rtp_codec;
	++sip_rtp_wSeqNum;
	prtpheader->wSeqNum = htons(sip_rtp_wSeqNum);
	stp_rtp_time_stamp_begin += 160;
	prtpheader->TimeStamp = htonl(stp_rtp_time_stamp_begin);
	prtpheader->SyncId  =  htonl(sip_rtp_session_id);
}

//вызывается в sip_task постоянно елси есть rtp сессия
void sip_rtp_adc_send_packet(void)
{
	RTPDATA rtpdata;

	if (sip_rtp_adc_buf_fill != -1) {

    	sip_rtp_data_fill_header(&rtpdata.header);

    	Law_Encode16(rtpdata.Payload, MAX_RTP_DATA_PAYLOAD_SZ, sip_rtp_adc_buf[sip_rtp_adc_buf_fill], MAX_RTP_DATA_PAYLOAD_SZ, rtp_codec);

    	uip_udp_send_my(sip_rtp_conn, &rtpdata, sizeof(RTPDATA));

		sip_rtp_adc_buf_fill = -1;

    	tmp_snd_rtp_counter += MAX_RTP_DATA_PAYLOAD_SZ;
	}
}




//-------------------------------------------------------------------------------------
// ----------------  СИП события RTP на ЦАПЕ -----------------------------------------
//-------------------------------------------------------------------------------------



//загрузить буфер ЦАП данными
void sip_rtp_dac_audio_fill(void)
{

	int index = -1;


	if (audio_buf_play_index > -1) { //воспроизводим буфер
		if (audio_buf_fill[1 - audio_buf_play_index] == FALSE) index = 1 - audio_buf_play_index;
	} else {
		if (audio_buf_fill[0] == FALSE) index = 0;
		else if (audio_buf_fill[1] == FALSE) index = 1;
	}


	if (index != -1) {

		char law_buf_data[AUDIO_BUF_SZ];

		size_t l = lwrb_get_full(&sip_rtp_dac_buf);
		if (l >= AUDIO_BUF_SZ) {
			lwrb_read(&sip_rtp_dac_buf, law_buf_data, AUDIO_BUF_SZ);
			Law_Decodes16(audio_buf[index], AUDIO_BUF_SZ, law_buf_data, AUDIO_BUF_SZ, rtp_codec);
			audio_buf_fill[index] = TRUE;
		}

	}


}



//сюда приходит rtp пакет
void sip_rcv_rtp_packet(char * data,  uint16_t data_len)
{
	set_tick(&sip_rtp_session_tick); //обновим таймер на таймоут сессии
    if (GPIO_READ_PIN(MUTE_GPIO_Port, MUTE_Pin) == GPIO_PIN_SET) MUTE_0;
    if (GPIO_READ_PIN(CD_GPIO_Port, CD_Pin) == GPIO_PIN_SET) CD_0;

    //HAL_GPIO_TogglePin(PA14_GPIO_Port, PA14_Pin);



	char * rtp_data =  data +  sizeof(RTPHEADER);
	uint16_t  rtp_data_len = data_len - sizeof(RTPHEADER);

	PRTPHEADER header = (PRTPHEADER)data;
	if ( (header->cPayloadType != 0) && (header->cPayloadType != 8) && (header->cPayloadType != 0x80) && (header->cPayloadType != 0x88) ) {
		uint8_t * pc = (uint8_t *)rtp_data;
		ulog_fmt("sip: PayloadType=%d rtp_data_len=%d (%d %d %d %d)\r\n", header->cPayloadType, rtp_data_len, pc[0], pc[1], pc[2], pc[3]);
		if ( (header->cPayloadType == 101) && (pc[0] == 0) ) sip_open_door(); //dtmf
		return;
	}


	lwrb_write(&sip_rtp_dac_buf, rtp_data, rtp_data_len);

    tmp_rcv_rtp_counter += rtp_data_len;
}

//-------------------------------------------------------------------------------------
// ----------------  СИП события ------------------------------------------------------
//-------------------------------------------------------------------------------------


//ответ по СИП протоколу
void sip_answer(int len, char * log)
{
	memcpy(uip_appdata, sip_buf, len);
  	uip_udp_send(len);
  	if (log) {
  		ulog(log);
  	} else {
  		sip_buf[len] = '\0';
  		ulog_fmt("sip: snd_answer (%d):\r\n", len);
  		ulog(sip_buf);
  		ulog("\r\n");
  	}
}



//создание RTP сессии
bool create_rtp_session(unsigned long RemoteRtpAddress, unsigned short RemoteRtpPort)
{

	if (sip_rtp_conn != NULL) {

		if ( (sip_rtp_conn->rport == HTONS(RemoteRtpPort)) && (memcmp(&sip_rtp_conn->ripaddr, &RemoteRtpAddress, 4) == 0) ) return TRUE;
		remove_rtp_session();
	}

	sip_rtp_conn = uip_udp_new((uip_ipaddr_t *)&RemoteRtpAddress, HTONS(RemoteRtpPort));
	if (sip_rtp_conn == NULL) {
		ulog("sip: can`t create sip_rtp_conn\r\n");
		return FALSE;
	}

	uip_udp_bind(sip_rtp_conn, HTONS(Call.LocalRtpPort));


	sip_rtp_session_id = 1 + RANDOM(10000);
	sip_rtp_wSeqNum = 1;


    memset(&sip_rtp_adc_buf, 0, sizeof(sip_rtp_adc_buf));
    //memset(&sip_rtp_dac_buf, 0, sizeof(sip_rtp_dac_buf));
    lwrb_reset(&sip_rtp_dac_buf);

    audio_buf_play_index = -1;
    audio_buf_fill[0] = FALSE;
    audio_buf_fill[1] = FALSE;


    sip_rtp_adc_buf_fill = -1;

	HAL_TIM_Base_Start_IT(&htim3);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&sip_rtp_adc_buf[0], MAX_RTP_DATA_PAYLOAD_SZ * 2);

	set_tick(&sip_rtp_session_tick);


	MUTE_0;
	CD_0;


	stp_rtp_time_stamp_begin = 0;

	uint16_t prescaler;
	uint16_t counter_period;

	get_stm32_tim_freq_param(72000000, 8000, &prescaler, &counter_period); //частоту таймера на 8кГц

	TIM6->PSC = prescaler - 1;
	TIM6->ARR = counter_period - 1;
	TIM6->EGR |= TIM_EGR_UG;

	sip_rtp_marker = TRUE;

	ulog_fmt("sip: new rpt session remote_ip=%s, remote_port=%d, local_port=%d\r\n", get_ip_str((unsigned char *)&RemoteRtpAddress), RemoteRtpPort, Call.LocalRtpPort);
	return TRUE;
}


//удаление RTP сессии
void remove_rtp_session(void)
{
	ulog("sip: remove_rtp_session\r\n");

	if (sip_rtp_conn != NULL) uip_udp_remove(sip_rtp_conn);
	sip_rtp_conn = NULL;

	HAL_ADC_Stop_DMA(&hadc1);
	HAL_TIM_Base_Stop_IT(&htim3);

	sip_rtp_session_tick = 0;

    MUTE_1;
	CD_1;

	sip_state = SIP_STATE_DEFAULT;
}


//печать приходящего пакета по протоколу СИП
void print_rcv_packet(char * data)
{
	ulog_fmt("sip: rcv_message (%d):\r\n", strlen(data));
	ulog(data);
	ulog("\r\n");
}

//заполнить call от M
void fill_call_from_meaagse(PSIPMESSAGE M)
{
	mystr2str(M->Contact, Call.RemoteTarget, MAX_SIP_REMOTE_TARGET_SIZE);
	mystr2str(M->ToTag, Call.RemoteTag, MAX_SIP_TAG_SIZE);
	mystr2str(M->FromURI, Call.LocalURI, MAX_SIP_LOCAL_URI_SIZE);
	mystr2str(M->FromTag, Call.LocalTag, MAX_SIP_TAG_SIZE);

	mystr2str(M->FromUser, Call.RemoteUser, MAX_SIP_REMOTE_USER_SIZE);
	mystr2str(M->Via, Call.Via, MAX_SIP_VIA_SIZE);
	mystr2str(M->ToAddr, Call.ToAddr, MAX_SIP_TO_ADDR_SIZE);
	mystr2str(M->FromAddr, Call.FromAddr, MAX_SIP_FROM_ADDR_SIZE);
}

//перед открытием RTP сессии
bool prepear_rtp_session(PSIPMESSAGE M)
{

	int16_t sip_rtp_codec = -1;
	int i;

	for (i = 0; i < M->RtpMaps_Count; i++) {
		if (M->RtpMaps[i] == 0) { sip_rtp_codec = 0; break; }
		else if (M->RtpMaps[i] == 8) { sip_rtp_codec = 8; break; }
	}

	//Call.Account.LocalSeqNum++;
		//Call.LocalSeqNum = Call.Account.LocalSeqNum;

	if (sip_rtp_codec == -1) {
		  fill_call_from_meaagse(M);
    	  sip_answer(CreateCancelRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), NULL);
    	  return FALSE;
	}


	if (!create_rtp_session(mystr2ipv4(M->RtpAddress), mystr2int(M->RtpPort))) {
		fill_call_from_meaagse(M);
		sip_answer(CreateCancelRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), NULL);
		return FALSE;
	}


	rtp_codec = (unsigned char)sip_rtp_codec;

	fill_call_from_meaagse(M);

	sip_talk_begin();

	return TRUE;

}

//аборт сип сессии
void sip_abort_session(void)
{

	int len;

	ulog("sip: sip_abort_session\r\n");

	sip_invite_abort = TRUE;

	sip_invite_uri[0] = '\0';


	if (sip_state != SIP_STATE_DEFAULT)	{

		if ( (sip_state == SIP_STATE_INVITE) || (sip_state == SIP_STATE_RINGING) )	{
			len = CreateCancelRequest(sip_buf, MAX_SIP_BUF_SZ, &Call);
			uip_udp_send_my(sip_conn, sip_buf, len);
			ulog("sip: snd CANCEL abort session\r\n");
		} else {
			len = CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call);
			uip_udp_send_my(sip_conn, sip_buf, len);
			ulog("sip: snd BYE abort session\r\n");
		}
	}

	if (sip_rtp_conn != NULL) {
		remove_rtp_session();
		sip_ringing_stop();
	}
	sip_state =  SIP_STATE_DEFAULT;
}


//разбор приходящего пакета по протоколу СИП
void sip_rcv_packet(char * data, uint16_t data_len)
{
	SIPMESSAGE M;
	int expires;


	MYSTR AText = {data, data_len};

	data[data_len] = '\0';


    SipMessageParse(AText, &M);

    if (mystrcmp(M.Res, mystr_null)) { // --------------------- ЗАПРОС НАЧАЛО-----------------------------------

      if (mystrcmp_str(M.Method, "OPTIONS")) {
    	  ulog("sip: rcv OPTIONS\r\n");
    	  sip_answer(CreateOptionsOkResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd OptionsOk response OPTIONS\r\n");
    	  if (was_registration == FALSE) {
    		  sip_was_registration();
    		  was_registration = TRUE;
    	  }
      }
      else if (mystrcmp_str(M.Method, "NOTIFY")) {
    	  ulog("sip: rcv NOTIFY\r\n");
    	  sip_answer(CreateOkResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd OK response NOTIFY\r\n");
    	  if (was_registration == FALSE) {
    		  sip_was_registration();
    		  was_registration = TRUE;
    	  }
      }
      else if (mystrcmp_str(M.Method, "INVITE")) {
    	  ulog("sip: rcv INVITE\r\n");
    	  sip_answer(CreateNotAcceptableResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd NotAcceptable response INVITE\r\n");
      }
      else if (mystrcmp_str(M.Method, "CANCEL")) {
    	  ulog("sip: rcv CANCEL\r\n");
    	  sip_ringing_stop();
    	  remove_rtp_session();
    	  sip_talk_finish();
    	  sip_answer(CreateOkResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd OK response CANCEL\r\n");
  		  sip_state = SIP_STATE_DEFAULT;
      }
      else if (mystrcmp_str(M.Method, "ACK")) {
    	  ulog("sip: rcv ACK\r\n");
    	  sip_answer(CreateOkResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd OK response ACK\r\n");
      }
      else if (mystrcmp_str(M.Method, "BYE")) {
    	  ulog("sip: rcv BYE\r\n");
    	  sip_ringing_stop();
    	  remove_rtp_session();
    	  sip_talk_finish();
    	  sip_answer(CreateOkResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd OK response BYE\r\n");
  		  sip_state = SIP_STATE_DEFAULT;
      }
      else print_rcv_packet(data);

    } // --------------------- ЗАПРОС КОНЕЦ-----------------------------------

    else { // --------------------- ОТВЕТ НАЧАЛО -----------------------------------

        if (mystrcmp_str(M.Res, "401")) { //требует аудентификации

        	ulog("sip: rcv 401 Unautorization\r\n");

            Call.Account.Realm[0] = '\0';
            Call.Realm[0] = '\0';
            Call.Account.Nonce[0] = '\0';
            Call.Nonce[0] = '\0';
            Call.Account.Opaque[0] = '\0';
            Call.Opaque[0] = '\0';
            Call.Account.Qop[0] = '\0';
            Call.AutResult[0] = '\0';


            mystr2str(M.Realm, Call.Account.Realm, MAX_SIP_REALM_SIZE);
            mystr2str(M.Realm, Call.Realm, MAX_SIP_REALM_SIZE);

            mystr2str(M.Nonce, Call.Account.Nonce, MAX_SIP_NONCE_SIZE);
            mystr2str(M.Nonce, Call.Nonce, MAX_SIP_NONCE_SIZE);

            mystr2str(M.Opaque, Call.Account.Opaque, MAX_SIP_OPAQUE_SIZE);
            mystr2str(M.Opaque, Call.Opaque, MAX_SIP_OPAQUE_SIZE);


            mystr2str(M.Qop, Call.Account.Qop, MAX_SIP_QOP_SIZE);
            mystr2str(M.Qop, Call.Opaque, MAX_SIP_QOP_SIZE);

            mystr2str(M.Res, Call.AutResult, MAX_SIP_AUTH_RESULT);


            if (mystrcmp_str(M.Method, "REGISTER")) { //метод Регистрация требует аудентификации
            	if (SendAutRegisterRequest == FALSE) {

    				Call.Account.LocalSeqNum++;

            		sip_answer(CreateAutRegisterRequest(sip_buf, MAX_SIP_BUF_SZ, &M, &Call.Account), "sip: snd AutRegisterRequest\r\n");
            		SendAutRegisterRequest = TRUE;
            	}
            }
            else if (mystrcmp_str(M.Method, "INVITE")) { //метод Вызов требует аудентификации

                if (sip_invite_abort) {
                	fill_call_from_meaagse(&M);
                	sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), NULL);
                }
                else if (SendAutInviteRequest == FALSE) {
                    Call.LocalSeqNum = Call.Account.LocalSeqNum + 1;
            		sip_answer(CreateAutInviteRequest(sip_buf, MAX_SIP_BUF_SZ, &Call, "0"), "sip: snd AutInviteRequest\r\n");
            		SendAutInviteRequest = TRUE;
            	} else {
    				//Call.Account.LocalSeqNum++;
    	  			//Call.LocalSeqNum = Call.Account.LocalSeqNum;
            		//sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M));
            	}
            }

        } //401
        else if (mystrcmp_str(M.Res, "200")) { //ответ Хорошо


        	if (mystrcmp_str(M.Method, "REGISTER")) { //ответ Хорошо на регистрацию
            	ulog("sip: rcv 200 Ok REGISTER\r\n");
        		if (M.Expires.len) { //перерегистрация через время Expires
        			expires = mystr2int(M.Expires);
        			if (expires > 0) {
        			  sip_time_register = expires * 1000 - 500;
        			  set_tick(&sip_regiser_tick);
        			}
        		}
          	    if (was_registration == FALSE) {
          	    	sip_was_registration();
          	    	was_registration = TRUE;
          	    }

        	}

        	else if (mystrcmp_str(M.Method, "INVITE")) { //ответ Хорошо на вызов
            	ulog("sip: rcv 200 Ok INVITE\r\n");

            	fill_call_from_meaagse(&M);
                if (sip_invite_abort) {
                	sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 200\r\n");
          			sip_state = SIP_STATE_DEFAULT;
                }
                else {
	  			  sip_ringing_stop();
            	  if (prepear_rtp_session(&M)) {
            		  sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK INVITE ok\r\n");
            		  sip_state = SIP_STATE_TALK;
            	  } else sip_state = SIP_STATE_DEFAULT;
                }

        	}
        	else {
            	ulog("sip: rcv 200 Ok ?\r\n");
            	print_rcv_packet(data);
        	}

        }
        else if (mystrcmp_str(M.Res, "100")) { //Trying
        	ulog("sip: rcv 100 Trying\r\n");
        	fill_call_from_meaagse(&M);

            if (sip_invite_abort) {
            	sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 100\r\n");
      			sip_state = SIP_STATE_DEFAULT;
            }
            else if (mystrcmp_str(M.Method, "INVITE")) {
        		sip_ringing_start();
      			sip_state = SIP_STATE_RINGING;
        	}
        }
        else if (mystrcmp_str(M.Res, "183")) { //Session Progress
        	ulog("sip: rcv 183 Session Progress\r\n");
        	fill_call_from_meaagse(&M);
            if (sip_invite_abort) {
            	sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 183\r\n");
      			sip_state = SIP_STATE_DEFAULT;
            }
            else if (mystrcmp_str(M.Method, "INVITE")) {
	  			//sip_ringing_start();
            	//prepear_rtp_session(&M); //ACK тут не посылаем
        	}
        }
        else if (mystrcmp_str(M.Res, "180")) { //Ringing
        	ulog("sip: rcv 180 Ringing\r\n");
        	fill_call_from_meaagse(&M);
            if (sip_invite_abort) {
            	sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 180\r\n");
      			sip_state = SIP_STATE_DEFAULT;
            }
            else if (mystrcmp_str(M.Method, "INVITE")) {
        		sip_ringing_start();
      			sip_state = SIP_STATE_RINGING;
        	}
        }
        else if (mystrcmp_str(M.Res, "486")) { //Busy Here
        	ulog("sip: rcv 486 Busy Here\r\n");
        	if (mystrcmp_str(M.Method, "INVITE")) {
        		sip_ringing_stop();
        		remove_rtp_session();
        		sip_busy();
        	}
    		fill_call_from_meaagse(&M);
            //if (sip_invite_abort) sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 486\r\n");
            //else
    		sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK Busy\r\n");
  			sip_state = SIP_STATE_DEFAULT;
        }
        else if (mystrcmp_str(M.Res, "403")) { //Forbidden
        	ulog("sip: rcv 403 Forbidden\r\n");
        	if (mystrcmp_str(M.Method, "INVITE")) {
        		sip_ringing_stop();
          	    remove_rtp_session();
          	    sip_forbidden();
        	}
        	fill_call_from_meaagse(&M);
            //if (sip_invite_abort) sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 403\r\n");
      	    //else
        	sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK Forbidden\r\n");
  			sip_state = SIP_STATE_DEFAULT;
        }
        else if (mystrcmp_str(M.Res, "487")) { //Request Terminated
        	ulog("sip: rcv 487 Request Terminated\r\n");
        	if (mystrcmp_str(M.Method, "INVITE")) {
        		sip_ringing_stop();
        		remove_rtp_session();
        		sip_terminated();
        	}
            fill_call_from_meaagse(&M);
            //if (sip_invite_abort) sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 487\r\n");
            //else
            sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK Request Terminated\r\n");
  			sip_state = SIP_STATE_DEFAULT;
        }
        else if (mystrcmp_str(M.Res, "404")) { //NOT FOUND
        	ulog("sip: rcv 404 Not Found\r\n");
        	if (mystrcmp_str(M.Method, "INVITE")) {
        		sip_ringing_stop();
        		remove_rtp_session();
        		sip_not_found();
        	}
    		fill_call_from_meaagse(&M);
            //if (sip_invite_abort) sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort 487\r\n");
            //else
    		sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK Not Found\r\n");
  			sip_state = SIP_STATE_DEFAULT;
        }
        else if (mystrcmp_str(M.Res, "500")) { //Server error
        	ulog("sip: rcv 500 Server error\r\n");
    		sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK Server error\r\n");
        }
        else {
        	print_rcv_packet(data);
            fill_call_from_meaagse(&M);
    		if (sip_invite_abort) sip_answer(CreateByeRequest(sip_buf, MAX_SIP_BUF_SZ, &Call), "sip: snd BYE sip_invite_abort\r\n");
    		else sip_answer(CreateAckResponse(sip_buf, MAX_SIP_BUF_SZ, &M), "sip: snd ACK ?\r\n");
        }

    } // --------------------- ОТВЕТ КОНЕЦ -----------------------------------

}

//инициализация СИП
void sip_init(void)
{
	sip_conn = NULL;
	sip_rtp_conn = NULL;
	sip_rtp_session_id = 0;
	sip_rtp_wSeqNum = 0;
	sip_regiser_tick = 0;
	sip_invite_uri[0] = '\0';
	memset(&Call, 0, sizeof(Call));

	u16_t * ipaddr = resolv_lookup(settings.sip_server);
	if (ipaddr == NULL) return;


	xsprintf(Call.Account.ServerIP, "%d.%d.%d.%d", htons(ipaddr[0]) >> 8, htons(ipaddr[0]) & 0xff, htons(ipaddr[1]) >> 8, htons(ipaddr[1]) & 0xff);

    ulog_fmt("sip: resolv found %s=%s\r\n", settings.sip_server, Call.Account.ServerIP);



	Call.SessionID = 2000 + RANDOM(1000);
	Call.SessionNo = Call.SessionID;
    Call.Account.LocalPort = 23156;
	Call.LocalRtpPort = 20280;
    memstrncpy(Call.LocalTag, GenerateTag(), MAX_SIP_TAG_SIZE);
	memstrncpy(Call.Account.Server, settings.sip_server, MAX_SIP_SERVER_NAME_SIZE);
    Call.Account.ServerPort = settings.sip_server_port;
    memstrncpy(Call.Account.Branch, GenerateBranch(), MAX_SIP_BRANCH_SIZE);
    memstrncpy(Call.Account.Tag, GenerateTag(), MAX_SIP_TAG_SIZE);
    memstrncpy(Call.Account.CallID, GenerateCallID(), MAX_SIP_CALL_ID_SIZE);
    Call.Account.LocalSeqNum = 100 + RANDOM(100);
    Call.Account.Expires = 300;
    memstrncpy(Call.AudioCodec.GetId, "0", 90);
    memstrncpy(Call.Account.FormattedName, settings.sip_user, MAX_SIP_FORMATTED_NAME_SIZE);
    memstrncpy(Call.Account.User, settings.sip_user, MAX_SIP_USER_SIZE);
    memstrncpy(Call.Account.Password, settings.sip_password, MAX_SIP_PASSWORD_SIZE);
    memstrncpy(Call.Account.LocalIP, get_ip_str((unsigned char *)&uip_hostaddr), MAX_SIP_IP_ADDRESS_SIZE);
    memstrncpy(Call.Account.MappedLocalIP, get_ip_str((unsigned char *)&uip_hostaddr), MAX_SIP_IP_ADDRESS_SIZE);
    memstrncpy(Call.Account.LocalURI , "sip:", MAX_SIP_URI_SIZE);
    mystrncat(Call.Account.LocalURI , Call.Account.User, MAX_SIP_URI_SIZE);
    mystrncat(Call.Account.LocalURI , "@", MAX_SIP_URI_SIZE);
    mystrncat(Call.Account.LocalURI , Call.Account.LocalIP, MAX_SIP_URI_SIZE);
    mystrncat(Call.Account.LocalURI , ":", MAX_SIP_URI_SIZE);
    mystrncat(Call.Account.LocalURI , int2str(Call.Account.LocalPort), MAX_SIP_URI_SIZE);

    Call.LocalSeqNum = Call.Account.LocalSeqNum;

    sip_conn = uip_udp_new((uip_ipaddr_t *)ipaddr, HTONS(Call.Account.ServerPort));
	uip_udp_bind(sip_conn, HTONS(Call.Account.LocalPort));


    sip_regiser_tick = HAL_GetTick();

    lwrb_init(&sip_rtp_dac_buf, sip_rtp_dac_buf_data, sizeof(sip_rtp_dac_buf_data));

    //941 -частота звука кнопки 0 на телефоне
    //goertzel_coeff_941 = goertzel_get_coeff(AUDIO_BUF_SZ / GOERTZEL_SPLIT, 941, 8000);
    //и 1366
    //goertzel_coeff_1336 = goertzel_get_coeff(AUDIO_BUF_SZ / GOERTZEL_SPLIT, 1366, 8000);

    sip_state = SIP_STATE_DEFAULT;

    was_registration = FALSE;


    uip_udp_send_my(sip_conn, sip_buf, CreateUnRegisterRequest(sip_buf, MAX_SIP_BUF_SZ, &Call.Account));

	ulog("sip: init finish\r\n");
}


void sip_appcall(void)
{

	  char * ptr;

	  if (uip_poll()) {

		  	if (sip_conn == uip_udp_conn) {

		  		if ((sip_rtp_conn == NULL) && (HAL_Diff(sip_regiser_tick) > sip_time_register)) {
		  			SendAutRegisterRequest = FALSE;
					Call.LocalSeqNum++;
	                Call.Account.LocalSeqNum = Call.LocalSeqNum;
		  			sip_answer(CreateRegisterRequest(sip_buf, MAX_SIP_BUF_SZ, &Call.Account), "sip: snd RegisterRequest\r\n"); //перерегистрация
		  			set_tick(&sip_regiser_tick);
		  		}


		  		else if (strlen(sip_invite_uri) != 0) {

		  			strcpy(Call.RemoteURI, "sip:");
		  			mystrncat(Call.RemoteURI, sip_invite_uri, MAX_SIP_URI_SIZE);
		  			memstrncpy(Call.Branch, GenerateBranch(), MAX_SIP_BRANCH_SIZE);
		  			memstrncpy(Call.CallID,  GenerateCallID(), MAX_SIP_CALL_ID_SIZE);
		  			memstrncpy(Call.SendRecv, "sendrecv", MAX_SIP_SEND_RECV_SIZE);
		  			memstrncpy(Call.RemoteUser, sip_invite_uri, MAX_SIP_REMOTE_USER_SIZE);
					if ((ptr = strstr(Call.RemoteUser, "@")) != NULL )  *ptr = '\0';
					Call.LocalSeqNum++;
		  			Call.RemoteTag[0] = '\0';
	                Call.Account.LocalSeqNum = Call.LocalSeqNum;
	                Call.Via[0] = '\0';

		  			sip_answer(CreateInviteRequest(sip_buf, MAX_SIP_BUF_SZ, &Call, "0"), "sip: snd InviteRequest\r\n");
		  			sip_invite_uri[0] = '\0';
		  			sip_invite_abort = FALSE;
		  			SendAutInviteRequest = FALSE;
		  			sip_state =  SIP_STATE_INVITE;
		  		}
		  	}


	  }

	  else if (uip_newdata()) {

		  if (sip_conn == uip_udp_conn) sip_rcv_packet(uip_appdata,  uip_datalen());
		  else if (sip_rtp_conn == uip_udp_conn) sip_rcv_rtp_packet(uip_appdata,  uip_datalen());
		  else ulog_fmt("sip: new_data lport=%d ip=%s rport=%d uip_datalen=%d\r\n", HTONS(uip_udp_conn->lport), get_ip_str((unsigned char *)&uip_udp_conn->ripaddr), HTONS(uip_udp_conn->rport), uip_datalen());

	  }

}

