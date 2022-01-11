/*
 *   MIRACL I/O routines 2.
 *   mrio2.c
 *
 *   Copyright (c) 1988-1997 Shamus Software Ltd.
 */

#include "miracl.h"

#ifndef MR_SIMPLE_BASE
#ifndef MR_SIMPLE_IO

static void cbase(_MIPD_ big x,mr_small oldbase,big y)
{   /*  change radix of x from oldbase to base  */
    int i,s;
    mr_small n;
    BOOL done;
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif
    if (mr_mip->ERNUM) return;
    if (mr_mip->base==oldbase)
    {
        copy(x,y);
        return;
    }

    MR_IN(13)

    s=exsign(x);
#ifdef MR_FLASH
    numer(_MIPP_ x,mr_mip->w1);
    denom(_MIPP_ x,mr_mip->w2);
    done=FALSE;
#else
    copy(x,mr_mip->w1);
    done=TRUE;
#endif
    insign(PLUS,mr_mip->w1);

    forever
    {
        zero(mr_mip->w6);
        convert(_MIPP_ 1,mr_mip->w0);

        for (i=0; i<(int)mr_mip->w1->len; i++)
        {   /* this is a bit slow - but not time critical */


            mr_pmul(_MIPP_ mr_mip->w0,mr_mip->w1->w[i],mr_mip->w5);

            add(_MIPP_ mr_mip->w6,mr_mip->w5,mr_mip->w6);
            if (oldbase==0)
            {   /* bit of a frig! */
                n=mr_shiftbits(1,MIRACL/2);
                mr_pmul(_MIPP_ mr_mip->w0,n,mr_mip->w0);
                mr_pmul(_MIPP_ mr_mip->w0,n,mr_mip->w0);
            }
            else mr_pmul(_MIPP_ mr_mip->w0,oldbase,mr_mip->w0);
        }
        if (mr_mip->ERNUM || done) break;
#ifdef MR_FLASH
        copy(mr_mip->w2,mr_mip->w1);
        copy(mr_mip->w6,mr_mip->w7);
        done=TRUE;
#endif
    }

#ifdef MR_FLASH
    fpack(_MIPP_ mr_mip->w7,mr_mip->w6,y);
#else
    copy(mr_mip->w6,y);
#endif

    insign(s,y);
    MR_OUT
}

int cinstr(_MIPD_ flash x,char *string)
{   /*  input big number in base IOBASE  */
    mr_small newb,oldb,b;
    mr_lentype lx;
    int ipt;
#ifdef MR_OS_THREADS
    miracl *mr_mip=get_mip();
#endif
    if (mr_mip->ERNUM) return 0;

    MR_IN(78)

    newb=mr_mip->IOBASE;
    oldb=mr_mip->apbase;
    mr_setbase(_MIPP_ newb); /* temporarily change base ... */
    b=mr_mip->base;
    mr_mip->check=OFF;
    ipt=instr(_MIPP_ mr_mip->w5,string); /* ... and get number  */

    mr_mip->check=ON;
    lx=(mr_mip->w5->len&MR_OBITS);
#ifdef MR_FLASH
    if ((int)(lx&MR_MSK)>mr_mip->nib || (int)((lx>>MR_BTS)&MR_MSK)>mr_mip->nib)
#else
    if ((int)lx>mr_mip->nib)
#endif
    {   /* numerator or denominator too big */
        mr_berror(_MIPP_ MR_ERR_OVERFLOW);
        MR_OUT
        return 0;
    }
    mr_setbase(_MIPP_ oldb);      /* restore original base */

    cbase(_MIPP_ mr_mip->w5,b,x);

    MR_OUT
    return ipt;
}

#endif
#endif
