#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

extern void printHex(void *buf, int n);
extern void printCMpi(CMpi *a);

CMpi* CMpi_Assign_d(CMpi *n, unsigned int iInitial)
{
	n->m_iMySign = POSITIVE;
	
	if (iInitial==0)
		n->m_iLengthInInts = 0;
	else
		n->m_iLengthInInts = 1;
	n->m_aiMyInt[0]= iInitial;

	n->m_iCarry = 0;
	return n;
}

CMpi* CMpi_Assign(CMpi *a, CMpi *b) // b = a
{
	int i;
	b->m_iMySign = a->m_iMySign;
	b->m_iLengthInInts =  a->m_iLengthInInts;
	b->m_iCarry = 0;
	for (i = b->m_iLengthInInts - 1; i >= 0; --i) {
		b->m_aiMyInt[i] = a->m_aiMyInt[i];
	}
	return b;
}

CMpi* CMpi_Add(CMpi *a, CMpi *b)
{
	int i,j;
	register DOUBLE_INT direg;
	CMpi temp;
	a->m_iCarry = 0;
//	if (m_iMySign == MPI_INFINITE)
//		return *this;
//	if (m.m_iMySign == MPI_INFINITE)
//	{
//		m_iMySign = MPI_INFINITE;
//		return *this;
//	}
	if (a->m_iMySign==POSITIVE)
	{
		if (b->m_iMySign!=POSITIVE)
		{
			return CMpi_Substract(a, CMpi_Negate(b, &temp));
		}
	}
	else
	{
		if (b->m_iMySign==POSITIVE)
		{
			CMpi_Negate(a, &temp);
			return CMpi_Substract2(b, &temp, a);
		}
	}

	j=__min(a->m_iLengthInInts, b->m_iLengthInInts);
	direg=0;
	for (i=0;i<j;i++)
	{	
		direg+=a->m_aiMyInt[i];
		direg+=b->m_aiMyInt[i];
		a->m_aiMyInt[i]=(unsigned int)direg;
		direg>>=32;
	}

	if (a->m_iLengthInInts > b->m_iLengthInInts)
	{
		j = a->m_iLengthInInts;
		for (; i < j; i++)
		{
			direg+=a->m_aiMyInt[i];
			a->m_aiMyInt[i]=(unsigned int)direg;
			direg>>=32;
		}
	}
	else
	{
		j = b->m_iLengthInInts;
		for (; i < j; i++)
		{
			direg+=b->m_aiMyInt[i];
			a->m_aiMyInt[i]=(unsigned int)direg;
			direg>>=32;
		}
	}

	if (j==MPI_LENGTH)
	{
		a->m_iCarry = (unsigned int)direg;
		a->m_iLengthInInts = j;
	}
	else
	{
		a->m_aiMyInt[j]=(unsigned int)direg;
		if (a->m_aiMyInt[j])
			a->m_iLengthInInts=j+1;
		else
			a->m_iLengthInInts=j;
	}
	return a;
}


CMpi* CMpi_Add2(CMpi *a, CMpi *b, CMpi *c)
{
	return CMpi_Add(CMpi_Assign(a, c), b);
}

CMpi* CMpi_Negate(CMpi *a, CMpi *b)
{
	CMpi_Assign(a, b);
	if (a->m_iMySign == POSITIVE)
		b->m_iMySign = NEGTIVE;
	else
		b->m_iMySign = POSITIVE;

	return b;
}

CMpi* CMpi_Substract2(CMpi *a, CMpi *b, CMpi *c)
{
	return CMpi_Substract(CMpi_Assign(a, c), b);
}

CMpi* CMpi_Substract(CMpi *a, CMpi *b)
{
	CMpi temp;
	int i,j;
	register DOUBLE_INT direg;
	const unsigned int *pm1, *pm2;
	int len2;

	a->m_iCarry = 0;
	if (a->m_iMySign != b->m_iMySign)
	{
		CMpi_Negate(b, &temp);
		return CMpi_Add(a, &temp);
	}

	if (CMpi_LargerThan_abs(b, a))
	{
		pm2= a->m_aiMyInt;
		len2 = a->m_iLengthInInts;
		pm1= b->m_aiMyInt;

		if (b->m_iMySign == POSITIVE)
			a->m_iMySign = NEGTIVE;
		else
			a->m_iMySign = POSITIVE;
	}
	else
	{
		pm1 = a->m_aiMyInt;
		pm2 = b->m_aiMyInt;
		len2 = b->m_iLengthInInts;
	}
	j=__max(a->m_iLengthInInts,b->m_iLengthInInts);

#ifdef _MSC_VER
	direg=0x100000000L + pm1[0];
#else
	direg=0x100000000ll+ pm1[0];
#endif
	for (i=0;i<len2;i++)
	{
		direg -= pm2[i];
		a->m_aiMyInt[i]=(unsigned int)direg;
		direg >>= 32;
		direg += pm1[i+1];
		direg += 0xffffffffL;
	}

	for (; i < j; i++)
	{
		a->m_aiMyInt[i]=(unsigned int)direg;
		direg >>= 32;
		direg += pm1[i+1];
		direg += 0xffffffffL;
	}

	a->m_iLengthInInts=j;
	for (i=j-1;i>=0;i--)
	{
		if (a->m_aiMyInt[i])
			break;
		a->m_iLengthInInts=i;
	}
	return a;
}

