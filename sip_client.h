/*
 * sip_client.h
 *
 *  Created on: 14 янв. 2021 г.
 *      Author: lenovo
 */

#ifndef INC_SIP_CLIENT_H_
#define INC_SIP_CLIENT_H_

#include "mydef.h"

#include "mysipmessage.h"
#include "mysiprequest.h"

#define MAX_SIP_BUF_SZ 1300

#pragma pack(push, 1)

typedef struct _PRTPHEADER {
    unsigned char  cVersion;             //0x80;   ==== 128
    unsigned char  cPayloadType;         //= 0x12 значит G729 вроде === 0
    unsigned short wSeqNum;              //= htons(cSeq); cSeq++;      // порядковый номер
    unsigned long TimeStamp;    //=htonl(GetTickCount());  // временная метка
    unsigned long SyncId;       //от балды, значение для сеанса
} RTPHEADER, * PRTPHEADER;



#define MAX_RTP_DATA_PAYLOAD_SZ 160

typedef struct _PRTPDATA {
	RTPHEADER header;
	int8_t Payload[MAX_RTP_DATA_PAYLOAD_SZ];          //memcpy( RTPBUF+sizeof(RTP), Payload,20 ); // кодированный голос
} RTPDATA, * PRTPDATA;

#pragma pack(pop)

extern SIPCALL Call;

extern uint32_t sip_regiser_tick;
extern uint32_t sip_time_register;

extern struct uip_udp_conn *sip_conn;
extern struct uip_udp_conn *sip_rtp_conn;


extern char sip_invite_uri[MAX_SIP_URI_SIZE];

extern uint32_t sip_rtp_session_tick;
extern uint32_t sip_rtp_session_start_tick;

void remove_rtp_session(void);

void sip_rtp_data_fill_header(PRTPHEADER prtpheader);

void sip_init(void);

void sip_appcall(void);


void sip_ringing_stop(void);

void uip_udp_send_my(struct uip_udp_conn *c, const void *data, int len);



extern int tmp_rcv_rtp_counter;
extern int tmp_snd_rtp_counter;
//extern int tmp_magnitude_941;
//extern int tmp_magnitude_1336;






//вызывается в sip_task постоянно елси есть rtp сессия
void sip_rtp_adc_send_packet(void);

//загрузить буфер ЦАП данными
void sip_rtp_dac_audio_fill(void);


extern int sip_rtp_adc_buf_fill;

void sip_abort_session(void);

//extern uint16_t sip_dtmf;

extern bool sip_invite_abort;


#define SIP_STATE_DEFAULT	0
#define SIP_STATE_INVITE	1
#define SIP_STATE_RINGING	2
#define SIP_STATE_TALK		3

//идет состояние сип
extern uint8_t sip_state;

extern bool was_registration;

#endif /* INC_SIP_CLIENT_H_ */
