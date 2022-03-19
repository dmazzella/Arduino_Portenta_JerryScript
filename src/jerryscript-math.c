/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is based on work under the following copyright and permission
 * notice:
 *
 *     Copyright (C) 1993, 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 *     Developed at SunSoft, a Sun Microsystems, Inc. business.
 *     Permission to use, copy, modify, and distribute this
 *     software is freely granted, provided that this notice
 *     is preserved.
 *
 *     @(#)k_rem_pio2.c 1.3 95/01/18
 *     @(#)e_rem_pio2.c 1.4 95/01/18
 *     @(#)k_sin.c 1.3 95/01/18
 *     @(#)k_cos.c 1.3 95/01/18
 *     @(#)k_tan.c 1.5 04/04/22
 *     @(#)s_sin.c 1.3 95/01/18
 *     @(#)s_cos.c 1.3 95/01/18
 *     @(#)s_tan.c 1.3 95/01/18
 */


#ifndef JERRY_MATH_INTERNAL_H
#define JERRY_MATH_INTERNAL_H

/* Sometimes it's necessary to define __LITTLE_ENDIAN explicitly
   but these catch some common cases. */

#ifndef __LITTLE_ENDIAN
/* Check if compiler has byte order macro. Some older versions do not.
 * If byte order is supported and set to little or target is among common
 * cases checked define __LITTLE_ENDIAN.
 */
#if (defined(i386) || defined(__i386) || defined(__i386__) || defined(i486) || defined(__i486) || defined(__i486__)  \
     || defined(intel) || defined(x86) || defined(i86pc) || defined(_M_IX86) || defined(_M_AMD64) || defined(_M_X64) \
     || defined(__alpha) || defined(__osf__) || defined(__x86_64__) || defined(__arm__) || defined(__aarch64__)      \
     || defined(_M_ARM) || defined(_M_ARM64) || defined(__xtensa__) || defined(__MIPSEL))                            \
  || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
#define __LITTLE_ENDIAN
#endif /* platform defines */
#endif /* !__LITTLE_ENDIAN */

#ifdef __LITTLE_ENDIAN
#define __HI(x) *(1 + (const int *) &x)
#define __LO(x) *(const int *) &x
typedef union
{
  double dbl;
  struct
  {
    int lo;
    int hi;
  } as_int;
} double_accessor;
#else /* !__LITTLE_ENDIAN */
#define __HI(x) *(const int *) &x
#define __LO(x) *(1 + (const int *) &x)

typedef union
{
  double dbl;
  struct
  {
    int hi;
    int lo;
  } as_int;
} double_accessor;
#endif /* __LITTLE_ENDIAN */

#ifndef NAN
#ifdef _MSC_VER
#define INFINITY ((float) (1e+300 * 1e+300)) /* 1e+300*1e+300 must overflow */
#define NAN      ((float) (INFINITY * 0.0f))
#else /* !_MSC_VER */
#define INFINITY ((float) (1.0 / 0.0))
#define NAN      ((float) (0.0 / 0.0))
#endif /* _MSC_VER */
#endif /* !NAN */

/*
 * ANSI/POSIX
 */
double acos (double x);
double asin (double x);
double atan (double x);
double atan2 (double y, double x);
double cos (double x);
double sin (double x);
double tan (double x);

double cosh (double x);
double sinh (double x);
double tanh (double x);

double acosh (double x);
double asinh (double x);
double atanh (double x);

double exp (double x);
double expm1 (double x);
double log (double x);
double log1p (double x);
double log2 (double x);
double log10 (double);

double pow (double x, double y);
double sqrt (double x);
double cbrt (double);

double ceil (double x);
double fabs (double x);
double floor (double x);
double fmod (double x, double y);

double nextafter (double x, double y);

/*
 * Functions callable from C, intended to support IEEE arithmetic.
 */
double copysign (double x, double y);
double scalbn (double x, int n);

#endif /* !JERRY_MATH_INTERNAL_H */

#define zero   0.00000000000000000000e+00 /* 0x00000000, 0x00000000 */
#define half   5.00000000000000000000e-01 /* 0x3FE00000, 0x00000000 */
#define one    1.00000000000000000000e+00 /* 0x3FF00000, 0x00000000 */
#define two24  1.67772160000000000000e+07 /* 0x41700000, 0x00000000 */
#define twon24 5.96046447753906250000e-08 /* 0x3E700000, 0x00000000 */

/* __kernel_rem_pio2(x,y,e0,nx,prec)
 * double x[],y[]; int e0,nx,prec;
 *
 * __kernel_rem_pio2 return the last three digits of N with
 *              y = x - N*pi/2
 * so that |y| < pi/2.
 *
 * The method is to compute the integer (mod 8) and fraction parts of
 * (2/pi)*x without doing the full multiplication. In general we
 * skip the part of the product that are known to be a huge integer (
 * more accurately, = 0 mod 8 ). Thus the number of operations are
 * independent of the exponent of the input.
 *
 * (2/pi) is represented by an array of 24-bit integers in ipio2[].
 *
 * Input parameters:
 *      x[]     The input value (must be positive) is broken into nx
 *              pieces of 24-bit integers in double precision format.
 *              x[i] will be the i-th 24 bit of x. The scaled exponent
 *              of x[0] is given in input parameter e0 (i.e., x[0]*2^e0
 *              match x's up to 24 bits.
 *
 *              Example of breaking a double positive z into x[0]+x[1]+x[2]:
 *                      e0 = ilogb(z)-23
 *                      z  = scalbn(z,-e0)
 *              for i = 0,1,2
 *                      x[i] = floor(z)
 *                      z    = (z-x[i])*2**24
 *
 *      y[]     ouput result in an array of double precision numbers.
 *              The dimension of y[] is:
 *                      24-bit  precision       1
 *                      53-bit  precision       2
 *                      64-bit  precision       2
 *                      113-bit precision       3
 *              The actual value is the sum of them. Thus for 113-bit
 *              precison, one may have to do something like:
 *
 *              long double t,w,r_head, r_tail;
 *              t = (long double)y[2] + (long double)y[1];
 *              w = (long double)y[0];
 *              r_head = t+w;
 *              r_tail = w - (r_head - t);
 *
 *      e0      The exponent of x[0]
 *
 *      nx      dimension of x[]
 *
 *      prec    an integer indicating the precision:
 *                      0       24  bits (single)
 *                      1       53  bits (double)
 *                      2       64  bits (extended)
 *                      3       113 bits (quad)
 *
 * External function:
 *      double scalbn(), floor();
 *
 * Here is the description of some local variables:
 *
 *      ipio2[] integer array, contains the (24*i)-th to (24*i+23)-th
 *              bit of 2/pi after binary point. The corresponding
 *              floating value is
 *
 *                      ipio2[i] * 2^(-24(i+1)).
 *
 *      jk      jk+1 is the initial number of terms of ipio2[] needed
 *              in the computation. The recommended value is 2,3,4,
 *              6 for single, double, extended,and quad.
 *
 *      jz      local integer variable indicating the number of
 *              terms of ipio2[] used.
 *
 *      jx      nx - 1
 *
 *      jv      index for pointing to the suitable ipio2[] for the
 *              computation. In general, we want
 *                      ( 2^e0*x[0] * ipio2[jv-1]*2^(-24jv) )/8
 *              is an integer. Thus
 *                      e0-3-24*jv >= 0 or (e0-3)/24 >= jv
 *              Hence jv = max(0,(e0-3)/24).
 *
 *      jp      jp+1 is the number of terms in PIo2[] needed, jp = jk.
 *
 *      q[]     double array with integral value, representing the
 *              24-bits chunk of the product of x and 2/pi.
 *
 *      q0      the corresponding exponent of q[0]. Note that the
 *              exponent for q[i] would be q0-24*i.
 *
 *      PIo2[]  double precision array, obtained by cutting pi/2
 *              into 24 bits chunks.
 *
 *      f[]     ipio2[] in floating point
 *
 *      iq[]    integer array by breaking up q[] in 24-bits chunk.
 *
 *      fq[]    final product of x*(2/pi) in fq[0],..,fq[jk]
 *
 *      ih      integer. If >0 it indicates q[] is >= 0.5, hence
 *              it also indicates the *sign* of the result.
 */

/*
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

/* initial value for jk */
static const int init_jk[] = { 2, 3, 4, 6 };

static const double PIo2[] = {
  1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
  7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
  5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
  3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
  1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
  1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
  2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
  2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
};

/*
 * Table of constants for 2/pi, 396 Hex digits (476 decimal) of 2/pi
 */
static const int ipio2[] = {
  0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62, 0x95993C, 0x439041, 0xFE5163, 0xABDEBB, 0xC561B7,
  0x246E3A, 0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129, 0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C,
  0x7026B4, 0x5F7E41, 0x3991D6, 0x398353, 0x39F49C, 0x845F8B, 0xBDF928, 0x3B1FF8, 0x97FFDE, 0x05980F, 0xEF2F11,
  0x8B5A0A, 0x6D1F6D, 0x367ECF, 0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5, 0xF17B3D, 0x0739F7,
  0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08, 0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3, 0x91615E,
  0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880, 0x4D7327, 0x310606, 0x1556CA, 0x73A8C9, 0x60E27B, 0xC08C6B,
};

static int
__kernel_rem_pio2 (double *x, double *y, int e0, int nx, int prec)
{
  int jz, jx, jv, jp, jk, carry, n, iq[20], i, j, k, m, q0, ih;
  double z, fw, f[20], fq[20], q[20];

  /* initialize jk */
  jk = init_jk[prec];
  jp = jk;

  /* determine jx, jv, q0, note that 3 > q0 */
  jx = nx - 1;
  jv = (e0 - 3) / 24;
  if (jv < 0)
  {
    jv = 0;
  }
  q0 = e0 - 24 * (jv + 1);

  /* set up f[0] to f[jx + jk] where f[jx + jk] = ipio2[jv + jk] */
  j = jv - jx;
  m = jx + jk;
  for (i = 0; i <= m; i++, j++)
  {
    f[i] = (j < 0) ? zero : (double) ipio2[j];
  }

  /* compute q[0], q[1], ... q[jk] */
  for (i = 0; i <= jk; i++)
  {
    for (j = 0, fw = 0.0; j <= jx; j++)
    {
      fw += x[j] * f[jx + i - j];
    }
    q[i] = fw;
  }

  jz = jk;
recompute:
  /* distill q[] into iq[] reversingly */
  for (i = 0, j = jz, z = q[jz]; j > 0; i++, j--)
  {
    fw = (double) ((int) (twon24 * z));
    iq[i] = (int) (z - two24 * fw);
    z = q[j - 1] + fw;
  }

  /* compute n */
  z = scalbn (z, q0); /* actual value of z */
  z -= 8.0 * floor (z * 0.125); /* trim off integer >= 8 */
  n = (int) z;
  z -= (double) n;
  ih = 0;
  if (q0 > 0) /* need iq[jz - 1] to determine n */
  {
    i = (iq[jz - 1] >> (24 - q0));
    n += i;
    iq[jz - 1] -= i << (24 - q0);
    ih = iq[jz - 1] >> (23 - q0);
  }
  else if (q0 == 0)
  {
    ih = iq[jz - 1] >> 23;
  }
  else if (z >= 0.5)
  {
    ih = 2;
  }

  if (ih > 0) /* q > 0.5 */
  {
    n += 1;
    carry = 0;
    for (i = 0; i < jz; i++) /* compute 1 - q */
    {
      j = iq[i];
      if (carry == 0)
      {
        if (j != 0)
        {
          carry = 1;
          iq[i] = 0x1000000 - j;
        }
      }
      else
      {
        iq[i] = 0xffffff - j;
      }
    }
    if (q0 > 0) /* rare case: chance is 1 in 12 */
    {
      switch (q0)
      {
        case 1:
        {
          iq[jz - 1] &= 0x7fffff;
          break;
        }
        case 2:
        {
          iq[jz - 1] &= 0x3fffff;
          break;
        }
      }
    }
    if (ih == 2)
    {
      z = one - z;
      if (carry != 0)
      {
        z -= scalbn (one, q0);
      }
    }
  }

  /* check if recomputation is needed */
  if (z == zero)
  {
    j = 0;
    for (i = jz - 1; i >= jk; i--)
    {
      j |= iq[i];
    }
    if (j == 0) /* need recomputation */
    {
      for (k = 1; iq[jk - k] == 0; k++) /* k = no. of terms needed */
      {
      }

      for (i = jz + 1; i <= jz + k; i++) /* add q[jz + 1] to q[jz + k] */
      {
        f[jx + i] = (double) ipio2[jv + i];
        for (j = 0, fw = 0.0; j <= jx; j++)
        {
          fw += x[j] * f[jx + i - j];
        }
        q[i] = fw;
      }
      jz += k;
      goto recompute;
    }
  }

  /* chop off zero terms */
  if (z == 0.0)
  {
    jz -= 1;
    q0 -= 24;
    while (iq[jz] == 0)
    {
      jz--;
      q0 -= 24;
    }
  }
  else
  { /* break z into 24-bit if necessary */
    z = scalbn (z, -q0);
    if (z >= two24)
    {
      fw = (double) ((int) (twon24 * z));
      iq[jz] = (int) (z - two24 * fw);
      jz += 1;
      q0 += 24;
      iq[jz] = (int) fw;
    }
    else
    {
      iq[jz] = (int) z;
    }
  }

  /* convert integer "bit" chunk to floating-point value */
  fw = scalbn (one, q0);
  for (i = jz; i >= 0; i--)
  {
    q[i] = fw * (double) iq[i];
    fw *= twon24;
  }

  /* compute PIo2[0, ..., jp] * q[jz, ..., 0] */
  for (i = jz; i >= 0; i--)
  {
    for (fw = 0.0, k = 0; k <= jp && k <= jz - i; k++)
    {
      fw += PIo2[k] * q[i + k];
    }
    fq[jz - i] = fw;
  }

  /* compress fq[] into y[] */
  switch (prec)
  {
    case 0:
    {
      fw = 0.0;
      for (i = jz; i >= 0; i--)
      {
        fw += fq[i];
      }
      y[0] = (ih == 0) ? fw : -fw;
      break;
    }
    case 1:
    case 2:
    {
      fw = 0.0;
      for (i = jz; i >= 0; i--)
      {
        fw += fq[i];
      }
      y[0] = (ih == 0) ? fw : -fw;
      fw = fq[0] - fw;
      for (i = 1; i <= jz; i++)
      {
        fw += fq[i];
      }
      y[1] = (ih == 0) ? fw : -fw;
      break;
    }
    case 3: /* painful */
    {
      for (i = jz; i > 0; i--)
      {
        fw = fq[i - 1] + fq[i];
        fq[i] += fq[i - 1] - fw;
        fq[i - 1] = fw;
      }
      for (i = jz; i > 1; i--)
      {
        fw = fq[i - 1] + fq[i];
        fq[i] += fq[i - 1] - fw;
        fq[i - 1] = fw;
      }
      for (fw = 0.0, i = jz; i >= 2; i--)
      {
        fw += fq[i];
      }
      if (ih == 0)
      {
        y[0] = fq[0];
        y[1] = fq[1];
        y[2] = fw;
      }
      else
      {
        y[0] = -fq[0];
        y[1] = -fq[1];
        y[2] = -fw;
      }
    }
  }
  return n & 7;
} /* __kernel_rem_pio2 */

/* __ieee754_rem_pio2(x,y)
 * return the remainder of x rem pi/2 in y[0]+y[1]
 * use __kernel_rem_pio2()
 */

static const int npio2_hw[] = {
  0x3FF921FB, 0x400921FB, 0x4012D97C, 0x401921FB, 0x401F6A7A, 0x4022D97C, 0x4025FDBB, 0x402921FB,
  0x402C463A, 0x402F6A7A, 0x4031475C, 0x4032D97C, 0x40346B9C, 0x4035FDBB, 0x40378FDB, 0x403921FB,
  0x403AB41B, 0x403C463A, 0x403DD85A, 0x403F6A7A, 0x40407E4C, 0x4041475C, 0x4042106C, 0x4042D97C,
  0x4043A28C, 0x40446B9C, 0x404534AC, 0x4045FDBB, 0x4046C6CB, 0x40478FDB, 0x404858EB, 0x404921FB,
};

/*
 * invpio2:  53 bits of 2/pi
 * pio2_1:   first  33 bit of pi/2
 * pio2_1t:  pi/2 - pio2_1
 * pio2_2:   second 33 bit of pi/2
 * pio2_2t:  pi/2 - (pio2_1 + pio2_2)
 * pio2_3:   third  33 bit of pi/2
 * pio2_3t:  pi/2 - (pio2_1 + pio2_2 + pio2_3)
 */
#define invpio2 6.36619772367581382433e-01 /* 0x3FE45F30, 0x6DC9C883 */
#define pio2_1  1.57079632673412561417e+00 /* 0x3FF921FB, 0x54400000 */
#define pio2_1t 6.07710050650619224932e-11 /* 0x3DD0B461, 0x1A626331 */
#define pio2_2  6.07710050630396597660e-11 /* 0x3DD0B461, 0x1A600000 */
#define pio2_2t 2.02226624879595063154e-21 /* 0x3BA3198A, 0x2E037073 */
#define pio2_3  2.02226624871116645580e-21 /* 0x3BA3198A, 0x2E000000 */
#define pio2_3t 8.47842766036889956997e-32 /* 0x397B839A, 0x252049C1 */

static int
__ieee754_rem_pio2 (double x, double *y)
{
  double_accessor z;
  double w, t, r, fn;
  double tx[3];
  int e0, i, j, nx, n, ix, hx;

  hx = __HI (x); /* high word of x */
  ix = hx & 0x7fffffff;
  if (ix <= 0x3fe921fb) /* |x| ~<= pi/4 , no need for reduction */
  {
    y[0] = x;
    y[1] = 0;
    return 0;
  }
  if (ix < 0x4002d97c) /* |x| < 3pi/4, special case with n = +-1 */
  {
    if (hx > 0)
    {
      z.dbl = x - pio2_1;
      if (ix != 0x3ff921fb) /* 33 + 53 bit pi is good enough */
      {
        y[0] = z.dbl - pio2_1t;
        y[1] = (z.dbl - y[0]) - pio2_1t;
      }
      else /* near pi/2, use 33 + 33 + 53 bit pi */
      {
        z.dbl -= pio2_2;
        y[0] = z.dbl - pio2_2t;
        y[1] = (z.dbl - y[0]) - pio2_2t;
      }
      return 1;
    }
    else /* negative x */
    {
      z.dbl = x + pio2_1;
      if (ix != 0x3ff921fb) /* 33 + 53 bit pi is good enough */
      {
        y[0] = z.dbl + pio2_1t;
        y[1] = (z.dbl - y[0]) + pio2_1t;
      }
      else /* near pi/2, use 33 + 33 + 53 bit pi */
      {
        z.dbl += pio2_2;
        y[0] = z.dbl + pio2_2t;
        y[1] = (z.dbl - y[0]) + pio2_2t;
      }
      return -1;
    }
  }
  if (ix <= 0x413921fb) /* |x| ~<= 2^19 * (pi/2), medium size */
  {
    t = fabs (x);
    n = (int) (t * invpio2 + half);
    fn = (double) n;
    r = t - fn * pio2_1;
    w = fn * pio2_1t; /* 1st round good to 85 bit */
    if (n < 32 && ix != npio2_hw[n - 1])
    {
      y[0] = r - w; /* quick check no cancellation */
    }
    else
    {
      j = ix >> 20;
      y[0] = r - w;
      i = j - (((__HI (y[0])) >> 20) & 0x7ff);
      if (i > 16) /* 2nd iteration needed, good to 118 */
      {
        t = r;
        w = fn * pio2_2;
        r = t - w;
        w = fn * pio2_2t - ((t - r) - w);
        y[0] = r - w;
        i = j - (((__HI (y[0])) >> 20) & 0x7ff);
        if (i > 49) /* 3rd iteration need, 151 bits acc, will cover all possible cases */
        {
          t = r;
          w = fn * pio2_3;
          r = t - w;
          w = fn * pio2_3t - ((t - r) - w);
          y[0] = r - w;
        }
      }
    }
    y[1] = (r - y[0]) - w;
    if (hx < 0)
    {
      y[0] = -y[0];
      y[1] = -y[1];
      return -n;
    }
    else
    {
      return n;
    }
  }
  /*
   * all other (large) arguments
   */
  if (ix >= 0x7ff00000) /* x is inf or NaN */
  {
    y[0] = y[1] = x - x;
    return 0;
  }
  /* set z = scalbn(|x|, ilogb(x) - 23) */
  z.as_int.lo = __LO (x);
  e0 = (ix >> 20) - 1046; /* e0 = ilogb(z) - 23; */
  z.as_int.hi = ix - (e0 << 20);
  for (i = 0; i < 2; i++)
  {
    tx[i] = (double) ((int) (z.dbl));
    z.dbl = (z.dbl - tx[i]) * two24;
  }
  tx[2] = z.dbl;
  nx = 3;
  while (tx[nx - 1] == zero) /* skip zero term */
  {
    nx--;
  }
  n = __kernel_rem_pio2 (tx, y, e0, nx, 2);
  if (hx < 0)
  {
    y[0] = -y[0];
    y[1] = -y[1];
    return -n;
  }
  return n;
} /* __ieee754_rem_pio2 */

/* __kernel_sin( x, y, iy)
 * kernel sin function on [-pi/4, pi/4], pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 * Input iy indicates whether y is 0. (if iy=0, y assume to be 0).
 *
 * Algorithm
 *      1. Since sin(-x) = -sin(x), we need only to consider positive x.
 *      2. if x < 2^-27 (hx<0x3e400000 0), return x with inexact if x!=0.
 *      3. sin(x) is approximated by a polynomial of degree 13 on
 *         [0,pi/4]
 *                               3            13
 *              sin(x) ~ x + S1*x + ... + S6*x
 *         where
 *
 *      |sin(x)         2     4     6     8     10     12  |     -58
 *      |----- - (1+S1*x +S2*x +S3*x +S4*x +S5*x  +S6*x   )| <= 2
 *      |  x                                               |
 *
 *      4. sin(x+y) = sin(x) + sin'(x')*y
 *                  ~ sin(x) + (1-x*x/2)*y
 *         For better accuracy, let
 *                   3      2      2      2      2
 *              r = x *(S2+x *(S3+x *(S4+x *(S5+x *S6))))
 *         then                   3    2
 *              sin(x) = x + (S1*x + (x *(r-y/2)+y))
 */

#define S1 -1.66666666666666324348e-01 /* 0xBFC55555, 0x55555549 */
#define S2 8.33333333332248946124e-03 /* 0x3F811111, 0x1110F8A6 */
#define S3 -1.98412698298579493134e-04 /* 0xBF2A01A0, 0x19C161D5 */
#define S4 2.75573137070700676789e-06 /* 0x3EC71DE3, 0x57B1FE7D */
#define S5 -2.50507602534068634195e-08 /* 0xBE5AE5E6, 0x8A2B9CEB */
#define S6 1.58969099521155010221e-10 /* 0x3DE5D93A, 0x5ACFD57C */

static double
__kernel_sin (double x, double y, int iy)
{
  double z, r, v;
  int ix;

  ix = __HI (x) & 0x7fffffff; /* high word of x */
  if (ix < 0x3e400000) /* |x| < 2**-27 */
  {
    if ((int) x == 0)
    {
      return x; /* generate inexact */
    }
  }
  z = x * x;
  v = z * x;
  r = S2 + z * (S3 + z * (S4 + z * (S5 + z * S6)));
  if (iy == 0)
  {
    return x + v * (S1 + z * r);
  }
  else
  {
    return x - ((z * (half * y - v * r) - y) - v * S1);
  }
} /* __kernel_sin */

/*
 * __kernel_cos( x,  y )
 * kernel cos function on [-pi/4, pi/4], pi/4 ~ 0.785398164
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 *
 * Algorithm
 *      1. Since cos(-x) = cos(x), we need only to consider positive x.
 *      2. if x < 2^-27 (hx<0x3e400000 0), return 1 with inexact if x!=0.
 *      3. cos(x) is approximated by a polynomial of degree 14 on
 *         [0,pi/4]
 *                                       4            14
 *              cos(x) ~ 1 - x*x/2 + C1*x + ... + C6*x
 *         where the remez error is
 *
 *      |              2     4     6     8     10    12     14 |     -58
 *      |cos(x)-(1-.5*x +C1*x +C2*x +C3*x +C4*x +C5*x  +C6*x  )| <= 2
 *      |                                                      |
 *
 *                     4     6     8     10    12     14
 *      4. let r = C1*x +C2*x +C3*x +C4*x +C5*x  +C6*x  , then
 *             cos(x) = 1 - x*x/2 + r
 *         since cos(x+y) ~ cos(x) - sin(x)*y
 *                        ~ cos(x) - x*y,
 *         a correction term is necessary in cos(x) and hence
 *              cos(x+y) = 1 - (x*x/2 - (r - x*y))
 *         For better accuracy when x > 0.3, let qx = |x|/4 with
 *         the last 32 bits mask off, and if x > 0.78125, let qx = 0.28125.
 *         Then
 *              cos(x+y) = (1-qx) - ((x*x/2-qx) - (r-x*y)).
 *         Note that 1-qx and (x*x/2-qx) is EXACT here, and the
 *         magnitude of the latter is at least a quarter of x*x/2,
 *         thus, reducing the rounding error in the subtraction.
 */

#define C1 4.16666666666666019037e-02 /* 0x3FA55555, 0x5555554C */
#define C2 -1.38888888888741095749e-03 /* 0xBF56C16C, 0x16C15177 */
#define C3 2.48015872894767294178e-05 /* 0x3EFA01A0, 0x19CB1590 */
#define C4 -2.75573143513906633035e-07 /* 0xBE927E4F, 0x809C52AD */
#define C5 2.08757232129817482790e-09 /* 0x3E21EE9E, 0xBDB4B1C4 */
#define C6 -1.13596475577881948265e-11 /* 0xBDA8FAE9, 0xBE8838D4 */

static double
__kernel_cos (double x, double y)
{
  double a, hz, z, r;
  int ix;

  ix = __HI (x) & 0x7fffffff; /* ix = |x|'s high word */
  if (ix < 0x3e400000) /* if x < 2**27 */
  {
    if (((int) x) == 0)
    {
      return one; /* generate inexact */
    }
  }
  z = x * x;
  r = z * (C1 + z * (C2 + z * (C3 + z * (C4 + z * (C5 + z * C6)))));
  if (ix < 0x3FD33333) /* if |x| < 0.3 */
  {
    return one - (0.5 * z - (z * r - x * y));
  }
  else
  {
    double_accessor qx;
    if (ix > 0x3fe90000) /* x > 0.78125 */
    {
      qx.dbl = 0.28125;
    }
    else
    {
      qx.as_int.hi = ix - 0x00200000; /* x / 4 */
      qx.as_int.lo = 0;
    }
    hz = 0.5 * z - qx.dbl;
    a = one - qx.dbl;
    return a - (hz - (z * r - x * y));
  }
} /* __kernel_cos */

/* __kernel_tan( x, y, k )
 * kernel tan function on [-pi/4, pi/4], pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 * Input k indicates whether tan (if k = 1) or -1/tan (if k = -1) is returned.
 *
 * Algorithm
 *      1. Since tan(-x) = -tan(x), we need only to consider positive x.
 *      2. if x < 2^-28 (hx<0x3e300000 0), return x with inexact if x!=0.
 *      3. tan(x) is approximated by a odd polynomial of degree 27 on
 *         [0,0.67434]
 *                               3             27
 *              tan(x) ~ x + T1*x + ... + T13*x
 *         where
 *
 *              |tan(x)         2     4            26   |     -59.2
 *              |----- - (1+T1*x +T2*x +.... +T13*x    )| <= 2
 *              |  x                                    |
 *
 *         Note: tan(x+y) = tan(x) + tan'(x)*y
 *                        ~ tan(x) + (1+x*x)*y
 *         Therefore, for better accuracy in computing tan(x+y), let
 *                   3      2      2       2       2
 *              r = x *(T2+x *(T3+x *(...+x *(T12+x *T13))))
 *         then
 *                                  3    2
 *              tan(x+y) = x + (T1*x + (x *(r+y)+y))
 *
 *      4. For x in [0.67434,pi/4],  let y = pi/4 - x, then
 *              tan(x) = tan(pi/4-y) = (1-tan(y))/(1+tan(y))
 *                     = 1 - 2*(tan(y) - (tan(y)^2)/(1+tan(y)))
 */

#define T0     3.33333333333334091986e-01 /* 3FD55555, 55555563 */
#define T1     1.33333333333201242699e-01 /* 3FC11111, 1110FE7A */
#define T2     5.39682539762260521377e-02 /* 3FABA1BA, 1BB341FE */
#define T3     2.18694882948595424599e-02 /* 3F9664F4, 8406D637 */
#define T4     8.86323982359930005737e-03 /* 3F8226E3, E96E8493 */
#define T5     3.59207910759131235356e-03 /* 3F6D6D22, C9560328 */
#define T6     1.45620945432529025516e-03 /* 3F57DBC8, FEE08315 */
#define T7     5.88041240820264096874e-04 /* 3F4344D8, F2F26501 */
#define T8     2.46463134818469906812e-04 /* 3F3026F7, 1A8D1068 */
#define T9     7.81794442939557092300e-05 /* 3F147E88, A03792A6 */
#define T10    7.14072491382608190305e-05 /* 3F12B80F, 32F0A7E9 */
#define T11    -1.85586374855275456654e-05 /* BEF375CB, DB605373 */
#define T12    2.59073051863633712884e-05 /* 3EFB2A70, 74BF7AD4 */
#define pio4   7.85398163397448278999e-01 /* 3FE921FB, 54442D18 */
#define pio4lo 3.06161699786838301793e-17 /* 3C81A626, 33145C07 */

static double
__kernel_tan (double x, double y, int iy)
{
  double_accessor z;
  double r, v, w, s;
  int ix, hx;

  hx = __HI (x); /* high word of x */
  ix = hx & 0x7fffffff; /* high word of |x| */
  if (ix < 0x3e300000) /* x < 2**-28 */
  {
    if ((int) x == 0) /* generate inexact */
    {
      if (((ix | __LO (x)) | (iy + 1)) == 0)
      {
        return one / fabs (x);
      }
      else
      {
        if (iy == 1)
        {
          return x;
        }
        else /* compute -1 / (x + y) carefully */
        {
          double a;
          double_accessor t;

          z.dbl = w = x + y;
          z.as_int.lo = 0;
          v = y - (z.dbl - x);
          t.dbl = a = -one / w;
          t.as_int.lo = 0;
          s = one + t.dbl * z.dbl;
          return t.dbl + a * (s + t.dbl * v);
        }
      }
    }
  }
  if (ix >= 0x3FE59428) /* |x| >= 0.6744 */
  {
    if (hx < 0)
    {
      x = -x;
      y = -y;
    }
    z.dbl = pio4 - x;
    w = pio4lo - y;
    x = z.dbl + w;
    y = 0.0;
  }
  z.dbl = x * x;
  w = z.dbl * z.dbl;
  /*
   * Break x^5 * (T[1] + x^2 * T[2] + ...) into
   * x^5 (T[1] + x^4 * T[3] + ... + x^20 * T[11]) +
   * x^5 (x^2 * (T[2] + x^4 * T[4] + ... + x^22 * [T12]))
   */
  r = T1 + w * (T3 + w * (T5 + w * (T7 + w * (T9 + w * T11))));
  v = z.dbl * (T2 + w * (T4 + w * (T6 + w * (T8 + w * (T10 + w * T12)))));
  s = z.dbl * x;
  r = y + z.dbl * (s * (r + v) + y);
  r += T0 * s;
  w = x + r;
  if (ix >= 0x3FE59428)
  {
    v = (double) iy;
    return (double) (1 - ((hx >> 30) & 2)) * (v - 2.0 * (x - (w * w / (w + v) - r)));
  }
  if (iy == 1)
  {
    return w;
  }
  else
  {
    /*
     * if allow error up to 2 ulp, simply return
     * -1.0 / (x + r) here
     */
    /* compute -1.0 / (x + r) accurately */
    double a;
    double_accessor t;

    z.dbl = w;
    z.as_int.lo = 0;
    v = r - (z.dbl - x); /* z + v = r + x */
    t.dbl = a = -1.0 / w; /* a = -1.0 / w */
    t.as_int.lo = 0;
    s = 1.0 + t.dbl * z.dbl;
    return t.dbl + a * (s + t.dbl * v);
  }
} /* __kernel_tan */

/* Method:
 *      Let S,C and T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *      in [-pi/4 , +pi/4], and let n = k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          S           C             T
 *          1          C          -S            -1/T
 *          2         -S          -C             T
 *          3         -C           S            -1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

/* sin(x)
 * Return sine function of x.
 *
 * kernel function:
 *      __kernel_sin            ... sine function on [-pi/4,pi/4]
 *      __kernel_cos            ... cose function on [-pi/4,pi/4]
 *      __ieee754_rem_pio2      ... argument reduction routine
 */
double
sin (double x)
{
  double y[2], z = 0.0;
  int n, ix;

  /* High word of x. */
  ix = __HI (x);

  /* |x| ~< pi/4 */
  ix &= 0x7fffffff;
  if (ix <= 0x3fe921fb)
  {
    return __kernel_sin (x, z, 0);
  }

  /* sin(Inf or NaN) is NaN */
  else if (ix >= 0x7ff00000)
  {
    return x - x;
  }

  /* argument reduction needed */
  else
  {
    n = __ieee754_rem_pio2 (x, y);
    switch (n & 3)
    {
      case 0:
      {
        return __kernel_sin (y[0], y[1], 1);
      }
      case 1:
      {
        return __kernel_cos (y[0], y[1]);
      }
      case 2:
      {
        return -__kernel_sin (y[0], y[1], 1);
      }
      default:
      {
        return -__kernel_cos (y[0], y[1]);
      }
    }
  }
} /* sin */

/* cos(x)
 * Return cosine function of x.
 *
 * kernel function:
 *      __kernel_sin            ... sine function on [-pi/4,pi/4]
 *      __kernel_cos            ... cosine function on [-pi/4,pi/4]
 *      __ieee754_rem_pio2      ... argument reduction routine
 */

double
cos (double x)
{
  double y[2], z = 0.0;
  int n, ix;

  /* High word of x. */
  ix = __HI (x);

  /* |x| ~< pi/4 */
  ix &= 0x7fffffff;
  if (ix <= 0x3fe921fb)
  {
    return __kernel_cos (x, z);
  }

  /* cos(Inf or NaN) is NaN */
  else if (ix >= 0x7ff00000)
  {
    return x - x;
  }

  /* argument reduction needed */
  else
  {
    n = __ieee754_rem_pio2 (x, y);
    switch (n & 3)
    {
      case 0:
      {
        return __kernel_cos (y[0], y[1]);
      }
      case 1:
      {
        return -__kernel_sin (y[0], y[1], 1);
      }
      case 2:
      {
        return -__kernel_cos (y[0], y[1]);
      }
      default:
      {
        return __kernel_sin (y[0], y[1], 1);
      }
    }
  }
} /* cos */

/* tan(x)
 * Return tangent function of x.
 *
 * kernel function:
 *      __kernel_tan            ... tangent function on [-pi/4,pi/4]
 *      __ieee754_rem_pio2      ... argument reduction routine
 */

double
tan (double x)
{
  double y[2], z = 0.0;
  int n, ix;

  /* High word of x. */
  ix = __HI (x);

  /* |x| ~< pi/4 */
  ix &= 0x7fffffff;
  if (ix <= 0x3fe921fb)
  {
    return __kernel_tan (x, z, 1);
  }

  /* tan(Inf or NaN) is NaN */
  else if (ix >= 0x7ff00000)
  {
    return x - x; /* NaN */
  }

  /* argument reduction needed */
  else
  {
    n = __ieee754_rem_pio2 (x, y);
    return __kernel_tan (y[0], y[1], 1 - ((n & 1) << 1)); /*   1 -- n even, -1 -- n odd */
  }
} /* tan */

#undef zero
#undef half
#undef one
#undef two24
#undef twon24
#undef invpio2
#undef pio2_1
#undef pio2_1t
#undef pio2_2
#undef pio2_2t
#undef pio2_3
#undef pio2_3t
#undef S1
#undef S2
#undef S3
#undef S4
#undef S5
#undef S6
#undef C1
#undef C2
#undef C3
#undef C4
#undef C5
#undef C6
#undef T0
#undef T1
#undef T2
#undef T3
#undef T4
#undef T5
#undef T6
#undef T7
#undef T8
#undef T9
#undef T10
#undef T11
#undef T12
#undef pio4
#undef pio4lo


/* tanh(x)
 * Return the Hyperbolic Tangent of x
 *
 * Method:
 *                                 x   -x
 *                                e -  e
 *  0. tanh(x) is defined to be -----------
 *                                 x    -x
 *                                e  +  e
 *
 *  1. reduce x to non-negative by tanh(-x) = -tanh(x).
 *  2.  0      <= x <= 2**-55 : tanh(x) := x * (one + x)
 *
 *                                          -t
 *      2**-55 <  x <=  1     : tanh(x) := -----; t = expm1(-2x)
 *                                         t + 2
 *
 *                                               2
 *      1      <= x <=  22.0  : tanh(x) := 1-  ----- ; t = expm1(2x)
 *                                             t + 2
 *
 *      22.0   <  x <= INF    : tanh(x) := 1.
 *
 * Special cases:
 *  tanh(NaN) is NaN;
 *  only tanh(0) = 0 is exact for finite x.
 */
#define one  1.0
#define two  2.0
#define tiny 1.0e-300

double
tanh (double x)
{
  double t, z;
  int jx, ix;

  /* High word of |x|. */
  jx = __HI (x);
  ix = jx & 0x7fffffff;

  /* x is INF or NaN */
  if (ix >= 0x7ff00000)
  {
    if (jx >= 0)
    {
      /* tanh(+-inf) = +-1 */
      return one / x + one;
    }
    else
    {
      /* tanh(NaN) = NaN */
      return one / x - one;
    }
  }

  /* |x| < 22 */
  if (ix < 0x40360000)
  {
    /* |x| < 2**-55 */
    if (ix < 0x3c800000)
    {
      /* tanh(small) = small */
      return x * (one + x);
    }
    if (ix >= 0x3ff00000)
    {
      /* |x| >= 1  */
      t = expm1 (two * fabs (x));
      z = one - two / (t + two);
    }
    else
    {
      t = expm1 (-two * fabs (x));
      z = -t / (t + two);
    }
  }
  else
  {
    /* |x| > 22, return +-1 */
    z = one - tiny; /* raised inexact flag */
  }
  return (jx >= 0) ? z : -z;
} /* tanh */

#undef one
#undef two
#undef tiny


/* sqrt(x)
 * Return correctly rounded sqrt.
 *
 *           ------------------------------------------
 *           |  Use the hardware sqrt if you have one |
 *           ------------------------------------------
 *
 * Method:
 *   Bit by bit method using integer arithmetic. (Slow, but portable)
 *   1. Normalization
 *      Scale x to y in [1,4) with even powers of 2:
 *      find an integer k such that  1 <= (y=x*2^(2k)) < 4, then
 *              sqrt(x) = 2^k * sqrt(y)
 *   2. Bit by bit computation
 *      Let q  = sqrt(y) truncated to i bit after binary point (q = 1),
 *           i                                                   0
 *                                     i+1         2
 *          s  = 2*q , and      y  =  2   * ( y - q  ).         (1)
 *           i      i            i                 i
 *
 *      To compute q    from q , one checks whether
 *                  i+1       i
 *
 *                            -(i+1) 2
 *                      (q + 2      ) <= y.                     (2)
 *                        i
 *                                                            -(i+1)
 *      If (2) is false, then q   = q ; otherwise q   = q  + 2      .
 *                             i+1   i             i+1   i
 *
 *      With some algebric manipulation, it is not difficult to see
 *      that (2) is equivalent to
 *                             -(i+1)
 *                      s  +  2       <= y                      (3)
 *                       i                i
 *
 *      The advantage of (3) is that s  and y  can be computed by
 *                                    i      i
 *      the following recurrence formula:
 *          if (3) is false
 *
 *          s     =  s  ,       y    = y   ;                    (4)
 *           i+1      i          i+1    i
 *
 *          otherwise,
 *                         -i                     -(i+1)
 *          s     =  s  + 2  ,  y    = y  -  s  - 2             (5)
 *           i+1      i          i+1    i     i
 *
 *      One may easily use induction to prove (4) and (5).
 *      Note. Since the left hand side of (3) contain only i+2 bits,
 *            it does not necessary to do a full (53-bit) comparison
 *            in (3).
 *   3. Final rounding
 *      After generating the 53 bits result, we compute one more bit.
 *      Together with the remainder, we can decide whether the
 *      result is exact, bigger than 1/2ulp, or less than 1/2ulp
 *      (it will never equal to 1/2ulp).
 *      The rounding mode can be detected by checking whether
 *      huge + tiny is equal to huge, and whether huge - tiny is
 *      equal to huge for some floating point number "huge" and "tiny".
 *
 * Special cases:
 *      sqrt(+-0) = +-0         ... exact
 *      sqrt(inf) = inf
 *      sqrt(-ve) = NaN         ... with invalid signal
 *      sqrt(NaN) = NaN         ... with invalid signal for signaling NaN
 *
 * Other methods: see the appended file at the end of the program below.
 */

#define one  1.0
#define tiny 1.0e-300

double
sqrt (double x)
{
  int sign = (int) 0x80000000;
  unsigned r, t1, s1, ix1, q1;
  int ix0, s0, q, m, t, i;

  ix0 = __HI (x); /* high word of x */
  ix1 = __LO (x); /* low word of x */

  /* take care of Inf and NaN */
  if ((ix0 & 0x7ff00000) == 0x7ff00000)
  {
    return x * x + x; /* sqrt(NaN) = NaN, sqrt(+inf) = +inf, sqrt(-inf) = sNaN */
  }
  /* take care of zero */
  if (ix0 <= 0)
  {
    if (((ix0 & (~sign)) | ix1) == 0) /* sqrt(+-0) = +-0 */
    {
      return x;
    }
    else if (ix0 < 0) /* sqrt(-ve) = sNaN */
    {
      return NAN;
    }
  }
  /* normalize x */
  m = (ix0 >> 20);
  if (m == 0) /* subnormal x */
  {
    while (ix0 == 0)
    {
      m -= 21;
      ix0 |= (ix1 >> 11);
      ix1 <<= 21;
    }
    for (i = 0; (ix0 & 0x00100000) == 0; i++)
    {
      ix0 <<= 1;
    }
    m -= i - 1;
    ix0 |= (ix1 >> (32 - i));
    ix1 <<= i;
  }
  m -= 1023; /* unbias exponent */
  ix0 = (ix0 & 0x000fffff) | 0x00100000;
  if (m & 1) /* odd m, double x to make it even */
  {
    ix0 += ix0 + ((ix1 & sign) >> 31);
    ix1 += ix1;
  }
  m >>= 1; /* m = [m / 2] */

  /* generate sqrt(x) bit by bit */
  ix0 += ix0 + ((ix1 & sign) >> 31);
  ix1 += ix1;
  q = q1 = s0 = s1 = 0; /* [q,q1] = sqrt(x) */
  r = 0x00200000; /* r = moving bit from right to left */

  while (r != 0)
  {
    t = s0 + r;
    if (t <= ix0)
    {
      s0 = t + r;
      ix0 -= t;
      q += r;
    }
    ix0 += ix0 + ((ix1 & sign) >> 31);
    ix1 += ix1;
    r >>= 1;
  }

  r = sign;
  while (r != 0)
  {
    t1 = s1 + r;
    t = s0;
    if ((t < ix0) || ((t == ix0) && (t1 <= ix1)))
    {
      s1 = t1 + r;
      if (((t1 & sign) == sign) && (s1 & sign) == 0)
      {
        s0 += 1;
      }
      ix0 -= t;
      if (ix1 < t1)
      {
        ix0 -= 1;
      }
      ix1 -= t1;
      q1 += r;
    }
    ix0 += ix0 + ((ix1 & sign) >> 31);
    ix1 += ix1;
    r >>= 1;
  }

  double_accessor ret;

  /* use floating add to find out rounding direction */
  if ((ix0 | ix1) != 0)
  {
    ret.dbl = one - tiny; /* trigger inexact flag */
    if (ret.dbl >= one)
    {
      ret.dbl = one + tiny;
      if (q1 == (unsigned) 0xffffffff)
      {
        q1 = 0;
        q += 1;
      }
      else if (ret.dbl > one)
      {
        if (q1 == (unsigned) 0xfffffffe)
        {
          q += 1;
        }
        q1 += 2;
      }
      else
      {
        q1 += (q1 & 1);
      }
    }
  }
  ix0 = (q >> 1) + 0x3fe00000;
  ix1 = q1 >> 1;
  if ((q & 1) == 1)
  {
    ix1 |= sign;
  }
  ix0 += (m << 20);
  ret.as_int.hi = ix0;
  ret.as_int.lo = ix1;
  return ret.dbl;
} /* sqrt */

#undef one
#undef tiny

/*
Other methods  (use floating-point arithmetic)
-------------
(This is a copy of a drafted paper by Prof W. Kahan
and K.C. Ng, written in May, 1986)

        Two algorithms are given here to implement sqrt(x)
        (IEEE double precision arithmetic) in software.
        Both supply sqrt(x) correctly rounded. The first algorithm (in
        Section A) uses newton iterations and involves four divisions.
        The second one uses reciproot iterations to avoid division, but
        requires more multiplications. Both algorithms need the ability
        to chop results of arithmetic operations instead of round them,
        and the INEXACT flag to indicate when an arithmetic operation
        is executed exactly with no roundoff error, all part of the
        standard (IEEE 754-1985). The ability to perform shift, add,
        subtract and logical AND operations upon 32-bit words is needed
        too, though not part of the standard.

A.  sqrt(x) by Newton Iteration

   (1)  Initial approximation

        Let x0 and x1 be the leading and the trailing 32-bit words of
        a floating point number x (in IEEE double format) respectively

            1    11                  52                           ...widths
           ------------------------------------------------------
        x: |s|    e     |             f                         |
           ------------------------------------------------------
              msb    lsb  msb                                 lsb ...order

             ------------------------        ------------------------
        x0:  |s|   e    |    f1     |    x1: |          f2           |
             ------------------------        ------------------------

        By performing shifts and subtracts on x0 and x1 (both regarded
        as integers), we obtain an 8-bit approximation of sqrt(x) as
        follows.

                k  := (x0>>1) + 0x1ff80000;
                y0 := k - T1[31&(k>>15)].       ... y ~ sqrt(x) to 8 bits
        Here k is a 32-bit integer and T1[] is an integer array containing
        correction terms. Now magically the floating value of y (y's
        leading 32-bit word is y0, the value of its trailing word is 0)
        approximates sqrt(x) to almost 8-bit.

        Value of T1:
        static int T1[32]= {
        0,      1024,   3062,   5746,   9193,   13348,  18162,  23592,
        29598,  36145,  43202,  50740,  58733,  67158,  75992,  85215,
        83599,  71378,  60428,  50647,  41945,  34246,  27478,  21581,
        16499,  12183,  8588,   5674,   3403,   1742,   661,    130,};

    (2) Iterative refinement

        Apply Heron's rule three times to y, we have y approximates
        sqrt(x) to within 1 ulp (Unit in the Last Place):

                y := (y+x/y)/2          ... almost 17 sig. bits
                y := (y+x/y)/2          ... almost 35 sig. bits
                y := y-(y-x/y)/2        ... within 1 ulp

        Remark 1.
            Another way to improve y to within 1 ulp is:

                y := (y+x/y)            ... almost 17 sig. bits to 2*sqrt(x)
                y := y - 0x00100006     ... almost 18 sig. bits to sqrt(x)

                                2
                            (x-y )*y
                y := y + 2* ----------  ...within 1 ulp
                               2
                             3y  + x

        This formula has one division fewer than the one above; however,
        it requires more multiplications and additions. Also x must be
        scaled in advance to avoid spurious overflow in evaluating the
        expression 3y*y+x. Hence it is not recommended uless division
        is slow. If division is very slow, then one should use the
        reciproot algorithm given in section B.

    (3) Final adjustment

        By twiddling y's last bit it is possible to force y to be
        correctly rounded according to the prevailing rounding mode
        as follows. Let r and i be copies of the rounding mode and
        inexact flag before entering the square root program. Also we
        use the expression y+-ulp for the next representable floating
        numbers (up and down) of y. Note that y+-ulp = either fixed
        point y+-1, or multiply y by nextafter(1,+-inf) in chopped
        mode.

        I := FALSE;     ... reset INEXACT flag I
        R := RZ;        ... set rounding mode to round-toward-zero
                z := x/y;       ... chopped quotient, possibly inexact
                If(not I) then {        ... if the quotient is exact
                    if(z=y) {
                        I := i;  ... restore inexact flag
                        R := r;  ... restore rounded mode
                        return sqrt(x):=y.
                    } else {
                        z := z - ulp;   ... special rounding
                    }
                }
                i := TRUE;              ... sqrt(x) is inexact
                If (r=RN) then z=z+ulp  ... rounded-to-nearest
                If (r=RP) then {        ... round-toward-+inf
                    y = y+ulp; z=z+ulp;
                }
                y := y+z;               ... chopped sum
                y0:=y0-0x00100000;      ... y := y/2 is correctly rounded.
                I := i;                 ... restore inexact flag
                R := r;                 ... restore rounded mode
                return sqrt(x):=y.

    (4) Special cases

        Square root of +inf, +-0, or NaN is itself;
        Square root of a negative number is NaN with invalid signal.

B.  sqrt(x) by Reciproot Iteration

   (1)  Initial approximation

        Let x0 and x1 be the leading and the trailing 32-bit words of
        a floating point number x (in IEEE double format) respectively
        (see section A). By performing shifs and subtracts on x0 and y0,
        we obtain a 7.8-bit approximation of 1/sqrt(x) as follows.

            k := 0x5fe80000 - (x0>>1);
            y0:= k - T2[63&(k>>14)].    ... y ~ 1/sqrt(x) to 7.8 bits

        Here k is a 32-bit integer and T2[] is an integer array
        containing correction terms. Now magically the floating
        value of y (y's leading 32-bit word is y0, the value of
        its trailing word y1 is set to zero) approximates 1/sqrt(x)
        to almost 7.8-bit.

        Value of T2:
        static int T2[64]= {
        0x1500, 0x2ef8, 0x4d67, 0x6b02, 0x87be, 0xa395, 0xbe7a, 0xd866,
        0xf14a, 0x1091b,0x11fcd,0x13552,0x14999,0x15c98,0x16e34,0x17e5f,
        0x18d03,0x19a01,0x1a545,0x1ae8a,0x1b5c4,0x1bb01,0x1bfde,0x1c28d,
        0x1c2de,0x1c0db,0x1ba73,0x1b11c,0x1a4b5,0x1953d,0x18266,0x16be0,
        0x1683e,0x179d8,0x18a4d,0x19992,0x1a789,0x1b445,0x1bf61,0x1c989,
        0x1d16d,0x1d77b,0x1dddf,0x1e2ad,0x1e5bf,0x1e6e8,0x1e654,0x1e3cd,
        0x1df2a,0x1d635,0x1cb16,0x1be2c,0x1ae4e,0x19bde,0x1868e,0x16e2e,
        0x1527f,0x1334a,0x11051,0xe951, 0xbe01, 0x8e0d, 0x5924, 0x1edd,};

    (2) Iterative refinement

        Apply Reciproot iteration three times to y and multiply the
        result by x to get an approximation z that matches sqrt(x)
        to about 1 ulp. To be exact, we will have
                -1ulp < sqrt(x)-z<1.0625ulp.

        ... set rounding mode to Round-to-nearest
           y := y*(1.5-0.5*x*y*y)       ... almost 15 sig. bits to 1/sqrt(x)
           y := y*((1.5-2^-30)+0.5*x*y*y)... about 29 sig. bits to 1/sqrt(x)
        ... special arrangement for better accuracy
           z := x*y                     ... 29 bits to sqrt(x), with z*y<1
           z := z + 0.5*z*(1-z*y)       ... about 1 ulp to sqrt(x)

        Remark 2. The constant 1.5-2^-30 is chosen to bias the error so that
        (a) the term z*y in the final iteration is always less than 1;
        (b) the error in the final result is biased upward so that
                -1 ulp < sqrt(x) - z < 1.0625 ulp
            instead of |sqrt(x)-z|<1.03125ulp.

    (3) Final adjustment

        By twiddling y's last bit it is possible to force y to be
        correctly rounded according to the prevailing rounding mode
        as follows. Let r and i be copies of the rounding mode and
        inexact flag before entering the square root program. Also we
        use the expression y+-ulp for the next representable floating
        numbers (up and down) of y. Note that y+-ulp = either fixed
        point y+-1, or multiply y by nextafter(1,+-inf) in chopped
        mode.

        R := RZ;                ... set rounding mode to round-toward-zero
        switch(r) {
            case RN:            ... round-to-nearest
               if(x<= z*(z-ulp)...chopped) z = z - ulp; else
               if(x<= z*(z+ulp)...chopped) z = z; else z = z+ulp;
               break;
            case RZ:case RM:    ... round-to-zero or round-to--inf
               R:=RP;           ... reset rounding mod to round-to-+inf
               if(x<z*z ... rounded up) z = z - ulp; else
               if(x>=(z+ulp)*(z+ulp) ...rounded up) z = z+ulp;
               break;
            case RP:            ... round-to-+inf
               if(x>(z+ulp)*(z+ulp)...chopped) z = z+2*ulp; else
               if(x>z*z ...chopped) z = z+ulp;
               break;
        }

        Remark 3. The above comparisons can be done in fixed point. For
        example, to compare x and w=z*z chopped, it suffices to compare
        x1 and w1 (the trailing parts of x and w), regarding them as
        two's complement integers.

        ...Is z an exact square root?
        To determine whether z is an exact square root of x, let z1 be the
        trailing part of z, and also let x0 and x1 be the leading and
        trailing parts of x.

        If ((z1&0x03ffffff)!=0) ... not exact if trailing 26 bits of z!=0
            I := 1;             ... Raise Inexact flag: z is not exact
        else {
            j := 1 - [(x0>>20)&1]       ... j = logb(x) mod 2
            k := z1 >> 26;              ... get z's 25-th and 26-th
                                            fraction bits
            I := i or (k&j) or ((k&(j+j+1))!=(x1&3));
        }
        R:= r           ... restore rounded mode
        return sqrt(x):=z.

        If multiplication is cheaper then the foregoing red tape, the
        Inexact flag can be evaluated by

            I := i;
            I := (z*z!=x) or I.

        Note that z*z can overwrite I; this value must be sensed if it is
        True.

        Remark 4. If z*z = x exactly, then bit 25 to bit 0 of z1 must be
        zero.

                    --------------------
                z1: |        f2        |
                    --------------------
                bit 31             bit 0

        Further more, bit 27 and 26 of z1, bit 0 and 1 of x1, and the odd
        or even of logb(x) have the following relations:

        -------------------------------------------------
        bit 27,26 of z1         bit 1,0 of x1   logb(x)
        -------------------------------------------------
        00                      00              odd and even
        01                      01              even
        10                      10              odd
        10                      00              even
        11                      01              even
        -------------------------------------------------

    (4) Special cases (see (4) of Section A).
 */


/* __sinh(x)
 * Method:
 * mathematically sinh(x) if defined to be (exp(x) - exp(-x)) / 2
 *  1. Replace x by |x| (sinh(-x) = -sinh(x)).
 *  2.
 *                                             E + E/(E+1)
 *      0        <= x <= 22     :  sinh(x) := -------------, E = expm1(x)
 *                                                  2
 *
 *      22       <= x <= lnovft :  sinh(x) := exp(x) / 2
 *      lnovft   <= x <= ln2ovft:  sinh(x) := exp(x / 2) / 2 * exp(x / 2)
 *      ln2ovft  <  x           :  sinh(x) := x * shuge (overflow)
 *
 * Special cases:
 *  sinh(x) is |x| if x is +INF, -INF, or NaN.
 *  only sinh(0) = 0 is exact for finite x.
 */

#define one   1.0
#define half  0.5
#define shuge 1.0e307

double
sinh (double x)
{
  double t, w, h;
  int ix, jx;
  unsigned lx;

  /* High word of |x|. */
  jx = __HI (x);
  ix = jx & 0x7fffffff;

  /* x is INF or NaN */
  if (ix >= 0x7ff00000)
  {
    return x + x;
  }

  h = 0.5;
  if (jx < 0)
  {
    h = -h;
  }
  /* |x| in [0,22], return sign(x) * 0.5 * (E + E / (E + 1))) */
  if (ix < 0x40360000)
  {
    /* |x| < 22 */
    if (ix < 0x3e300000)
    {
      /* |x| < 2**-28 */
      if (shuge + x > one)
      {
        /* sinh(tiny) = tiny with inexact */
        return x;
      }
    }
    t = expm1 (fabs (x));
    if (ix < 0x3ff00000)
    {
      return h * (2.0 * t - t * t / (t + one));
    }
    return h * (t + t / (t + one));
  }

  /* |x| in [22, log(maxdouble)] return 0.5*exp(|x|) */
  if (ix < 0x40862E42)
  {
    return h * exp (fabs (x));
  }
  /* |x| in [log(maxdouble), overflowthresold] */
  lx = ((1 >> 29) + (unsigned int) x);
  if (ix < 0x408633CE || ((ix == 0x408633ce) && (lx <= (unsigned) 0x8fb9f87d)))
  {
    w = exp (0.5 * fabs (x));
    t = h * w;
    return t * w;
  }

  /* |x| > overflowthresold, sinh(x) overflow */
  return x * shuge;
} /* sinh */

#undef one
#undef half
#undef huge


/* scalbn(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

#define two54  1.80143985094819840000e+16 /* 0x43500000, 0x00000000 */
#define twom54 5.55111512312578270212e-17 /* 0x3C900000, 0x00000000 */
#define huge   1.0e+300
#define tiny   1.0e-300

double
scalbn (double x, int n)
{
  int k, hx, lx;

  hx = __HI (x);
  lx = __LO (x);
  k = (hx & 0x7ff00000) >> 20; /* extract exponent */
  if (k == 0) /* 0 or subnormal x */
  {
    if ((lx | (hx & 0x7fffffff)) == 0) /* +-0 */
    {
      return x;
    }
    x *= two54;
    hx = __HI (x);
    k = ((hx & 0x7ff00000) >> 20) - 54;
    if (n < -50000) /*underflow */
    {
      return tiny * x;
    }
  }
  if (k == 0x7ff) /* NaN or Inf */
  {
    return x + x;
  }
  k = k + n;
  if (k > 0x7fe) /* overflow  */
  {
    return huge * copysign (huge, x);
  }
  if (k > 0) /* normal result */
  {
    double_accessor ret;
    ret.dbl = x;
    ret.as_int.hi = (hx & 0x800fffff) | (k << 20);
    return ret.dbl;
  }
  if (k <= -54)
  {
    if (n > 50000) /* in case integer overflow in n + k */
    {
      return huge * copysign (huge, x); /*overflow */
    }
    else
    {
      return tiny * copysign (tiny, x); /*underflow */
    }
  }
  k += 54; /* subnormal result */
  double_accessor ret;
  ret.dbl = x;
  ret.as_int.hi = (hx & 0x800fffff) | (k << 20);
  return ret.dbl * twom54;
} /* scalbn */

#undef two54
#undef twom54
#undef huge
#undef tiny


/* pow(x,y) return x**y
 *
 *                    n
 * Method:  Let x =  2   * (1+f)
 *      1. Compute and return log2(x) in two pieces:
 *              log2(x) = w1 + w2,
 *         where w1 has 53-24 = 29 bit trailing zeros.
 *      2. Perform y*log2(x) = n+y' by simulating muti-precision
 *         arithmetic, where |y'|<=0.5.
 *      3. Return x**y = 2**n*exp(y'*log2)
 *
 * Special cases:
 *      0.  +1 ** (anything) is 1
 *      1.  (anything) ** 0  is 1
 *      2.  (anything) ** 1  is itself
 *      3.  (anything) ** NAN is NAN
 *      4.  NAN ** (anything except 0) is NAN
 *      5.  +-(|x| > 1) **  +INF is +INF
 *      6.  +-(|x| > 1) **  -INF is +0
 *      7.  +-(|x| < 1) **  +INF is +0
 *      8.  +-(|x| < 1) **  -INF is +INF
 *      9.  -1          ** +-INF is 1
 *      10. +0 ** (+anything except 0, NAN)               is +0
 *      11. -0 ** (+anything except 0, NAN, odd integer)  is +0
 *      12. +0 ** (-anything except 0, NAN)               is +INF
 *      13. -0 ** (-anything except 0, NAN, odd integer)  is +INF
 *      14. -0 ** (odd integer) = -( +0 ** (odd integer) )
 *      15. +INF ** (+anything except 0,NAN) is +INF
 *      16. +INF ** (-anything except 0,NAN) is +0
 *      17. -INF ** (anything)  = -0 ** (-anything)
 *      18. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
 *      19. (-anything except 0 and inf) ** (non-integer) is NAN
 *
 * Accuracy:
 *      pow(x,y) returns x**y nearly rounded. In particular
 *                      pow(integer,integer)
 *      always returns the correct integer provided it is
 *      representable.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double bp[] = {
  1.0,
  1.5,
};
static const double dp_h[] = {
  0.0,
  5.84962487220764160156e-01, /* 0x3FE2B803, 0x40000000 */
};
static const double dp_l[] = {
  0.0,
  1.35003920212974897128e-08, /* 0x3E4CFDEB, 0x43CFD006 */
};

#define zero  0.0
#define one   1.0
#define two   2.0
#define two53 9007199254740992.0 /* 0x43400000, 0x00000000 */
#define huge  1.0e300
#define tiny  1.0e-300
/* poly coefs for (3/2) * (log(x) - 2s - 2/3 * s**3 */
#define L1      5.99999999999994648725e-01 /* 0x3FE33333, 0x33333303 */
#define L2      4.28571428578550184252e-01 /* 0x3FDB6DB6, 0xDB6FABFF */
#define L3      3.33333329818377432918e-01 /* 0x3FD55555, 0x518F264D */
#define L4      2.72728123808534006489e-01 /* 0x3FD17460, 0xA91D4101 */
#define L5      2.30660745775561754067e-01 /* 0x3FCD864A, 0x93C9DB65 */
#define L6      2.06975017800338417784e-01 /* 0x3FCA7E28, 0x4A454EEF */
#define P1      1.66666666666666019037e-01 /* 0x3FC55555, 0x5555553E */
#define P2      -2.77777777770155933842e-03 /* 0xBF66C16C, 0x16BEBD93 */
#define P3      6.61375632143793436117e-05 /* 0x3F11566A, 0xAF25DE2C */
#define P4      -1.65339022054652515390e-06 /* 0xBEBBBD41, 0xC5D26BF1 */
#define P5      4.13813679705723846039e-08 /* 0x3E663769, 0x72BEA4D0 */
#define lg2     6.93147180559945286227e-01 /* 0x3FE62E42, 0xFEFA39EF */
#define lg2_h   6.93147182464599609375e-01 /* 0x3FE62E43, 0x00000000 */
#define lg2_l   -1.90465429995776804525e-09 /* 0xBE205C61, 0x0CA86C39 */
#define ovt     8.0085662595372944372e-0017 /* -(1024-log2(ovfl+.5ulp)) */
#define cp      9.61796693925975554329e-01 /* 0x3FEEC709, 0xDC3A03FD = 2 / (3 ln2) */
#define cp_h    9.61796700954437255859e-01 /* 0x3FEEC709, 0xE0000000 = (float) cp */
#define cp_l    -7.02846165095275826516e-09 /* 0xBE3E2FE0, 0x145B01F5 = tail of cp_h */
#define ivln2   1.44269504088896338700e+00 /* 0x3FF71547, 0x652B82FE = 1 / ln2 */
#define ivln2_h 1.44269502162933349609e+00 /* 0x3FF71547, 0x60000000 = 24b 1 / ln2 */
#define ivln2_l 1.92596299112661746887e-08 /* 0x3E54AE0B, 0xF85DDF44 = 1 / ln2 tail */

double
pow (double x, double y)
{
  double_accessor t1, ax, p_h, y1, t, z;
  double z_h, z_l, p_l;
  double t2, r, s, u, v, w;
  int i, j, k, yisint, n;
  int hx, hy, ix, iy;
  unsigned lx, ly;

  hx = __HI (x);
  lx = __LO (x);
  hy = __HI (y);
  ly = __LO (y);
  ix = hx & 0x7fffffff;
  iy = hy & 0x7fffffff;

  /* x == one: 1**y = 1 */
  if (((hx - 0x3ff00000) | lx) == 0)
  {
    return one;
  }

  /* y == zero: x**0 = 1 */
  if ((iy | ly) == 0)
  {
    return one;
  }

  /* +-NaN return x + y */
  if (ix > 0x7ff00000 || ((ix == 0x7ff00000) && (lx != 0)) || iy > 0x7ff00000 || ((iy == 0x7ff00000) && (ly != 0)))
  {
    return x + y;
  }

  /* determine if y is an odd int when x < 0
   * yisint = 0 ... y is not an integer
   * yisint = 1 ... y is an odd int
   * yisint = 2 ... y is an even int
   */
  yisint = 0;
  if (hx < 0)
  {
    if (iy >= 0x43400000) /* even integer y */
    {
      yisint = 2;
    }
    else if (iy >= 0x3ff00000)
    {
      k = (iy >> 20) - 0x3ff; /* exponent */
      if (k > 20)
      {
        j = ly >> (52 - k);
        if ((j << (52 - k)) == ly)
        {
          yisint = 2 - (j & 1);
        }
      }
      else if (ly == 0)
      {
        j = iy >> (20 - k);
        if ((j << (20 - k)) == iy)
        {
          yisint = 2 - (j & 1);
        }
      }
    }
  }

  /* special value of y */
  if (ly == 0)
  {
    if (iy == 0x7ff00000) /* y is +-inf */
    {
      if (((ix - 0x3ff00000) | lx) == 0) /* +-1**+-inf is 1 */
      {
        return one;
      }
      else if (ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
      {
        return (hy >= 0) ? y : zero;
      }
      else /* (|x|<1)**-,+inf = inf,0 */
      {
        return (hy < 0) ? -y : zero;
      }
    }
    if (iy == 0x3ff00000) /* y is +-1 */
    {
      if (hy < 0)
      {
        return one / x;
      }
      else
      {
        return x;
      }
    }
    if (hy == 0x40000000) /* y is 2 */
    {
      return x * x;
    }
    if (hy == 0x3fe00000) /* y is 0.5 */
    {
      if (hx >= 0) /* x >= +0 */
      {
        return sqrt (x);
      }
    }
  }

  ax.dbl = fabs (x);
  /* special value of x */
  if (lx == 0)
  {
    if (ix == 0x7ff00000 || ix == 0 || ix == 0x3ff00000)
    {
      z.dbl = ax.dbl; /* x is +-0,+-inf,+-1 */
      if (hy < 0)
      {
        z.dbl = one / z.dbl; /* z = (1 / |x|) */
      }
      if (hx < 0)
      {
        if (((ix - 0x3ff00000) | yisint) == 0)
        {
          z.dbl = NAN; /* (-1)**non-int is NaN */
        }
        else if (yisint == 1)
        {
          z.dbl = -z.dbl; /* (x<0)**odd = -(|x|**odd) */
        }
      }
      return z.dbl;
    }
  }

  n = (hx < 0) ? 0 : 1;

  /* (x<0)**(non-int) is NaN */
  if ((n | yisint) == 0)
  {
    return NAN;
  }

  s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
  if ((n | (yisint - 1)) == 0)
  {
    s = -one; /* (-ve)**(odd int) */
  }

  /* |y| is huge */
  if (iy > 0x41e00000) /* if |y| > 2**31 */
  {
    if (iy > 0x43f00000) /* if |y| > 2**64, must o/uflow */
    {
      if (ix <= 0x3fefffff)
      {
        return (hy < 0) ? huge * huge : tiny * tiny;
      }
      if (ix >= 0x3ff00000)
      {
        return (hy > 0) ? huge * huge : tiny * tiny;
      }
    }
    /* over/underflow if x is not close to one */
    if (ix < 0x3fefffff)
    {
      return (hy < 0) ? s * huge * huge : s * tiny * tiny;
    }
    if (ix > 0x3ff00000)
    {
      return (hy > 0) ? s * huge * huge : s * tiny * tiny;
    }
    /* now |1 - x| is tiny <= 2**-20, suffice to compute
       log(x) by x - x^2 / 2 + x^3 / 3 - x^4 / 4 */
    t.dbl = ax.dbl - one; /* t has 20 trailing zeros */
    w = (t.dbl * t.dbl) * (0.5 - t.dbl * (0.3333333333333333333333 - t.dbl * 0.25));
    u = ivln2_h * t.dbl; /* ivln2_h has 21 sig. bits */
    v = t.dbl * ivln2_l - w * ivln2;
    t1.dbl = u + v;
    t1.as_int.lo = 0;
    t2 = v - (t1.dbl - u);
  }
  else
  {
    double_accessor s_h, t_h;
    double ss, s2, s_l, t_l;

    n = 0;
    /* take care subnormal number */
    if (ix < 0x00100000)
    {
      ax.dbl *= two53;
      n -= 53;
      ix = ax.as_int.hi;
    }
    n += ((ix) >> 20) - 0x3ff;
    j = ix & 0x000fffff;
    /* determine interval */
    ix = j | 0x3ff00000; /* normalize ix */
    if (j <= 0x3988E) /* |x| < sqrt(3/2) */
    {
      k = 0;
    }
    else if (j < 0xBB67A) /* |x| < sqrt(3) */
    {
      k = 1;
    }
    else
    {
      k = 0;
      n += 1;
      ix -= 0x00100000;
    }
    ax.as_int.hi = ix;

    /* compute ss = s_h + s_l = (x - 1) / (x + 1) or (x - 1.5) / (x + 1.5) */
    u = ax.dbl - bp[k]; /* bp[0] = 1.0, bp[1] = 1.5 */
    v = one / (ax.dbl + bp[k]);
    ss = u * v;
    s_h.dbl = ss;
    s_h.as_int.lo = 0;
    /* t_h = ax + bp[k] High */
    t_h.dbl = zero;
    t_h.as_int.hi = ((ix >> 1) | 0x20000000) + 0x00080000 + (k << 18);
    t_l = ax.dbl - (t_h.dbl - bp[k]);
    s_l = v * ((u - s_h.dbl * t_h.dbl) - s_h.dbl * t_l);
    /* compute log(ax) */
    s2 = ss * ss;
    r = s2 * s2 * (L1 + s2 * (L2 + s2 * (L3 + s2 * (L4 + s2 * (L5 + s2 * L6)))));
    r += s_l * (s_h.dbl + ss);
    s2 = s_h.dbl * s_h.dbl;
    t_h.dbl = 3.0 + s2 + r;
    t_h.as_int.lo = 0;
    t_l = r - ((t_h.dbl - 3.0) - s2);
    /* u + v = ss * (1 + ...) */
    u = s_h.dbl * t_h.dbl;
    v = s_l * t_h.dbl + t_l * ss;
    /* 2 / (3 * log2) * (ss + ...) */
    p_h.dbl = u + v;
    p_h.as_int.lo = 0;
    p_l = v - (p_h.dbl - u);
    z_h = cp_h * p_h.dbl; /* cp_h + cp_l = 2 / (3 * log2) */
    z_l = cp_l * p_h.dbl + p_l * cp + dp_l[k];
    /* log2(ax) = (ss + ...) * 2 / (3 * log2) = n + dp_h + z_h + z_l */
    t.dbl = (double) n;
    t1.dbl = (((z_h + z_l) + dp_h[k]) + t.dbl);
    t1.as_int.lo = 0;
    t2 = z_l - (((t1.dbl - t.dbl) - dp_h[k]) - z_h);
  }

  /* split up y into y1 + y2 and compute (y1 + y2) * (t1 + t2) */
  y1.dbl = y;
  y1.as_int.lo = 0;
  p_l = (y - y1.dbl) * t1.dbl + y * t2;
  p_h.dbl = y1.dbl * t1.dbl;
  z.dbl = p_l + p_h.dbl;
  j = z.as_int.hi;
  i = z.as_int.lo;
  if (j >= 0x40900000) /* z >= 1024 */
  {
    if (((j - 0x40900000) | i) != 0) /* if z > 1024 */
    {
      return s * huge * huge; /* overflow */
    }
    else
    {
      if (p_l + ovt > z.dbl - p_h.dbl)
      {
        return s * huge * huge; /* overflow */
      }
    }
  }
  else if ((j & 0x7fffffff) >= 0x4090cc00) /* z <= -1075 */
  {
    if (((j - 0xc090cc00) | i) != 0) /* z < -1075 */
    {
      return s * tiny * tiny; /* underflow */
    }
    else
    {
      if (p_l <= z.dbl - p_h.dbl)
      {
        return s * tiny * tiny; /* underflow */
      }
    }
  }
  /*
   * compute 2**(p_h + p_l)
   */
  i = j & 0x7fffffff;
  k = (i >> 20) - 0x3ff;
  n = 0;
  if (i > 0x3fe00000) /* if |z| > 0.5, set n = [z + 0.5] */
  {
    n = j + (0x00100000 >> (k + 1));
    k = ((n & 0x7fffffff) >> 20) - 0x3ff; /* new k for n */
    t.dbl = zero;
    t.as_int.hi = (n & ~(0x000fffff >> k));
    n = ((n & 0x000fffff) | 0x00100000) >> (20 - k);
    if (j < 0)
    {
      n = -n;
    }
    p_h.dbl -= t.dbl;
  }
  t.dbl = p_l + p_h.dbl;
  t.as_int.lo = 0;
  u = t.dbl * lg2_h;
  v = (p_l - (t.dbl - p_h.dbl)) * lg2 + t.dbl * lg2_l;
  z.dbl = u + v;
  w = v - (z.dbl - u);
  t.dbl = z.dbl * z.dbl;
  t1.dbl = z.dbl - t.dbl * (P1 + t.dbl * (P2 + t.dbl * (P3 + t.dbl * (P4 + t.dbl * P5))));
  r = (z.dbl * t1.dbl) / (t1.dbl - two) - (w + z.dbl * w);
  z.dbl = one - (r - z.dbl);
  j = z.as_int.hi;
  j += (n << 20);
  if ((j >> 20) <= 0) /* subnormal output */
  {
    z.dbl = scalbn (z.dbl, n);
  }
  else
  {
    z.as_int.hi += (n << 20);
  }
  return s * z.dbl;
} /* pow */

#undef zero
#undef one
#undef two
#undef two53
#undef huge
#undef tiny
#undef L1
#undef L2
#undef L3
#undef L4
#undef L5
#undef L6
#undef P1
#undef P2
#undef P3
#undef P4
#undef P5
#undef lg2
#undef lg2_h
#undef lg2_l
#undef ovt
#undef cp
#undef cp_h
#undef cp_l
#undef ivln2
#undef ivln2_h
#undef ivln2_l


double
nextafter (double x, double y)
{
  int hx, hy, ix, iy;
  unsigned lx, ly;
  double_accessor ret;

  hx = __HI (x); /* high word of x */
  lx = __LO (x); /* low  word of x */
  hy = __HI (y); /* high word of y */
  ly = __LO (y); /* low  word of y */
  ix = hx & 0x7fffffff; /* |x| */
  iy = hy & 0x7fffffff; /* |y| */

  if (((ix >= 0x7ff00000) && ((ix - 0x7ff00000) | lx) != 0) /* x is nan */
      || ((iy >= 0x7ff00000) && ((iy - 0x7ff00000) | ly) != 0)) /* y is nan */
  {
    return x + y;
  }

  if (x == y)
  {
    return x; /* x=y, return x */
  }

  if ((ix | lx) == 0)
  { /* x == 0 */
    ret.as_int.hi = hy & 0x80000000; /* return +-minsubnormal */
    ret.as_int.lo = 1;
    y = ret.dbl * ret.dbl;
    if (y == ret.dbl)
    {
      return y;
    }
    else
    {
      return ret.dbl; /* raise underflow flag */
    }
  }

  if (hx >= 0)
  { /* x > 0 */
    if (hx > hy || ((hx == hy) && (lx > ly)))
    { /* x > y, x -= ulp */
      if (lx == 0)
      {
        hx -= 1;
      }

      lx -= 1;
    }
    else
    { /* x < y, x += ulp */
      lx += 1;

      if (lx == 0)
      {
        hx += 1;
      }
    }
  }
  else
  { /* x < 0 */
    if (hy >= 0 || hx > hy || ((hx == hy) && (lx > ly)))
    { /* x < y, x -= ulp */
      if (lx == 0)
      {
        hx -= 1;
      }

      lx -= 1;
    }
    else
    { /* x > y, x += ulp */
      lx += 1;

      if (lx == 0)
      {
        hx += 1;
      }
    }
  }

  hy = hx & 0x7ff00000;

  if (hy >= 0x7ff00000)
  {
    return x + x; /* overflow */
  }

  if (hy < 0x00100000)
  { /* underflow */
    y = x * x;
    if (y != x)
    { /* raise underflow flag */
      ret.as_int.hi = hx;
      ret.as_int.lo = lx;
      return ret.dbl;
    }
  }

  ret.as_int.hi = hx;
  ret.as_int.lo = lx;
  return ret.dbl;
} /* nextafter */


/* log2(x)
 * Return the base 2 logarithm of x.  See e_log.c and k_log.h for most
 * comments.
 *
 * This reduces x to {k, 1+f} exactly as in e_log.c, then calls the kernel,
 * then does the combining and scaling steps
 *    log2(x) = (f - 0.5*f*f + k_log1p(f)) / ln2 + k
 * in not-quite-routine extra precision.
 */

#define zero    0.0
#define two54   1.80143985094819840000e+16 /* 0x43500000, 0x00000000 */
#define ivln2hi 1.44269504072144627571e+00 /* 0x3FF71547, 0x65200000 */
#define ivln2lo 1.67517131648865118353e-10 /* 0x3DE705FC, 0x2EEFA200 */
#define Lg1     6.666666666666735130e-01 /* 0x3FE55555, 0x55555593 */
#define Lg2     3.999999999940941908e-01 /* 0x3FD99999, 0x9997FA04 */
#define Lg3     2.857142874366239149e-01 /* 0x3FD24924, 0x94229359 */
#define Lg4     2.222219843214978396e-01 /* 0x3FCC71C5, 0x1D8E78AF */
#define Lg5     1.818357216161805012e-01 /* 0x3FC74664, 0x96CB03DE */
#define Lg6     1.531383769920937332e-01 /* 0x3FC39A09, 0xD078C69F */
#define Lg7     1.479819860511658591e-01 /* 0x3FC2F112, 0xDF3E5244 */

double
log2 (double x)
{
  double f, hfsq, hi, lo, r, val_hi, val_lo, w, y;
  int i, k, hx;
  unsigned int lx;
  double_accessor temp;

  hx = __HI (x); /* high word of x */
  lx = __LO (x); /* low word of x */

  k = 0;
  if (hx < 0x00100000)
  { /* x < 2**-1022  */
    if (((hx & 0x7fffffff) | lx) == 0)
    {
      return -INFINITY; /* log(+-0)=-inf */
    }
    if (hx < 0)
    {
      return NAN; /* log(-#) = NaN */
    }
    k -= 54;
    x *= two54; /* subnormal number, scale up x */
    hx = __HI (x); /* high word of x */
  }
  if (hx >= 0x7ff00000)
  {
    return x + x;
  }
  if (hx == 0x3ff00000 && lx == 0)
  {
    return zero; /* log(1) = +0 */
  }
  k += (hx >> 20) - 1023;
  hx &= 0x000fffff;
  i = (hx + 0x95f64) & 0x100000;
  temp.dbl = x;
  temp.as_int.hi = hx | (i ^ 0x3ff00000); /* normalize x or x/2 */
  k += (i >> 20);
  y = (double) k;
  f = temp.dbl - 1.0;
  hfsq = 0.5 * f * f;
  double s, z, R, t1, t2;

  s = f / (2.0 + f);
  z = s * s;
  w = z * z;
  t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
  t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
  R = t2 + t1;
  r = s * (hfsq + R);
  /*
   * f-hfsq must (for args near 1) be evaluated in extra precision
   * to avoid a large cancellation when x is near sqrt(2) or 1/sqrt(2).
   * This is fairly efficient since f-hfsq only depends on f, so can
   * be evaluated in parallel with R.  Not combining hfsq with R also
   * keeps R small (though not as small as a true `lo' term would be),
   * so that extra precision is not needed for terms involving R.
   *
   * Compiler bugs involving extra precision used to break Dekker's
   * theorem for spitting f-hfsq as hi+lo, unless double_t was used
   * or the multi-precision calculations were avoided when double_t
   * has extra precision.  These problems are now automatically
   * avoided as a side effect of the optimization of combining the
   * Dekker splitting step with the clear-low-bits step.
   *
   * y must (for args near sqrt(2) and 1/sqrt(2)) be added in extra
   * precision to avoid a very large cancellation when x is very near
   * these values.  Unlike the above cancellations, this problem is
   * specific to base 2.  It is strange that adding +-1 is so much
   * harder than adding +-ln2 or +-log10_2.
   *
   * This uses Dekker's theorem to normalize y+val_hi, so the
   * compiler bugs are back in some configurations, sigh.  And I
   * don't want to used double_t to avoid them, since that gives a
   * pessimization and the support for avoiding the pessimization
   * is not yet available.
   *
   * The multi-precision calculations for the multiplications are
   * routine.
   */
  hi = f - hfsq;
  temp.dbl = hi;
  temp.as_int.lo = 0;

  lo = (f - hi) - hfsq + r;
  val_hi = hi * ivln2hi;
  val_lo = (lo + hi) * ivln2lo + lo * ivln2hi;

  /* spadd(val_hi, val_lo, y), except for not using double_t: */
  w = y + val_hi;
  val_lo += (y - w) + val_hi;
  val_hi = w;

  return val_lo + val_hi;
} /* log2 */

#undef zero
#undef two54
#undef ivln2hi
#undef ivln2lo
#undef Lg1
#undef Lg2
#undef Lg3
#undef Lg4
#undef Lg5
#undef Lg6
#undef Lg7


/* log1p(x)
 * Method :
 *   1. Argument Reduction: find k and f such that
 *      1+x = 2^k * (1+f),
 *     where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *      Note. If k=0, then f=x is exact. However, if k!=0, then f
 *  may not be representable exactly. In that case, a correction
 *  term is need. Let u=1+x rounded. Let c = (1+x)-u, then
 *  log(1+x) - log(u) ~ c/u. Thus, we proceed to compute log(u),
 *  and add back the correction term c/u.
 *  (Note: when x > 2**53, one can simply return log(x))
 *
 *   2. Approximation of log1p(f).
 *  Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *     = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *          = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 *   a polynomial of degree 14 to approximate R The maximum error
 *  of this polynomial approximation is bounded by 2**-58.45. In
 *  other words,
 *            2      4      6      8      10      12      14
 *      R(z) ~ Lp1*s +Lp2*s +Lp3*s +Lp4*s +Lp5*s  +Lp6*s  +Lp7*s
 *    (the values of Lp1 to Lp7 are listed in the program)
 *  and
 *      |      2          14          |     -58.45
 *      | Lp1*s +...+Lp7*s    -  R(z) | <= 2
 *      |                             |
 *  Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *  In order to guarantee error in log below 1ulp, we compute log
 *  by
 *    log1p(f) = f - (hfsq - s*(hfsq+R)).
 *
 *  3. Finally, log1p(x) = k*ln2 + log1p(f).
 *            = k*ln2_hi+(f-(hfsq-(s*(hfsq+R)+k*ln2_lo)))
 *     Here ln2 is split into two floating point number:
 *      ln2_hi + ln2_lo,
 *     where n*ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *  log1p(x) is NaN with signal if x < -1 (including -INF) ;
 *  log1p(+INF) is +INF; log1p(-1) is -INF with signal;
 *  log1p(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *  according to an error analysis, the error is always less than
 *  1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 *
 * Note: Assuming log() return accurate answer, the following
 *    algorithm can be used to compute log1p(x) to within a few ULP:
 *
 *    u = 1+x;
 *    if(u==1.0) return x ; else
 *         return log(u)*(x/(u-1.0));
 *
 *   See HP-15C Advanced Functions Handbook, p.193.
 */

#define zero   0.0
#define ln2_hi 6.93147180369123816490e-01 /* 3fe62e42 fee00000 */
#define ln2_lo 1.90821492927058770002e-10 /* 3dea39ef 35793c76 */
#define two54  1.80143985094819840000e+16 /* 43500000 00000000 */
#define Lp1    6.666666666666735130e-01 /* 3FE55555 55555593 */
#define Lp2    3.999999999940941908e-01 /* 3FD99999 9997FA04 */
#define Lp3    2.857142874366239149e-01 /* 3FD24924 94229359 */
#define Lp4    2.222219843214978396e-01 /* 3FCC71C5 1D8E78AF */
#define Lp5    1.818357216161805012e-01 /* 3FC74664 96CB03DE */
#define Lp6    1.531383769920937332e-01 /* 3FC39A09 D078C69F */
#define Lp7    1.479819860511658591e-01 /* 3FC2F112 DF3E5244 */

double
log1p (double x)
{
  double hfsq, f, c, s, z, R;
  double_accessor u;
  int k, hx, hu, ax;

  hx = __HI (x);
  ax = hx & 0x7fffffff;
  c = 0;
  k = 1;
  if (hx < 0x3FDA827A)
  {
    /* 1+x < sqrt(2)+ */
    if (ax >= 0x3ff00000)
    {
      /* x <= -1.0 */
      if (x == -1.0)
      {
        /* log1p(-1) = -inf */
        return -INFINITY;
      }
      else
      {
        /* log1p(x<-1) = NaN */
        return NAN;
      }
    }
    if (ax < 0x3e200000)
    { /* |x| < 2**-29 */
      if ((two54 + x > zero) /* raise inexact */
          && (ax < 0x3c900000)) /* |x| < 2**-54 */
      {
        return x;
      }
      else
      {
        return x - x * x * 0.5;
      }
    }
    if ((hx > 0) || hx <= ((int) 0xbfd2bec4))
    {
      /* sqrt(2)/2- <= 1+x < sqrt(2)+ */
      k = 0;
      f = x;
      hu = 1;
    }
  }
  if (hx >= 0x7ff00000)
  {
    return x + x;
  }
  if (k != 0)
  {
    if (hx < 0x43400000)
    {
      u.dbl = 1.0 + x;
      hu = u.as_int.hi;
      k = (hu >> 20) - 1023;
      c = (k > 0) ? 1.0 - (u.dbl - x) : x - (u.dbl - 1.0); /* correction term */
      c /= u.dbl;
    }
    else
    {
      u.dbl = x;
      hu = u.as_int.hi;
      k = (hu >> 20) - 1023;
      c = 0;
    }
    hu &= 0x000fffff;
    /*
     * The approximation to sqrt(2) used in thresholds is not
     * critical.  However, the ones used above must give less
     * strict bounds than the one here so that the k==0 case is
     * never reached from here, since here we have committed to
     * using the correction term but don't use it if k==0.
     */
    if (hu < 0x6a09e)
    {
      /* u ~< sqrt(2) */
      u.as_int.hi = hu | 0x3ff00000; /* normalize u */
    }
    else
    {
      k += 1;
      u.as_int.hi = hu | 0x3fe00000; /* normalize u/2 */
      hu = (0x00100000 - hu) >> 2;
    }
    f = u.dbl - 1.0;
  }
  hfsq = 0.5 * f * f;
  if (hu == 0)
  {
    /* |f| < 2**-20 */
    if (f == zero)
    {
      if (k == 0)
      {
        return zero;
      }
      else
      {
        c += k * ln2_lo;
        return k * ln2_hi + c;
      }
    }
    R = hfsq * (1.0 - 0.66666666666666666 * f);
    if (k == 0)
    {
      return f - R;
    }
    else
    {
      return k * ln2_hi - ((R - (k * ln2_lo + c)) - f);
    }
  }
  s = f / (2.0 + f);
  z = s * s;
  R = z * (Lp1 + z * (Lp2 + z * (Lp3 + z * (Lp4 + z * (Lp5 + z * (Lp6 + z * Lp7))))));
  if (k == 0)
  {
    return f - (hfsq - s * (hfsq + R));
  }
  else
  {
    return k * ln2_hi - ((hfsq - (s * (hfsq + R) + (k * ln2_lo + c))) - f);
  }
} /* log1p */

#undef zero
#undef ln2_hi
#undef ln2_lo
#undef two54
#undef Lp1
#undef Lp2
#undef Lp3
#undef Lp4
#undef Lp5
#undef Lp6
#undef Lp7


/* log10(x)
 * Return the base 10 logarithm of x
 *
 * Method :
 *  Let log10_2hi = leading 40 bits of log10(2) and
 *      log10_2lo = log10(2) - log10_2hi,
 *      ivln10   = 1/log(10) rounded.
 *  Then
 *    n = ilogb(x),
 *    if(n<0)  n = n+1;
 *    x = scalbn(x,-n);
 *    log10(x) := n*log10_2hi + (n*log10_2lo + ivln10*log(x))
 *
 * Note 1:
 *  To guarantee log10(10**n)=n, where 10**n is normal, the rounding
 *  mode must set to Round-to-Nearest.
 * Note 2:
 *  [1/log(10)] rounded to 53 bits has error  .198   ulps;
 *  log10 is monotonic at all binary break points.
 *
 * Special cases:
 *  log10(x) is NaN with signal if x < 0;
 *  log10(+INF) is +INF with no signal; log10(0) is -INF with signal;
 *  log10(NaN) is that NaN with no signal;
 *  log10(10**N) = N  for N=0,1,...,22.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following constants.
 * The decimal values may be used, provided that the compiler will convert
 * from decimal to binary accurately enough to produce the hexadecimal values
 * shown.
 */

#define zero      0.0
#define two54     1.80143985094819840000e+16 /* 0x43500000, 0x00000000 */
#define ivln10    4.34294481903251816668e-01 /* 0x3FDBCB7B, 0x1526E50E */
#define log10_2hi 3.01029995663611771306e-01 /* 0x3FD34413, 0x509F6000 */
#define log10_2lo 3.69423907715893078616e-13 /* 0x3D59FEF3, 0x11F12B36 */

double
log10 (double x)
{
  double y, z;
  int i, k, hx;
  unsigned lx;
  double_accessor temp;

  hx = __HI (x); /* high word of x */
  lx = __LO (x); /* low word of x */

  k = 0;
  if (hx < 0x00100000)
  {
    /* x < 2**-1022  */
    if (((hx & 0x7fffffff) | lx) == 0)
    {
      /* log(+-0)=-inf */
      return -INFINITY;
    }
    if (hx < 0)
    {
      /* log(-#) = NaN */
      return NAN;
    }
    k -= 54;
    x *= two54; /* subnormal number, scale up x */
    hx = __HI (x); /* high word of x */
  }
  if (hx >= 0x7ff00000)
  {
    return x + x;
  }
  k += (hx >> 20) - 1023;
  i = ((unsigned) k & 0x80000000) >> 31;
  hx = (hx & 0x000fffff) | ((0x3ff - i) << 20);
  y = (double) (k + i);
  temp.dbl = x;
  temp.as_int.hi = hx;
  z = y * log10_2lo + ivln10 * log (temp.dbl);
  return z + y * log10_2hi;
} /* log10 */

#undef zero
#undef two54
#undef ivln10
#undef log10_2hi
#undef log10_2lo


/* log(x)
 * Return the logrithm of x
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *                      x = 2^k * (1+f),
 *         where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *      Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *               = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *               = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 *      a polynomial of degree 14 to approximate R The maximum error
 *      of this polynomial approximation is bounded by 2**-58.45. In
 *      other words,
 *                      2      4      6      8      10      12      14
 *          R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 *      (the values of Lg1 to Lg7 are listed in the program)
 *      and
 *          |      2          14          |     -58.45
 *          | Lg1*s +...+Lg7*s    -  R(z) | <= 2
 *          |                             |
 *      Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *      In order to guarantee error in log below 1ulp, we compute log
 *      by
 *              log(1+f) = f - s*(f - R)                (if f is not too large)
 *              log(1+f) = f - (hfsq - s*(hfsq+R)).     (better accuracy)
 *
 *      3. Finally,  log(x) = k*ln2 + log(1+f).
 *                          = k*ln2_hi+(f-(hfsq-(s*(hfsq+R)+k*ln2_lo)))
 *         Here ln2 is split into two floating point number:
 *                      ln2_hi + ln2_lo,
 *         where n*ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *      log(x) is NaN with signal if x < 0 (including -INF) ;
 *      log(+INF) is +INF; log(0) is -INF with signal;
 *      log(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

#define zero   0.0
#define ln2_hi 6.93147180369123816490e-01 /* 3fe62e42 fee00000 */
#define ln2_lo 1.90821492927058770002e-10 /* 3dea39ef 35793c76 */
#define two54  1.80143985094819840000e+16 /* 43500000 00000000 */
#define Lg1    6.666666666666735130e-01 /* 3FE55555 55555593 */
#define Lg2    3.999999999940941908e-01 /* 3FD99999 9997FA04 */
#define Lg3    2.857142874366239149e-01 /* 3FD24924 94229359 */
#define Lg4    2.222219843214978396e-01 /* 3FCC71C5 1D8E78AF */
#define Lg5    1.818357216161805012e-01 /* 3FC74664 96CB03DE */
#define Lg6    1.531383769920937332e-01 /* 3FC39A09 D078C69F */
#define Lg7    1.479819860511658591e-01 /* 3FC2F112 DF3E5244 */

double
log (double x)
{
  double hfsq, f, s, z, R, w, t1, t2, dk;
  int k, hx, i, j;
  unsigned lx;

  hx = __HI (x); /* high word of x */
  lx = __LO (x); /* low  word of x */

  k = 0;
  if (hx < 0x00100000) /* x < 2**-1022  */
  {
    if (((hx & 0x7fffffff) | lx) == 0) /* log(+-0) = -inf */
    {
      return -INFINITY;
    }
    if (hx < 0) /* log(-#) = NaN */
    {
      return NAN;
    }
    k -= 54;
    x *= two54; /* subnormal number, scale up x */
    hx = __HI (x); /* high word of x */
  }
  if (hx >= 0x7ff00000)
  {
    return x + x;
  }
  k += (hx >> 20) - 1023;
  hx &= 0x000fffff;
  i = (hx + 0x95f64) & 0x100000;

  double_accessor temp;
  temp.dbl = x;
  temp.as_int.hi = hx | (i ^ 0x3ff00000); /* normalize x or x / 2 */
  k += (i >> 20);
  f = temp.dbl - 1.0;

  if ((0x000fffff & (2 + hx)) < 3) /* |f| < 2**-20 */
  {
    if (f == zero)
    {
      if (k == 0)
      {
        return zero;
      }
      else
      {
        dk = (double) k;
        return dk * ln2_hi + dk * ln2_lo;
      }
    }
    R = f * f * (0.5 - 0.33333333333333333 * f);
    if (k == 0)
    {
      return f - R;
    }
    else
    {
      dk = (double) k;
      return dk * ln2_hi - ((R - dk * ln2_lo) - f);
    }
  }
  s = f / (2.0 + f);
  dk = (double) k;
  z = s * s;
  i = hx - 0x6147a;
  w = z * z;
  j = 0x6b851 - hx;
  t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
  t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
  i |= j;
  R = t2 + t1;
  if (i > 0)
  {
    hfsq = 0.5 * f * f;
    if (k == 0)
    {
      return f - (hfsq - s * (hfsq + R));
    }
    else
    {
      return dk * ln2_hi - ((hfsq - (s * (hfsq + R) + dk * ln2_lo)) - f);
    }
  }
  else
  {
    if (k == 0)
    {
      return f - s * (f - R);
    }
    else
    {
      return dk * ln2_hi - ((s * (f - R) - dk * ln2_lo) - f);
    }
  }
} /* log */

#undef zero
#undef ln2_hi
#undef ln2_lo
#undef two54
#undef Lg1
#undef Lg2
#undef Lg3
#undef Lg4
#undef Lg5
#undef Lg6
#undef Lg7


/* fmod(x,y)
 * Return x mod y in exact arithmetic
 *
 * Method: shift and subtract
 */

static const double Zero[] = {
  0.0,
  -0.0,
};

double
fmod (double x, double y)
{
  int n, hx, hy, hz, ix, iy, sx, i;
  unsigned lx, ly, lz;

  hx = __HI (x); /* high word of x */
  lx = __LO (x); /* low  word of x */
  hy = __HI (y); /* high word of y */
  ly = __LO (y); /* low  word of y */
  sx = hx & 0x80000000; /* sign of x */
  hx ^= sx; /* |x| */
  hy &= 0x7fffffff; /* |y| */

  /* purge off exception values */
  if ((hy | ly) == 0 || (hx >= 0x7ff00000) || /* y = 0, or x not finite */
      ((hy | ((ly | -ly) >> 31)) > 0x7ff00000)) /* or y is NaN */
  {
    return NAN;
  }
  if (hx <= hy)
  {
    if ((hx < hy) || (lx < ly)) /* |x| < |y| return x */
    {
      return x;
    }
    if (lx == ly) /* |x| = |y| return x * 0 */
    {
      return Zero[(unsigned) sx >> 31];
    }
  }

  /* determine ix = ilogb(x) */
  if (hx < 0x00100000) /* subnormal x */
  {
    if (hx == 0)
    {
      for (ix = -1043, i = lx; i > 0; i <<= 1)
      {
        ix -= 1;
      }
    }
    else
    {
      for (ix = -1022, i = (hx << 11); i > 0; i <<= 1)
      {
        ix -= 1;
      }
    }
  }
  else
  {
    ix = (hx >> 20) - 1023;
  }

  /* determine iy = ilogb(y) */
  if (hy < 0x00100000) /* subnormal y */
  {
    if (hy == 0)
    {
      for (iy = -1043, i = ly; i > 0; i <<= 1)
      {
        iy -= 1;
      }
    }
    else
    {
      for (iy = -1022, i = (hy << 11); i > 0; i <<= 1)
      {
        iy -= 1;
      }
    }
  }
  else
  {
    iy = (hy >> 20) - 1023;
  }

  /* set up {hx,lx}, {hy,ly} and align y to x */
  if (ix >= -1022)
  {
    hx = 0x00100000 | (0x000fffff & hx);
  }
  else /* subnormal x, shift x to normal */
  {
    n = -1022 - ix;
    if (n <= 31)
    {
      hx = (((unsigned int) hx) << n) | (lx >> (32 - n));
      lx <<= n;
    }
    else
    {
      hx = lx << (n - 32);
      lx = 0;
    }
  }
  if (iy >= -1022)
  {
    hy = 0x00100000 | (0x000fffff & hy);
  }
  else /* subnormal y, shift y to normal */
  {
    n = -1022 - iy;
    if (n <= 31)
    {
      hy = (((unsigned int) hy) << n) | (ly >> (32 - n));
      ly <<= n;
    }
    else
    {
      hy = ly << (n - 32);
      ly = 0;
    }
  }

  /* fix point fmod */
  n = ix - iy;
  while (n--)
  {
    hz = hx - hy;
    lz = lx - ly;
    if (lx < ly)
    {
      hz -= 1;
    }
    if (hz < 0)
    {
      hx = hx + hx + (lx >> 31);
      lx = lx + lx;
    }
    else
    {
      if ((hz | lz) == 0) /* return sign(x) * 0 */
      {
        return Zero[(unsigned) sx >> 31];
      }
      hx = hz + hz + (lz >> 31);
      lx = lz + lz;
    }
  }
  hz = hx - hy;
  lz = lx - ly;
  if (lx < ly)
  {
    hz -= 1;
  }
  if (hz >= 0)
  {
    hx = hz;
    lx = lz;
  }

  /* convert back to floating value and restore the sign */
  if ((hx | lx) == 0) /* return sign(x) * 0 */
  {
    return Zero[(unsigned) sx >> 31];
  }
  while (hx < 0x00100000) /* normalize x */
  {
    hx = hx + hx + (lx >> 31);
    lx = lx + lx;
    iy -= 1;
  }

  double_accessor ret;
  if (iy >= -1022) /* normalize output */
  {
    hx = ((hx - 0x00100000) | ((iy + 1023) << 20));
    ret.as_int.hi = hx | sx;
    ret.as_int.lo = lx;
  }
  else /* subnormal output */
  {
    n = -1022 - iy;
    if (n <= 20)
    {
      lx = (lx >> n) | ((unsigned) hx << (32 - n));
      hx >>= n;
    }
    else if (n <= 31)
    {
      lx = (hx << (32 - n)) | (lx >> n);
      hx = sx;
    }
    else
    {
      lx = hx >> (n - 32);
      hx = sx;
    }
    ret.as_int.hi = hx | sx;
    ret.as_int.lo = lx;
  }
  return ret.dbl; /* exact output */
} /* fmod */


/* floor(x)
 * Return x rounded toward -inf to integral value
 *
 * Method:
 *      Bit twiddling.
 *
 * Exception:
 *      Inexact flag raised if x not equal to floor(x).
 */

#define huge 1.0e300

double
floor (double x)
{
  int i0, i1, j0;
  unsigned i, j;

  i0 = __HI (x);
  i1 = __LO (x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (j0 < 20)
  {
    if (j0 < 0) /* raise inexact if x != 0 */
    {
      if (huge + x > 0.0) /* return 0 * sign(x) if |x| < 1 */
      {
        if (i0 >= 0)
        {
          i0 = i1 = 0;
        }
        else if (((i0 & 0x7fffffff) | i1) != 0)
        {
          i0 = 0xbff00000;
          i1 = 0;
        }
      }
    }
    else
    {
      i = (0x000fffff) >> j0;
      if (((i0 & i) | i1) == 0) /* x is integral */
      {
        return x;
      }
      if (huge + x > 0.0) /* raise inexact flag */
      {
        if (i0 < 0)
        {
          i0 += (0x00100000) >> j0;
        }
        i0 &= (~i);
        i1 = 0;
      }
    }
  }
  else if (j0 > 51)
  {
    if (j0 == 0x400) /* inf or NaN */
    {
      return x + x;
    }
    else /* x is integral */
    {
      return x;
    }
  }
  else
  {
    i = ((unsigned) (0xffffffff)) >> (j0 - 20);
    if ((i1 & i) == 0) /* x is integral */
    {
      return x;
    }
    if (huge + x > 0.0) /* raise inexact flag */
    {
      if (i0 < 0)
      {
        if (j0 == 20)
        {
          i0 += 1;
        }
        else
        {
          j = i1 + (1 << (52 - j0));
          if (j < i1) /* got a carry */
          {
            i0 += 1;
          }
          i1 = j;
        }
      }
      i1 &= (~i);
    }
  }

  double_accessor ret;
  ret.as_int.hi = i0;
  ret.as_int.lo = i1;
  return ret.dbl;
} /* floor */

#undef huge


/* fabs(x) returns the absolute value of x.
 */

double
fabs (double x)
{
  double_accessor ret;
  ret.dbl = x;
  ret.as_int.hi &= 0x7fffffff;
  return ret.dbl;
} /* fabs */


/* expm1(x)
 * Returns exp(x)-1, the exponential of x minus 1.
 *
 * Method
 *   1. Argument reduction:
 *  Given x, find r and integer k such that
 *
 *               x = k*ln2 + r,  |r| <= 0.5*ln2 ~ 0.34658
 *
 *      Here a correction term c will be computed to compensate
 *  the error in r when rounded to a floating-point number.
 *
 *   2. Approximating expm1(r) by a special rational function on
 *  the interval [0,0.34658]:
 *  Since
 *      r*(exp(r)+1)/(exp(r)-1) = 2+ r^2/6 - r^4/360 + ...
 *  we define R1(r*r) by
 *      r*(exp(r)+1)/(exp(r)-1) = 2+ r^2/6 * R1(r*r)
 *  That is,
 *      R1(r**2) = 6/r *((exp(r)+1)/(exp(r)-1) - 2/r)
 *         = 6/r * ( 1 + 2.0*(1/(exp(r)-1) - 1/r))
 *         = 1 - r^2/60 + r^4/2520 - r^6/100800 + ...
 *      We use a special Reme algorithm on [0,0.347] to generate
 *   a polynomial of degree 5 in r*r to approximate R1. The
 *  maximum error of this polynomial approximation is bounded
 *  by 2**-61. In other words,
 *      R1(z) ~ 1.0 + Q1*z + Q2*z**2 + Q3*z**3 + Q4*z**4 + Q5*z**5
 *  where   Q1  =  -1.6666666666666567384E-2,
 *     Q2  =   3.9682539681370365873E-4,
 *     Q3  =  -9.9206344733435987357E-6,
 *     Q4  =   2.5051361420808517002E-7,
 *     Q5  =  -6.2843505682382617102E-9;
 *    z   =  r*r,
 *  with error bounded by
 *      |                  5           |     -61
 *      | 1.0+Q1*z+...+Q5*z   -  R1(z) | <= 2
 *      |                              |
 *
 *  expm1(r) = exp(r)-1 is then computed by the following
 *   specific way which minimize the accumulation rounding error:
 *                        2     3
 *                        r     r    [ 3 - (R1 + R1*r/2)  ]
 *        expm1(r) = r + --- + --- * [--------------------]
 *                        2     2    [ 6 - r*(3 - R1*r/2) ]
 *
 *  To compensate the error in the argument reduction, we use
 *    expm1(r+c) = expm1(r) + c + expm1(r)*c
 *         ~ expm1(r) + c + r*c
 *  Thus c+r*c will be added in as the correction terms for
 *  expm1(r+c). Now rearrange the term to avoid optimization
 *   screw up:
 *                  (      2                                    2 )
 *                  ({  ( r    [ R1 -  (3 - R1*r/2) ]  )  }    r  )
 *   expm1(r+c)~r - ({r*(--- * [--------------------]-c)-c} - --- )
 *                  ({  ( 2    [ 6 - r*(3 - R1*r/2) ]  )  }    2  )
 *                  (                                             )
 *
 *       = r - E
 *   3. Scale back to obtain expm1(x):
 *  From step 1, we have
 *     expm1(x) = either 2^k*[expm1(r)+1] - 1
 *              = or     2^k*[expm1(r) + (1-2^-k)]
 *   4. Implementation notes:
 *  (A). To save one multiplication, we scale the coefficient Qi
 *       to Qi*2^i, and replace z by (x^2)/2.
 *  (B). To achieve maximum accuracy, we compute expm1(x) by
 *    (i)   if x < -56*ln2, return -1.0, (raise inexact if x!=inf)
 *    (ii)  if k=0, return r-E
 *    (iii) if k=-1, return 0.5*(r-E)-0.5
 *    (iv)  if k=1 if r < -0.25, return 2*((r+0.5)- E)
 *                  else       return  1.0+2.0*(r-E);
 *    (v)   if (k<-2||k>56) return 2^k(1-(E-r)) - 1 (or exp(x)-1)
 *    (vi)  if k <= 20, return 2^k((1-2^-k)-(E-r)), else
 *    (vii) return 2^k(1-((E+2^-k)-r))
 *
 * Special cases:
 *  expm1(INF) is INF, expm1(NaN) is NaN;
 *  expm1(-INF) is -1, and
 *  for finite argument, only expm1(0)=0 is exact.
 *
 * Accuracy:
 *  according to an error analysis, the error is always less than
 *  1 ulp (unit in the last place).
 *
 * Misc. info.
 *  For IEEE double
 *      if x >  7.09782712893383973096e+02 then expm1(x) overflow
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

#define one         1.0
#define huge        1.0e+300
#define tiny        1.0e-300
#define o_threshold 7.09782712893383973096e+02 /* 0x40862E42, 0xFEFA39EF */
#define ln2_hi      6.93147180369123816490e-01 /* 0x3fe62e42, 0xfee00000 */
#define ln2_lo      1.90821492927058770002e-10 /* 0x3dea39ef, 0x35793c76 */
#define invln2      1.44269504088896338700e+00 /* 0x3ff71547, 0x652b82fe */

/* Scaled Q's: Qn_here = 2**n * Qn_above, for R(2*z) where z = hxs = x*x/2: */
#define Q1 -3.33333333333331316428e-02 /* BFA11111 111110F4 */
#define Q2 1.58730158725481460165e-03 /* 3F5A01A0 19FE5585 */
#define Q3 -7.93650757867487942473e-05 /* BF14CE19 9EAADBB7 */
#define Q4 4.00821782732936239552e-06 /* 3ED0CFCA 86E65239 */
#define Q5 -2.01099218183624371326e-07 /* BE8AFDB7 6E09C32D */

double
expm1 (double x)
{
  double y, hi, lo, c, e, hxs, hfx, r1;
  double_accessor t, twopk;
  int k, xsb;
  unsigned int hx;

  hx = __HI (x);
  xsb = hx & 0x80000000; /* sign bit of x */
  hx &= 0x7fffffff; /* high word of |x| */

  /* filter out huge and non-finite argument */
  if (hx >= 0x4043687A)
  {
    /* if |x|>=56*ln2 */
    if (hx >= 0x40862E42)
    {
      /* if |x|>=709.78... */
      if (hx >= 0x7ff00000)
      {
        unsigned int low;
        low = __LO (x);
        if (((hx & 0xfffff) | low) != 0)
        {
          /* NaN */
          return x + x;
        }
        else
        {
          /* exp(+-inf)-1={inf,-1} */
          return (xsb == 0) ? x : -1.0;
        }
      }
      if (x > o_threshold)
      {
        /* overflow */
        return huge * huge;
      }
    }
    if (xsb != 0)
    {
      /* x < -56*ln2, return -1.0 with inexact */
      if (x + tiny < 0.0) /* raise inexact */
      {
        /* return -1 */
        return tiny - one;
      }
    }
  }

  /* argument reduction */
  if (hx > 0x3fd62e42)
  {
    /* if  |x| > 0.5 ln2 */
    if (hx < 0x3FF0A2B2)
    {
      /* and |x| < 1.5 ln2 */
      if (xsb == 0)
      {
        hi = x - ln2_hi;
        lo = ln2_lo;
        k = 1;
      }
      else
      {
        hi = x + ln2_hi;
        lo = -ln2_lo;
        k = -1;
      }
    }
    else
    {
      k = (int) (invln2 * x + ((xsb == 0) ? 0.5 : -0.5));
      t.dbl = k;
      hi = x - t.dbl * ln2_hi; /* t*ln2_hi is exact here */
      lo = t.dbl * ln2_lo;
    }
    x = hi - lo;
    c = (hi - x) - lo;
  }
  else if (hx < 0x3c900000)
  {
    /* when |x|<2**-54, return x */
    return x;
  }
  else
  {
    k = 0;
  }

  /* x is now in primary range */
  hfx = 0.5 * x;
  hxs = x * hfx;
  r1 = one + hxs * (Q1 + hxs * (Q2 + hxs * (Q3 + hxs * (Q4 + hxs * Q5))));
  t.dbl = 3.0 - r1 * hfx;
  e = hxs * ((r1 - t.dbl) / (6.0 - x * t.dbl));
  if (k == 0)
  {
    /* c is 0 */
    return x - (x * e - hxs);
  }
  else
  {
    twopk.as_int.hi = 0x3ff00000 + ((unsigned int) k << 20); /* 2^k */
    twopk.as_int.lo = 0;
    e = (x * (e - c) - c);
    e -= hxs;
    if (k == -1)
    {
      return 0.5 * (x - e) - 0.5;
    }
    if (k == 1)
    {
      if (x < -0.25)
      {
        return -2.0 * (e - (x + 0.5));
      }
      else
      {
        return one + 2.0 * (x - e);
      }
    }
    if ((k <= -2) || (k > 56))
    {
      /* suffice to return exp(x)-1 */
      y = one - (e - x);
      if (k == 1024)
      {
        const double twop1023 = ((double_accessor){ .as_int = { .hi = 0x7fe00000, .lo = 0 } }).dbl; /* 0x1p1023 */
        y = y * 2.0 * twop1023;
      }
      else
      {
        y = y * twopk.dbl;
      }
      return y - one;
    }
    t.dbl = one;
    if (k < 20)
    {
      t.as_int.hi = (0x3ff00000 - (0x200000 >> k)); /* t=1-2^-k */
      y = t.dbl - (e - x);
      y = y * twopk.dbl;
    }
    else
    {
      t.as_int.hi = ((0x3ff - k) << 20); /* 2^-k */
      y = x - (e + t.dbl);
      y += one;
      y = y * twopk.dbl;
    }
  }
  return y;
} /* expm1 */

#undef one
#undef huge
#undef tiny
#undef o_threshold
#undef ln2_hi
#undef ln2_lo
#undef invln2
#undef Q1
#undef Q2
#undef Q3
#undef Q4
#undef Q5


/* exp(x)
 * Returns the exponential of x.
 *
 * Method:
 *   1. Argument reduction:
 *      Reduce x to an r so that |r| <= 0.5*ln2 ~ 0.34658.
 *      Given x, find r and integer k such that
 *
 *               x = k*ln2 + r,  |r| <= 0.5*ln2.
 *
 *      Here r will be represented as r = hi-lo for better
 *      accuracy.
 *
 *   2. Approximation of exp(r) by a special rational function on
 *      the interval [0,0.34658]:
 *      Write
 *          R(r**2) = r*(exp(r)+1)/(exp(r)-1) = 2 + r*r/6 - r**4/360 + ...
 *      We use a special Remes algorithm on [0,0.34658] to generate
 *      a polynomial of degree 5 to approximate R. The maximum error
 *      of this polynomial approximation is bounded by 2**-59. In
 *      other words,
 *          R(z) ~ 2.0 + P1*z + P2*z**2 + P3*z**3 + P4*z**4 + P5*z**5
 *      (where z=r*r, and the values of P1 to P5 are listed below)
 *      and
 *          |                  5          |     -59
 *          | 2.0+P1*z+...+P5*z   -  R(z) | <= 2
 *          |                             |
 *      The computation of exp(r) thus becomes
 *                             2*r
 *              exp(r) = 1 + -------
 *                            R - r
 *                                 r*R1(r)
 *                     = 1 + r + ----------- (for better accuracy)
 *                                2 - R1(r)
 *      where
 *                               2       4             10
 *              R1(r) = r - (P1*r  + P2*r  + ... + P5*r   ).
 *
 *   3. Scale back to obtain exp(x):
 *      From step 1, we have
 *         exp(x) = 2^k * exp(r)
 *
 * Special cases:
 *      exp(INF) is INF, exp(NaN) is NaN;
 *      exp(-INF) is 0, and
 *      for finite argument, only exp(0)=1 is exact.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Misc. info:
 *      For IEEE double
 *          if x >  7.09782712893383973096e+02 then exp(x) overflow
 *          if x < -7.45133219101941108420e+02 then exp(x) underflow
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double halF[2] = {
  0.5,
  -0.5,
};
static const double ln2HI[2] = {
  6.93147180369123816490e-01, /* 0x3fe62e42, 0xfee00000 */
  -6.93147180369123816490e-01, /* 0xbfe62e42, 0xfee00000 */
};
static const double ln2LO[2] = {
  1.90821492927058770002e-10, /* 0x3dea39ef, 0x35793c76 */
  -1.90821492927058770002e-10, /* 0xbdea39ef, 0x35793c76 */
};

#define one         1.0
#define huge        1.0e+300
#define twom1000    9.33263618503218878990e-302 /* 2**-1000=0x01700000,0 */
#define o_threshold 7.09782712893383973096e+02 /* 0x40862E42, 0xFEFA39EF */
#define u_threshold -7.45133219101941108420e+02 /* 0xc0874910, 0xD52D3051 */
#define invln2      1.44269504088896338700e+00 /* 0x3ff71547, 0x652b82fe */
#define P1          1.66666666666666019037e-01 /* 0x3FC55555, 0x5555553E */
#define P2          -2.77777777770155933842e-03 /* 0xBF66C16C, 0x16BEBD93 */
#define P3          6.61375632143793436117e-05 /* 0x3F11566A, 0xAF25DE2C */
#define P4          -1.65339022054652515390e-06 /* 0xBEBBBD41, 0xC5D26BF1 */
#define P5          4.13813679705723846039e-08 /* 0x3E663769, 0x72BEA4D0 */

double
exp (double x) /* default IEEE double exp */
{
  double hi, lo, c, t;
  int k = 0, xsb;
  unsigned hx;

  hx = __HI (x); /* high word of x */
  xsb = (hx >> 31) & 1; /* sign bit of x */
  hx &= 0x7fffffff; /* high word of |x| */

  /* filter out non-finite argument */
  if (hx >= 0x40862E42) /* if |x| >= 709.78... */
  {
    if (hx >= 0x7ff00000)
    {
      if (((hx & 0xfffff) | __LO (x)) != 0) /* NaN */
      {
        return x + x;
      }
      else /* exp(+-inf) = {inf,0} */
      {
        return (xsb == 0) ? x : 0.0;
      }
    }
    if (x > o_threshold) /* overflow */
    {
      return huge * huge;
    }
    if (x < u_threshold) /* underflow */
    {
      return twom1000 * twom1000;
    }
  }

  /* argument reduction */
  if (hx > 0x3fd62e42) /* if  |x| > 0.5 ln2 */
  {
    if (hx < 0x3FF0A2B2) /* and |x| < 1.5 ln2 */
    {
      hi = x - ln2HI[xsb];
      lo = ln2LO[xsb];
      k = 1 - xsb - xsb;
    }
    else
    {
      k = (int) (invln2 * x + halF[xsb]);
      t = k;
      hi = x - t * ln2HI[0]; /* t * ln2HI is exact here */
      lo = t * ln2LO[0];
    }
    x = hi - lo;
  }
  else if (hx < 0x3e300000) /* when |x| < 2**-28 */
  {
    if (huge + x > one) /* trigger inexact */
    {
      return one + x;
    }
  }
  else
  {
    k = 0;
  }

  double_accessor ret;

  /* x is now in primary range */
  t = x * x;
  c = x - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
  if (k == 0)
  {
    return one - ((x * c) / (c - 2.0) - x);
  }
  else
  {
    ret.dbl = one - ((lo - (x * c) / (2.0 - c)) - hi);
  }
  if (k >= -1021)
  {
    ret.as_int.hi += (((unsigned int) k) << 20); /* add k to y's exponent */
    return ret.dbl;
  }
  else
  {
    ret.as_int.hi += ((k + 1000) << 20); /* add k to y's exponent */
    return ret.dbl * twom1000;
  }
} /* exp */

#undef one
#undef huge
#undef twom1000
#undef o_threshold
#undef u_threshold
#undef invln2
#undef P1
#undef P2
#undef P3
#undef P4
#undef P5


/* cosh(x)
 * Method:
 * mathematically cosh(x) if defined to be (exp(x) + exp(-x)) / 2
 *  1. Replace x by |x| (cosh(x) = cosh(-x)).
 *  2.
 *                                                 [ exp(x) - 1 ]^2
 *      0        <= x <= ln2/2  :  cosh(x) := 1 + -------------------
 *                                                     2*exp(x)
 *
 *                                             exp(x) +  1/exp(x)
 *      ln2/2    <= x <= 22     :  cosh(x) := -------------------
 *                                                    2
 *
 *      22       <= x <= lnovft :  cosh(x) := exp(x)/2
 *      lnovft   <= x <= ln2ovft:  cosh(x) := exp(x/2)/2 * exp(x/2)
 *      ln2ovft  <  x           :  cosh(x) := huge * huge (overflow)
 *
 * Special cases:
 *  cosh(x) is |x| if x is +INF, -INF, or NaN.
 *  only cosh(0) = 1 is exact for finite x.
 */

#define one  1.0
#define half 0.5
#define huge 1.0e300

double
cosh (double x)
{
  double t, w;
  int ix;
  unsigned lx;

  /* High word of |x|. */
  ix = __HI (x);
  ix &= 0x7fffffff;

  /* x is INF or NaN */
  if (ix >= 0x7ff00000)
  {
    return x * x;
  }
  /* |x| in [0, 0.5 * ln2], return 1 + expm1(|x|)^2 / (2 * exp(|x|)) */
  if (ix < 0x3fd62e43)
  {
    t = expm1 (fabs (x));
    w = one + t;
    if (ix < 0x3c800000)
    {
      /* cosh(tiny) = 1 */
      return w;
    }
    return one + (t * t) / (w + w);
  }

  /* |x| in [0.5 * ln2, 22], return (exp(|x|) + 1 / exp(|x|) / 2; */
  if (ix < 0x40360000)
  {
    t = exp (fabs (x));
    return half * t + half / t;
  }

  /* |x| in [22, log(maxdouble)] return half * exp(|x|) */
  if (ix < 0x40862E42)
  {
    return half * exp (fabs (x));
  }
  /* |x| in [log(maxdouble), overflowthresold] */
  lx = ((1 >> 29) + (unsigned int) x);
  if ((ix < 0x408633CE) || ((ix == 0x408633ce) && (lx <= (unsigned) 0x8fb9f87d)))
  {
    w = exp (half * fabs (x));
    t = half * w;
    return t * w;
  }

  /* |x| > overflowthresold, cosh(x) overflow */
  return huge * huge;
} /* cosh */

#undef one
#undef half
#undef huge


/* copysign(x,y) returns a value with the magnitude of x and
 * with the sign bit of y.
 */

double
copysign (double x, double y)
{
  double_accessor ret;
  ret.dbl = x;
  ret.as_int.hi = (__HI (x) & 0x7fffffff) | (__HI (y) & 0x80000000);
  return ret.dbl;
} /* copysign */


/* ceil(x)
 * Return x rounded toward -inf to integral value
 *
 * Method:
 *      Bit twiddling.
 *
 * Exception:
 *      Inexact flag raised if x not equal to ceil(x).
 */

#define huge 1.0e300

double
ceil (double x)
{
  int i0, i1, j0;
  unsigned i, j;

  i0 = __HI (x);
  i1 = __LO (x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (j0 < 20)
  {
    if (j0 < 0) /* raise inexact if x != 0 */
    {
      if (huge + x > 0.0) /* return 0 * sign(x) if |x| < 1 */
      {
        if (i0 < 0)
        {
          i0 = 0x80000000;
          i1 = 0;
        }
        else if ((i0 | i1) != 0)
        {
          i0 = 0x3ff00000;
          i1 = 0;
        }
      }
    }
    else
    {
      i = (0x000fffff) >> j0;
      if (((i0 & i) | i1) == 0) /* x is integral */
      {
        return x;
      }
      if (huge + x > 0.0) /* raise inexact flag */
      {
        if (i0 > 0)
        {
          i0 += (0x00100000) >> j0;
        }
        i0 &= (~i);
        i1 = 0;
      }
    }
  }
  else if (j0 > 51)
  {
    if (j0 == 0x400) /* inf or NaN */
    {
      return x + x;
    }
    else /* x is integral */
    {
      return x;
    }
  }
  else
  {
    i = ((unsigned) (0xffffffff)) >> (j0 - 20);
    if ((i1 & i) == 0) /* x is integral */
    {
      return x;
    }
    if (huge + x > 0.0) /* raise inexact flag */
    {
      if (i0 > 0)
      {
        if (j0 == 20)
        {
          i0 += 1;
        }
        else
        {
          j = i1 + (1 << (52 - j0));
          if (j < i1) /* got a carry */
          {
            i0 += 1;
          }
          i1 = j;
        }
      }
      i1 &= (~i);
    }
  }

  double_accessor ret;
  ret.as_int.hi = i0;
  ret.as_int.lo = i1;
  return ret.dbl;
} /* ceil */

#undef huge


/* cbrt(x)
 * Return cube root of x
 */

#define B1 715094163 /* B1 = (682 - 0.03306235651) * 2**20 */
#define B2 696219795 /* B2 = (664 - 0.03306235651) * 2**20 */
#define C  5.42857142857142815906e-01 /* 19/35     = 0x3FE15F15, 0xF15F15F1 */
#define D  -7.05306122448979611050e-01 /* -864/1225 = 0xBFE691DE, 0x2532C834 */
#define E  1.41428571428571436819e+00 /* 99/70     = 0x3FF6A0EA, 0x0EA0EA0F */
#define F  1.60714285714285720630e+00 /* 45/28     = 0x3FF9B6DB, 0x6DB6DB6E */
#define G  3.57142857142857150787e-01 /* 5/14      = 0x3FD6DB6D, 0xB6DB6DB7 */

double
cbrt (double x)
{
  double r, s, w;
  double_accessor t, temp;
  unsigned int sign;
  t.dbl = 0.0;
  temp.dbl = x;

  sign = temp.as_int.hi & 0x80000000; /* sign = sign(x) */
  temp.as_int.hi ^= sign;

  if (temp.as_int.hi >= 0x7ff00000)
  {
    /* cbrt(NaN, INF) is itself */
    return (x + x);
  }
  if ((temp.as_int.hi | temp.as_int.lo) == 0)
  {
    /* cbrt(0) is itself */
    return (x);
  }
  /* rough cbrt to 5 bits */
  if (temp.as_int.hi < 0x00100000) /* subnormal number */
  {
    t.as_int.hi = 0x43500000; /* set t= 2**54 */
    t.dbl *= temp.dbl;
    t.as_int.hi = t.as_int.hi / 3 + B2;
  }
  else
  {
    t.as_int.hi = temp.as_int.hi / 3 + B1;
  }

  /* new cbrt to 23 bits, may be implemented in single precision */
  r = t.dbl * t.dbl / temp.dbl;
  s = C + r * t.dbl;
  t.dbl *= G + F / (s + E + D / s);

  /* chopped to 20 bits and make it larger than cbrt(x) */
  t.as_int.lo = 0;
  t.as_int.hi += 0x00000001;

  /* one step newton iteration to 53 bits with error less than 0.667 ulps */
  s = t.dbl * t.dbl; /* t*t is exact */
  r = temp.dbl / s;
  w = t.dbl + t.dbl;
  r = (r - t.dbl) / (w + r); /* r-s is exact */
  t.dbl = t.dbl + (t.dbl * r);

  /* retore the sign bit */
  t.as_int.hi |= sign;
  return (t.dbl);
} /* cbrt */

#undef B1
#undef B2
#undef C
#undef D
#undef E
#undef F
#undef G


/* atanh(x)
 * Method :
 *    1.Reduced x to positive by atanh(-x) = -atanh(x)
 *    2.For x >= 0.5
 *              1              2x                          x
 *  atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *              2             1 - x                      1 - x
 *
 *   For x < 0.5
 *    atanh(x) = 0.5 * log1p(2x + 2x * x / (1 - x))
 *
 * Special cases:
 *  atanh(x) is NaN if |x| > 1 with signal;
 *  atanh(NaN) is that NaN with no signal;
 *  atanh(+-1) is +-INF with signal.
 *
 */

#define zero 0.0
#define one  1.0
#define huge 1.0e+300

double
atanh (double x)
{
  double t;
  int hx, ix;
  double_accessor temp;
  temp.dbl = x;
  hx = temp.as_int.hi;
  ix = hx & 0x7fffffff;

  /* |x| > 1 */
  if ((ix | ((unsigned int) (temp.as_int.lo | (-temp.as_int.lo)) >> 31)) > 0x3ff00000)
  {
    return NAN;
  }
  if (ix == 0x3ff00000)
  {
    return x / zero;
  }
  if (ix < 0x3e300000 && (huge + x) > zero)
  {
    return x; /* x<2**-28 */
  }

  /* x <- |x| */
  temp.as_int.hi = ix;
  if (ix < 0x3fe00000)
  {
    /* x < 0.5 */
    t = temp.dbl + temp.dbl;
    t = 0.5 * log1p (t + t * temp.dbl / (one - temp.dbl));
  }
  else
  {
    t = 0.5 * log1p ((temp.dbl + temp.dbl) / (one - temp.dbl));
  }
  if (hx >= 0)
  {
    return t;
  }
  else
  {
    return -t;
  }
} /* atanh */

#undef zero
#undef one
#undef huge


/* atan2(y,x)
 *
 * Method:
 *      1. Reduce y to positive by atan2(y,x)=-atan2(-y,x).
 *      2. Reduce x to positive by (if x and y are unexceptional):
 *              ARG (x+iy) = arctan(y/x)           ... if x > 0,
 *              ARG (x+iy) = pi - arctan[y/(-x)]   ... if x < 0,
 *
 * Special cases:
 *      ATAN2((anything), NaN ) is NaN;
 *      ATAN2(NAN , (anything) ) is NaN;
 *      ATAN2(+-0, +(anything but NaN)) is +-0  ;
 *      ATAN2(+-0, -(anything but NaN)) is +-pi ;
 *      ATAN2(+-(anything but 0 and NaN), 0) is +-pi/2;
 *      ATAN2(+-(anything but INF and NaN), +INF) is +-0 ;
 *      ATAN2(+-(anything but INF and NaN), -INF) is +-pi;
 *      ATAN2(+-INF,+INF ) is +-pi/4 ;
 *      ATAN2(+-INF,-INF ) is +-3pi/4;
 *      ATAN2(+-INF, (anything but,0,NaN, and INF)) is +-pi/2;
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

#define tiny   1.0e-300
#define zero   0.0
#define pi_o_4 7.8539816339744827900E-01 /* 0x3FE921FB, 0x54442D18 */
#define pi_o_2 1.5707963267948965580E+00 /* 0x3FF921FB, 0x54442D18 */
#define pi     3.1415926535897931160E+00 /* 0x400921FB, 0x54442D18 */
#define pi_lo  1.2246467991473531772E-16 /* 0x3CA1A626, 0x33145C07 */

double
atan2 (double y, double x)
{
  double_accessor z;
  int k, m, hx, hy, ix, iy;
  unsigned lx, ly;

  hx = __HI (x);
  ix = hx & 0x7fffffff;
  lx = __LO (x);
  hy = __HI (y);
  iy = hy & 0x7fffffff;
  ly = __LO (y);
  if (((ix | ((lx | -lx) >> 31)) > 0x7ff00000) || ((iy | ((ly | -ly) >> 31)) > 0x7ff00000)) /* x or y is NaN */
  {
    return x + y;
  }
  if (((hx - 0x3ff00000) | lx) == 0) /* x = 1.0 */
  {
    return atan (y);
  }
  m = ((hy < 0) ? 1 : 0) + ((hx < 0) ? 2 : 0); /* 2 * sign(x) + sign(y) */

  /* when y = 0 */
  if ((iy | ly) == 0)
  {
    switch (m)
    {
      case 0:
      case 1:
      {
        return y; /* atan(+-0,+anything) = +-0 */
      }
      case 2:
      {
        return pi + tiny; /* atan(+0,-anything) = pi */
      }
      case 3:
      {
        return -pi - tiny; /* atan(-0,-anything) = -pi */
      }
    }
  }
  /* when x = 0 */
  if ((ix | lx) == 0)
  {
    return (hy < 0) ? -pi_o_2 - tiny : pi_o_2 + tiny;
  }

  /* when x is INF */
  if (ix == 0x7ff00000)
  {
    if (iy == 0x7ff00000)
    {
      switch (m)
      {
        case 0: /* atan(+INF,+INF) */
        {
          return pi_o_4 + tiny;
        }
        case 1: /* atan(-INF,+INF) */
        {
          return -pi_o_4 - tiny;
        }
        case 2: /* atan(+INF,-INF) */
        {
          return 3.0 * pi_o_4 + tiny;
        }
        case 3: /* atan(-INF,-INF) */
        {
          return -3.0 * pi_o_4 - tiny;
        }
      }
    }
    else
    {
      switch (m)
      {
        case 0: /* atan(+...,+INF) */
        {
          return zero;
        }
        case 1: /* atan(-...,+INF) */
        {
          return -zero;
        }
        case 2: /* atan(+...,-INF) */
        {
          return pi + tiny;
        }
        case 3: /* atan(-...,-INF) */
        {
          return -pi - tiny;
        }
      }
    }
  }
  /* when y is INF */
  if (iy == 0x7ff00000)
  {
    return (hy < 0) ? -pi_o_2 - tiny : pi_o_2 + tiny;
  }

  /* compute y / x */
  k = (iy - ix) >> 20;
  if (k > 60) /* |y / x| > 2**60 */
  {
    z.dbl = pi_o_2 + 0.5 * pi_lo;
  }
  else if (hx < 0 && k < -60) /* |y| / x < -2**60 */
  {
    z.dbl = 0.0;
  }
  else /* safe to do y / x */
  {
    z.dbl = atan (fabs (y / x));
  }
  switch (m)
  {
    case 0: /* atan(+,+) */
    {
      return z.dbl;
    }
    case 1: /* atan(-,+) */
    {
      z.as_int.hi ^= 0x80000000;
      return z.dbl;
    }
    case 2: /* atan(+,-) */
    {
      return pi - (z.dbl - pi_lo);
    }
    /* case 3: */
    default: /* atan(-,-) */
    {
      return (z.dbl - pi_lo) - pi;
    }
  }
} /* atan2 */

#undef tiny
#undef zero
#undef pi_o_4
#undef pi_o_2
#undef pi
#undef pi_lo


/* atan(x)
 *
 * Method:
 *   1. Reduce x to positive by atan(x) = -atan(-x).
 *   2. According to the integer k=4t+0.25 chopped, t=x, the argument
 *      is further reduced to one of the following intervals and the
 *      arctangent of t is evaluated by the corresponding formula:
 *
 *      [0,7/16]      atan(x) = t-t^3*(a1+t^2*(a2+...(a10+t^2*a11)...)
 *      [7/16,11/16]  atan(x) = atan(1/2) + atan( (t-0.5)/(1+t/2) )
 *      [11/16.19/16] atan(x) = atan( 1 ) + atan( (t-1)/(1+t) )
 *      [19/16,39/16] atan(x) = atan(3/2) + atan( (t-1.5)/(1+1.5t) )
 *      [39/16,INF]   atan(x) = atan(INF) + atan( -1/t )
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double atanhi[] = {
  4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
  7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
  9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
  1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const double atanlo[] = {
  2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
  3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
  1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
  6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

#define aT0  3.33333333333329318027e-01 /* 0x3FD55555, 0x5555550D */
#define aT1  -1.99999999998764832476e-01 /* 0xBFC99999, 0x9998EBC4 */
#define aT2  1.42857142725034663711e-01 /* 0x3FC24924, 0x920083FF */
#define aT3  -1.11111104054623557880e-01 /* 0xBFBC71C6, 0xFE231671 */
#define aT4  9.09088713343650656196e-02 /* 0x3FB745CD, 0xC54C206E */
#define aT5  -7.69187620504482999495e-02 /* 0xBFB3B0F2, 0xAF749A6D */
#define aT6  6.66107313738753120669e-02 /* 0x3FB10D66, 0xA0D03D51 */
#define aT7  -5.83357013379057348645e-02 /* 0xBFADDE2D, 0x52DEFD9A */
#define aT8  4.97687799461593236017e-02 /* 0x3FA97B4B, 0x24760DEB */
#define aT9  -3.65315727442169155270e-02 /* 0xBFA2B444, 0x2C6A6C2F */
#define aT10 1.62858201153657823623e-02 /* 0x3F90AD3A, 0xE322DA11 */

#define one  1.0
#define huge 1.0e300

double
atan (double x)
{
  double w, s1, s2, z;
  int ix, hx, id;

  hx = __HI (x);
  ix = hx & 0x7fffffff;
  if (ix >= 0x44100000) /* if |x| >= 2^66 */
  {
    if (ix > 0x7ff00000 || (ix == 0x7ff00000 && (__LO (x) != 0)))
    {
      return x + x; /* NaN */
    }
    if (hx > 0)
    {
      return atanhi[3] + atanlo[3];
    }
    else
    {
      return -atanhi[3] - atanlo[3];
    }
  }
  if (ix < 0x3fdc0000) /* |x| < 0.4375 */
  {
    if (ix < 0x3e200000) /* |x| < 2^-29 */
    {
      if (huge + x > one) /* raise inexact */
      {
        return x;
      }
    }
    id = -1;
  }
  else
  {
    x = fabs (x);
    if (ix < 0x3ff30000) /* |x| < 1.1875 */
    {
      if (ix < 0x3fe60000) /* 7/16 <= |x| < 11/16 */
      {
        id = 0;
        x = (2.0 * x - one) / (2.0 + x);
      }
      else /* 11/16 <= |x| < 19/16 */
      {
        id = 1;
        x = (x - one) / (x + one);
      }
    }
    else
    {
      if (ix < 0x40038000) /* |x| < 2.4375 */
      {
        id = 2;
        x = (x - 1.5) / (one + 1.5 * x);
      }
      else /* 2.4375 <= |x| < 2^66 */
      {
        id = 3;
        x = -1.0 / x;
      }
    }
  }
  /* end of argument reduction */
  z = x * x;
  w = z * z;
  /* break sum from i=0 to 10 aT[i] z**(i+1) into odd and even poly */
  s1 = z * (aT0 + w * (aT2 + w * (aT4 + w * (aT6 + w * (aT8 + w * aT10)))));
  s2 = w * (aT1 + w * (aT3 + w * (aT5 + w * (aT7 + w * aT9))));
  if (id < 0)
  {
    return x - x * (s1 + s2);
  }
  else
  {
    z = atanhi[id] - ((x * (s1 + s2) - atanlo[id]) - x);
    return (hx < 0) ? -z : z;
  }
} /* atan */

#undef aT0
#undef aT1
#undef aT2
#undef aT3
#undef aT4
#undef aT5
#undef aT6
#undef aT7
#undef aT8
#undef aT9
#undef aT10
#undef one
#undef huge


/* asinh(x)
 * Method :
 *  Based on
 *    asinh(x) = sign(x) * log [ |x| + sqrt(x*x+1) ]
 *  we have
 *  asinh(x) := x  if  1 + x * x = 1,
 *           := sign(x) * (log(x)+ln2)) for large |x|, else
 *           := sign(x) * log(2|x| + 1 / (|x| + sqrt(x * x + 1))) if|x| > 2, else
 *           := sign(x) * log1p(|x| + x^2 / (1 + sqrt(1 + x^2)))
 */

#define one  1.0
#define ln2  6.93147180559945286227e-01 /* 0x3FE62E42, 0xFEFA39EF */
#define huge 1.0e+300

double
asinh (double x)
{
  double t, w;
  int hx, ix;
  hx = __HI (x);
  ix = hx & 0x7fffffff;
  if (ix >= 0x7ff00000)
  {
    /* x is inf or NaN */
    return x + x;
  }
  if (ix < 0x3e300000)
  {
    /* |x| < 2**-28 */
    if (huge + x > one)
    {
      /* return x inexact except 0 */
      return x;
    }
  }
  if (ix > 0x41b00000)
  {
    /* |x| > 2**28 */
    w = log (fabs (x)) + ln2;
  }
  else if (ix > 0x40000000)
  {
    /* 2**28 > |x| > 2.0 */
    t = fabs (x);
    w = log (2.0 * t + one / (sqrt (x * x + one) + t));
  }
  else
  {
    /* 2.0 > |x| > 2**-28 */
    t = x * x;
    w = log1p (fabs (x) + t / (one + sqrt (one + t)));
  }
  if (hx > 0)
  {
    return w;
  }
  else
  {
    return -w;
  }
} /* asinh */

#undef one
#undef ln2
#undef huge


/* asin(x)
 *
 * Method:
 *      Since  asin(x) = x + x^3/6 + x^5*3/40 + x^7*15/336 + ...
 *      we approximate asin(x) on [0,0.5] by
 *              asin(x) = x + x*x^2*R(x^2)
 *      where
 *              R(x^2) is a rational approximation of (asin(x)-x)/x^3
 *      and its remez error is bounded by
 *              |(asin(x)-x)/x^3 - R(x^2)| < 2^(-58.75)
 *
 *      For x in [0.5,1]
 *              asin(x) = pi/2-2*asin(sqrt((1-x)/2))
 *      Let y = (1-x), z = y/2, s := sqrt(z), and pio2_hi+pio2_lo=pi/2;
 *      then for x>0.98
 *              asin(x) = pi/2 - 2*(s+s*z*R(z))
 *                      = pio2_hi - (2*(s+s*z*R(z)) - pio2_lo)
 *      For x<=0.98, let pio4_hi = pio2_hi/2, then
 *              f = hi part of s;
 *              c = sqrt(z) - f = (z-f*f)/(s+f)         ...f+c=sqrt(z)
 *      and
 *              asin(x) = pi/2 - 2*(s+s*z*R(z))
 *                      = pio4_hi+(pio4-2s)-(2s*z*R(z)-pio2_lo)
 *                      = pio4_hi+(pio4-2f)-(2s*z*R(z)-(pio2_lo+2c))
 *
 * Special cases:
 *      if x is NaN, return x itself;
 *      if |x|>1, return NaN with invalid signal.
 */

#define one     1.00000000000000000000e+00 /* 0x3FF00000, 0x00000000 */
#define huge    1.000e+300
#define pio2_hi 1.57079632679489655800e+00 /* 0x3FF921FB, 0x54442D18 */
#define pio2_lo 6.12323399573676603587e-17 /* 0x3C91A626, 0x33145C07 */
#define pio4_hi 7.85398163397448278999e-01 /* 0x3FE921FB, 0x54442D18 */
/* coefficient for R(x^2) */
#define pS0 1.66666666666666657415e-01 /* 0x3FC55555, 0x55555555 */
#define pS1 -3.25565818622400915405e-01 /* 0xBFD4D612, 0x03EB6F7D */
#define pS2 2.01212532134862925881e-01 /* 0x3FC9C155, 0x0E884455 */
#define pS3 -4.00555345006794114027e-02 /* 0xBFA48228, 0xB5688F3B */
#define pS4 7.91534994289814532176e-04 /* 0x3F49EFE0, 0x7501B288 */
#define pS5 3.47933107596021167570e-05 /* 0x3F023DE1, 0x0DFDF709 */
#define qS1 -2.40339491173441421878e+00 /* 0xC0033A27, 0x1C8A2D4B */
#define qS2 2.02094576023350569471e+00 /* 0x40002AE5, 0x9C598AC8 */
#define qS3 -6.88283971605453293030e-01 /* 0xBFE6066C, 0x1B8D0159 */
#define qS4 7.70381505559019352791e-02 /* 0x3FB3B8C5, 0xB12E9282 */

double
asin (double x)
{
  double t, p, q, c, r, s;
  double_accessor w;
  int hx, ix;

  hx = __HI (x);
  ix = hx & 0x7fffffff;
  if (ix >= 0x3ff00000) /* |x| >= 1 */
  {
    if (((ix - 0x3ff00000) | __LO (x)) == 0) /* asin(1) = +-pi/2 with inexact */
    {
      return x * pio2_hi + x * pio2_lo;
    }
    return NAN; /* asin(|x|>1) is NaN */
  }
  else if (ix < 0x3fe00000) /* |x| < 0.5 */
  {
    if (ix < 0x3e400000) /* if |x| < 2**-27 */
    {
      if (huge + x > one) /* return x with inexact if x != 0 */
      {
        return x;
      }
    }
    t = x * x;
    p = t * (pS0 + t * (pS1 + t * (pS2 + t * (pS3 + t * (pS4 + t * pS5)))));
    q = one + t * (qS1 + t * (qS2 + t * (qS3 + t * qS4)));
    w.dbl = p / q;
    return x + x * w.dbl;
  }
  /* 1 > |x| >= 0.5 */
  w.dbl = one - fabs (x);
  t = w.dbl * 0.5;
  p = t * (pS0 + t * (pS1 + t * (pS2 + t * (pS3 + t * (pS4 + t * pS5)))));
  q = one + t * (qS1 + t * (qS2 + t * (qS3 + t * qS4)));
  s = sqrt (t);
  if (ix >= 0x3FEF3333) /* if |x| > 0.975 */
  {
    w.dbl = p / q;
    t = pio2_hi - (2.0 * (s + s * w.dbl) - pio2_lo);
  }
  else
  {
    w.dbl = s;
    w.as_int.lo = 0;
    c = (t - w.dbl * w.dbl) / (s + w.dbl);
    r = p / q;
    p = 2.0 * s * r - (pio2_lo - 2.0 * c);
    q = pio4_hi - 2.0 * w.dbl;
    t = pio4_hi - (p - q);
  }
  if (hx > 0)
  {
    return t;
  }
  else
  {
    return -t;
  }
} /* asin */

#undef one
#undef huge
#undef pio2_hi
#undef pio2_lo
#undef pio4_hi
#undef pS0
#undef pS1
#undef pS2
#undef pS3
#undef pS4
#undef pS5
#undef qS1
#undef qS2
#undef qS3
#undef qS4


/* acosh(x)
 * Method :
 *  Based on
 *    acosh(x) = log [ x + sqrt(x * x - 1) ]
 *  we have
 *    acosh(x) := log(x) + ln2, if x is large; else
 *    acosh(x) := log(2x - 1 / (sqrt(x * x - 1) + x)), if x > 2; else
 *    acosh(x) := log1p(t + sqrt(2.0 * t + t * t)); where t = x - 1.
 *
 * Special cases:
 *  acosh(x) is NaN with signal if x < 1.
 *  acosh(NaN) is NaN without signal.
 */

#define one 1.0
#define ln2 6.93147180559945286227e-01 /* 0x3FE62E42, 0xFEFA39EF */

double
acosh (double x)
{
  double t;
  int hx;
  hx = __HI (x);
  if (hx < 0x3ff00000)
  {
    /* x < 1 */
    return NAN;
  }
  else if (hx >= 0x41b00000)
  {
    /* x > 2**28 */
    if (hx >= 0x7ff00000)
    {
      /* x is inf of NaN */
      return x + x;
    }
    else
    {
      /* acosh(huge) = log(2x) */
      return log (x) + ln2;
    }
  }
  else if (((hx - 0x3ff00000) | __LO (x)) == 0)
  {
    /* acosh(1) = 0 */
    return 0.0;
  }
  else if (hx > 0x40000000)
  {
    /* 2**28 > x > 2 */
    t = x * x;
    return log (2.0 * x - one / (x + sqrt (t - one)));
  }
  else
  {
    /* 1 < x < 2 */
    t = x - one;
    return log1p (t + sqrt (2.0 * t + t * t));
  }
} /* acosh */

#undef one
#undef ln2


/* acos(x)
 *
 * Method:
 *      acos(x)  = pi/2 - asin(x)
 *      acos(-x) = pi/2 + asin(x)
 * For |x|<=0.5
 *      acos(x) = pi/2 - (x + x*x^2*R(x^2))     (see asin.c)
 * For x>0.5
 *      acos(x) = pi/2 - (pi/2 - 2asin(sqrt((1-x)/2)))
 *              = 2asin(sqrt((1-x)/2))
 *              = 2s + 2s*z*R(z)        ...z=(1-x)/2, s=sqrt(z)
 *              = 2f + (2c + 2s*z*R(z))
 *     where f=hi part of s, and c = (z-f*f)/(s+f) is the correction term
 *     for f so that f+c ~ sqrt(z).
 * For x<-0.5
 *      acos(x) = pi - 2asin(sqrt((1-|x|)/2))
 *              = pi - 0.5*(s+s*z*R(z)), where z=(1-|x|)/2,s=sqrt(z)
 *
 * Special cases:
 *      if x is NaN, return x itself;
 *      if |x|>1, return NaN with invalid signal.
 *
 * Function needed: sqrt
 */

#define one     1.00000000000000000000e+00 /* 0x3FF00000, 0x00000000 */
#define pi      3.14159265358979311600e+00 /* 0x400921FB, 0x54442D18 */
#define pio2_hi 1.57079632679489655800e+00 /* 0x3FF921FB, 0x54442D18 */
#define pio2_lo 6.12323399573676603587e-17 /* 0x3C91A626, 0x33145C07 */
#define pS0     1.66666666666666657415e-01 /* 0x3FC55555, 0x55555555 */
#define pS1     -3.25565818622400915405e-01 /* 0xBFD4D612, 0x03EB6F7D */
#define pS2     2.01212532134862925881e-01 /* 0x3FC9C155, 0x0E884455 */
#define pS3     -4.00555345006794114027e-02 /* 0xBFA48228, 0xB5688F3B */
#define pS4     7.91534994289814532176e-04 /* 0x3F49EFE0, 0x7501B288 */
#define pS5     3.47933107596021167570e-05 /* 0x3F023DE1, 0x0DFDF709 */
#define qS1     -2.40339491173441421878e+00 /* 0xC0033A27, 0x1C8A2D4B */
#define qS2     2.02094576023350569471e+00 /* 0x40002AE5, 0x9C598AC8 */
#define qS3     -6.88283971605453293030e-01 /* 0xBFE6066C, 0x1B8D0159 */
#define qS4     7.70381505559019352791e-02 /* 0x3FB3B8C5, 0xB12E9282 */

double
acos (double x)
{
  double z, p, q, r, w, s, c;
  int hx, ix;

  hx = __HI (x);
  ix = hx & 0x7fffffff;
  if (ix >= 0x3ff00000) /* |x| >= 1 */
  {
    if (((ix - 0x3ff00000) | __LO (x)) == 0) /* |x| == 1 */
    {
      if (hx > 0) /* acos(1) = 0  */
      {
        return 0.0;
      }
      else /* acos(-1) = pi */
      {
        return pi + 2.0 * pio2_lo;
      }
    }
    return NAN; /* acos(|x|>1) is NaN */
  }
  if (ix < 0x3fe00000) /* |x| < 0.5 */
  {
    if (ix <= 0x3c600000) /* if |x| < 2**-57 */
    {
      return pio2_hi + pio2_lo;
    }
    z = x * x;
    p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
    q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
    r = p / q;
    return pio2_hi - (x - (pio2_lo - x * r));
  }
  else if (hx < 0) /* x < -0.5 */
  {
    z = (one + x) * 0.5;
    p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
    q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
    s = sqrt (z);
    r = p / q;
    w = r * s - pio2_lo;
    return pi - 2.0 * (s + w);
  }
  else /* x > 0.5 */
  {
    double_accessor df;
    z = (one - x) * 0.5;
    s = sqrt (z);
    df.dbl = s;
    df.as_int.lo = 0;
    c = (z - df.dbl * df.dbl) / (s + df.dbl);
    p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
    q = one + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
    r = p / q;
    w = r * s + c;
    return 2.0 * (df.dbl + w);
  }
} /* acos */

#undef one
#undef pi
#undef pio2_hi
#undef pio2_lo
#undef pS0
#undef pS1
#undef pS2
#undef pS3
#undef pS4
#undef pS5
#undef qS1
#undef qS2
#undef qS3
#undef qS4
