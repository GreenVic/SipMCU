/*
 * mymore.h
 *
 *  Created on: 6 февр. 2020 г.
 *      Author: Rafael Boltachev
 */

#ifndef LIB_MYMORE_H_
#define LIB_MYMORE_H_

#include "mydef.h"
#include <stdarg.h>
#include "stm32hal_def.h"


//парсим 2 инта из строки с разделителем delimeter
bool parse_to_int(char * str, int *val1, int *val2, const char * delimeter);

//возвращает prescaler и counter_period для таймеров STM, base_freq - частота на входе таймера, required_freq - требуемая частота
void get_stm32_tim_freq_param(uint32_t base_freq, uint32_t required_freq, uint16_t * prescaler, uint16_t * counter_period);

//имя файла из пути
char * basename(char * path);

//парсим строку вида '111:222:333' в напряжения
bool parse_str_dial_voltage(char * str, float * voltage_base, float * voltage_handset_down, float * voltage_handset_up);

//катенация с максимальным размером dstr
void mystrncat(char *dstr, char * sstr, uint16_t dstr_size);

//из целого в строку
void uint64_to_str(uint64_t value, char * str);
char * uint64tostr(uint64_t val, char * format);
#define UINT64TOSTR(v) uint64tostr(v, "%llu") //не пашет
#define UINT32TOSTR(v) uint64tostr(v, "%lu")
#define UINT16TOSTR(v) uint64tostr(v, "%u")

void int64_to_str(int64_t value, char * str);
char * int64tostr(int64_t val, char * format);
#define INT64TOSTR(v) int64tostr(v, "%lld") //не пашет
#define INT32TOSTR(v) int64tostr(v, "%ld")
#define INT16TOSTR(v) int64tostr(v, "%d")

//из строки в целое
#define STRTOUINT64(v) strtoull(v, NULL, 0)
#define STRTOUINT32(v) strtoul(v, NULL, 0)

//из секунд в миллисекунд
#define SEC2MSEC(x)	(x * 1000)

//из миллисекунд в секунды
#define MSEC2SEC(x)	(int)(x / 1000)

//парсим диапазон вида `100-200`
bool str_to_dapazojne(char * str, uint16_t str_len, int * min, int * max);

//оставить в версии major и minor
char * version_major_minor(char * version);

//это цифра int
bool is_digit(char *str, uint8_t str_len);

//тоже что и strncpy только с 0 в конце
void memstrncpy(char * dst, char * src, size_t dst_max_size);

//разбиение строки по delim
typedef void  (*str_delim_func)(char * str, uint16_t len);
void str_delim(char * str, uint32_t str_len, const char * delim, str_delim_func func);

//сумма битов
uint8_t bit_sum(uint8_t * ptr, uint8_t min_bit, uint8_t max_bit);

//поиск символа в строке
char * strnchr(char * str, char c, uint16_t str_len);

//трим с длиной строки
char * strntrim(char * str, uint16_t str_len);

//трим
char * strtrim(char * str);

//сравнить строки
bool strstrcmp(char * str,  const char * str_c);

bool mystrncmp(char * str, uint16_t str_len, const char * str_c);

//сравнить строку с памятью
bool memstrcmp(char * buf, uint16_t buf_len, const char * str);

//в воротах
bool in_gate(float val, float min_val, float max_val);

//замена -u _printf_float
char * float_to_str(float f);

//замена -u _printf_float
float str_to_float(char * str);

//парсим ip строчку (аналог inet_addr)
void ip_str_parse(char * str, unsigned char *ipaddr);

//конвертируем ip в строку
void ip2str(unsigned char *ipaddr, char * str);

//возвращает строчку ip
char * get_ip_str(unsigned char *ipaddr);

//inet_addr
unsigned long myinet_addr(char * str);

//память в строку
void mem2str(unsigned char *mem, unsigned short len, char * str, unsigned short str_size);

//из мака в строку
void mac2str(unsigned char *mac, char * delim, char * str);

//возвращает строку мака
char * get_mac_str(unsigned char *mac, char * delim);

//из строки в мак
void str2mac(char * str, char * delim, unsigned char *mac);


//htonl
//unsigned long htonl(unsigned long v);

#ifndef htonl
#define htonl(a)			( (((a)>>24)&0xff) | (((a)>>8)&0xff00) |\
							(uint32_t)((((uint64_t)a)<<8) & 0xff0000) |\
							(uint32_t)((((uint64_t)a)<<24) & 0xff000000) )
#endif

//кс
uint64_t crc_sum(unsigned char * buf, uint32_t buf_len);

//поиск в строке str, строку от begin до end
char * find_str_gate(char * str, unsigned short str_len, char * begin, char * end, unsigned short *len);

// Find an instance of substr in an array of characters
//char *strnstr(char *str, const char *substr, size_t n);

//поиск в области памяти buf, участка what
void * find_mem(void * buf, unsigned int buf_len, void * what, unsigned int what_len);

//разница двух тиков
uint32_t HAL_Tick_Diff(uint32_t to, uint32_t tn);

//разница с текущим HAL_GetTick
uint32_t HAL_Diff(uint32_t t);

//установка таймера
void set_tick(uint32_t * val);

//время прошло
bool tick_passed(uint32_t tick, uint32_t timeout_tick);

//время прошло с переустановкой времени
bool tick_passed_per(uint32_t * tick, uint32_t timeout_tick);

//время прошло с проверкой на ноль
bool tick_passed0(uint32_t * tick, uint32_t timeout_tick);

//установка переменной val в диапазон от min_val до max_val
void normalization_val(float * val, float min_val, float max_val);

//установка переменной val в диапазон от 0.0 до 1.0
void normalization01(float * val);

//чтение напряжения с ADC
uint32_t get_adc_value(ADC_HandleTypeDef* hadc, uint32_t Timeout);

#ifdef STMF1XX_CHIPSET
//чтение напряжения с ADC с инжектированного канала
uint32_t get_injected_adc_value(ADC_HandleTypeDef* hadc, uint32_t InjectedRank, uint32_t Timeout);
#endif

//лучшая длина
uint8_t best_length(uint64_t val);

//#define MAX_GET_FMT_BUF_SIZE	256
//строка fmt
//char * get_fmt_str(const char * fmt, ... );
//char * get_vfmt_str(const char * fmt, va_list arp);

//индекс минимального занчения в массиве arr
int get_min_arr_index(uint32_t * arr, int arr_size);

#endif /* LIB_MYMORE_H_ */