int CMpi_LargerThan_abs(CMpi *a, CMpi *b)
{
	int i,j;
	j = __max(a->m_iLengthInInts,b->m_iLengthInInts);
	for (i=j-1;i>=0;i--)
	{
		if (a->m_iLengthInInts>i && b->m_iLengthInInts>i)
		{
			if (a->m_aiMyInt[i]>b->m_aiMyInt[i])
				return 1;
			if (a->m_aiMyInt[i]<b->m_aiMyInt[i])
				return 0;
		}
		else
		{
			if (a->m_iLengthInInts<=i)
			{
				if (b->m_aiMyInt[i])
					return 0;
				else
					continue;
			}
			if (b->m_iLengthInInts<=i)
			{
				if (a->m_aiMyInt[i])
					return 1;
				else
					continue;
			}
		}
	}
	return 0;
}

int CMpi_EqualsTo(CMpi *a, CMpi *b)
{
	int i,j;
	j = __max(a->m_iLengthInInts,b->m_iLengthInInts);
	for (i=j-1;i>=0;i--)
	{
		if (a->m_iLengthInInts>i && b->m_iLengthInInts>i)
		{
			if (a->m_aiMyInt[i]!=b->m_aiMyInt[i])
				return 0;
			else
				continue;
		}
		if (a->m_iLengthInInts<=i)
		{
			if (b->m_aiMyInt[i])
				return 0;
			else
				continue;
		}
		if (b->m_iLengthInInts<=i)
		{
			if (a->m_aiMyInt[i])
				return 0;
			else
				continue;
		}
	}
	return 1;
}

CMpi* CMpi_Multiply_d(CMpi *a, unsigned int n)
{
	register DOUBLE_INT direg;
	unsigned int icarry = 0;
	int i;
	a->m_iCarry = 0;
	if (n==0)
	{
		a->m_iMySign = POSITIVE;
		a->m_iLengthInInts=0;
		a->m_aiMyInt[0]= 0;
		return a;
	}

	for (i=0;i<a->m_iLengthInInts;i++)
	{
		direg = a->m_aiMyInt[i];
		direg *= n;
		direg += icarry;
		a->m_aiMyInt[i] = (unsigned int) direg;
		icarry = (unsigned int)(direg>>32);
	}
	if (i==MPI_LENGTH)
	{
		a->m_iCarry = icarry;
	}
	else
	{
		if ((a->m_aiMyInt[i]=icarry))
			a->m_iLengthInInts=i+1;
	}
	return a;
}

//		g_paramA.m_aiMyInt[0] = 0xFFFFFFFC;
//		g_paramA.m_aiMyInt[1] = 0xFFFFFFFF;
//		g_paramA.m_aiMyInt[2] = 0x00000000;
//		g_paramA.m_aiMyInt[3] = 0xFFFFFFFF;
//		g_paramA.m_aiMyInt[4] = 0xFFFFFFFF;
//		g_paramA.m_aiMyInt[5] = 0xFFFFFFFF;
//		g_paramA.m_aiMyInt[6] = 0xFFFFFFFF;
//		g_paramA.m_aiMyInt[7] = 0xFFFFFFFE;

/*
CMpl CMpi::FastMultiplication(const CMpi &m) const
{
	CMpl temp(*this);
	temp <<= MPI_LENGTH;

	CMpl temp2(*this);
	temp2 <<= (MPI_LENGTH-1);
	temp -= temp2;

	temp2 = *this;
	temp2 <<= 3;
	temp -= temp2;

	temp2 = *this;
	temp2 <<= 2;
	temp += temp2;

	temp2 = *this;
	temp2.BitShiftLeft(2);

	return temp -= temp2;
}
*/

CMpl* CMpi_Multiply(CMpi *a, CMpi *b, CMpl *c)
{
//	if (m.m_aiMyInt[0] == 0xFFFFFFFC
//		&& m.m_aiMyInt[1] == 0xFFFFFFFF
//		&& m.m_aiMyInt[2] == 0x00000000
//		&& m.m_aiMyInt[3] == 0xFFFFFFFF
//		&& m.m_aiMyInt[4] == 0xFFFFFFFF
//		&& m.m_aiMyInt[5] == 0xFFFFFFFF
//		&& m.m_aiMyInt[6] == 0xFFFFFFFF
//		&& m.m_aiMyInt[7] == 0xFFFFFFFE
//		&& m.m_iLengthInInts == MPI_LENGTH)
//	{
//		return FastMultiplication(m);
//	}

	DOUBLE_INT direg = 0,sum;
	int i,j,k,len, n;
	unsigned int r[MPI_LENGTH*2] = {
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0
	};

	int rlen = 0;
	len = a->m_iLengthInInts + b->m_iLengthInInts;

	for (i=0;i<MPI_LENGTH*2;i++)
		r[i]=0;

	for (k=0;k<len;k++)
	{
		for (i=0, n=k-i;i<=k;i++, n--)
		{
			if ((a->m_iLengthInInts>i) && b->m_iLengthInInts>n)
			{
				direg = a->m_aiMyInt[i];
				direg *= b->m_aiMyInt[n];

				sum = direg+r[k];
				r[k] = (unsigned int)sum;
				sum >>= BITS_OF_INT;

				sum += r[k+1];
				r[k+1] = (unsigned int)sum;
				sum >>= BITS_OF_INT;

				j = k+2;
				while (sum)
				{
					sum += r[j];
					r[j++] = (unsigned int)sum;
					sum >>= BITS_OF_INT;
				}
			}
			
		}
	}

	for (i=MPI_LENGTH*2-1;i>=0;i--)
	{
		if (r[i])
			break;
	}
	rlen = i+1;

	if (rlen>MPI_LENGTH)
	{
		c->l.m_iLengthInInts = MPI_LENGTH;
		c->h.m_iLengthInInts = rlen - MPI_LENGTH;

		for (i=0;i<c->h.m_iLengthInInts;i++)
			c->h.m_aiMyInt[i]=r[i+MPI_LENGTH];
	}
	else
	{
		c->l.m_iLengthInInts = rlen;
		c->h.m_iLengthInInts = 0;
	}
	for (i=0;i<c->l.m_iLengthInInts;i++)
		c->l.m_aiMyInt[i]=r[i];

	if (a->m_iMySign == b->m_iMySign)
		c->h.m_iMySign = c->l.m_iMySign = POSITIVE;
	else
		c->h.m_iMySign = c->l.m_iMySign = NEGTIVE;
	c->l.m_iCarry = c->h.m_iCarry = 0;

	return c;
}

