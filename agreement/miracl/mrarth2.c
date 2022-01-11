/*
 *   MIRACL arithmetic routines 2 - multiplying and dividing BIG NUMBERS.
 *   mrarth2.c
 *
 *   Copyright (c) 1988-2003 Shamus Software Ltd.
 */

#include "miracl.h"

#ifdef MR_FP
#include <math.h>
#endif

#ifdef MR_WIN64
#include <intrin.h>
#endif


/* If a number has more than this number of digits, then squaring is faster */

#define SQR_FASTER_THRESHOLD 5

mr_small normalise(_MIPD_ big x,big y)
{   /* normalise divisor */
    mr_small norm,r;
#ifdef MR_FP
    mr_small dres;
#endif
    int len;
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif

    MR_IN(4)

    if (x!=y) copy(x,y);
    len=(int)(y->len&MR_OBITS);
#ifndef MR_SIMPLE_BASE
    if (mr_mip->base==0)
    {
#endif
#ifndef MR_NOFULLWIDTH
        if ((r=y->w[len-1]+1)==0) norm=1;
#ifdef MR_NOASM
        else norm=(mr_small)(((mr_large)1 << MIRACL)/r);
#else
        else norm=muldvm((mr_small)1,(mr_small)0,r,&r);
#endif
        if (norm!=1) mr_pmul(_MIPP_ y,norm,y);
#endif
#ifndef MR_SIMPLE_BASE
    }
    else
    {
        norm=MR_DIV(mr_mip->base,(mr_small)(y->w[len-1]+1));
        if (norm!=1) mr_pmul(_MIPP_ y,norm,y);
    }
#endif
    MR_OUT
    return norm;
}

