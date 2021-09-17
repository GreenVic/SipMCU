#ifndef mysipmessageH
#define mysipmessageH

#include "mystr.h"

typedef struct _SIPMESSAGE {
    MYSTR Res;
    MYSTR RequestURI;
    MYSTR InviteUser;
    MYSTR RPort;
    //via
    MYSTR Via;
    MYSTR ViaAddress;
    MYSTR ViaPort;
    //to
    MYSTR ToAddr;
    MYSTR ToURI;
    MYSTR ToUser;
    MYSTR ToServer;
    MYSTR ToServerName;
    MYSTR ToTag;
    //from
    MYSTR FromAddr;
    MYSTR FromURI;
    MYSTR FromUser;
    MYSTR FromServer;
    MYSTR FromServerName;
    MYSTR FromTag;
    //cseq
    MYSTR CSeq;
    MYSTR Method;
    //contact
    MYSTR Contact;
    MYSTR ContactServer;
    MYSTR ContactPort;
    //callid
    MYSTR CallID;
    //expires
    MYSTR Expires;
    //www auth
    MYSTR Nonce;
    MYSTR Realm;
    MYSTR Opaque;
    MYSTR Qop;
    //recordroute
#define MAX_RECORD_ROUTE   10
    int RecordRoute_Count;
    MYSTR RecordRoute[MAX_RECORD_ROUTE];
    //sdp
    MYSTR Sdp;
    MYSTR Signal;
    MYSTR RtpAddress;
    MYSTR RtpPort;
#define MAX_RTP_MAP     10
    int RtpMaps_Count;
    int RtpMaps[MAX_RTP_MAP];
} SIPMESSAGE, * PSIPMESSAGE;


void SipMessageParse(MYSTR AText, PSIPMESSAGE pmes);

#endif