CMpi* CMpi_Truncate(CMpl *a, CMpi *b)
{
	CMpi_Assign(&a->l, b);
	b->m_iCarry = 0;
	return b;
}

int CMpi_EqualsTo_d(CMpi *a, unsigned int n)
{
	int i;
	if ((a->m_iLengthInInts==1 ) && (n==a->m_aiMyInt[0]))
		return 1;
	if ((a->m_iLengthInInts==0 ) && n==0)
		return 1;
	for (i=1;i<a->m_iLengthInInts;i++)
	{
		if (a->m_aiMyInt[i])
			return 0;
	}
	if (n==a->m_aiMyInt[0])
		return 1;
	return 0;
}

//int CMpi::operator !=(const unsigned int n) const
//{
//	return !(operator ==(n));
//}

// g_paramFieldP.m_oModulus.m_aiMyInt[0] = 0xFFFFFFFF;
// g_paramFieldP.m_oModulus.m_aiMyInt[1] = 0xFFFFFFFF;
// g_paramFieldP.m_oModulus.m_aiMyInt[2] = 0x00000000;
// g_paramFieldP.m_oModulus.m_aiMyInt[3] = 0xFFFFFFFF;
// g_paramFieldP.m_oModulus.m_aiMyInt[4] = 0xFFFFFFFF;
// g_paramFieldP.m_oModulus.m_aiMyInt[5] = 0xFFFFFFFF;
// g_paramFieldP.m_oModulus.m_aiMyInt[6] = 0xFFFFFFFF;
// g_paramFieldP.m_oModulus.m_aiMyInt[7] = 0xFFFFFFFE;

static CMpl* CMpl_FastReduction(CMpl *n, CMpi *m)
{
	int i, tflag;
	CMpl tempH;

	tflag = n->l.m_iMySign;
	n->h.m_iMySign = n->l.m_iMySign = POSITIVE;

	i = n->h.m_iLengthInInts;
	while (--i >= 0)
	{
		while (n->h.m_aiMyInt[i])
		{
			CMpi_Assign_d(&tempH.h, 0);
			CMpi_Assign_d(&tempH.l, 0);
			tempH.l.m_aiMyInt[7] = tempH.l.m_aiMyInt[0] = n->h.m_aiMyInt[i];
			tempH.l.m_aiMyInt[6] = tempH.l.m_aiMyInt[5] = tempH.l.m_aiMyInt[4] = tempH.l.m_aiMyInt[2] = tempH.l.m_aiMyInt[1] = 0;
			tempH.l.m_aiMyInt[2] = 0 - n->h.m_aiMyInt[i];
			tempH.l.m_aiMyInt[3] = n->h.m_aiMyInt[i] - 1;
			tempH.l.m_iLengthInInts = 8;

			CMpl_WordShiftLeft(&tempH, i);

			n->h.m_aiMyInt[i] = 0;

			// add
			CMpl_Add(n, &tempH);
		}
	}

	n->h.m_iLengthInInts = 0;
	n->h.m_iMySign = tflag;

	// here, m must > 0. The field.
	if (CMpi_ComparesTo(&n->l, m) == 1)
		CMpi_Substract(&n->l, m);

	if (CMpi_ComparesTo(&n->l, m) == 0)
		CMpi_Assign_d(&n->l, 0);
	else
		n->l.m_iMySign = tflag;
	return n;
}

