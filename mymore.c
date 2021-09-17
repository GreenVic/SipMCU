/*
 * mymore.cpp
 *
 *  Created on: 6 февр. 2020 г.
 *      Author: Rafael Boltachev
 */

#include "mymore.h"
#include <stdlib.h>
#include <string.h>
#include "xprintf.h"
#include <stdio.h>
//#include "myulog.h"

//парсим 2 инта из строки с разделителем delimeter
bool parse_to_int(char * str, int *val1, int *val2, const char * delimeter)
{
	char * ptr;

	if ( (ptr = strstr(str, delimeter)) == NULL) return FALSE;

	*val1 = atoi(str);
	*val2 = atoi(ptr + strlen(delimeter));

	return TRUE;
}


//возвращает prescaler и counter_period для таймеров STM, base_freq - частота на входе таймера, required_freq - требуемая частота
void get_stm32_tim_freq_param(uint32_t base_freq, uint32_t required_freq, uint16_t * prescaler, uint16_t * counter_period)
{
  uint16_t p, c;
  uint32_t freq;
  uint32_t min_diff_freq = 0xFFFFFFFF;
  uint32_t diff_freq;
  uint16_t min_diff_p_c = 0xFFFF;
  uint16_t diff_p_c;

  for (c = 2; c < 0xFFFF; c++) {
    p = base_freq / (c * required_freq);
    if (p < 2) continue;
    freq = (base_freq  / p) / c;

    if (freq > required_freq) diff_freq = freq - required_freq;
    else diff_freq = required_freq - freq;

    if (diff_freq <= min_diff_freq) {
        min_diff_freq = diff_freq;

        if (p > c) diff_p_c = p - c;
        else diff_p_c = c - p;

        if (diff_p_c <= min_diff_p_c) {
           min_diff_p_c = diff_p_c;
           *prescaler = p;
           *counter_period = c;
        }
    }
  }
}

//имя файла из пути
char * basename(char * path)
{
  if (path == NULL) return NULL;

  int len = strlen(path);
  if (len == 0) return NULL;
  while ((--len) >= 0) {
    if (path[len] == '/') {
      if (strlen(path + len + 1) == 0) return NULL;
      return path + len + 1;
    }
  }
  return path;
}


//парсим строку вида '111:222:333' в напряжения
bool parse_str_dial_voltage(char * str, float * voltage_base, float * voltage_handset_down, float * voltage_handset_up)
{
  uint8_t str_len = strlen(str);
  char * ptr;
  float vbase, vdown, vup;

  vbase = (float)atoi(str) / 10.00;
  if ( (ptr = strnstr(str, ":", str_len)) == NULL ) return FALSE;
  ++ptr;
  vdown = (float)atoi(ptr) / 10.00;
  if ( (ptr = strnstr(ptr, ":", str_len)) == NULL ) return FALSE;
  ++ptr;
  vup = (float)atoi(ptr) / 10.00;

  *voltage_base = vbase;
  *voltage_handset_down = vdown;
  *voltage_handset_up = vup;
  return TRUE;
}

//катенация с максимальным размером dstr
void mystrncat(char *dstr, char * sstr, uint16_t dstr_size)
{
	if (strlen(dstr) + strlen(sstr) > dstr_size - 1) {
		strncat(dstr, sstr, dstr_size - strlen(dstr) - 1);
	} else strcat(dstr, sstr);

}


void uint64_to_str(uint64_t value, char * str)
{
   char buf[21];
   char * ptr = buf;

   memset(buf, 0, 21);
   do
   {
      *ptr = value % 10 + '0';
      value /= 10;
      ptr++;
   }
   while (value != 0);

   int slen = strlen(buf);
   for (int i = 0; i < slen; ++i) str[i] = buf[slen - i - 1];
   str[slen] = '\0';

   //while (ptr >= buf)
   //{
//     ptr--;
  //   *str = *ptr;
     //str++;
   //}
   //*str = '\0';
}

void int64_to_str(int64_t value, char * str)
{
   if (value < 0) {
        str[0] = '-';
        uint64_to_str(-value, str + 1);
   }
   else uint64_to_str(value, str);
}

//uint64 в строку от 0 до 18 446 744 073 709 551 615
char * uint64tostr(uint64_t val, char * format)
{
	static char __uint64tostr[21];

	if (strcmp("%llu", format) == 0) uint64_to_str(val, __uint64tostr);
	else sprintf(__uint64tostr, format, val);

	return __uint64tostr;
}
//от −9223372036854775807 до +9223372036854775807
char * int64tostr(int64_t val, char * format)
{
	static char __int64tostr[21];

	if (strcmp("%lld", format) == 0) int64_to_str(val, __int64tostr);
	else sprintf(__int64tostr, format, val);

	return __int64tostr;
}