void multiply(_MIPD_ big x,big y,big z)
{   /*  multiply two big numbers: z=x.y  */
    int i,xl,yl,j,ti;
    mr_small carry;

#ifdef MR_ITANIUM
    mr_small tm;
#endif
#ifdef MR_WIN64
    mr_small tm,tr;
#endif
    mr_lentype sz;
    big w0;
#ifdef MR_NOASM
    union doubleword dble;
    mr_large dbled;
#endif
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif
    if (mr_mip->ERNUM) return;
    if (y->len==0 || x->len==0)
    {
        zero(z);
        return;
    }
    if (x!=mr_mip->w5 && y!=mr_mip->w5 && z==mr_mip->w5) w0=mr_mip->w5;
    else w0=mr_mip->w0;    /* local pointer */

    MR_IN(5)

#ifdef MR_FLASH
    if (mr_notint(x) || mr_notint(y))
    {
        mr_berror(_MIPP_ MR_ERR_INT_OP);
        MR_OUT
        return;
    }
#endif
    sz=((x->len&MR_MSBIT)^(y->len&MR_MSBIT));
    xl=(int)(x->len&MR_OBITS);
    yl=(int)(y->len&MR_OBITS);
    zero(w0);
    if (mr_mip->check && xl+yl>mr_mip->nib)
    {
        mr_berror(_MIPP_ MR_ERR_OVERFLOW);
        MR_OUT
        return;
    }
#ifndef MR_SIMPLE_BASE
    if (mr_mip->base==0)
    {
#endif
#ifndef MR_NOFULLWIDTH
        if (x==y && xl>SQR_FASTER_THRESHOLD)
            /* extra hassle make it not    */
            /* worth it for small numbers */
        {   /* fast squaring */
            for (i=0; i<xl-1; i++)
            {   /* long multiplication */
                /* inline - substitutes for loop below */
#ifndef INLINE_ASM
                carry=0;
                for (j=i+1; j<xl; j++)
                {   /* Only do above the diagonal */
#ifdef MR_NOASM
                    dble.d=(mr_large)x->w[i]*x->w[j]+carry+w0->w[i+j];
                    w0->w[i+j]=dble.h[MR_BOT];
                    carry=dble.h[MR_TOP];
#else
                    muldvd2(x->w[i],x->w[j],&carry,&w0->w[i+j]);
#endif
                }
                w0->w[xl+i]=carry;
#endif
            }
#ifndef INLINE_ASM
            w0->len=xl+xl-1;
            mr_padd(_MIPP_ w0,w0,w0);     /* double it */
            carry=0;
            for (i=0; i<xl; i++)
            {   /* add in squared elements */
                ti=i+i;
#ifdef MR_NOASM
                dble.d=(mr_large)x->w[i]*x->w[i]+carry+w0->w[ti];
                w0->w[ti]=dble.h[MR_BOT];
                carry=dble.h[MR_TOP];
#else
                muldvd2(x->w[i],x->w[i],&carry,&w0->w[ti]);
#endif
                w0->w[ti+1]+=carry;
                if (w0->w[ti+1]<carry) carry=1;
                else                   carry=0;
            }
#endif
        }
        else for (i=0; i<xl; i++)
            {   /* long multiplication */
                /* inline - substitutes for loop below */
#ifndef INLINE_ASM
                carry=0;
                for (j=0; j<yl; j++)
                {   /* multiply each digit of y by x[i] */
#ifdef MR_NOASM
                    dble.d=(mr_large)x->w[i]*y->w[j]+carry+w0->w[i+j];
                    w0->w[i+j]=dble.h[MR_BOT];
                    carry=dble.h[MR_TOP];
#else
                    muldvd2(x->w[i],y->w[j],&carry,&w0->w[i+j]);
#endif
                }
                w0->w[yl+i]=carry;
#endif
            }
#endif
#ifndef MR_SIMPLE_BASE
    }
    else
    {
        if (x==y && xl>SQR_FASTER_THRESHOLD)
        {   /* squaring can be done nearly twice as fast */
            for (i=0; i<xl-1; i++)
            {   /* long multiplication */
                carry=0;
                for (j=i+1; j<xl; j++)
                {   /* Only do above the diagonal */
#ifdef MR_NOASM
                    dbled=(mr_large)x->w[i]*x->w[j]+w0->w[i+j]+carry;
#ifdef MR_FP_ROUNDING
                    carry=(mr_small)MR_LROUND(dbled*mr_mip->inverse_base);
#else
#ifndef MR_FP
                    if (mr_mip->base==mr_mip->base2)
                        carry=(mr_small)(dbled>>mr_mip->lg2b);
                    else
#endif
                        carry=(mr_small)MR_LROUND(dbled/mr_mip->base);
#endif
                    w0->w[i+j]=(mr_small)(dbled-(mr_large)carry*mr_mip->base);
#else

#ifdef MR_FP_ROUNDING
                    carry=imuldiv(x->w[i],x->w[j],w0->w[i+j]+carry,mr_mip->base,mr_mip->inverse_base,&w0->w[i+j]);
#else
                    carry=muldiv(x->w[i],x->w[j],w0->w[i+j]+carry,mr_mip->base,&w0->w[i+j]);
#endif
#endif
                }
                w0->w[xl+i]=carry;
            }
            w0->len=xl+xl-1;
            mr_padd(_MIPP_ w0,w0,w0);     /* double it */
            carry=0;
            for (i=0; i<xl; i++)
            {   /* add in squared elements */
                ti=i+i;
#ifdef MR_NOASM
                dbled=(mr_large)x->w[i]*x->w[i]+w0->w[ti]+carry;
#ifdef MR_FP_ROUNDING
                carry=(mr_small)MR_LROUND(dbled*mr_mip->inverse_base);
#else
#ifndef MR_FP
                if (mr_mip->base==mr_mip->base2)
                    carry=(mr_small)(dbled>>mr_mip->lg2b);
                else
#endif
                    carry=(mr_small)MR_LROUND(dbled/mr_mip->base);
#endif
                w0->w[ti]=(mr_small)(dbled-(mr_large)carry*mr_mip->base);
#else

#ifdef MR_FP_ROUNDING
                carry=imuldiv(x->w[i],x->w[i],w0->w[ti]+carry,mr_mip->base,mr_mip->inverse_base,&w0->w[ti]);
#else
                carry=muldiv(x->w[i],x->w[i],w0->w[ti]+carry,mr_mip->base,&w0->w[ti]);
#endif

#endif
                w0->w[ti+1]+=carry;
                carry=0;
                if (w0->w[ti+1]>=mr_mip->base)
                {
                    carry=1;
                    w0->w[ti+1]-=mr_mip->base;
                }
            }
        }
        else for (i=0; i<xl; i++)
            {   /* long multiplication */
                carry=0;
                for (j=0; j<yl; j++)
                {   /* multiply each digit of y by x[i] */
#ifdef MR_NOASM
                    dbled=(mr_large)x->w[i]*y->w[j]+w0->w[i+j]+carry;

#ifdef MR_FP_ROUNDING
                    carry=(mr_small)MR_LROUND(dbled*mr_mip->inverse_base);
#else
#ifndef MR_FP
                    if (mr_mip->base==mr_mip->base2)
                        carry=(mr_small)(dbled>>mr_mip->lg2b);
                    else
#endif
                        carry=(mr_small)MR_LROUND(dbled/mr_mip->base);
#endif
                    w0->w[i+j]=(mr_small)(dbled-(mr_large)carry*mr_mip->base);
#else

#ifdef MR_FP_ROUNDING
                    carry=imuldiv(x->w[i],y->w[j],w0->w[i+j]+carry,mr_mip->base,mr_mip->inverse_base,&w0->w[i+j]);
#else
                    carry=muldiv(x->w[i],y->w[j],w0->w[i+j]+carry,mr_mip->base,&w0->w[i+j]);
#endif

#endif
                }
                w0->w[yl+i]=carry;
            }
    }
#endif
    w0->len=(sz|(xl+yl)); /* set length and sign of result */

    mr_lzero(w0);
    copy(w0,z);
    MR_OUT
}