static CMpl* CMpl_Reduction(CMpl *n, CMpi *m)
{
	if (m->m_aiMyInt[0] == 0xFFFFFFFF
		&& m->m_aiMyInt[1] == 0xFFFFFFFF
		&& m->m_aiMyInt[2] == 0x00000000
		&& m->m_aiMyInt[3] == 0xFFFFFFFF
		&& m->m_aiMyInt[4] == 0xFFFFFFFF
		&& m->m_aiMyInt[5] == 0xFFFFFFFF
		&& m->m_aiMyInt[6] == 0xFFFFFFFF
		&& m->m_aiMyInt[7] == 0xFFFFFFFE
		&& m->m_iLengthInInts == MPI_LENGTH)
	{
		return CMpl_FastReduction(n, m);
	}

	CMpi odiv;
	CMpi mt;
	CMpl temp,tdiv;
	int i,j,mflag,len;
	unsigned int ui,uj,flag;
	DOUBLE_INT t64;

	CMpi_Assign(m, &mt);
	mflag = n->l.m_iMySign;

	n->l.m_iMySign = n->h.m_iMySign = POSITIVE;

	for (i=mt.m_iLengthInInts-1;i>=0;i--)
	{
		if (mt.m_aiMyInt[i])
			break;
	}
	mt.m_iLengthInInts = i+1;

	if (CMpi_EqualsTo_d(&mt, 0))
	{
		return n;
	}

	mt.m_iMySign = POSITIVE;

	CMpi_Assign(&mt, &odiv);
	odiv.m_iMySign = POSITIVE;
	flag=0;
	ui=odiv.m_aiMyInt[odiv.m_iLengthInInts-1];
	
	while (!(0x80000000 & (ui << flag)))
		flag++;
	CMpi_BitShiftLeft(&odiv, flag);

	len = n->l.m_iLengthInInts + n->h.m_iLengthInInts -odiv.m_iLengthInInts+1;
	if (len <= 0)
	{
		n->l.m_iMySign = n->h.m_iMySign = mflag;
		return n;
	}

	if (n->h.m_iLengthInInts<MPI_LENGTH)
		n->h.m_aiMyInt[n->h.m_iLengthInInts]=0;

	ui = odiv.m_aiMyInt[odiv.m_iLengthInInts-1]+1;

	for (i = n->h.m_iLengthInInts-1, j = n->h.m_iLengthInInts + MPI_LENGTH - odiv.m_iLengthInInts; i>=0; i--, j--)
	{
		if ((i+1)>=MPI_LENGTH)
			t64 = 0;
		else
			t64 = n->h.m_aiMyInt[i+1];
		t64 <<=32 ;
		t64 += n->h.m_aiMyInt[i];
		if (ui)
			uj = (unsigned int)(t64/ui);
		else
			uj = (unsigned int)(t64>>32);
		CMpi_Assign(&odiv, &temp.l);
		CMpi_Assign_d(&temp.h, 0);
		CMpi_Multiply_d(&temp.l, uj);
		if (temp.l.m_iCarry)
		{
			temp.h.m_iLengthInInts = 1;
			temp.h.m_aiMyInt[0]=temp.l.m_iCarry;
		}
		else
		{
			temp.h.m_iLengthInInts = 0;
		}
		CMpl_WordShiftLeft(&temp, j);
		CMpi_Assign(&odiv, &tdiv.l);
		CMpi_Assign_d(&tdiv.h, 0);
		CMpl_WordShiftLeft(&tdiv, j);
		CMpl_Substract(n, &temp);

		while (!CMpi_LargerThan_abs(&tdiv.h, &n->h))
		{
			CMpl_Substract(n, &tdiv);
		}
	}
	if (n->l.m_iLengthInInts < MPI_LENGTH)
		n->l.m_aiMyInt[n->l.m_iLengthInInts]=0;

	ui=odiv.m_aiMyInt[odiv.m_iLengthInInts-1]+1;

	for (i=n->l.m_iLengthInInts-1, j=n->l.m_iLengthInInts-odiv.m_iLengthInInts; i>=odiv.m_iLengthInInts-1; i--, j--)
	{
		if (i == MPI_LENGTH-1)
		{
			if (n->h.m_iLengthInInts == 0)
				t64 = 0;
			else
				t64 = n->h.m_aiMyInt[0];
		}
		else
			t64=n->l.m_aiMyInt[i+1];
		t64<<=32;
		t64+=n->l.m_aiMyInt[i];

		if (ui)
			uj = (unsigned int)(t64/ui);
		else
			uj = (unsigned int)(t64>>32);
		CMpi_Assign_d(&temp.h, 0);
		CMpi_Assign(&odiv, &temp.l);
		CMpi_Multiply_d(&temp.l, uj);
		if (temp.l.m_iCarry)
		{
			temp.h.m_iLengthInInts = 1;
			temp.h.m_aiMyInt[0]=temp.l.m_iCarry;
		}
		else
		{
			temp.h.m_iLengthInInts = 0;
		}
		CMpl_WordShiftLeft(&temp, j);
		CMpi_Assign_d(&tdiv.h, 0);
		CMpi_Assign(&odiv, &tdiv.l);
		CMpl_WordShiftLeft(&tdiv, j);
		CMpl_Substract(n, &temp);

		while ((!CMpi_LargerThan_abs(&tdiv.l, &n->l))||(n->h.m_aiMyInt[0] && n->h.m_iLengthInInts))
		{
			CMpl_Substract(n, &tdiv);
		}
	}

	while (!CMpi_LargerThan_abs(&mt, &n->l))
	{
		uj=n->l.m_aiMyInt[n->l.m_iLengthInInts-1]/(mt.m_aiMyInt[mt.m_iLengthInInts-1]+1);
		if (0==uj)
			uj=1;
		CMpi_Assign(&mt, &temp.l);
		CMpi_Multiply_d(&temp.l, uj);
		CMpi_Substract(&n->l, &temp.l);
	}
	n->l.m_iMySign = n->h.m_iMySign = mflag;
	return n;
}

CMpl* CMpl_Add(CMpl *a, CMpl *b)
{
	CMpi o;
	int i;

	CMpi_Add(&a->l, &b->l);
	CMpi_Add(&a->h, &b->h);

	if (CMpi_EqualsTo_d(&a->h, 0))
		a->h.m_iMySign = a->l.m_iMySign;

	if (a->l.m_iMySign == a->h.m_iMySign)
	{
		if (a->l.m_iCarry)
		{
			CMpi_Assign_d(&o, a->l.m_iCarry);
			o.m_iMySign = a->l.m_iMySign;
			CMpi_Add(&a->h, &o);
			a->l.m_iCarry = 0;
		}
	}
	else
	{
		for (i=0;i<MPI_LENGTH;i++)
			o.m_aiMyInt[i]=HEX32BITS;
		o.m_iLengthInInts = MPI_LENGTH;
		o.m_iMySign = a->h.m_iMySign;
		CMpi_Add(&a->l, &o);
		
		o.m_iLengthInInts = 1;
		o.m_aiMyInt[1] = 0;
		o.m_aiMyInt[0] = 1;
		o.m_iMySign = a->h.m_iMySign;
		CMpi_Add(&a->l, &o);

		CMpi_Substract(&a->h, &o);
	}
	if (a->l.m_iLengthInInts < MPI_LENGTH && !CMpi_EqualsTo_d(&a->h, 0))
		a->l.m_iLengthInInts = MPI_LENGTH;
	return a;
}

