#ifndef md5H
#define md5H


#ifdef __alpha
typedef unsigned int UINT4;
#else
typedef unsigned long int UINT4;
#endif

#ifndef ULONG
typedef unsigned long ULONG;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef UCHAR
typedef unsigned char UCHAR;
#endif


#define MD5_INIT_STATE_0 0x67452301
#define MD5_INIT_STATE_1 0xefcdab89
#define MD5_INIT_STATE_2 0x98badcfe
#define MD5_INIT_STATE_3 0x10325476

void MD5Init(void);
void MD5Update(unsigned char *bug, unsigned int len);
void MD5Final(char* cReturnStr);
void Transform(UINT4 *buf, UINT4 *in);
void GetMD5(char* pBuf, UINT nLength,char* cReturnStr);

#endif
