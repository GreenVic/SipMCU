#include "mysipmessage.h"

#include <string.h>


const MYSTR mystr_sip_version = MSTRI("SIP/2.0");
const MYSTR mystr_tag_equally = MSTRI("tag=");
const MYSTR mystr_sip_colon = MSTRI("sip:");
const MYSTR mystr_rport_equally = MSTRI("rport=");


void SipMessageParse(MYSTR AText, PSIPMESSAGE pmes)
{
  int pos;
  MYSTR st;
  int i, j;
  MYSTR key, val;
  int intval;

#define MAX_MSG_LINES 20
  int sl_Count;
  MYSTR sl[MAX_MSG_LINES];

#define MAX_WORDS 10
  int words_Count = 0;
  MYSTR words[MAX_WORDS];

  memset(pmes, 0, sizeof(SIPMESSAGE));


  if ((pos = mystrpos(AText, mystr_r_n_r_n)) != -1) {
      pmes->Sdp.ptr = AText.ptr + pos + mystr_r_n_r_n.len;
      pmes->Sdp.len = AText.len - (pmes->Sdp.ptr - AText.ptr);
      AText.len = pos;
  }

  sl_Count = mystrdelim(AText, mystr_r_n, sl, MAX_MSG_LINES);



  if (sl_Count > 0)
  {
    pmes->RequestURI = mystrtrim(mystrsubstr(sl[0], mystrpos(sl[0], mystr_space), sl[0].len));
    if ( (pos = mystrpos(pmes->RequestURI, mystr_sip_version)) != -1)
      pmes->RequestURI = mystrtrim(mystrsubstr(pmes->RequestURI, 0 , pos));
    if (mystrcmp_str(mystrsubstr(sl[0], 0, 7), "INVITE ")) { // should have UpperCase()
      pmes->InviteUser = mystrtrim(mystrsubstr(sl[0], 11, sl[0].len));
      if ((pos = mystrpos(pmes->InviteUser, mystr_dog)) != -1)
        pmes->InviteUser = mystrtrim(mystrsubstr(pmes->InviteUser, 0, pos + 1));
      else pmes->InviteUser = mystr_null;
    } else if (mystrcmp_str(mystrsubstr(sl[0], 0, 7), "MESSAGE ")) { // should have UpperCase()
      pmes->InviteUser = mystrtrim(mystrsubstr(sl[0], 11, sl[0].len));
      if ((pos = mystrpos(pmes->InviteUser, mystr_dog)) != -1)
        pmes->InviteUser = mystrtrim(mystrsubstr(pmes->InviteUser, 0, pos));
      else pmes->InviteUser = mystr_null;
    }

    if (mystrcmp(mystrsubstr(sl[0], 0, mystr_sip_version.len), mystr_sip_version)) {
      pmes->Res = mystrtrim(mystrsubstr(sl[0], mystr_sip_version.len + 1, 3));
    }
  }

  pmes->RecordRoute_Count = 0;

  for (i = 1; i < sl_Count; ++i)
  {
    if (!mystr_parse_key_val(sl[i], mystr_colon_and_space, &key, &val)) continue;

    if ( mystrcmp_str(key, "Expires") ) {
      pmes->Expires = val;
    }
    //Call-ID: 401c88f7043847648aa340dba56e269d
    else if ( mystrcmp_str(key, "Call-ID") || mystrcmp_str(key, "i") ) {
        pmes->CallID = val;
    }
    //CSeq: 16316 INVITE
    else if ( mystrcmp_str(key, "CSeq") ) {
       if (mystr_parse_key_val(val, mystr_space, &key, &val)) {
         pmes->CSeq = key;
         pmes->Method = val;
       } else pmes->CSeq = val;
    }
    //From: <sip:3333@df.feelinhome.ru>;tag=cb0500fe3a03450aaa0c000db3b6fa73
    else if ( mystrcmp_str(key, "From") || mystrcmp_str(key, "f") ) {
      pmes->FromAddr = sl[i];
      if ((pos = mystrpos(val, mystr_tag_equally)) != -1)  //tag=
        pmes->FromTag = mystrsubstr(val, pos + mystr_tag_equally.len, val.len);

      if ( (pos = mystrpos(val, mystr_less)) != -1)  { //<
        pmes->FromURI = mystrsubstr(val, pos + 1 , val.len);
        pmes->FromURI = mystrsubstr(pmes->FromURI, 0, mystrpos(pmes->FromURI, mystr_more)); //>
      }
      if ( (pos = mystrpos(val, mystr_sip_colon))  != -1) { //sip: should be UpperCase()
        st = mystrsubstr(val, pos + mystr_sip_colon.len, val.len);
        if ( (pos = mystrpos(st,  mystr_dog)) != -1) {
          pmes->FromUser = mystrsubstr(st, 0, pos);
          for (j = pos + 1; i < st.len; j++) {
            if ( (st.ptr[j] == '>') || (st.ptr[j] == ' ') ) {
              pmes->FromServer.len = j - (pos + 1);
              pmes->FromServer.ptr =  st.ptr + pos + 1;
              break;
            }
            if ( (st.ptr[j] == '>') || (st.ptr[j] == ' ') || (st.ptr[j] == ':') || (st.ptr[j] == ';') ) {
              pmes->FromServerName.len = j - (pos + 1);
              pmes->FromServerName.ptr = st.ptr + pos + 1;
            }
          }
        }
      }
    }
    //To: <sip:3334@df.feelinhome.ru>;tag=cb0500fe3a03450aaa0c000db3b6fa73
    else if ( mystrcmp_str(key, "To") || mystrcmp_str(key, "t") ) {
      pmes->ToAddr = sl[i];
      if ((pos = mystrpos(val, mystr_tag_equally)) != -1)  //tag=
        pmes->ToTag = mystrsubstr(val, pos + mystr_tag_equally.len, val.len);

      if ( (pos = mystrpos(val, mystr_less)) != -1)  {
        pmes->ToURI = mystrsubstr(val, pos + 1 , val.len);
        pmes->ToURI = mystrsubstr(pmes->ToURI, 0, mystrpos(pmes->ToURI, mystr_more));
      }
      if ( (pos = mystrpos(val, mystr_sip_colon))  != -1) { //sip: should be UpperCase()
        st = mystrsubstr(val, pos + mystr_sip_colon.len, val.len);
        if ( (pos = mystrpos(st,  mystr_dog)) != -1) {
          pmes->ToUser = mystrsubstr(st, 0, pos);
          for (j = pos + 1; i < st.len; j++) {
            if ( (st.ptr[j] == '>') || (st.ptr[j] == ' ') ) {
              pmes->ToServer.len = j - (pos + 1);
              pmes->ToServer.ptr =  st.ptr + pos + 1;
              break;
            }
            if ( (st.ptr[j] == '>') || (st.ptr[j] == ' ') || (st.ptr[j] == ':') || (st.ptr[j] == ';') ) {
              pmes->ToServerName.len = j - (pos + 1);
              pmes->ToServerName.ptr = st.ptr + pos + 1;
            }
          }
        }
      }
    }
    //Via: SIP/2.0/UDP 192.168.43.51:59167;rport;branch=z9hG4bKPj57ba556fab2b4d7aaffe54ae55cdb558
    else if ( mystrcmp_str(key, "Via") || mystrcmp_str(key, "v") ) {
      pmes->Via = sl[i];
      if ( (pos = mystrpos(pmes->Via, mystr_rport_equally)) != -1 ) { //"rport="
        pmes->RPort.ptr = pmes->Via.ptr + pos + mystr_rport_equally.len;
        pmes->RPort.len = 0;
        for (j = pos + mystr_rport_equally.len; j < pmes->Via.len; j++) {
          if ( (pmes->Via.ptr[j] >= '0') && (pmes->Via.ptr[j] <= '9') ) pmes->RPort.len++;
          else break;
        }
      }
      pmes->ViaAddress = val;
      if ( (pos = mystrpos(pmes->ViaAddress, str2mystr("/UDP"))) != -1 ) {
        pmes->ViaAddress = mystrtrim(mystrsubstr(pmes->ViaAddress, pos + 5, pmes->ViaAddress.len));
        if ( (pos = mystrpos(pmes->ViaAddress, mystr_semicolon)) != -1) { //;
          pmes->ViaAddress = mystrsubstr(pmes->ViaAddress, 0, pos);
        }
        pmes->ViaPort = str2mystr("5060");
        if ( (pos = mystrpos(pmes->ViaAddress, mystr_colon)) != -1 ) { //:
          pmes->ViaPort = mystrsubstr(pmes->ViaAddress, pos + 1, pmes->ViaAddress.len);
          pmes->ViaAddress = mystrsubstr(pmes->ViaAddress, 0, pos - 1);
        }
      }
    }
    //Contact: <sip:3334@192.168.43.51:59167;ob>
    else if ( mystrcmp_str(key, "Contact") || mystrcmp_str(key, "c") ) {
      pmes->Contact = mystrsub(val, mystr_less, mystr_more);
      pmes->Contact = mystrsub(pmes->Contact, mystr_null, mystr_semicolon); //;
      if ( (pos = mystrpos(pmes->Contact, mystr_dog)) != -1) {
        pmes->ContactServer = mystrsubstr(pmes->Contact, pos + 1, pmes->Contact.len);
        if ( (pos = mystrpos(pmes->ContactServer, mystr_colon)) != -1) { //:
          pmes->ContactPort = mystrsubstr(pmes->ContactServer, pos + 1, pmes->ContactServer.len);
          pmes->ContactServer = mystrsubstr(pmes->ContactServer, 0, pos);
        }
      }
    }
    //Record-Route:
    else if ( mystrcmp_str(key, "Record-Route") ) {
      words_Count = mystrdelim(val, mystr_comma, words, MAX_WORDS); //,
      for (j = 0; j < words_Count; j++) {
        if (pmes->RecordRoute_Count >= MAX_RECORD_ROUTE) break;
        pmes->RecordRoute[pmes->RecordRoute_Count] = words[j];
        pmes->RecordRoute_Count++;
      }
    }
    //WWW-Authenticate: Digest algorithm=MD5, realm="asterisk", nonce="6ee64aa7"
    else if ( mystrcmp_str(key, "WWW-Authenticate") || mystrcmp_str(key, "Proxy-Authenticate") ) {

      if ( (pos = mystrpos_str(val, "nonce=\"")) != -1) {
        st = mystrsubstr(val, pos + 7, val.len);
        pmes->Nonce.ptr = st.ptr;
        for (j = 0; j < st.len ; j++) {
          if (st.ptr[j] != '"') pmes->Nonce.len++;
          else break;
        }
      }
      if ( (pos = mystrpos_str(val, "realm=\"")) != -1) {
        st = mystrsubstr(val, pos + 7, val.len);
        pmes->Realm.ptr = st.ptr;
        for (j = 0; j < st.len ; j++) {
          if (st.ptr[j] != '"') pmes->Realm.len++;
          else break;
        }
      }
      if ( (pos = mystrpos_str(val, "opaque=\"")) != -1) {
        st = mystrsubstr(val, pos + 8, val.len);
        pmes->Opaque.ptr = st.ptr;
        for (j = 0; j < st.len ; j++) {
          if (st.ptr[j] != '"') pmes->Opaque.len++;
          else break;
        }
      }
      if ( (pos = mystrpos_str(val, "qop=\"")) != -1) {
        st = mystrsubstr(val, pos + 5, val.len);
        pmes->Qop.ptr = st.ptr;
        for (j = 0; j < st.len ; j++) {
          if (st.ptr[j] != '"') pmes->Qop.len++;
          else break;
        }
      }
      if (mystrcmp_str(pmes->Qop, "auth,auth-int")) {
        pmes->Qop.ptr += 5;
        pmes->Qop.len -= 5;
      }
    }
  } //for (i = 1; i < sl_Count; ++i)


  pmes->RtpMaps_Count = 0;

  if (!mystrcmp(pmes->Sdp, mystr_null)) sl_Count = mystrdelim(pmes->Sdp, mystr_r_n, sl, MAX_MSG_LINES);
  else sl_Count = 0;

  for (i = 0; i < sl_Count; ++i) {

    if (!mystr_parse_key_val(sl[i], mystr_equally, &key, &val)) continue;

    if ( mystrcmp_str(key, "c") ) {
      if ( (pos = mystrpos(val, str2mystr("IP4"))) != -1 ) {
        pmes->RtpAddress = mystrtrim(mystrsubstr(val, pos + 4, 20));
      }
    }
    else if ( mystrcmp_str(key, "Signal") ) {
      pmes->Signal = val;
    }
    else if ( mystrcmp_str(key, "m") ) {
      if ( mystrcmp_str(mystrsubstr(val, 0, 5), "audio") ) {
        st = mystrtrim(mystrsubstr(val, 6, val.len));
        pmes->RtpPort.len = 0;
        pmes->RtpPort.ptr = st.ptr;
        for (j = 0; j < st.len; j++) {
          if ( (st.ptr[j] >= '0') && (st.ptr[j] <= '9') ) pmes->RtpPort.len++;
          else break;
        }
        if ( (pos = mystrpos_str(val, "RTP/AVP")) != -1) {
          st = mystrtrim(mystrsubstr(val, pos + 8, val.len));
          words_Count = mystrdelim(st, mystr_space, words, MAX_WORDS);//������
          for (j = 0 ; j < words_Count; j++) {
            if ( mystrcmp_str(words[j], "101") ) continue;
            if ( (intval = mystr2int_def(words[j], -1)) == -1) continue;
            if (pmes->RtpMaps_Count >= MAX_RTP_MAP) break;
            pmes->RtpMaps[pmes->RtpMaps_Count] = intval;
            pmes->RtpMaps_Count++;
          }
        }
      }
    }

  } //for (i = 0; i < sl_Count; ++i)


}
