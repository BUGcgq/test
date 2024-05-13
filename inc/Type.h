/************************************************************
Copyright (C), 2012-2020, 深圳英可瑞科技开发有限公司
文 件 名：Type.h
版    本：1.00
创建日期：2012-04-27
作    者：郭数理
功能描述：将标准数据类型重新定义

函数列表：

修改记录：
	作者      日期        版本     修改内容
	郭数理    2012-04-27  1.00     创建
**************************************************************/

#ifndef __TYPE_H_
#define __TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef   unsigned char     BOOL_T;
typedef   char              C8_T; 
typedef   unsigned char     U8_T;
typedef   signed   char     S8_T; 
typedef   unsigned short    U16_T; 
typedef   signed   short    S16_T; 
typedef   unsigned int      U32_T; 
typedef   signed   int      S32_T;
typedef   unsigned long long  U64_T;
typedef   signed long long  S64_T; 
typedef   float             F32_T; 
typedef   double            F64_T; 
typedef   char *            STR_T;
 
#define BYTE           S8_T
#define UBYTE          U8_T
#define WORD           S16_T
#define UWORD          U16_T
#define LONG           S32_T
#define ULONG          U32_T
#define LLONG          S64_T
#define ULLONG         U64_T

#ifndef TRUE
#define TRUE           1
#endif

#ifndef FALSE
#define FALSE          0
#endif


#ifdef __cplusplus
}
#endif

#endif
