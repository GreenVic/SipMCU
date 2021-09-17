#ifndef mysiprequestH
#define mysiprequestH

#include "mysipmessage.h"



#define MAX_SIP_SERVER_NAME_SIZE    60
#define MAX_SIP_IP_ADDRESS_SIZE   	40
#define MAX_SIP_BRANCH_SIZE 		60
#define MAX_SIP_FORMATTED_NAME_SIZE 60
#define MAX_SIP_USER_SIZE           60
#define MAX_SIP_TAG_SIZE    		60
#define MAX_SIP_CALL_ID_SIZE        60
#define MAX_SIP_URI_SIZE      		60
#define MAX_SIP_PASSWORD_SIZE       60
#define MAX_SIP_IM_TEXT_SIZE        60
#define MAX_SIP_REALM_SIZE          60
#define MAX_SIP_QOP_SIZE            60
#define MAX_SIP_NONCE_SIZE          60
#define MAX_SIP_OPAQUE_SIZE         60

typedef struct _SIPACCOUNT {
  char Server[MAX_SIP_SERVER_NAME_SIZE];  //df.feelinhome.ru
  char LocalIP[MAX_SIP_IP_ADDRESS_SIZE]; //192.168.100.5
  unsigned short LocalPort;    //56432
  char Branch[MAX_SIP_BRANCH_SIZE];  //z9hG4bKPj57ba556fab2b4d7aaffe54ae55cdb558
  char FormattedName[MAX_SIP_FORMATTED_NAME_SIZE]; //3334
  char User[MAX_SIP_USER_SIZE]; //3334
  char Tag[MAX_SIP_TAG_SIZE]; //e3228d7dac5c41a0b53ca65a25b81b44
  char CallID[MAX_SIP_CALL_ID_SIZE]; //d4ff74f019d44f04a50c1e4eceb350a8
  unsigned int LocalSeqNum; //21
  char LocalURI[MAX_SIP_URI_SIZE]; //sip:3334@192.168.43.51:59167
  unsigned short Expires; //300
  char Password[MAX_SIP_PASSWORD_SIZE];
  char IMText[MAX_SIP_IM_TEXT_SIZE];
  char Realm[MAX_SIP_REALM_SIZE];
  char Qop[MAX_SIP_QOP_SIZE];
  char Nonce[MAX_SIP_NONCE_SIZE];
  char Opaque[MAX_SIP_OPAQUE_SIZE];
  char MappedLocalIP[MAX_SIP_IP_ADDRESS_SIZE];
  //���
  unsigned short ServerPort;    //56432
  char ServerIP[MAX_SIP_IP_ADDRESS_SIZE];  //ip �������
} SIPACCOUNT, *PSIPACCOUNT;


char * NewStrGuid(void);
char * GenerateBranch(void);
char * GenerateTag(void);
char * GenerateCallID(void);


int CreateRegisterRequest(char * buf, int buf_len, PSIPACCOUNT Account);
int CreateAutRegisterRequest(char * buf, int buf_len, PSIPMESSAGE M, PSIPACCOUNT Account);
int CreateOptionsOkResponse(char * buf, int buf_len, PSIPMESSAGE M);
int CreateOkResponse(char * buf, int buf_len, PSIPMESSAGE M);
int CreateStatusResponse(char * buf, int buf_len, PSIPMESSAGE M, char * Status);
int CreateAckResponse(char * buf, int buf_len, PSIPMESSAGE M);
int CreateUnRegisterRequest(char * buf, int buf_len, PSIPACCOUNT Account);
int CreateNotAcceptableResponse(char * buf, int buf_len, PSIPMESSAGE M);




#define MAX_SIP_SEND_RECV_SIZE 		60
#define MAX_SIP_AUTH_RESULT 		60
#define MAX_SIP_REMOTE_USER_SIZE    60
#define MAX_SIP_AUT_LINE_SIZE       128
#define MAX_SIP_REMOTE_TARGET_SIZE 	60
#define MAX_SIP_RECORD_ROUTE_SIZE   128
#define MAX_SIP_LOCAL_URI_SIZE      60
#define MAX_SIP_VIA_SIZE            200
#define MAX_SIP_TO_ADDR_SIZE        128
#define MAX_SIP_FROM_ADDR_SIZE      128
#define MAX_SIP_ROUTE_RECORD        128

typedef struct _SIPAUDIOCODECS {
  char GetId[90];
} SIPAUDIOCODECS;

typedef struct _SIPCALL {
  SIPACCOUNT Account;
  char RemoteURI[MAX_SIP_URI_SIZE]; //=M.FromURI
  char LocalTag[MAX_SIP_TAG_SIZE];
  char Branch[MAX_SIP_BRANCH_SIZE];
  char RemoteTag[MAX_SIP_TAG_SIZE]; //=M.FromTag;
  unsigned short LocalSeqNum; //=GetNextSeqNo;
  char CallID[MAX_SIP_CALL_ID_SIZE];
  unsigned short SessionID;
  unsigned short SessionNo;
  char SendRecv[MAX_SIP_SEND_RECV_SIZE];
  unsigned short LocalRtpPort; //= GetNextRtpPort
  char AutResult[MAX_SIP_AUTH_RESULT];
  char Realm[MAX_SIP_REALM_SIZE];
  char Qop[MAX_SIP_QOP_SIZE];
  char Nonce[MAX_SIP_NONCE_SIZE];
  char Opaque[MAX_SIP_OPAQUE_SIZE];
  char RemoteUser[MAX_SIP_REMOTE_USER_SIZE]; //=M.FromUser;
  char AutLine[MAX_SIP_AUT_LINE_SIZE];
  char RemoteTarget[MAX_SIP_REMOTE_TARGET_SIZE]; //=M.Contact;
  int RecordRoute_Count;
  char RecordRoute[MAX_RECORD_ROUTE][MAX_SIP_RECORD_ROUTE_SIZE];
  char LocalURI[MAX_SIP_LOCAL_URI_SIZE];  //=M.ToURI;
  char Via[MAX_SIP_VIA_SIZE];  //=M.Via;
  char ToAddr[MAX_SIP_TO_ADDR_SIZE]; //=M.ToAddr;
  char FromAddr[MAX_SIP_FROM_ADDR_SIZE]; //=M.FromAddr;
  unsigned int RemoteSeqNum; //=StrToIntDef(M.CSeq,0);
  SIPAUDIOCODECS AudioCodec;
  char RouteRecord[MAX_SIP_ROUTE_RECORD];
} SIPCALL, *PSIPCALL;

int CreateInviteOkResponse(char * buf, int buf_len, PSIPCALL Call);
int CreateInviteRequest(char * buf, int buf_len, PSIPCALL Call, char * Codecs);
int CreateAutInviteRequest(char * buf, int buf_len, PSIPCALL Call, char * Codecs);
int CreateByeRequest(char * buf, int buf_len, PSIPCALL Call);
int CreateCancelRequest(char * buf, int buf_len, PSIPCALL Call);

#endif
