#include "mysiprequest.h"
#include "mystr.h"
#include <string.h>
#include "md5.h"
#include <stdlib.h>

const MYSTR mystr_MaxForwards = MSTRI("70");
const MYSTR mystr_ClientVers = MSTRI("MySip 1.0");
const MYSTR mystr_AllowString = MSTRI("Allow: INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, INFO, MESSAGE");
const MYSTR mystr_SdpS = MSTRI("s=MySip component");

#define ADDS(x) mystrcat_str_s(&mystr_buf, x, strlen(x), buf_len)
#define ADDMS(x) mystrcat_s(&mystr_buf, x, buf_len)
#define ADDI(x) mystrcat_s(&mystr_buf, int2mystr(x), buf_len)
#define CRLF ADDMS(mystr_r_n)

#ifndef RANDOM
#define RANDOM(a) (rand() % (a))
#endif

char rand_num_or_letter(void)
{
  if ( RANDOM(2) ) return 48 + RANDOM(10);
  else return 97 + RANDOM(25);
}

char * NewStrGuid(void)
{
  static char __NewStrGuid[20];
  int i;

  for (i = 0; i < 19; i++)  __NewStrGuid[i] = rand_num_or_letter();
  __NewStrGuid[19] = '\0';
  return __NewStrGuid;
}

char * GenerateBranch(void)
{
  static char __GenerateBranch[20];
  memset(__GenerateBranch, 0, sizeof(__GenerateBranch));
  strcpy(__GenerateBranch, "z9hG4bK");
  strncat(__GenerateBranch, NewStrGuid(), 10);
  return __GenerateBranch;
}

char * GenerateTag(void)
{
  static char __GenerateTag[12];
  memset(__GenerateTag, 0, sizeof(__GenerateTag));
  strncpy(__GenerateTag, NewStrGuid(), 9);
  return __GenerateTag;
}

char * GenerateCallID(void)
{
  static char __GenerateCallID[34];
  memset(__GenerateCallID, 0, sizeof(__GenerateCallID));
  for (int i = 0; i < 32; i++)  __GenerateCallID[i] = rand_num_or_letter();
  return __GenerateCallID;
}

#define MAX_MD5_SIZE    33

char * AutResponse(char * Method, char * Phone, char * Server, char * User,
      MYSTR Realm, char * Password, MYSTR Qop, MYSTR Nonce, MYSTR Opaque, MYSTR EntityBody)
{
  static char __AutResponse[MAX_MD5_SIZE];
  char str_buf[256];
  int str_buf_sz = 256;
  char A1_buf[MAX_MD5_SIZE] = {0};
  char A2_buf[MAX_MD5_SIZE] = {0};
  MYSTR str = {str_buf, 0};



  memset(str.ptr, 0, str_buf_sz);
  mystrcat_s(&str, str2mystr(User), str_buf_sz);
  mystrcat_s(&str, mystr_colon, str_buf_sz);
  mystrcat_s(&str, Realm, str_buf_sz);
  mystrcat_s(&str, mystr_colon, str_buf_sz);
  mystrcat_s(&str, str2mystr(Password), str_buf_sz);
  GetMD5(str.ptr, str.len, A1_buf);

  str.len = 0;
  memset(str.ptr, 0, str_buf_sz);
  mystrcat_s(&str, str2mystr(Method), str_buf_sz);
  mystrcat_s(&str, mystr_colon, str_buf_sz);
  mystrcat_s(&str, str2mystr("sip:"), str_buf_sz);
  if ((Phone != NULL) && (strlen(Phone) != 0)) {
    mystrcat_s(&str, str2mystr(Phone), str_buf_sz);
    mystrcat_s(&str, str2mystr("@"), str_buf_sz);
  }
  mystrcat_s(&str, str2mystr(Server), str_buf_sz);
  if (mystrcmp(Qop, str2mystr("auth-in")) && !mystrcmp(EntityBody, mystr_null) ) {
    mystrcat_s(&str, mystr_colon, str_buf_sz);
    GetMD5(EntityBody.ptr, EntityBody.len, str.ptr + str.len);
  }
  GetMD5(str.ptr, strlen(str.ptr), A2_buf);

  str.len = 0;
  memset(str.ptr, 0, str_buf_sz);
  mystrcat_s(&str, str2mystr(A1_buf), str_buf_sz);
  mystrcat_s(&str, mystr_colon, str_buf_sz);
  mystrcat_s(&str, Nonce, str_buf_sz);
  if( (Qop.len != 0) && !mystrcmp(Qop, mystr_null)) {
    mystrcat_s(&str, str2mystr(":00000001:"), str_buf_sz);
    mystrcat_s(&str, Opaque, str_buf_sz);
    mystrcat_s(&str, mystr_colon, str_buf_sz);
    mystrcat_s(&str, Qop, str_buf_sz);
  }
  mystrcat_s(&str, mystr_colon, str_buf_sz);
  mystrcat_s(&str, str2mystr(A2_buf), str_buf_sz);

  memset(__AutResponse, 0, MAX_MD5_SIZE);
  GetMD5(str.ptr, str.len, __AutResponse);

  return __AutResponse;
}