//парсим диапазон вида `100-200`
bool str_to_dapazojne(char * str, uint16_t str_len, int * min, int * max)
{
  char * ptr;

  if ( (ptr = strnstr(str, "-", str_len)) == NULL ) return FALSE;

  *min = atoi(str);
  if (*min == 0) return FALSE;
  *max = atoi(ptr + 1);
  if (*max == 0) return FALSE;
  return TRUE;
}


//оставить в версии major и minor
char * version_major_minor(char * version)
{
  static char __version_major_minor[16];
  char * ptr;

  if ((ptr = strstr(version, ".")) == NULL) return version;
  if ((ptr = strstr(ptr + 1, ".")) == NULL) return version;
  memset(__version_major_minor, 0, 16);
  memcpy(__version_major_minor, version, ptr - version);
  return __version_major_minor;
}

//это цифра int
bool is_digit(char *str, uint8_t str_len)
{
  for (uint8_t i = 0; i < str_len; ++i) {
        if ( (str[i] < '0') || (str[i] > '9') ) return FALSE;
  }
  return TRUE;
}

//тоже что и strncpy только с 0 в конце
void memstrncpy(char * dst, char * src, size_t dst_max_size)
{
	size_t len = strlen(src) < dst_max_size - 1 ? strlen(src) : dst_max_size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}


//разбиение строки по delim
void str_delim(char * str, uint32_t str_len, const char * delim, str_delim_func func)
{
  char * ptr = str, * ptre;
  uint32_t len;

  len = str_len;
  while ( len > 0)
  {
      if ( (ptre = strnstr(ptr, delim, len)) == NULL ) {
        func(ptr, len);
        break;
      } else {
        func(ptr, ptre - ptr);
        ptr = ptre + strlen(delim);
        len =  str_len - (ptr - str);
      }
  }
}

//сумма битов
uint8_t bit_sum(uint8_t * ptr, uint8_t min_bit, uint8_t max_bit)
{
	uint8_t res = 0;
	uint8_t byte_num;
	uint8_t bit_num;

	for (uint8_t i = min_bit; i <= max_bit; ++i) {
		byte_num = (uint8_t)(i / 8);
		bit_num = i - byte_num * 8;
		if ( GET_BIT(ptr[byte_num], bit_num) ) ++res;
	}
	return res;
}


//поиск символа в строке
char * strnchr(char * str, char c, uint16_t str_len)
{
  for (uint16_t i = 0; i < str_len; i++) {
    if (str[i] == c) return str + i;
  }
  return NULL;
}

//трим с длиной строки
char * strntrim(char * str, uint16_t str_len)
{
  int i;
  char * res = NULL;

  if (str == NULL) return NULL;

  if (str_len == 0) return NULL;

  for (i = 0; i < str_len; ++i) {
    if ( str[i] > ' ') {
      res = str + i;
      break;
    }
  }

  if (res == NULL) return NULL;

  for (i = str_len - 1 ; i > -1; --i) {
    if ( str[i] > ' ') {
      if (i != str_len - 1) str[i + 1] = '\0';
      break;
    }
  }

  return res;
}

//трим
char * strtrim(char * str)
{
	return strntrim(str, strlen(str));
}

//сравнить строки
bool strstrcmp(char * str,  const char * str_c)
{
	if (str == NULL) return FALSE;
	if ( strlen(str) != strlen(str_c) ) return FALSE;
	return (strcmp(str, str_c) == 0);
}


bool mystrncmp(char * str, uint16_t str_len, const char * str_c)
{
	if (str == NULL) return FALSE;
	if ( str_len != strlen(str_c) ) return FALSE;
	return (strncmp(str, str_c, str_len) == 0);
}


//сравнить строку с памятью
bool memstrcmp(char * buf, uint16_t buf_len, const char * str)
{
	if (buf == NULL) return FALSE;
	if (buf_len == 0) return FALSE;
	if (buf_len < strlen(str) ) return FALSE;
	return (memcmp(buf, str,  strlen(str)) == 0);
}

//в воротах
bool in_gate(float val, float min_val, float max_val)
{
	return  (val >= min_val) && (val <= max_val);
}

//замена -u _printf_float
char * float_to_str(float f)
{
	static char __float_to_str[64];

	float m;

	if (f < 0) m = -f;
	else m = f;

	xsnprintf(__float_to_str, 64, "%d.%d", (int)f, (int)((m - (float)(int)m) * 10.0) );
	return __float_to_str;
}

