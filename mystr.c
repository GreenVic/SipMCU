#include "mystr.h"
#include <stdlib.h>
#include <string.h>



const MYSTR mystr_null = {NULL, 0};
const MYSTR mystr_r_n_r_n = MSTRI("\r\n\r\n");
const MYSTR mystr_r_n = MSTRI("\r\n");
const MYSTR mystr_comma = MSTRI(",");
const MYSTR mystr_semicolon = MSTRI(";");
const MYSTR mystr_n = MSTRI("\n");
const MYSTR mystr_n_n = MSTRI("\n\n");
const MYSTR mystr_equally = MSTRI("=");
const MYSTR mystr_colon = MSTRI(":");
const MYSTR mystr_space = MSTRI(" ");
const MYSTR mystr_less = MSTRI("<");
const MYSTR mystr_more = MSTRI(">");
const MYSTR mystr_colon_and_space = MSTRI(": ");
const MYSTR mystr_slash = MSTRI("/");
const MYSTR mystr_dog = MSTRI("@");
const MYSTR mystr_comma_and_space = MSTRI(", ");
const MYSTR mystr_quote = MSTRI("\"");



char * strfind(char * buf, int buf_len, char * what, int what_len)
{
  char * ptr = buf, * ptre = buf + buf_len - what_len;

  while (ptr <= ptre)
  {
    if (*ptr == *what) {
        if (memcmp(ptr, what, what_len) == 0) return ptr;
    }
    ++ptr;
  }

  return NULL;
}

char * int2str(int val)
{
   static char __int2str[20];
   itoa(val, __int2str, 10);
   return __int2str;
}


unsigned long mystr2ipv4(MYSTR str)
{
  static char __mystr2ip[16];

  int len = str.len < 16 ? str.len : 15;
  memcpy(__mystr2ip, str.ptr, len);
  __mystr2ip[len] = '\0';
  return myinet_addr(__mystr2ip);
}

void mystrcpy_alloc(PMYSTR pdstm, MYSTR src)
{
  pdstm->ptr = (char *)ALLOC(src.len + 1);
  memcpy(pdstm->ptr, src.ptr, src.len);
  pdstm->ptr[src.len] = '\0';
  pdstm->len = src.len;
}

void mystrfree(PMYSTR pstr)
{
  if (pstr->ptr) FREE(pstr->ptr);
  pstr->ptr = NULL;
  pstr->len = 0;
}

MYSTR mystrtrim(MYSTR str)
{
  int i;
  MYSTR res = MYSTRI_NULL;


  for (i = 0; i < str.len; ++i) {
    if ( str.ptr[i] > ' ') {
      res.ptr = str.ptr + i;
      break;
    }
  }

  if (res.ptr == NULL) return res;

  res.len = str.len - (res.ptr - str.ptr);

  for (i = str.len - 1 ; i > -1; --i) {
    if ( str.ptr[i] > ' ') {
      res.len = str.ptr + i  + 1 - res.ptr;
      break;
    }
  }

  return res;
}


MYSTR mystrsub(MYSTR str, MYSTR left, MYSTR right)
{
  char * ptr;
  MYSTR res = str;

  if ( !mystrcmp(left, mystr_null) ) {
    if ( (ptr = strfind(str.ptr, str.len, left.ptr, left.len)) != NULL) {
       res.ptr = ptr + left.len;
       res.len = str.len - (res.ptr - str.ptr);
    }
  }

  if ( !mystrcmp(right, mystr_null) ) {
    if ( (ptr = strfind(res.ptr, res.len, right.ptr, right.len)) != NULL) {
       res.len = ptr - res.ptr;
    }
  }

  return res;
}

MYSTR mystr_open_quote(MYSTR str)
{
  MYSTR res;
  res = mystrtrim(str);
  if (res.ptr[0] == '"') {
    res.ptr++;
    res.len--;
  }
  if (res.ptr[res.len - 1] == '"') {
    res.len--;
  }

  return res;
}

void mystr2str(MYSTR mystr, char * str, int str_size)
{
	if ( (mystr.len < 1) || (mystr.ptr == NULL) ) { str[0] = '\0'; return; }
	int sz = mystr.len > (str_size - 1) ? (str_size - 1) : mystr.len;
	if (sz < 1) return;
	memcpy(str, mystr.ptr, sz);
	str[sz] = '\0';
}


MYSTR str2mystr(char * str)
{
  MYSTR res;

  res.ptr = str;
  res.len = strlen(str);
  return res;
}

