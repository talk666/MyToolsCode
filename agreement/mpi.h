#if !defined(AFX_MPI_H__B3328615_E05B_11D4_961E_0050FC0F4715__INCLUDED_)
#define AFX_MPI_H__B3328615_E05B_11D4_961E_0050FC0F4715__INCLUDED_

#include <stdint.h>

#ifdef _MSC_VER
	#if _MSC_VER > 1000
		#pragma once
	#endif // _MSC_VER > 1000
#endif // _MSC_VER

//#if !defined(BYTE)
//typedef unsigned char BYTE;
//#endif

#ifndef NULL
  #define NULL	0L
#endif // NULL


#define MPI_LENGTH		8


#define POSITIVE 1
#define NEGTIVE (-1)

#define HEX28BITS 0xFFFFFFF
#define HEX32BITS 0xFFFFFFFF
// #define MPI_INFINITE 0xFFFFFFF
#define UNSIGNEDLEFTBIT 0x80000000
#define BITS_OF_INT 32


#if 0
	#define DOUBLE_INT unsigned __int64
	#define __max(x,y) (x>y?x:y)
	#define __min(x,y) (x<y?x:y)
#else	// ARM ads
	#define DOUBLE_INT unsigned long long
#ifndef   __max
	#define __max(x,y) (x>y?x:y)
#endif
#ifndef   __min
	#define __min(x,y) (x<y?x:y)
#endif  
#endif

struct CMpl;
//struct CModulus;

typedef struct CMpl CMpl;
//typedef struct CModulus CModulus;

typedef struct CMpi {
	unsigned int m_aiMyInt[MPI_LENGTH];
	unsigned int m_iCarry;
	int          m_iMySign;
	int          m_iLengthInInts;
} CMpi;

int CMpi_IsNegative(CMpi *n);
int CMpi_GetLengthInBits(CMpi *n);
int CMpi_Import(CMpi *n, unsigned char *abContent, int iLength);
//int CMpi_Export(CMpi *n, BYTE *abOutBytes);
int CMpi_Export(CMpi *n, unsigned char *abOutBytes, int iMinLength);
int CMpi_GetLengthInBytes(CMpi *n);
CMpi* CMpi_BitShiftLeft(CMpi *n, int iShiftBits);
CMpi* CMpi_BitShiftRight(CMpi *n, int iShiftBits);
CMpi* CMpi_WordShiftLeft(CMpi *a, int n);
CMpi* CMpi_WordShiftRight(CMpi *a, int n);
CMpi* CMpi_Regularize(CMpi *n);
CMpi* CMpi_ChangeSign(CMpi *n);
CMpl* CMpi_FastSquare(CMpi *n, CMpl* n2);
CMpi* CMpi_Truncate(CMpl *n, CMpi *m);
CMpl* CMpi_Multiply(CMpi *a, CMpi *b, CMpl *c);
CMpi* CMpi_Multiply_d(CMpi *n, unsigned int d);
int CMpi_EqualsTo_d(CMpi *n, unsigned int d);
//int CMpi_NotEqualsTo_d(CMpi *n, unsigned int d);
int CMpi_EqualsTo(CMpi *a, CMpi *b);
//int CMpi_NotEqualsTo(CMpi *a, CMpi *b);
//<<=
//>>=
//int operator << (const CMpi &m) const;
//int operator >> (const CMpi &m) const;
int CMpi_ComparesTo(CMpi *a, CMpi *b);
int CMpi_LargerThan_abs(CMpi *a, CMpi *b);
CMpi* CMpi_Substract2(CMpi *a, CMpi *b, CMpi *c);
CMpi* CMpi_Substract(CMpi *a, CMpi *b); // a -= b
CMpi* CMpi_Negate(CMpi *a, CMpi *b); //b = -a
CMpi* CMpi_Add2(CMpi *a, CMpi *b, CMpi *c);
CMpi* CMpi_Add(CMpi *a, CMpi *b); // a += b
CMpi* CMpi_Assign_d(CMpi *n, unsigned int iInitial);

CMpi* CMpi_Assign(CMpi *m, CMpi *x);
//CMpi* CMpi_Initialize(CMpi *m);
CMpi* CMpi_BinaryInverse2(CMpi *m, CMpi *x, CMpi *x1);



CMpl* CMpl_BitShiftRight(CMpl *n, int iShiftbits);
CMpl* CMpl_BitShiftLeft(CMpl *n, int iShiftbits);
CMpl* CMpl_WordShiftRight(CMpl *a, int n);
CMpl* CMpl_WordShiftLeft(CMpl *a, int n);
CMpl* CMpl_Initialize(CMpl *n);
CMpl* CMpl_Assign_i(CMpl *n, CMpi *m);
CMpl* CMpl_Mod(CMpl *n, CMpi *m); //n %= m
CMpl* CMpl_Substract(CMpl *a, CMpl *b); // a -= b
CMpl* CMpl_Substracti(CMpl *a, CMpi *b); // a -= b
CMpl* CMpl_Add(CMpl *a, CMpl *b); //a += b
CMpl* CMpl_Addi(CMpl *a, CMpi *b); //a += b
int CMpl_EqualsTo(CMpl *a, CMpl *b); // a == b
//static CMpl* CMpl_Reduction(CMpl *n, CMpi *m);
//static CMpl* CMpl_FastReduction(CMpl *n, CMpi *m); //for P

struct CMpl {
	CMpi h;
	CMpi l;
};

//int CModulus_GetLengthInBytes(CModulus *m);
//CModulus* CModulus_Assign(CModulus *m, CMpi *x);
//CModulus* CModulus_Initialize(CModulus *m);
//CMpi* CModulus_BinaryInverse2(CModulus *m, CMpi *x, CMpi *x1);

//struct CModulus {
//	CMpi m_oModulus;
//};



#endif // !defined(AFX_MPI_H__B3328615_E05B_11D4_961E_0050FC0F4715__INCLUDED_)