CMpl* CMpl_Addi(CMpl *a, CMpi *b)
{
	CMpi o;
	int i;

	CMpi_Add(&a->l, b);

	if (CMpi_EqualsTo_d(&a->h, 0))
		a->h.m_iMySign = a->l.m_iMySign;

	if (a->l.m_iMySign == a->h.m_iMySign)
	{
		if (a->l.m_iCarry)
		{
			CMpi_Assign_d(&o, a->l.m_iCarry);
			o.m_iMySign = a->l.m_iMySign;
			CMpi_Add(&a->h, &o);
			a->l.m_iCarry = 0;
		}
	}
	else
	{
		for (i=0;i<MPI_LENGTH;i++)
			o.m_aiMyInt[i]=HEX32BITS;
		o.m_iLengthInInts = MPI_LENGTH;
		o.m_iMySign = a->h.m_iMySign;
		CMpi_Add(&a->l, &o);
		
		o.m_iLengthInInts = 1;
		o.m_aiMyInt[1] = 0;
		o.m_aiMyInt[0] = 1;
		o.m_iMySign = a->h.m_iMySign;
		CMpi_Add(&a->l, &o);

		CMpi_Substract(&a->h, &o);
	}
	if (a->l.m_iLengthInInts < MPI_LENGTH && !CMpi_EqualsTo_d(&a->h, 0))
		a->l.m_iLengthInInts = MPI_LENGTH;
	return a;
}

CMpl* CMpl_Substract(CMpl *a, CMpl *b)
{
	CMpl temp;
//	if (l.m_iMySign == MPI_INFINITE)
//		return *this;
//	if (oMpl.l.m_iMySign == MPI_INFINITE)
//	{
//		h.m_iMySign = l.m_iMySign = MPI_INFINITE;
//		return *this;
//	}
	CMpi_Assign(&b->h, &temp.h);
	CMpi_Assign(&b->l, &temp.l);
	if (b->l.m_iMySign == POSITIVE)
	{
		temp.l.m_iMySign = temp.h.m_iMySign = NEGTIVE;
	}
	else
	{
		temp.l.m_iMySign = temp.h.m_iMySign = POSITIVE;
	}
	return CMpl_Add(a, &temp);
}

CMpl* CMpl_Substracti(CMpl *a, CMpi *b)
{
  CMpi temp;
  
  CMpi_Assign(b, &temp);

	if (b->m_iMySign == POSITIVE)
	{
		temp.m_iMySign = NEGTIVE;
	}
	else
	{
		temp.m_iMySign = POSITIVE;
	}
	return CMpl_Addi(a, &temp);
}

//CMpl::CMpl(const CMpi &m)
//{
//	l = m;
//	h.m_iMySign = m.m_iMySign;
//	if (h.m_aiMyInt[0] = m.m_iCarry)
//	{
//		h.m_iLengthInInts = 1;
//	}
//	else
//	{
//		h.m_iLengthInInts = 0;
//	}
//}

CMpi* CMpi_WordShiftRight(CMpi *a, int n)
{
	CMpl temp;
	
	CMpi_Assign_d(&temp.h, 0);
	CMpi_Assign(a, &temp.l);
	CMpl_WordShiftRight(&temp, n);
	CMpi_Truncate(&temp, a);
	a->m_iCarry = 0;
	return a;
}

CMpl* CMpl_WordShiftRight(CMpl *a, int n)
{
	int i,j;
	if (n<0)
		return CMpl_WordShiftLeft(a, -n);

	for (i=0;i<a->l.m_iLengthInInts;i++) {
		if ((i+n) < a->l.m_iLengthInInts) {
			a->l.m_aiMyInt[i] = a->l.m_aiMyInt[i+n];
		} else {
			if (a->l.m_iLengthInInts == MPI_LENGTH) {
				j = i+n-MPI_LENGTH;
				if (j<a->h.m_iLengthInInts) {
                    if(j >=0)
					    a->l.m_aiMyInt[i] = a->h.m_aiMyInt[j];
					continue;
				} else {
					a->l.m_aiMyInt[i] = 0;
					a->l.m_iLengthInInts = i;
					a->h.m_iLengthInInts = 0;
					return a;
				}
			} else {
				a->l.m_aiMyInt[i]=0;
				a->l.m_iLengthInInts = i;
				a->h.m_iLengthInInts = 0;
				return a;
			}
		}
	}

	for (i=0;i<a->h.m_iLengthInInts;i++)
	{
		if ((i+n)<a->h.m_iLengthInInts)
		{
			a->h.m_aiMyInt[i] = a->h.m_aiMyInt[n+i];
		}
		else
		{
			a->h.m_aiMyInt[i]=0;
			a->h.m_iLengthInInts = i;
			return a;
		}
	}
	return a;
}

//CMpl::CMpl()
//{
//	l.m_iMySign = h.m_iMySign = POSITIVE;
//	l.m_iLengthInInts = h.m_iLengthInInts = 0;
//}


