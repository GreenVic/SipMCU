/*
 * def.h
 *
 *  Created on: 31 янв. 2020 г.
 *      Author: Rafael Boltachev
 */

#ifndef CORE_INC_MYDEF_H_
#define CORE_INC_NYDEF_H_

//#include "stm32f1xx_hal.h"

#include "stm32hal_def.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef bool
#define bool _Bool
#endif

#ifndef BOOL
#define BOOL _Bool
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//стартовый адрес флэш
//#ifndef FLASH_BASE
//#define FLASH_BASE     			0x08000000UL
//#endif


//размер страницы флэш
//#ifndef FLASH_PAGE_SIZE
//#define FLASH_PAGE_SIZE         0x400
//#endif

//конечный адрес флэш
#ifndef FLASH_BANK1_END
#define FLASH_BANK1_END 		0x0801FFFFUL
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

//количество страниц флэш
//#define FLASH_PAGE_COUNT        ((FLASH_BANK1_END - FLASH_BASE) / FLASH_PAGE_SIZE)


#define GET_BIT(v, n) 			((0u == (v & (1<<n))) ? 0u : 1u)
#define SETT_BIT(v, n) 			(v |= (1<<n))
#define CLR_BIT(v, n)       	(v &= (~(1<<n)))
#define INV_BIT(v, n)          	(v ^= (1<<n))

#ifndef RANDOM
#define RANDOM(a) (rand() % (a))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef BOUND
#define BOUND(low, high, value) MAX(MIN(high, value), low)
#endif


#endif /* CORE_INC_MYDEF_H_ */