int mystr2int(MYSTR val)
{
  static char __mystr2int_buf[20];
  int len = val.len < 19 ? val.len : 19;
  memcpy(__mystr2int_buf, val.ptr, len);
  __mystr2int_buf[len] = '\0';
  return atoi(__mystr2int_buf);
}


int mystr2int_def(MYSTR val, int def)
{
  if (val.len == 0) return def;
  if ( (val.ptr[0] >= '0') && (val.ptr[0] <= '9') ) return mystr2int(val);
  if ( (val.ptr[0] == '-') && (val.len > 1) && (val.ptr[1] >= '0') && (val.ptr[1] <= '9') ) return mystr2int(val);
  return def;
}

MYSTR int2mystr(int val)
{
  static char __int2mystr_buf[20];
  MYSTR res;

  itoa(val, __int2mystr_buf, 10);
  res.ptr = __int2mystr_buf;
  res.len = strlen(__int2mystr_buf);
  return res;
}


MYSTR mystrfind(MYSTR s1, MYSTR s2)
{
  MYSTR res;

  if ((res.ptr = strfind(s1.ptr, s1.len, s2.ptr, s2.len)) != NULL) res.len = s2.len;
  else res.len = 0;
  return res;
}

bool mystrcat_str(PMYSTR pdst, char * ptr, int len)
{
  if ( (pdst->ptr == NULL) || (ptr == NULL) || (len == 0) ) return FALSE;
  memcpy(pdst->ptr + pdst->len, ptr, len);
  pdst->len += len;
  return TRUE;
}

bool mystrcat_str_s(PMYSTR pdst, char * ptr, int len, int dst_max_size)
{
  if (pdst->len + len > dst_max_size) return FALSE;
  return mystrcat_str(pdst, ptr, len);
}


bool mystrcat(PMYSTR pdst, MYSTR src)
{
  return mystrcat_str(pdst, src.ptr, src.len);
}

bool mystrcat_s(PMYSTR pdst, MYSTR src, int dst_max_size)
{
  return mystrcat_str_s(pdst, src.ptr, src.len, dst_max_size);
}


MYSTR mystrsubstr(MYSTR s, int pos, int len)
{
  MYSTR res;

  if (pos < 0) pos = 0;
  res.ptr = s.ptr + pos;

  if (len < 0) res.len = s.len - pos;
  else if (len > s.len - pos) res.len = s.len - pos;
  else res.len = len;

  return res;
}


bool mystrcmp(MYSTR s1, MYSTR s2)
{
  if (s1.len != s2.len) return FALSE;
  if ( (s1.len == 0) || (s1.ptr == NULL) || (s2.len == 0) || (s2.ptr == NULL)) {
    return s1.ptr == s2.ptr;
  }
  return memcmp(s1.ptr, s2.ptr, s1.len) == 0;
}

bool mystrcmp_str(MYSTR s1, char * str)
{
   return mystrcmp(s1, str2mystr(str));
}


int mystrpos(MYSTR s1, MYSTR s2)
{
    char * ptr = strfind(s1.ptr, s1.len, s2.ptr, s2.len);
    if (ptr == NULL) return -1;
    return ptr - s1.ptr;
}

int mystrpos_str(MYSTR s1, char * str)
{
  return mystrpos(s1, str2mystr(str));
}


int mystrdelim(MYSTR str, MYSTR delim, PMYSTR pmystr, int max_cnt)
{
  char * ptr = str.ptr, * ptre;
  int len;
  int cnt;

  if (str.len == 0) return 0;

  len = str.len;
  cnt = 0;
  while ( len > 0)
  {
      if ( (ptre = strfind(ptr, len, delim.ptr, delim.len)) == NULL ) {
        pmystr[cnt].len = len;
        pmystr[cnt].ptr = ptr;
        cnt++;
        break;
      } else {
        pmystr[cnt].len = ptre - ptr;
        pmystr[cnt].ptr = ptr;
        ptr = ptre + delim.len;
        len =  str.len - (ptr - str.ptr);
        cnt++;
      }
      if (cnt >= max_cnt) break;
  }

   return cnt;
}


bool str_parse_key_val(char * str, int len, char * delim, int delim_len, MYSTR * key, MYSTR * val)
{
      char * ptr;
      if (len == 0) return FALSE;
      if ( (ptr = strfind(str, len, delim, delim_len)) == NULL ) return FALSE;

      key->ptr = str;
      key->len = ptr - str;

      ptr += delim_len;

      val->ptr = ptr;
      val->len = len - (ptr - str);
      return TRUE;
}

bool mystr_parse_key_val(MYSTR str, MYSTR delim, MYSTR * key, MYSTR * val)
{
  return str_parse_key_val(str.ptr, str.len, delim.ptr, delim.len, key, val);
}