CMpl* CMpl_WordShiftLeft(CMpl *a, int n)
{
	int i,j,k,p;
	if (n<0)
		return CMpl_WordShiftRight(a, -n);
	k = a->h.m_iLengthInInts + a->l.m_iLengthInInts;
	if (k==0)
		return a;
	if (k==1 && a->l.m_aiMyInt[0]==0)
		return a;

	k += n;
	if (k >= MPI_LENGTH*2)
		k = MPI_LENGTH*2;
	j = k-MPI_LENGTH;

	for (i=j-1, p=i-n; i>=0; i--, p--)
	{
		if (p>=0)
		{
			a->h.m_aiMyInt[i] = a->h.m_aiMyInt[p];
		}
		else
		{
			if ((p+MPI_LENGTH)>=0)
			{
				a->h.m_aiMyInt[i] = a->l.m_aiMyInt[MPI_LENGTH+p];
				continue;
			}
			else
			{
				a->h.m_aiMyInt[i]=0;
				continue;
			}
		}
	}

	if (k>=MPI_LENGTH)
		j = MPI_LENGTH;
	else
		j = k;
	for (i=j-1, p=i-n;i>=0;i--, p--)
	{
		if (p>=0)
		{
			a->l.m_aiMyInt[i]=a->l.m_aiMyInt[p];
		}
		else
		{
			a->l.m_aiMyInt[i]=0;
		}
	}
	a->l.m_iLengthInInts = j;
	if (j>=MPI_LENGTH)
		a->h.m_iLengthInInts = k-MPI_LENGTH;
	else
		a->h.m_iLengthInInts = 0;
	return a;
}

CMpi* CMpi_WordShiftLeft(CMpi *a, int n)
{
	CMpl temp;
	CMpi_Assign_d(&temp.h, 0);
	CMpi_Assign(a, &temp.l);
	CMpl_WordShiftLeft(&temp, n);
	CMpi_Truncate(&temp, a);
	a->m_iCarry = 0;
	return a;
}

//int CMpi::operator !=(const CMpi &m) const
//{
//	return !(operator ==(m));
//}

CMpl* CMpl_Mod(CMpl *a, CMpi *m)
{
	return CMpl_Reduction(a, m);
}


CMpi* CMpi_BitShiftLeft(CMpi *a, int iShiftBits)
{
	int i,j,k;
	j = iShiftBits/BITS_OF_INT;
	if (j)
	{
		CMpi_WordShiftLeft(a, j);
	}

	j = iShiftBits%BITS_OF_INT;

	if (j==0)
		return a;

	k = a->m_iLengthInInts;
	if (k<MPI_LENGTH)
	{
		a->m_aiMyInt[k] = a->m_aiMyInt[k-1]>>(BITS_OF_INT-j);
		if (a->m_aiMyInt[k])
			a->m_iLengthInInts++;
	}
	else
		a->m_iCarry = a->m_aiMyInt[MPI_LENGTH-1]>>(BITS_OF_INT-j);

	for (i=k-1;i>0;i--)
	{
		a->m_aiMyInt[i]<<=j;
		a->m_aiMyInt[i] += a->m_aiMyInt[i-1]>>(BITS_OF_INT-j);
	}

	a->m_aiMyInt[0]<<=j;

	return a;
}

CMpi* CMpi_BitShiftRight(CMpi *a, int iShiftBits)
{
	int i,j;
	j = iShiftBits/BITS_OF_INT;
	if (j)
	{
		CMpi_WordShiftRight(a, j);
	}

	j = iShiftBits%BITS_OF_INT;

	if (j==0)
		return a;

	for (i=0; i<a->m_iLengthInInts-1;i++)
	{
		a->m_aiMyInt[i] >>= j;
		a->m_aiMyInt[i] += (a->m_aiMyInt[i+1]<<(BITS_OF_INT-j));
	}

	a->m_aiMyInt[a->m_iLengthInInts-1] >>= j;

	if (a->m_aiMyInt[a->m_iLengthInInts-1]==0)
		a->m_iLengthInInts--;

	return a;
}


CMpl* CMpl_BitShiftLeft(CMpl *n, int iShiftBits)
{
	int i,j,k;
	j = iShiftBits/BITS_OF_INT;
	if (j)
	{
		CMpl_WordShiftLeft(n, j);
	}
	j = iShiftBits%BITS_OF_INT;

	if (j==0)
		return n;

	k = n->h.m_iLengthInInts ;
	if (k>0)
	{
		if (k<MPI_LENGTH)
		{
			n->h.m_aiMyInt[k] = n->h.m_aiMyInt[k-1]>>(BITS_OF_INT-j);
			if (n->h.m_aiMyInt[k])
				n->h.m_iLengthInInts ++;
		}
		for (i = k-1; i>0;i--)
		{
			n->h.m_aiMyInt[i] <<= j;
			n->h.m_aiMyInt[i] += n->h.m_aiMyInt[i-1]>>(BITS_OF_INT-j);
		}
		n->h.m_aiMyInt[0] <<= j;
		n->h.m_aiMyInt[0] += n->l.m_aiMyInt[MPI_LENGTH-1]>>(BITS_OF_INT-j);
	}
	else
	{
		if (n->l.m_iLengthInInts==MPI_LENGTH)
		{
			n->h.m_aiMyInt[0] = n->l.m_aiMyInt[MPI_LENGTH-1]>>(BITS_OF_INT-j);
			if (n->h.m_aiMyInt[0])
				n->h.m_iLengthInInts = 1;
		}
	}

	k = n->l.m_iLengthInInts;
	if (k<MPI_LENGTH)
	{
			n->l.m_aiMyInt[k] = n->l.m_aiMyInt[k-1]>>(BITS_OF_INT-j);
			if (n->l.m_aiMyInt[k])
				n->l.m_iLengthInInts ++;
	}
	for (i=k-1;i>0;i--)
	{
		n->l.m_aiMyInt[i]<<=j;
		n->l.m_aiMyInt[i] += n->l.m_aiMyInt[i-1]>>(BITS_OF_INT-j);
	}
	n->l.m_aiMyInt[0]<<=j;

	return n;
}