void divide(_MIPD_ big x,big y,big z)
{   /*  divide two big numbers  z=x/y : x=x mod y  *
     *  returns quotient only if  divide(x,y,x)    *
     *  returns remainder only if divide(x,y,y)    */
    mr_small carry,attemp,ldy,sdy,ra,r,d,tst,psum;
#ifdef MR_FP
    mr_small dres;
#endif
    mr_lentype sx,sy,sz;
    mr_small borrow,dig;
    int i,k,m,x0,y0,w00;
    big w0;

#ifdef MR_ITANIUM
    mr_small tm;
#endif
#ifdef MR_WIN64
    mr_small tm;
#endif
#ifdef MR_NOASM
    union doubleword dble;
    mr_large dbled;
#endif
    BOOL check;
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif
    if (mr_mip->ERNUM) return;
    w0=mr_mip->w0;

    MR_IN(6)

    if (x==y) mr_berror(_MIPP_ MR_ERR_BAD_PARAMETERS);
#ifdef MR_FLASH
    if (mr_notint(x) || mr_notint(y)) mr_berror(_MIPP_ MR_ERR_INT_OP);
#endif
    if (y->len==0) mr_berror(_MIPP_ MR_ERR_DIV_BY_ZERO);
    if (mr_mip->ERNUM)
    {
        MR_OUT
        return;
    }
    sx=(x->len&MR_MSBIT);   /*  extract signs ... */
    sy=(y->len&MR_MSBIT);
    sz=(sx^sy);
    x->len&=MR_OBITS;   /* ... and force operands to positive  */
    y->len&=MR_OBITS;
    x0=(int)x->len;
    y0=(int)y->len;
    copy(x,w0);
    w00=(int)w0->len;
    if (mr_mip->check && (w00-y0+1>mr_mip->nib))
    {
        mr_berror(_MIPP_ MR_ERR_OVERFLOW);
        MR_OUT
        return;
    }
    d=0;
    if (x0==y0)
    {
        if (x0==1) /* special case - x and y are both mr_smalls */
        {
            d=MR_DIV(w0->w[0],y->w[0]);
            w0->w[0]=MR_REMAIN(w0->w[0],y->w[0]);
            mr_lzero(w0);
        }
        else if (MR_DIV(w0->w[x0-1],4)<y->w[x0-1])
            while (mr_compare(w0,y)>=0)
            {   /* mr_small quotient - so do up to four subtracts instead */
                mr_psub(_MIPP_ w0,y,w0);
                d++;
            }
    }
    if (mr_compare(w0,y)<0)
    {   /*  x less than y - so x becomes remainder */
        if (x!=z)  /* testing parameters */
        {
            copy(w0,x);
            if (x->len!=0) x->len|=sx;
        }
        if (y!=z)
        {
            zero(z);
            z->w[0]=d;
            if (d>0) z->len=(sz|1);
        }
        y->len|=sy;
        MR_OUT
        return;
    }

    if (y0==1)
    {   /* y is int - so use subdiv instead */
#ifdef MR_FP_ROUNDING
        r=mr_sdiv(_MIPP_ w0,y->w[0],mr_invert(y->w[0]),w0);
#else
        r=mr_sdiv(_MIPP_ w0,y->w[0],w0);
#endif
        if (y!=z)
        {
            copy(w0,z);
            z->len|=sz;
        }
        if (x!=z)
        {
            zero(x);
            x->w[0]=r;
            if (r>0) x->len=(sx|1);
        }
        y->len|=sy;
        MR_OUT
        return;
    }
    if (y!=z) zero(z);
    d=normalise(_MIPP_ y,y);
    check=mr_mip->check;
    mr_mip->check=OFF;
#ifndef MR_SIMPLE_BASE
    if (mr_mip->base==0)
    {
#endif
#ifndef MR_NOFULLWIDTH
        if (d!=1) mr_pmul(_MIPP_ w0,d,w0);
        ldy=y->w[y0-1];
        sdy=y->w[y0-2];
        for (k=w00-1; k>=y0-1; k--)
        {   /* long division */
#ifndef INLINE_ASM
            carry=0;
            if (w0->w[k+1]==ldy) /* guess next quotient digit */
            {
                attemp=(mr_small)(-1);
                ra=ldy+w0->w[k];
                if (ra<ldy) carry=1;
            }
#ifdef MR_NOASM
            else
            {
                dble.h[MR_BOT]=w0->w[k];
                dble.h[MR_TOP]=w0->w[k+1];
                attemp=(mr_small)(dble.d/ldy);
                ra=(mr_small)(dble.d-(mr_large)attemp*ldy);
            }
#else
            else attemp=muldvm(w0->w[k+1],w0->w[k],ldy,&ra);
#endif
            while (carry==0)
            {
#ifdef MR_NOASM
                dble.d=(mr_large)attemp*sdy;
                r=dble.h[MR_BOT];
                tst=dble.h[MR_TOP];
#else
                tst=muldvd(sdy,attemp,(mr_small)0,&r);
#endif
                if (tst< ra || (tst==ra && r<=w0->w[k-1])) break;
                attemp--;  /* refine guess */
                ra+=ldy;
                if (ra<ldy) carry=1;
            }
#endif
            m=k-y0+1;
            if (attemp>0)
            {   /* do partial subtraction */
                borrow=0;
                /*  inline - substitutes for loop below */
#ifndef INLINE_ASM
                for (i=0; i<y0; i++)
                {
#ifdef MR_NOASM
                    dble.d=(mr_large)attemp*y->w[i]+borrow;
                    dig=dble.h[MR_BOT];
                    borrow=dble.h[MR_TOP];
#else
                    borrow=muldvd(attemp,y->w[i],borrow,&dig);
#endif
                    if (w0->w[m+i]<dig) borrow++;
                    w0->w[m+i]-=dig;
                }
#endif

                if (w0->w[k+1]<borrow)
                {   /* whoops! - over did it */
                    w0->w[k+1]=0;
                    carry=0;
                    for (i=0; i<y0; i++)
                    {   /* compensate for error ... */
                        psum=w0->w[m+i]+y->w[i]+carry;
                        if (psum>y->w[i]) carry=0;
                        if (psum<y->w[i]) carry=1;
                        w0->w[m+i]=psum;
                    }
                    attemp--;  /* ... and adjust guess */
                }
                else w0->w[k+1]-=borrow;
            }
            if (k==w00-1 && attemp==0) w00--;
            else if (y!=z) z->w[m]=attemp;
        }
#endif
#ifndef MR_SIMPLE_BASE
    }
    else
    {   /* have to do it the hard way */
        if (d!=1) mr_pmul(_MIPP_ w0,d,w0);
        ldy=y->w[y0-1];
        sdy=y->w[y0-2];

        for (k=w00-1; k>=y0-1; k--)
        {   /* long division */


            if (w0->w[k+1]==ldy) /* guess next quotient digit */
            {
                attemp=mr_mip->base-1;
                ra=ldy+w0->w[k];
            }
#ifdef MR_NOASM
            else
            {
                dbled=(mr_large)w0->w[k+1]*mr_mip->base+w0->w[k];
                attemp=(mr_small)MR_LROUND(dbled/ldy);
                ra=(mr_small)(dbled-(mr_large)attemp*ldy);
            }
#else
            else attemp=muldiv(w0->w[k+1],mr_mip->base,w0->w[k],ldy,&ra);
#endif
            while (ra<mr_mip->base)
            {
#ifdef MR_NOASM
                dbled=(mr_large)sdy*attemp;
#ifdef MR_FP_ROUNDING
                tst=(mr_small)MR_LROUND(dbled*mr_mip->inverse_base);
#else
#ifndef MR_FP
                if (mr_mip->base==mr_mip->base2)
                    tst=(mr_small)(dbled>>mr_mip->lg2b);
                else
#endif
                    tst=(mr_small)MR_LROUND(dbled/mr_mip->base);
#endif
                r=(mr_small)(dbled-(mr_large)tst*mr_mip->base);
#else
#ifdef MR_FP_ROUNDING
                tst=imuldiv(sdy,attemp,(mr_small)0,mr_mip->base,mr_mip->inverse_base,&r);
#else
                tst=muldiv(sdy,attemp,(mr_small)0,mr_mip->base,&r);
#endif
#endif
                if (tst< ra || (tst==ra && r<=w0->w[k-1])) break;
                attemp--;  /* refine guess */
                ra+=ldy;
            }
            m=k-y0+1;
            if (attemp>0)
            {   /* do partial subtraction */
                borrow=0;
                for (i=0; i<y0; i++)
                {
#ifdef MR_NOASM
                    dbled=(mr_large)attemp*y->w[i]+borrow;
#ifdef MR_FP_ROUNDING
                    borrow=(mr_small)MR_LROUND(dbled*mr_mip->inverse_base);
#else
#ifndef MR_FP
                    if (mr_mip->base==mr_mip->base2)
                        borrow=(mr_small)(dbled>>mr_mip->lg2b);
                    else
#endif
                        borrow=(mr_small)MR_LROUND(dbled/mr_mip->base);
#endif
                    dig=(mr_small)(dbled-(mr_large)borrow*mr_mip->base);
#else
#ifdef MR_FP_ROUNDING
                    borrow=imuldiv(attemp,y->w[i],borrow,mr_mip->base,mr_mip->inverse_base,&dig);
#else
                    borrow=muldiv(attemp,y->w[i],borrow,mr_mip->base,&dig);
#endif
#endif
                    if (w0->w[m+i]<dig)
                    {   /* set borrow */
                        borrow++;
                        w0->w[m+i]+=(mr_mip->base-dig);
                    }
                    else w0->w[m+i]-=dig;
                }
                if (w0->w[k+1]<borrow)
                {   /* whoops! - over did it */
                    w0->w[k+1]=0;
                    carry=0;
                    for (i=0; i<y0; i++)
                    {   /* compensate for error ... */
                        psum=w0->w[m+i]+y->w[i]+carry;
                        carry=0;
                        if (psum>=mr_mip->base)
                        {
                            carry=1;
                            psum-=mr_mip->base;
                        }
                        w0->w[m+i]=psum;
                    }
                    attemp--;  /* ... and adjust guess */
                }
                else
                    w0->w[k+1]-=borrow;
            }
            if (k==w00-1 && attemp==0) w00--;
            else if (y!=z) z->w[m]=attemp;
        }
    }
#endif
    if (y!=z) z->len=((w00-y0+1)|sz); /* set sign and length of result */

    w0->len=y0;

    mr_lzero(y);
    mr_lzero(z);

    if (x!=z)
    {
        mr_lzero(w0);
#ifdef MR_FP_ROUNDING
        if (d!=1) mr_sdiv(_MIPP_ w0,d,mr_invert(d),x);
#else
        if (d!=1) mr_sdiv(_MIPP_ w0,d,x);
#endif
        else copy(w0,x);
        if (x->len!=0) x->len|=sx;
    }
#ifdef MR_FP_ROUNDING
    if (d!=1) mr_sdiv(_MIPP_ y,d,mr_invert(d),y);
#else
    if (d!=1) mr_sdiv(_MIPP_ y,d,y);
#endif
    y->len|=sy;
    mr_mip->check=check;

    MR_OUT
}