//---------------------------- SIPMESSAGE --------------------------------------
int CreateStatusResponse(char * buf, int buf_len, PSIPMESSAGE M, char * Status)
{
  MYSTR mystr_buf = {buf, 0};
  int i;

  ADDS("SIP/2.0 "); ADDS(Status); CRLF;
  ADDMS(M->Via); CRLF;
  ADDMS(M->ToAddr); CRLF;
  ADDMS(M->FromAddr); CRLF;
  for (i = 0; i < M->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDMS(M->RecordRoute[i]);
    CRLF;
  }
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDS("CSeq: "); ADDMS(M->CSeq); ADDS(" "); ADDMS(M->Method); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateOkResponse (char * buf, int buf_len, PSIPMESSAGE M)
{
  MYSTR mystr_buf = {buf, 0};
  int i;

  ADDS("SIP/2.0 200 OK"); CRLF;
  ADDMS(M->Via); CRLF;
  ADDMS(M->FromAddr); CRLF;
  ADDMS(M->ToAddr); CRLF;
  for (i = 0; i < M->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDMS(M->RecordRoute[i]);
    CRLF;
  }
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDS("CSeq: "); ADDMS(M->CSeq); ADDS(" "); ADDMS(M->Method); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateOptionsOkResponse(char * buf, int buf_len, PSIPMESSAGE M)
{
  MYSTR mystr_buf = {buf, 0};
  int i;

  ADDS("SIP/2.0 200 OK"); CRLF;
  ADDMS(M->Via); CRLF;
  ADDMS(M->ToAddr); CRLF;
  ADDMS(M->FromAddr); CRLF;
  for (i = 0; i < M->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDMS(M->RecordRoute[i]);
    CRLF;
  }
  ADDS("CSeq: "); ADDMS(M->CSeq); ADDS(" "); ADDMS(M->Method); CRLF;
  ADDS("Accept: application/sdp"); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateAckResponse(char * buf, int buf_len, PSIPMESSAGE M)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("ACK ");
        if (mystrcmp(M->Contact, mystr_null)) {
          ADDS("sip:"); ADDMS(M->ToUser); ADDS("@"); ADDMS(M->ToServer);
        } else ADDMS(M->Contact);
        ADDS(" SIP/2.0"); CRLF;
  ADDMS(M->Via); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDMS(M->FromAddr); CRLF;
  ADDMS(M->ToAddr); CRLF;
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDS("CSeq: "); ADDMS(M->CSeq); ADDS(" ACK"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateNotAcceptableResponse(char * buf, int buf_len, PSIPMESSAGE M)
{

  MYSTR mystr_buf = {buf, 0};

  ADDS("SIP/2.0 488 Not Acceptable Here"); CRLF;
  ADDMS(M->Via); CRLF;
  ADDMS(M->FromAddr); CRLF;
  ADDMS(M->ToAddr); CRLF;
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDS("CSeq: "); ADDMS(M->CSeq); ADDS(" "); ADDMS(M->Method); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


//---------------------------- SIPACCOUNT --------------------------------------

int CreateRegisterRequest(char * buf, int buf_len, PSIPACCOUNT Account)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("REGISTER sip:"); ADDS(Account->Server); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Account->LocalIP); ADDS(":"); ADDI(Account->LocalPort);
        ADDS(";rport;branch="); ADDS(Account->Branch); CRLF;
  ADDS("From: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">;tag="); ADDS(Account->Tag); CRLF;
  ADDS("To: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">"); CRLF;
  ADDS("Call-ID: "); ADDS(Account->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Account->LocalSeqNum); ADDS(" REGISTER"); CRLF;
  ADDS("Contact: <"); ADDS(Account->LocalURI); ADDS(">;expires="); ADDI(Account->Expires); ADDS(";q=0.90"); CRLF;
  ADDS("User-Agent: "); ADDMS(mystr_ClientVers); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Expires: "); ADDI(Account->Expires); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateUnRegisterRequest(char * buf, int buf_len, PSIPACCOUNT Account)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("REGISTER sip:"); ADDS(Account->Server); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Account->LocalIP); ADDS(":"); ADDI(Account->LocalPort);
        ADDS(";rport;branch="); ADDS(Account->Branch); CRLF;
  ADDS("From: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">;tag="); ADDS(Account->Tag); CRLF;
  ADDS("To: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">"); CRLF;
  ADDS("Call-ID: "); ADDS(Account->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Account->LocalSeqNum); ADDS(" REGISTER"); CRLF;
  ADDS("Contact: *"); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Expires: 0");  CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


int CreateAutTextMessageRequest(char * buf, int buf_len, PSIPMESSAGE M, PSIPACCOUNT Account, char * PhoneNumber)
{
  MYSTR mystr_buf = {buf, 0};
  int i;

  ADDS("MESSAGE sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Account->Server); ADDS(" SIP/2.0"); CRLF;
  ADDMS(M->Via); CRLF;
  ADDS("From: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">;tag="); ADDS(Account->Tag); CRLF;
  ADDS("To: <sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Account->Server); CRLF;
  for (i = 0; i < M->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDMS(M->RecordRoute[i]);
    CRLF;
  }
  ADDS("CSeq: "); ADDI(Account->LocalSeqNum); ADDS(" MESSAGE"); CRLF;
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Contact: <"); ADDS(Account->LocalURI); ADDS(">"); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;

  if ( (mystrcmp(M->Res, str2mystr("401"))) || (mystrcmp(M->Res, str2mystr("403"))) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Account->User); ADDS("\",realm=\""); ADDMS(M->Realm);
        ADDS("\",nonce=\""); ADDMS(M->Nonce); ADDS("\",uri=\"sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS("\",response=\"");
        ADDS(AutResponse("MESSAGE", PhoneNumber, Account->Server, Account->User, M->Realm, Account->Password, M->Qop, M->Nonce, M->Opaque, mystr_null));
        ADDS("\"");
        if (M->Qop.len > 0) {
                ADDS(",qop=\""); ADDMS(M->Qop); ADDS("\",cnonce=\""); ADDMS(M->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDMS(M->Opaque); ADDS("\"");
        }
        ADDS(",algorithm=MD5"); CRLF;


  ADDS("Content-Type: text/plain"); CRLF;
  ADDS("Content-Length: "); ADDI(strlen(Account->IMText)); CRLF; CRLF;
  ADDS(Account->IMText);
  return mystr_buf.len;
}


int CreateTextMessageRequest(char * buf, int buf_len, PSIPMESSAGE M, PSIPACCOUNT Account, char * PhoneNumber,  char * Text)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("MESSAGE sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Account->Server); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Account->LocalIP); ADDS(":"); ADDI(Account->LocalPort);
        ADDS(";rport;branch="); ADDS(GenerateBranch()); CRLF;
  ADDS("From: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">;tag="); ADDS(Account->Tag); CRLF;
  ADDS("To: <sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Account->Server); CRLF;
  ADDS("Call-ID: "); ADDS(NewStrGuid()); CRLF;
  ADDS("CSeq: "); ADDI(Account->LocalSeqNum); ADDS(" MESSAGE"); CRLF;
  ADDS("Contact: <"); ADDS(Account->LocalURI); ADDS(">"); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Content-Type: text/plain"); CRLF;
  ADDS("Content-Length: "); ADDI(strlen(Text)); CRLF; CRLF;
  ADDS(Text);
  return mystr_buf.len;
}

int CreateAutRegisterRequest(char * buf, int buf_len, PSIPMESSAGE M, PSIPACCOUNT Account)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("REGISTER sip:"); ADDS(Account->Server); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Account->LocalIP); ADDS(":"); ADDI(Account->LocalPort);
        ADDS(";rport;branch="); ADDS(Account->Branch); CRLF;
  ADDS("From: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">;tag="); ADDS(Account->Tag); CRLF;
  ADDS("To: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">"); CRLF;
  ADDS("Call-ID: "); ADDS(Account->CallID); CRLF;
  ADDS("User-Agent: "); ADDMS(mystr_ClientVers); CRLF;
  ADDS("CSeq: "); ADDI(Account->LocalSeqNum); ADDS(" REGISTER"); CRLF;
  ADDS("Contact: <"); ADDS(Account->LocalURI); ADDS(">;expires="); ADDI(Account->Expires);  ADDS(";q=0.90"); CRLF;

  if ( (mystrcmp_str(M->Res, "401")) || (mystrcmp_str(M->Res, "403")) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Account->User); ADDS("\",realm=\""); ADDS(Account->Realm);
        ADDS("\",nonce=\""); ADDS(Account->Nonce); ADDS("\",uri=\"sip:"); ADDS(Account->Server);
        ADDS("\",response=\"");
        ADDS(AutResponse("REGISTER", NULL, Account->Server, Account->User, str2mystr(Account->Realm), Account->Password, str2mystr(Account->Qop), str2mystr(Account->Nonce), str2mystr(Account->Opaque), mystr_null));
        ADDS("\"");
        if (strlen(Account->Qop) > 0) {
          if (strlen(Account->Opaque) > 0) {
            ADDS(",qop=\""); ADDS(Account->Qop); ADDS("\",cnonce=\""); ADDS(Account->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDS(Account->Opaque); ADDS("\"");
          } else {
            char * guid = NewStrGuid();
            GetMD5(guid, strlen(guid), Account->Opaque);
            ADDS(",cnonce=\""); ADDS(Account->Opaque); ADDS("\",nc=00000001,qop="); ADDS(Account->Qop);
          }
        }
        else ADDS(",opaque=\"\"");
        ADDS(",algorithm=MD5"); CRLF;

  ADDMS(mystr_AllowString); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Expires: "); ADDI(Account->Expires); CRLF;

  ADDS("Content-Type: text/plain"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateAutUnRegisterRequest(char * buf, int buf_len, PSIPMESSAGE M, PSIPACCOUNT Account)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("REGISTER sip:"); ADDS(Account->Server); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Account->LocalIP); ADDS(":"); ADDI(Account->LocalPort);
        ADDS(";rport;branch="); ADDS(Account->Branch); CRLF;
  ADDS("From: "); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">;tag="); ADDS(Account->Tag); CRLF;
  ADDS("To: <sip:"); ADDS(Account->FormattedName); ADDS("<sip:"); ADDS(Account->User);
        ADDS("@"); ADDS(Account->Server); ADDS(">"); CRLF;
  ADDS("Call-ID: "); ADDS(Account->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Account->LocalSeqNum); ADDS(" REGISTER"); CRLF;
  ADDS("Contact: *");  CRLF;

  if ( (mystrcmp(M->Res, str2mystr("401"))) || (mystrcmp(M->Res, str2mystr("403"))) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Account->User); ADDS("\",realm=\""); ADDS(Account->Realm);
        ADDS("\",nonce=\""); ADDS(Account->Nonce); ADDS("\",uri=\"sip:"); ADDS(Account->Server);
        ADDS("\",response=\"");
        ADDS(AutResponse("REGISTER", NULL, Account->Server, Account->User, str2mystr(Account->Realm), Account->Password, str2mystr(Account->Qop), str2mystr(Account->Nonce), str2mystr(Account->Opaque), mystr_null));
        ADDS("\"");
        if (strlen(Account->Qop) > 0) {
          if (strlen(Account->Opaque) > 0) {
            ADDS(",qop=\""); ADDS(Account->Qop); ADDS("\",cnonce=\""); ADDS(Account->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDS(Account->Opaque); ADDS("\"");
          } else {
            char * guid = NewStrGuid();
            GetMD5(guid, strlen(guid), Account->Opaque);
            ADDS(",cnonce=\""); ADDS(Account->Opaque); ADDS("\",nc=00000001,qop="); ADDS(Account->Qop);
          }
        }
        else ADDS(",opaque=\"\"");
        ADDS(",algorithm=MD5"); CRLF;

  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Expires: 0"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;

  return mystr_buf.len;
}

// ------------------------- SIPCALL -------------------------------------------

int CreateInviteRequest(char * buf, int buf_len, PSIPCALL Call, char * Codecs)
{
  MYSTR mystr_buf = {buf, 0};
  MYSTR sdp;

  ADDS("v=0"); CRLF;
  ADDS("o=- "); ADDI(Call->SessionID); ADDS(" "); ADDI(Call->SessionNo); ADDS(" IN IP4 "); ADDS(Call->Account.MappedLocalIP); CRLF;
  ADDMS(mystr_SdpS); CRLF;
  ADDS("c=IN IP4 ");
        if (strcmp(Call->SendRecv, "sendonly") == 0) ADDS("0.0.0.0");
        else ADDS(Call->Account.MappedLocalIP);
        CRLF;
  ADDS("t=0 0"); CRLF;
  ADDS("m=audio "); ADDI(Call->LocalRtpPort); ADDS(" RTP/AVP "); ADDS(Codecs); ADDS(" 101"); CRLF;
  ADDS("a=rtpmap:101 telephone-event/8000"); CRLF;
  ADDS("a=fmtp:101 0-15"); CRLF;
  ADDS("a=ptime:20"); CRLF;
  //ADDS("a=ebuacip:plength 0 80"); CRLF;
  //ADDS("a=ebuacip:plength 8 80"); CRLF;
  ADDS("a="); ADDS(Call->SendRecv); CRLF;

  sdp.ptr = buf + buf_len - mystr_buf.len;
  sdp.len = mystr_buf.len;
  memcpy(sdp.ptr, mystr_buf.ptr, sdp.len); //�������� � ����� ������

  mystr_buf.len = 0;

  ADDS("INVITE "); ADDS(Call->RemoteURI); ADDS(" SIP/2.0"); CRLF;
  ADDS("From: "); ADDS(Call->Account.FormattedName); ADDS("<sip:"); ADDS(Call->Account.User);
        ADDS("@"); ADDS(Call->Account.Server); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("To: <"); ADDS(Call->RemoteURI); ADDS(">");
        if (strlen(Call->RemoteTag) != 0) { ADDS(";tag="); ADDS(Call->RemoteTag); }
        CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort);
        ADDS(";rport;branch="); ADDS(Call->Branch); CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" INVITE"); CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDMS(mystr_AllowString);  CRLF;
  ADDS("Contact: <sip:"); ADDS(Call->Account.User); ADDS("@"); ADDS(Call->Account.MappedLocalIP); ADDS(":");
        ADDI(Call->Account.LocalPort); ADDS(">"); CRLF;

  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Content-Type: application/sdp"); CRLF;
  ADDS("Content-Length: "); ADDI(sdp.len); CRLF; CRLF;
  ADDMS(sdp);

  return mystr_buf.len;
}

int CreateAutInviteRequest(char * buf, int buf_len, PSIPCALL Call, char * Codecs)
{
  MYSTR mystr_buf = {buf, 0};
  MYSTR sdp, aut_line;

  ADDS("v=0"); CRLF;
  ADDS("o=- "); ADDI(Call->SessionID); ADDS(" "); ADDI(Call->SessionNo); ADDS(" IN IP4 "); ADDS(Call->Account.MappedLocalIP); CRLF;
  ADDMS(mystr_SdpS); CRLF;
  ADDS("c=IN IP4 ");
        if (strcmp(Call->SendRecv, "sendonly") == 0) ADDS("0.0.0.0");
        else ADDS(Call->Account.MappedLocalIP);
        CRLF;
  ADDS("t=0 0"); CRLF;
  ADDS("m=audio "); ADDI(Call->LocalRtpPort); ADDS(" RTP/AVP "); ADDS(Codecs); ADDS(" 101"); CRLF;
  ADDS("a=rtpmap:101 telephone-event/8000"); CRLF;
  ADDS("a=fmtp:101 0-15"); CRLF;
  ADDS("a=ptime:20"); CRLF;
  //ADDS("a=ebuacip:plength 0 80"); CRLF;
  //ADDS("a=ebuacip:plength 8 80"); CRLF;
  ADDS("a="); ADDS(Call->SendRecv); CRLF;

  sdp.ptr = buf + buf_len - mystr_buf.len;
  sdp.len = mystr_buf.len;
  memcpy(sdp.ptr, mystr_buf.ptr, sdp.len); //�������� � ����� ������
  mystr_buf.len = 0;


  ADDS("INVITE "); ADDS(Call->RemoteURI); ADDS(" SIP/2.0"); CRLF;
  ADDS("From: "); ADDS(Call->Account.FormattedName); ADDS("<sip:"); ADDS(Call->Account.User);
        ADDS("@"); ADDS(Call->Account.Server); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("To: <"); ADDS(Call->RemoteURI); ADDS(">");
        if (strlen(Call->RemoteTag) != 0) { ADDS(";tag="); ADDS(Call->RemoteTag); }
        CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort);
  	  ADDS(";rport;branch="); ADDS(Call->Branch);
  	  CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" INVITE"); CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDMS(mystr_AllowString);  CRLF;
  ADDS("Contact: <sip:"); ADDS(Call->Account.User); ADDS("@"); ADDS(Call->Account.MappedLocalIP); ADDS(":");
        ADDI(Call->Account.LocalPort); ADDS(">"); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;


  aut_line.ptr = mystr_buf.ptr + mystr_buf.len;

  if ( mystrcmp(str2mystr(Call->AutResult), str2mystr("401")) || mystrcmp(str2mystr(Call->AutResult), str2mystr("403")) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Call->Account.User); ADDS("\",realm=\""); ADDS(Call->Realm);
        ADDS("\",nonce=\""); ADDS(Call->Nonce); ADDS("\",uri=\""); ADDS(Call->RemoteURI);
        ADDS("\",response=\"");
        ADDS(AutResponse("INVITE", Call->RemoteUser, Call->Account.Server, Call->Account.User, str2mystr(Call->Realm), Call->Account.Password, str2mystr(Call->Qop), str2mystr(Call->Nonce), str2mystr(Call->Opaque), sdp));
        ADDS("\"");
        if (strlen(Call->Qop) > 0) {
          if (strlen(Call->Opaque) > 0) {
            ADDS(",qop=\""); ADDS(Call->Qop); ADDS("\",cnonce=\""); ADDS(Call->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDS(Call->Opaque); ADDS("\"");
          } else {
            char * guid = NewStrGuid();
            GetMD5(guid, strlen(guid), Call->Opaque);
            ADDS(",cnonce=\""); ADDS(Call->Opaque); ADDS("\",nc=00000001,qop="); ADDS(Call->Qop);
          }
        }
        else ADDS(",opaque=\"\"");
        ADDS(",algorithm=MD5"); CRLF;

  aut_line.len = mystr_buf.ptr + mystr_buf.len - aut_line.ptr;
  memcpy(Call->AutLine, aut_line.ptr, aut_line.len < MAX_SIP_AUT_LINE_SIZE ? aut_line.len : MAX_SIP_AUT_LINE_SIZE);

  ADDS("Content-Type: application/sdp"); CRLF;
  ADDS("Content-Length: "); ADDI(sdp.len); CRLF; CRLF;
  ADDMS(sdp);
  return mystr_buf.len;
}




int GetDtmfInfo(char * buf, int buf_len, PSIPCALL Call, char * Digit)
{
  MYSTR mystr_buf = {buf, 0};
  MYSTR dtfm;
  int i;

  ADDS("Signal= "); ADDS(Digit); CRLF;
  ADDS("Duration= 160"); CRLF;

  dtfm.ptr = buf + buf_len - mystr_buf.len;
  memcpy(dtfm.ptr, mystr_buf.ptr, mystr_buf.len); //�������� � ���
  dtfm.len = mystr_buf.len;


  mystr_buf.len = 0;

  ADDS("INFO "); ADDS(Call->RemoteTarget); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort);
        ADDS(";branch="); ADDS(GenerateBranch()); CRLF;
  for (i = 0; i < Call->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDS(Call->RecordRoute[i]);
    CRLF;
  }
  ADDS("From: "); ADDS(Call->Account.FormattedName); ADDS("<"); ADDS(Call->LocalURI);
        ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("To: <"); ADDS(Call->RemoteURI); ADDS(">");
        if (strlen(Call->RemoteTag) != 0) { ADDS(";tag="); ADDS(Call->RemoteTag); }
        CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" INFO"); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Contact: <sip:"); ADDS(Call->Account.User); ADDS("@"); ADDS(Call->Account.MappedLocalIP); ADDS(":");
        ADDI(Call->Account.LocalPort); ADDS(">"); CRLF;

  ADDS("application/dtmf-relay"); CRLF;
  ADDS("Content-Length: "); ADDI(dtfm.len); CRLF; CRLF;
  ADDMS(dtfm);
  return mystr_buf.len;
}

int CreateInviteOkResponse(char * buf, int buf_len, PSIPCALL Call)
{
  MYSTR mystr_buf = {buf, 0};
  MYSTR sdp;
  int i;

  ADDS("v=0"); CRLF;
  ADDS("o=- "); ADDI(Call->SessionID); ADDS(" "); ADDI(Call->SessionNo); ADDS(" IN IP4 "); ADDS(Call->Account.MappedLocalIP); CRLF;
  ADDMS(mystr_SdpS); CRLF;
  ADDS("c=IN IP4 "); ADDS(Call->Account.MappedLocalIP); CRLF;
  ADDS("t=0 0"); CRLF;
  ADDS("m=audio "); ADDI(Call->LocalRtpPort); ADDS(" RTP/AVP "); ADDS(Call->AudioCodec.GetId); ADDS(" 101"); CRLF;
  ADDS("a=rtpmap:101 telephone-event/8000"); CRLF;
  ADDS("a=fmtp:101 0-15"); CRLF;
//  ADDS("a=recvonly"); CRLF;
  ADDS("a=sendrecv"); CRLF;

  sdp.ptr = buf + buf_len - mystr_buf.len;
  sdp.len = mystr_buf.len;
  memcpy(sdp.ptr, mystr_buf.ptr, sdp.len); //�������� � ����� ������
  mystr_buf.len = 0;


  ADDS("SIP/2.0 200 OK"); CRLF;
  ADDS(Call->Via); CRLF;
  ADDS(Call->ToAddr); ADDS(";tag="); ADDS(Call->LocalTag); CRLF;
  ADDS(Call->FromAddr); CRLF;
  for (i = 0; i < Call->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDS(Call->RecordRoute[i]);
    CRLF;
  }
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Call->RemoteSeqNum); ADDS(" INVITE"); CRLF;
  ADDS("Contact: <sip:"); ADDS(Call->Account.User); ADDS("@"); ADDS(Call->Account.MappedLocalIP); ADDS(":");
        ADDI(Call->Account.LocalPort); ADDS(">"); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Content-Type: application/sdp"); CRLF;
  ADDS("Content-Length: "); ADDI(sdp.len); CRLF; CRLF;
  ADDMS(sdp);
  return mystr_buf.len;
}

int CreateCancelRequest(char * buf, int buf_len, PSIPCALL Call)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("CANCEL "); ADDS(Call->RemoteURI); ADDS(" SIP/2.0"); CRLF;
  //if (strlen(Call->Via) == 0) {
    ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort); ADDS(";branch=");
      ADDS(Call->Branch); ADDS(";rport"); CRLF;
  //} else ADDS(Call->Via);
  ADDS("From: "); ADDS(Call->Account.FormattedName); ADDS("<"); ADDS(Call->LocalURI);
        ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("To: <"); ADDS(Call->RemoteURI); ADDS(">"); CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" CANCEL"); CRLF;

  if ( mystrcmp(str2mystr(Call->AutResult), str2mystr("401")) || mystrcmp(str2mystr(Call->AutResult), str2mystr("403")) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Call->Account.User); ADDS("\",realm=\""); ADDS(Call->Realm);
        ADDS("\",nonce=\""); ADDS(Call->Nonce); ADDS("\",uri=\""); ADDS(Call->RemoteURI);
        ADDS("\",response=\"");
        ADDS(AutResponse("INVITE", Call->RemoteUser, Call->Account.Server, Call->Account.User, str2mystr(Call->Realm), Call->Account.Password, str2mystr(Call->Qop), str2mystr(Call->Nonce), str2mystr(Call->Opaque), mystr_null));
        ADDS("\"");
        if (strlen(Call->Qop) > 0) {
            ADDS(",qop=\""); ADDS(Call->Qop); ADDS("\",cnonce=\""); ADDS(Call->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDS(Call->Opaque); ADDS("\"");
        }
        ADDS(",algorithm=MD5"); CRLF;

  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


int CreateTransferRequest(char * buf, int buf_len, PSIPCALL Call, char * PhoneNumber)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("REFER "); ADDS(Call->RemoteURI); ADDS(" SIP/2.0"); CRLF;
  if (strlen(Call->Via) == 0) {
    ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort); ADDS(";branch=");
      ADDS(Call->Branch); ADDS(";rport"); CRLF;
  } else ADDS(Call->Via);
  ADDS("From: <"); ADDS(Call->LocalURI); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("To: <sip:"); ADDS(Call->RemoteURI); ADDS(">;tag="); ADDS(Call->RemoteTag); CRLF;
  ADDS("Refer-To: <sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Call->Account.Server); ADDS(">"); CRLF;
  ADDS("Referred-By: <"); ADDS(Call->LocalURI); ADDS(">"); CRLF;
  ADDS("Contact: <sip:"); ADDS(Call->Account.User); ADDS("@"); ADDS(Call->Account.MappedLocalIP); ADDS(":");
        ADDI(Call->Account.LocalPort); ADDS(">"); CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" REFER"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}

int CreateAutTransferRequest (char * buf, int buf_len, PSIPCALL Call, char * PhoneNumber)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("REFER "); ADDS(Call->RemoteURI); ADDS(" SIP/2.0"); CRLF;
  if (strlen(Call->Via) == 0) {
    ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort); ADDS(";branch=");
      ADDS(Call->Branch); ADDS(";rport"); CRLF;
  } else ADDS(Call->Via);
  ADDS("From: <"); ADDS(Call->LocalURI); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("To: <sip:"); ADDS(Call->RemoteURI); ADDS(">;tag="); ADDS(Call->RemoteTag); CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  if ( mystrcmp(str2mystr(Call->AutResult), str2mystr("401")) || mystrcmp(str2mystr(Call->AutResult), str2mystr("403")) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Call->Account.User); ADDS("\",realm=\""); ADDS(Call->Realm);
        ADDS("\",nonce=\""); ADDS(Call->Nonce); ADDS("\",uri=\""); ADDS(Call->RemoteURI);
        ADDS("\",response=\"");
        ADDS(AutResponse("REFER", Call->RemoteUser, Call->Account.Server, Call->Account.User, str2mystr(Call->Realm), Call->Account.Password, str2mystr(Call->Qop), str2mystr(Call->Nonce), str2mystr(Call->Opaque), mystr_null));
        ADDS("\"");
        if (strlen(Call->Qop) > 0) {
            ADDS(",qop=\""); ADDS(Call->Qop); ADDS("\",cnonce=\""); ADDS(Call->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDS(Call->Opaque); ADDS("\"");
        }
        ADDS(",algorithm=MD5"); CRLF;


  ADDS("Refer-To: <sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Call->Account.Server); ADDS(">"); CRLF;
  ADDS("Referred-By: <"); ADDS(Call->LocalURI); ADDS(">"); CRLF;
  ADDS("Contact: <sip:"); ADDS(Call->Account.User); ADDS("@"); ADDS(Call->Account.MappedLocalIP); ADDS(":");
        ADDI(Call->Account.LocalPort); ADDS(">"); CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" REFER"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


int CreateByeRequest(char * buf, int buf_len, PSIPCALL Call)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("BYE "); ADDS(Call->RemoteTarget); ADDS(" SIP/2.0"); CRLF;
  ADDS("Via: SIP/2.0/UDP "); ADDS(Call->Account.LocalIP); ADDS(":"); ADDI(Call->Account.LocalPort); ADDS(";rport;branch=");
    ADDS(Call->Branch); CRLF;

  ADDS(Call->RouteRecord);
  //ADDS("To: <sip:"); ADDS(Call->RemoteURI); ADDS(">;tag="); ADDS(Call->RemoteTag); CRLF;
  ADDS("To: <"); ADDS(Call->RemoteURI); ADDS(">;tag="); ADDS(Call->RemoteTag); CRLF;
  ADDS("From: <"); ADDS(Call->LocalURI); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDS("CSeq: "); ADDI(Call->LocalSeqNum); ADDS(" REFER"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


int CreateRedirectResponse(char * buf, int buf_len, PSIPCALL Call, char * PhoneNumber)
{
  MYSTR mystr_buf = {buf, 0};
  int i;

  ADDS("SIP/2.0 320 Moved Temporarily"); CRLF;
  ADDS(Call->Via);
  ADDS("To: <"); ADDS(Call->LocalURI); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("From: <"); ADDS(Call->RemoteURI); ADDS(">;tag="); ADDS(Call->RemoteTag); CRLF;
  for (i = 0; i < Call->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDS(Call->RecordRoute[i]);
    CRLF;
  }
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Call->RemoteSeqNum); ADDS(" REFER"); CRLF;
  ADDS("Contact: <sip:"); ADDS(PhoneNumber); ADDS("@"); ADDS(Call->Account.Server); ADDS(">"); CRLF;
  ADDMS(mystr_AllowString); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


int CreateInviteErrorResponse (char * buf, int buf_len, PSIPCALL Call, char * Error)
{
  MYSTR mystr_buf = {buf, 0};
  int i;

  ADDS("SIP/2.0 "); ADDS(Error); CRLF;
  ADDS(Call->Via);
  ADDS("To: <"); ADDS(Call->LocalURI); ADDS(">;tag="); ADDS(Call->LocalTag); CRLF;
  ADDS("From: <"); ADDS(Call->RemoteURI); ADDS(">;tag="); ADDS(Call->RemoteTag); CRLF;
  for (i = 0; i < Call->RecordRoute_Count; i++) {
    ADDS("Record-Route: ");
    ADDS(Call->RecordRoute[i]);
    CRLF;
  }
  ADDS("Call-ID: "); ADDS(Call->CallID); CRLF;
  ADDS("CSeq: "); ADDI(Call->RemoteSeqNum); ADDS(" INVITE"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


int CreateInviteOkAckResponse(char * buf, int buf_len, PSIPCALL Call, PSIPMESSAGE M)
{
  MYSTR mystr_buf = {buf, 0};

  ADDS("ACK ");
        if (mystrcmp(M->Contact, mystr_null)) {
          ADDS("sip:"); ADDMS(M->ToUser); ADDS("@"); ADDMS(M->ToServer);
        } else ADDMS(M->Contact);
        CRLF;
  ADDMS(M->Via); CRLF;
  ADDS("Max-Forwards: "); ADDMS(mystr_MaxForwards); CRLF;
  ADDMS(M->FromAddr); CRLF;
  ADDMS(M->ToAddr); CRLF;
  ADDS(Call->RouteRecord);
  ADDS("Call-ID: "); ADDMS(M->CallID); CRLF;
  ADDS("Contact: <"); ADDS(Call->Account.LocalURI); ADDS(">"); CRLF;
  ADDS("CSeq: "); ADDMS(M->CSeq); ADDS(" ACK"); CRLF;


  if ( mystrcmp(str2mystr(Call->AutResult), str2mystr("401")) || mystrcmp(str2mystr(Call->AutResult), str2mystr("403")) ) ADDS("Authorization: ");
  else ADDS("Proxy-Authorization: ");
        ADDS("Digest username=\""); ADDS(Call->Account.User); ADDS("\",realm=\""); ADDS(Call->Realm);
        ADDS("\",nonce=\""); ADDS(Call->Nonce); ADDS("\",uri=\""); ADDS(Call->RemoteURI);
        ADDS("\",response=\"");
        ADDS(AutResponse("ACK", Call->RemoteUser, Call->Account.Server, Call->Account.User, str2mystr(Call->Realm), Call->Account.Password, str2mystr(Call->Qop), str2mystr(Call->Nonce), str2mystr(Call->Opaque), mystr_null));
        ADDS("\"");
        if (strlen(Call->Qop) > 0) {
          if (strlen(Call->Opaque) > 0) {
            ADDS(",qop=\""); ADDS(Call->Qop); ADDS("\",cnonce=\""); ADDS(Call->Opaque); ADDS("\",nc=00000001,opaque=\""); ADDS(Call->Opaque); ADDS("\"");
          } else {
            char * guid = NewStrGuid();
            GetMD5(guid, strlen(guid), Call->Opaque);
            ADDS(",cnonce=\""); ADDS(Call->Opaque); ADDS("\",nc=00000001,qop="); ADDS(Call->Qop);
          }
        }
        ADDS(",algorithm=MD5"); CRLF;
  ADDS("Content-Length: 0"); CRLF; CRLF;
  return mystr_buf.len;
}