CMpl* CMpi_FastSquare(CMpi *n, CMpl *n2)
{
	unsigned int c[MPI_LENGTH*2] = {
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0
	};
	int k;
	int len = n->m_iLengthInInts*2;

//	for (k=0; k<len; k++)
//		c[k]=0;

	int i, j;
	DOUBLE_INT uv, sum;
	unsigned int R0 = 0, R1 = 0, R2 = 0;
	int imax;

	for (k = 0; k <= len-2; k++)
	{
//		imax = __min(k, m_iLengthInInts-1);
//		imax = __min(k/2, imax);
		imax = __min(k/2, n->m_iLengthInInts-1);
		for (i = imax, j = k-i; i >= 0; i--, j++)
		{
			if (j >= n->m_iLengthInInts)
				break;

			uv = n->m_aiMyInt[i];
			uv *= n->m_aiMyInt[j];
			if (i < j)
			{
				if (0x8000000000000000ull & uv)
					R2++;
				uv <<= 1;
			}
			sum = R0;
			sum += (unsigned int)uv;
			R0 = (unsigned int)sum;
			sum >>= BITS_OF_INT;

			sum += R1;
			sum += (uv >> BITS_OF_INT);
			R1 = (unsigned int)sum;
			sum >>= BITS_OF_INT;

			R2 += (unsigned int)sum;
		}
		c[k] = R0;
		R0 = R1;
		R1 = R2;
		R2 = 0;
	}

	c[len-1] = R0;

	CMpi_Assign_d(&n2->l, 0);
	CMpi_Assign_d(&n2->h, 0);
	if (len>MPI_LENGTH)
	{
		n2->l.m_iLengthInInts = MPI_LENGTH;
		n2->h.m_iLengthInInts = len - MPI_LENGTH;

		for (i=0;i<n2->h.m_iLengthInInts;i++)
			n2->h.m_aiMyInt[i]=c[i+MPI_LENGTH];
	}
	else
	{
		n2->l.m_iLengthInInts = len;
		n2->h.m_iLengthInInts = 0;
	}

	for (i=0;i<n2->l.m_iLengthInInts;i++)
		n2->l.m_aiMyInt[i]=c[i];

	return n2;
}


CMpl* CMpl_BitShiftRight(CMpl *n, int iShiftBits)
{
	int i,j,k;
	j = iShiftBits/(BITS_OF_INT);
	if (j)
	{
		CMpl_WordShiftRight(n, j);
	}
	j = iShiftBits%(BITS_OF_INT);

	if (j==0)
		return n;

	k = n->l.m_iLengthInInts;

	for (i=0; i<n->l.m_iLengthInInts-1;i++)
	{
		n->l.m_aiMyInt[i]>>=j;
		n->l.m_aiMyInt[i] += n->l.m_aiMyInt[i+1]<<(BITS_OF_INT-j);
	}
	if (k<MPI_LENGTH)
	{
		n->l.m_aiMyInt[n->l.m_iLengthInInts-1]>>=j;
		if (n->l.m_aiMyInt[n->l.m_iLengthInInts-1]==0)
			n->l.m_iLengthInInts --;
	}
	else
	{
		n->l.m_aiMyInt[n->l.m_iLengthInInts-1]>>=j;
		if (n->h.m_iLengthInInts>0)
			n->l.m_aiMyInt[n->l.m_iLengthInInts-1]+= n->h.m_aiMyInt[0]<<(BITS_OF_INT-j);
	}
	
	k = n->h.m_iLengthInInts ;
	if (k>0)
	{
		for (i =0;i< k-1; i++)
		{
			n->h.m_aiMyInt[i] >>= j;
			n->h.m_aiMyInt[i] += n->h.m_aiMyInt[i+1]<<(BITS_OF_INT-j);
		}

		n->h.m_aiMyInt[k-1] >>= j;
		if (n->h.m_aiMyInt[k-1]==0)
			n->h.m_iLengthInInts --;
	}
	return n;
}



CMpi* CMpi_BinaryInverse2(CMpi *m, CMpi *x, CMpi *xi)
{
	CMpi u, v;
	CMpl x1, x2, mod;

	CMpi_Assign(x, &u);
	CMpi_Assign(m, &v);
	CMpi_Assign_d(&x1.h, 0);
	CMpi_Assign_d(&x1.l, 1);
	CMpi_Assign_d(&x2.h, 0);
	CMpi_Assign_d(&x2.l, 0);
	CMpi_Regularize(m);
	CMpi_Assign_d(&mod.h, 0);
	CMpi_Assign(m, &mod.l);

	while (!CMpi_EqualsTo_d(&u, 1) && !CMpi_EqualsTo_d(&v, 1))
	{
		while (!(u.m_aiMyInt[0] & 0x01))
		{
			CMpi_BitShiftRight(&u, 1);
			if (!(x1.l.m_aiMyInt[0] & 0x01))
				CMpl_BitShiftRight(&x1, 1);
			else
			{
				CMpl_BitShiftRight(CMpl_Add(&x1, &mod), 1);
			}
		}

		while (!(v.m_aiMyInt[0] & 0x01))
		{
			CMpi_BitShiftRight(&v, 1);
			if (!(x2.l.m_aiMyInt[0] & 0x01))
				CMpl_BitShiftRight(&x2, 1);
			else
			{
				CMpl_BitShiftRight(CMpl_Add(&x2, &mod), 1);
			}
		}

		if (CMpi_ComparesTo(&u, &v) == -1)
		{
			CMpi_Substract(&v, &u);
			CMpl_Substract(&x2, &x1);
		}
		else
		{
			CMpi_Substract(&u, &v);
			CMpl_Substract(&x1, &x2);
		}
	}

	if (CMpi_EqualsTo_d(&u, 1) == 1)
	{
		CMpl_Mod(&x1, m);
		if (CMpi_IsNegative(&x1.l))
			CMpl_Add(&x1, &mod);
		CMpi_Assign(&x1.l, xi);
	}
	else
	{
		CMpl_Mod(&x2, m);
		if (CMpi_IsNegative(&x2.l))
			CMpl_Add(&x2, &mod);
		CMpi_Assign(&x2.l, xi);
	}
	return xi;
}