//замена -u _printf_float
float str_to_float(char * str)
{
   char * ptr;
   float res;
   float f;

   res = (float)atoi(str);

   if ( (ptr = strstr(str, ".")) == NULL) ptr = strstr(str, ",");
   if (ptr != NULL) {
     f = (float)atoi(ptr + 1);
     if (f < 10.0) f /= 10.0;
     else if (f < 100.0) f /= 100.0;
     else if (f < 1000.0) f /= 1000.0;
     else if (f < 10000.0) f /= 10000.0;
     else if (f < 100000.0) f /= 100000.0;
     else if (f < 1000000.0) f /= 1000000.0;
     else f = 0.0;
     if (res < 0) {
       res -= f;
       res -= 0.00001;
     }
     else {
       res += f;
       res += 0.00001;
     }
   }

   return res;
}


//парсим ip строчку (аналог inet_addr)
void ip_str_parse(char * str, unsigned char *ipaddr)
{
  char * ptr = str;
  int i = 0;

  do
  {
    ipaddr[i] = (unsigned char)atoi(ptr);
    ++i;
    if ( (ptr = strchr(ptr, '.')) != NULL ) ++ptr;
  }
  while ( ptr != NULL );
}


//конвертируем ip в строку
void ip2str(unsigned char *ipaddr, char * str)
{
  xsprintf(str, "%d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
}

//возвращает строчку ip
char * get_ip_str(unsigned char *ipaddr)
{
  static char __get_ip_str[16];
  ip2str(ipaddr, __get_ip_str);
  return __get_ip_str;
}


//inet_addr
unsigned long myinet_addr(char * str)
{
	unsigned long res = 0;

	ip_str_parse(str, (unsigned char *)&res);
	return res;
}


//память в строку
void mem2str(unsigned char *mem, unsigned short len, char * str, unsigned short str_size)
{
  unsigned short i;
  char tmp[3];
  unsigned short sz = MIN(len, (str_size / 2) - 1);

  str[0] = '\0';
  for (i = 0; i < sz; ++i) {
    sprintf(tmp, "%02X", mem[i]);
    strcat(str, tmp);
  }
}


//из мака в строку
void mac2str(unsigned char *mac, char * delim, char * str)
{
  unsigned char i;
  char tmp[3];

  str[0] = '\0';
  for (i = 0; i < 6; ++i) {
    if (strlen(str) != 0) strcat(str, delim);
    sprintf(tmp, "%02X", mac[i]);
    strcat(str, tmp);
  }
}

//возвращает строку мака
char * get_mac_str(unsigned char *mac, char * delim)
{
  static char __get_mac_str[18];

  mac2str(mac, delim, __get_mac_str);
  return __get_mac_str;
}

//из строки в мак
void str2mac(char * str, char * delim, unsigned char *mac)
{
  char * ptr = str;
  unsigned char i = 0;

  do
  {
    mac[i++] = strtoul(ptr, NULL, 16);
    if ( (ptr = strstr(ptr, delim)) != NULL ) ++ptr;
  }
  while ( ptr != NULL );

}


//htonl
//unsigned long htonl(unsigned long v)
//{
//    unsigned char *s = (unsigned char *)&v;
//    return (unsigned long)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
//}

//кс
uint64_t crc_sum(unsigned char * buf, uint32_t buf_len)
{
	uint64_t crc = 0;

	for (uint32_t i = 0; i < buf_len; i++) crc += buf[i];
	return crc;
}

/*
// Find an instance of substr in an array of characters
// the array of characters does not have to be null terminated
// the search is limited to the first n characters in the array.
char *strnstr(char *str, const char *substr, size_t n)
{
    char *p = str, *pEnd = str+n;
    size_t substr_len = strlen(substr);

    if(0 == substr_len)
        return str;//the empty string is contained everywhere.

    pEnd -= (substr_len - 1);
    for(;p < pEnd; ++p)
    {
        if(0 == strncmp(p, substr, substr_len))
            return p;
    }
    return NULL;
}
*/

//поиск в строке str, строку от begin до end
char * find_str_gate(char * str, unsigned short str_len, char * begin, char * end, unsigned short *len)
{
  char * ptrb, * ptre;

  if ( (ptrb = strnstr(str, begin, str_len)) != NULL ) {
    ptrb += strlen(begin);
    if ( (ptre = strnstr(ptrb, end, str_len - (ptrb - str))) != NULL ) {
      *len = ptre - ptrb;
      return ptrb;
    }
  }
  return NULL;
}

//поиск в области памяти buf, участка what
void * find_mem(void * buf, unsigned int buf_len, void * what, unsigned int what_len)
{
  unsigned char * ptr = (unsigned char *)buf, * ptre = (unsigned char *)buf + buf_len - what_len;
  unsigned char what_first = *(unsigned char *)what;

  while (ptr <= ptre)
  {
    if (*ptr == what_first) {
        if (memcmp(ptr, what, what_len) == 0) return ptr;
    }
    ++ptr;
  }

  return NULL;
}

//разница двух тиков
uint32_t HAL_Tick_Diff(uint32_t to, uint32_t tn)
{
	if (tn >= to) return tn - to;
	else return 0xFFFFFFFF - to + tn;
}

//разница с текущим HAL_GetTick
uint32_t HAL_Diff(uint32_t t)
{
	return HAL_Tick_Diff(t, HAL_GetTick());
}


//установка таймера
void set_tick(uint32_t * tick)
{
	*tick = HAL_GetTick();
    if (*tick == 0) *tick = 1;
}

//время прошло
bool tick_passed(uint32_t tick, uint32_t timeout_tick)
{
	return HAL_Diff(tick) > timeout_tick;
}

//время прошло с переустановкой времени
bool tick_passed_per(uint32_t * tick, uint32_t timeout_tick)
{
	bool res;

	res = tick_passed(*tick, timeout_tick);
	if (res) *tick = HAL_GetTick();

	return res;
}


//время прошло с проверкой на ноль
bool tick_passed0(uint32_t * tick, uint32_t timeout_tick) {
	if (*tick == 0) return FALSE;
	return tick_passed(*tick, timeout_tick);
}


//установка переменной val в диапазон от min_val до max_val
void normalization_val(float * val, float min_val, float max_val)
{
	if (*val < min_val) *val = min_val;
	else if (*val > max_val) *val = max_val;
}

//установка переменной val в диапазон от 0.0 до 1.0
void normalization01(float * val)
{
	normalization_val(val, 0.0, 1.0);
}


uint32_t get_adc_value(ADC_HandleTypeDef* hadc, uint32_t Timeout)
{
	uint32_t adc;

	HAL_ADC_Start(hadc); // запускаем преобразование сигнала АЦП
    HAL_ADC_PollForConversion(hadc, Timeout); // ожидаем окончания преобразования
    adc = HAL_ADC_GetValue(hadc); // читаем полученное значение в переменную adc
    HAL_ADC_Stop(hadc); // останавливаем АЦП
	return adc;
}

#ifdef STMF1XX_CHIPSET
uint32_t get_injected_adc_value(ADC_HandleTypeDef* hadc, uint32_t InjectedRank, uint32_t Timeout)
{
	uint32_t res;

	HAL_ADCEx_InjectedStart(hadc); // запускаем опрос инжект. каналов
	HAL_ADC_PollForConversion(hadc, Timeout); // ждём окончания
	res = HAL_ADCEx_InjectedGetValue(hadc, InjectedRank);
	HAL_ADCEx_InjectedStop(hadc);
	return res;
}
#endif

//лучшая длина
uint8_t best_length(uint64_t val)
{
  if (val <= 0xFF) return 1;
  else if (val <= 0xFFFF) return 2;
  else if (val <= 0xFFFFFF) return 3;
  else if (val <= 0xFFFFFFFF) return 4;
  else if (val <= 0xFFFFFFFFFF) return 5;
  else if (val <= 0xFFFFFFFFFFFF) return 6;
  else if (val <= 0xFFFFFFFFFFFFFF) return 7;
  else return 8;
}


/*
static char __get_fmt_str_buf[2][MAX_GET_FMT_BUF_SIZE];
uint8_t __get_fmt_str_buf_num = 0;

char * get_vfmt_str(const char * fmt, va_list arp)
{
	__get_fmt_str_buf_num++;
	if (__get_fmt_str_buf_num > 1) __get_fmt_str_buf_num = 0;
	xvsnprintf(__get_fmt_str_buf[__get_fmt_str_buf_num], MAX_GET_FMT_BUF_SIZE, fmt, arp);
	__get_fmt_str_buf[__get_fmt_str_buf_num][MAX_GET_FMT_BUF_SIZE - 1] = 0;
	return __get_fmt_str_buf[__get_fmt_str_buf_num];
}

//строка fmt
char * get_fmt_str(const char * fmt, ... )
{
	va_list arg;
	char * str;

	va_start(arg, fmt);
	str = get_vfmt_str(fmt, arg);
	va_end(arg);

	return str;
}
*/

//индекс минимального занчения в массиве arr
int get_min_arr_index(uint32_t * arr, int arr_size)
{
	int min_index = 0;

	for (int i = 1; i < arr_size; i++) {
		if (arr[i] < arr[min_index]) min_index = i;
	}
	return min_index;
}


