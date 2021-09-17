#ifndef mystrH
#define mystrH

#include "mydef.h"

char * strfind(char * buf, int buf_len, char * what, int what_len);

char * int2str(int val);

typedef struct _MYSTR {
        char * ptr;        int len;} MYSTR, *PMYSTR;

#define RSTR(cstring) (cstring), (sizeof(cstring) - 1)

#define MSTRS(cstring) (sizeof(cstring) - 1)
#define MSTRR(cstring) (cstring), MSTRS(cstring)
#define MSTRI(cstring) { MSTRR(cstring) }
#define MYSTRI_NULL {(NULL), (0)}
#define MSTRSL(mstr) mstr.ptr, mstr.len

#define MYSTRS_NULL(mystr) { mystr.ptr = NULL; mystr.len = 0; }

#define STRL(str) str, strlen(str)

#define ALLOC(x)   malloc(x)
#define FREE(x)    free(x)

typedef struct _MYSTRKEYVAL {
        MYSTR key;        MYSTR val;} MYSTRKEYVAL, *PMYSTRKEYVAL;


typedef struct _MYSTRNUM {
        MYSTR name;        int num;} MYSTRNUM, PMYSTRNUM;

void mystr2str(MYSTR mystr, char * str, int str_size);

unsigned long mystr2ipv4(MYSTR str);

MYSTR str2mystr(char * str);

int mystr2int(MYSTR val);

int mystr2int_def(MYSTR val, int def);

MYSTR int2mystr(int val);


MYSTR mystrtrim(MYSTR str);

MYSTR mystr_open_quote(MYSTR str);

MYSTR mystrfind(MYSTR s1, MYSTR s2);

int mystrpos(MYSTR s1, MYSTR s2);

int mystrpos_str(MYSTR s1, char * str);

MYSTR mystrsubstr(MYSTR s, int pos, int len);

MYSTR mystrsub(MYSTR str, MYSTR left, MYSTR right);

bool mystrcmp(MYSTR s1, MYSTR s2);

bool mystrcmp_str(MYSTR s1, char * str);


bool mystrcat(PMYSTR dst, MYSTR src);

bool mystrcat_s(PMYSTR pdst, MYSTR src, int dst_max_size);

bool mystrcat_str(PMYSTR pdst, char * ptr, int len);

bool mystrcat_str_s(PMYSTR pdst, char * ptr, int len, int dst_max_size);



int mystrdelim(MYSTR str, MYSTR delim, PMYSTR pmystr, int max_cnt);

bool mystr_parse_key_val(MYSTR str, MYSTR delim, MYSTR * key, MYSTR * val);


void mystrcpy_alloc(PMYSTR pdstm, MYSTR src);

void mystrfree(PMYSTR pstr);



extern const MYSTR mystr_null;
extern const MYSTR mystr_r_n;
extern const MYSTR mystr_comma;
extern const MYSTR mystr_semicolon;
extern const MYSTR mystr_n;
extern const MYSTR mystr_n_n;
extern const MYSTR mystr_equally;
extern const MYSTR mystr_colon;
extern const MYSTR mystr_space;
extern const MYSTR mystr_less;
extern const MYSTR mystr_more;
extern const MYSTR mystr_colon_and_space;
extern const MYSTR mystr_slash;
extern const MYSTR mystr_dog;
extern const MYSTR mystr_comma_and_space;
extern const MYSTR mystr_quote;
extern const MYSTR mystr_r_n_r_n;

#endif