CMpi* CMpi_Regularize(CMpi *n)
{
	int i;

	for (i=n->m_iLengthInInts-1;i>=0;i--)
	{
		if (n->m_aiMyInt[i])
			break;
	}
	n->m_iLengthInInts = i+1;

	return n;
}



//CModulus::CModulus(const CMpi &oMpi)
//{
//	m_oModulus = oMpi;
//	m_oModulus.Regularize();
//}

//int CMpl::operator ==(const CMpl &oMpl) const
//{
//	if ((l==oMpl.l) && (h==oMpl.h))
//		return 1;
//	return 0;
//}

int CMpi_LargerThan(CMpi *a, CMpi *b)
{
	if (a->m_iMySign!=NEGTIVE)
	{
		if (b->m_iMySign==NEGTIVE)
			return 1;
		if (a->m_iMySign==POSITIVE)
		{
			if (b->m_iMySign == POSITIVE)
			{
				if (CMpi_LargerThan_abs(a, b))
					return 1;
				else
					return 0;
			}
			else
			{
				return 0;
			}
		}
		else
			return 0;
	}
	if (a->m_iMySign==NEGTIVE)
	{
		if (b->m_iMySign!=NEGTIVE)
			return 0;
		else
		{
			if (CMpi_LargerThan_abs(a, b))
				return 1;
			else
				return 0;
		}
	}
	return 0;
}

int CMpi_ComparesTo(CMpi *a, CMpi *b)
{
	if (CMpi_EqualsTo(a, b))
		return 0;
	if (CMpi_LargerThan(a, b))
		return 1;
	return -1;
}

//int CMpi::operator < (const CMpi &m) const
//{
//	return (m>(*this));
//}


int CMpi_GetLengthInBytes(CMpi *n)
{
	int i;
	unsigned int j,k;
	CMpi_Regularize(n);
	if (n->m_iLengthInInts==0)
		return 0;
	j = n->m_aiMyInt[n->m_iLengthInInts-1];
	k = 0xff;
	for (i=sizeof(unsigned int)-1;i>=0;i--)
	{
		if (j&(k<<(i*8)))
			break;
	}
	return i+1+(n->m_iLengthInInts-1)*sizeof(unsigned int);
}

//int CModulus::GetLengthInBytes() 
//{
//	return m_oModulus.GetLengthInBytes();
//}


int CMpi_Import(CMpi *n, uint8_t *abContent, int iLength)
{
	int i,j;
	n->m_iCarry = 0;
	n->m_iLengthInInts = 0; 
	n->m_iMySign = POSITIVE;

	if (iLength>MPI_LENGTH*sizeof(unsigned int))
	{
		return 0;
	}

	j = 0;
	n->m_aiMyInt[n->m_iLengthInInts] = 0;
	for (i = iLength-1; i>=0 ; i--)
	{
		n->m_aiMyInt[n->m_iLengthInInts] += abContent[i]<<(8*j);
		j++;
		if (j>=sizeof(unsigned int))
		{
			j=0;
			n->m_iLengthInInts++;
			if (n->m_iLengthInInts!=MPI_LENGTH)
				n->m_aiMyInt[n->m_iLengthInInts] = 0;
		}
	}
	if (j)
		n->m_iLengthInInts++;
	return n->m_iLengthInInts;
}

int CMpi_Export(CMpi *n, uint8_t *abOutBytes, int iMinLength)
{
	int j,k;
	int iLengthOfExport;
	unsigned int u;
	iLengthOfExport = 0;
	j = n->m_iLengthInInts-1;
	if (j<0)
		return 0;

	u = n->m_aiMyInt[j];
	while (u==0)
	{
		j--;
		if (j < 0)
			return 0;
		u = n->m_aiMyInt[j];
	}

	k = sizeof(unsigned int)-1;
	while (0==(u>>(k*8)))
		k--;

	int iOut = j*sizeof(unsigned int) + k + 1;
	while (iOut < iMinLength)
	{
		abOutBytes[iLengthOfExport++] = 0x00;
		iMinLength--;
	}

	while (k>=0)
	{
		abOutBytes[iLengthOfExport] = (uint8_t )(u>>(k*8));
		iLengthOfExport++;
		k--;
	}

	j--;
	u = n->m_aiMyInt[j];
	while (j>=0)
	{
		u = n->m_aiMyInt[j];
		for (k=sizeof(unsigned int)-1;k>=0;k--)
		{
			abOutBytes[iLengthOfExport] = (uint8_t )(u>>(k*8));
			iLengthOfExport++;
		}
		j--;
	}

	return iLengthOfExport;
}


int CMpi_GetLengthInBits(CMpi *n)
{
	int i;
	unsigned int k;
	int iLength;
	for (i=n->m_iLengthInInts-1;i>=0;i--)
	{
		if (n->m_aiMyInt[i])
			break;
	}
	iLength = (i+1)*8*sizeof(unsigned int);
	k = n->m_aiMyInt[i];
	for (i=0;i<8*sizeof(unsigned int);i++)
	{
		if (UNSIGNEDLEFTBIT & (k<<i))
			break;
	}
	return iLength - i;
}

int CMpi_IsNegative(CMpi *n)
{
	return n->m_iMySign == NEGTIVE;
}


CMpi* CMpi_ChangeSign(CMpi *n)
{
	n->m_iMySign *= -1;
	return n;
}