BOOL divisible(_MIPD_ big x,big y)
{   /* returns y|x, that is TRUE if y divides x exactly */
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif
    if (mr_mip->ERNUM) return FALSE;

    MR_IN(87)

    copy (x,mr_mip->w0);
    divide(_MIPP_ mr_mip->w0,y,y);

    MR_OUT
    if (size(mr_mip->w0)==0) return TRUE;
    else                    return FALSE;
}

void mad(_MIPD_ big x,big y,big z,big w,big q,big r)
{   /* Multiply, Add and Divide; q=(x*y+z)/w remainder r   *
     * returns remainder only if w=q, quotient only if q=r *
     * add done only if x, y and z are distinct.           */
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif
    BOOL check;
    if (mr_mip->ERNUM) return;

    MR_IN(24)
    if (w==r)
    {
        mr_berror(_MIPP_ MR_ERR_BAD_PARAMETERS);
        MR_OUT
        return;
    }
    check=mr_mip->check;
    mr_mip->check=OFF;           /* turn off some error checks */

    multiply(_MIPP_ x,y,mr_mip->w0);
    if (x!=z && y!=z) add(_MIPP_ mr_mip->w0,z,mr_mip->w0);

    divide(_MIPP_ mr_mip->w0,w,q);
    if (q!=r) copy(mr_mip->w0,r);
    mr_mip->check=check;
    MR_OUT
}

