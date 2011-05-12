/* RSL version of the OdForce Wiki's WikiComplex.h file, and the OdWiki article
 * on Reflectance Functions, by Mario Marengo:
 * http://odforce.net/wiki/index.php/ReflectanceFunctions
 * Many Thanks to Jason, Marc, Mario and everyone at Odforce:
 * http://www.odforce.ne
 * */

/****************************************************************************
          File:  wikiComplex.h
       Authors:  The od[wiki] community at
                 http://www.odforce.net/wiki
 
   Description:  VEX package for complex number arithmetic.

                 Complex numbers in this VEX implementation are 
                 represented using the 'vector' data type, where 
                 the x-component is used for the real part, and 
                 the y-component for the imaginary part. However; the
                 module also defines the data type 'complex' so as to
                 make application code a little clearer and make the 
                 underlying vector type transparent -- i.e: application 
                 code should use the type 'complex' after including
                 this module.

                 Wherever it makes sense, functions are given alternative
                 versions using the suffixes 'R' (Real) and 'C' (complex)
                 to denote various combinations of data types for their
                 arguments. This lets us do complex arithmetic with mixed
                 data types... but due to the fact that we can't overload,
                 the function names start getting a little complicated, but
                 the overall pattern is consistent.

 -----------------------------------------------------------------------------
   Part of the small but growing od[wiki] library of VEX functions...
   This software is placed in the public domain and is provided as is
   without express or implied warranty.
 *****************************************************************************/

#ifndef ODWIKI_COMPLEX_H
#define ODWIKI_COMPLEX_H 1

/* Some tweaks were needed to fit Shrimp's structure */
#include "rsl_helpers.h"

////////////////////////////////////////////////////////////////////////////////
// Complex data type
#ifndef complex
	#define complex vector
#endif

////////////////////////////////////////////////////////////////////////////////
// Real part of the complex number z
float cx_re( complex z; ) {
	return xcomp(z);
}

////////////////////////////////////////////////////////////////////////////////
// Imaginary part of the complex number z
float cx_im( complex z; ) {
	return ycomp(z);
}

////////////////////////////////////////////////////////////////////////////////
// Create a complex number
complex cx_set( float re, im; )  {
	return vector( re, im, 0 );
}

complex cx_setR( float re; ) {
	return vector( re, 0, 0 );
}

complex cx_setV ( vector z; ) {
	return vector( xcomp(z), ycomp(z), 0 );
}

////////////////////////////////////////////////////////////////////////////////
// The distance from origin in polar coords, -- a.k.a. 'complex modulus', or
// 'norm', or 'length'.
float cx_mod( complex z; ) {
	float l = 0.0;
	if ( xcomp(z) != 0.0 || ycomp(z) != 0.0) {
		l = length( vector( xcomp(z), ycomp(z), 0.0 ));
	}
	return l;
}

float cx_mod( float x; ) {
	return x < 0.0 ? -x : x;
}

float cx_mod2( complex z; ) {
	float l = 0.0;
	if ( xcomp(z) != 0.0 || ycomp(z) != 0.0) {
		l = length( vector( xcomp(z), ycomp(z), 0.0 ));
		l = l*l;
	}
	return l;
}

float cx_mod2R( float x; ) {
	return x*x;
}

#define cx_norm			cx_mod
#define cx_normR		cx_mod
#define cx_norm2		cx_mod2
#define cx_norm2R		cx_mod2R
#define cx_length		cx_mod
#define cx_lengthR		cx_mod
#define cx_length2		cx_mod2
#define cx_length2R		cx_mod2R

////////////////////////////////////////////////////////////////////////////////
// Absolute value (the magnitude of the complex number)
// An alternative implementation of cx_mod adapted from Numerical Recipes.
float cx_abs( complex z; ) {
	float result;
	float x = abs( xcomp(z)), y = abs( ycomp(z)), d;
	if ( x == 0.0 && y == 0.0) {
		result = 0.0;
	} else if (x >= y) {
		d = y / x;
		result = x * sqrt( 1.0 + d*d );
	} else {
		d = x / y;
		result = y * sqrt( 1.0 + d*d );
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// 'Argument' (angle in radians with the x-axis in polar coords)
float cx_arg( complex z; ) {
	return atan( ycomp(z), xcomp(z) );
}

////////////////////////////////////////////////////////////////////////////////
// Conjugate (mult of conjugate pairs produce a real)
// Conjugate appears "flipped" across the real axis.
complex cx_conj( complex z; ) {
	return cx_set( xcomp(z), -ycomp(z));
}

////////////////////////////////////////////////////////////////////////////////
// Inverse
complex cx_inv( complex z; ) {
	float re, im, ss;
	float zx = xcomp(z), zy = ycomp(z);

	if ( abs(zx) >= abs(zy)) {
		ss = 1.0 / (zx + zy * (zy / zx));
		re = ss;
		im = ss * (-zy / zx);
	} else {
		ss = 1.0 / (zx * (zx / zy) + zy);
		re = ss * (zx / zy);
		im = -ss;
	}
	return cx_set( re, im );
}

////////////////////////////////////////////////////////////////////////////////
// Negation
complex cx_neg( complex z; ) {
	return cx_set( -xcomp(z), -ycomp(z) );
}

complex cx_neg( float x; ) {
	return cx_setR( -x );
}

////////////////////////////////////////////////////////////////////////////////
// Addition (a+b)
complex cx_add( complex a; float b; ) {
	return cx_set( xcomp(a) + b, ycomp(a) );
}

complex cx_add( float a; complex b; ) {
	return cx_set( a + xcomp(b), ycomp(b) );
}

complex cx_add( complex a, b; ) {
	return cx_set( xcomp(a) + xcomp(b), ycomp(a) + ycomp(b) );
}

////////////////////////////////////////////////////////////////////////////////
// Subtraction (a-b)
complex cx_sub( complex a; float b; ) {
	return cx_set( xcomp(a) - b, ycomp(a) );
}

complex cx_sub( float a; complex b; ) {
	return cx_set( a - xcomp(b), -ycomp(b) );
}

complex cx_sub( complex a, b; ) {
	return cx_set( xcomp(a) - xcomp(b), ycomp(a) - ycomp(b) );
}

////////////////////////////////////////////////////////////////////////////////
// Multiplication (a*b)
complex cx_mul( complex a; float b; ) {
	return cx_set( xcomp(a) * b, ycomp(a) * b);
}

complex cx_mul( float a; complex b; ) {
	return cx_set( a * xcomp(b), a * ycomp(b) );
}

complex cx_mul( complex a, b; ) {
	return cx_set( xcomp(a) * xcomp(b) - ycomp(a) * ycomp(b),
			xcomp(a) * ycomp(b) + ycomp(a) * xcomp(b) );
}

////////////////////////////////////////////////////////////////////////////////
// Division (a/b)
// Adapted from Numerical Recipes
// This does not check for division by zero!
complex cx_div( complex a; float b; ) {
	float l2 = b * b;
	return l2 == 0 ? complex(0) : cx_set( ( xcomp(a) * b) / l2,
			( ycomp(a) * b) / l2 );
}

complex cx_div( complex a, b; ) {
	float re, im, ss;
	float ax = xcomp(a), ay = ycomp(a),
		  bx = xcomp(b), by = ycomp(b);

	if (  abs(bx) >= abs(by)) {
		float byox = by / bx;
		ss = 1.0 / (bx + by * byox);
		re = ss * (ax + ay * byox);
		im = ss * (ay - ax * byox);
	} else {
		float bxoy = bx / by;
		ss = 1.0 / (bx * bxoy + by);
		re = ss * (ax * bxoy + ay);
		im = ss * (ay * bxoy - ax);
	}
	return cx_set( re, im );
}

complex cx_div( float c; complex d; ) {
	return cx_div( cx_setR(c), d );
}

////////////////////////////////////////////////////////////////////////////////
// Exponential
complex cx_exp( complex z; ) {
	return cx_set( exp( xcomp(z)) * cos( ycomp(z)),
			exp( xcomp(z)) * sin( ycomp(z)) );
}

complex cx_exp( float x; ) {
	return cx_setR( exp( x ));
}

////////////////////////////////////////////////////////////////////////////////
// Power
// complex raised to complex power
complex cx_pow( complex base, expo; ) {
	float expx = xcomp(expo), expy = ycomp(expo),
		  re = log( cx_abs( base )),
		  im = cx_arg( base ),
		  re2 = (re * expx) - (im * expy),
		  im2 = (re * expy) + (im * expx),
		  ere = exp( re2 );
	return cx_set( ere * cos(im2), ere * sin(im2));
}

// complex raised to a real power
complex cx_pow( complex base; float expo; ) {
	float re = log( cx_abs( base )),
		  im = cx_arg( base ),
		  ere = exp( re );
	return cx_set( ere * cos(im), ere * sin(im));
}

// real raised to a complex power
complex cx_pow( float base; complex expo; ) {
	float expx = xcomp(expo), expy = ycomp(expo),
		  re = log( cx_abs( complex(base) )),
		  im = S_2PI, /* PI*2 */
		  re2 = (re * expx) - (im * expy),
		  im2 = (re * expy) + (im * expx),
		  ere = exp( re2 );
	return cx_set( ere * cos(im2), ere * sin(im2));
}

////////////////////////////////////////////////////////////////////////////////
// Natural Logarithm (principal branch, where {-pi < arg <= pi} )
complex cx_log( complex z; ) {
	return cx_set( log( cx_mod(z)), cx_arg(z));
}

complex cx_log( float x; ) {
	return cx_setR( log(x) );
}

////////////////////////////////////////////////////////////////////////////////
// Square Root (one of the two roots)
complex cx_sqrt( complex z; ) {
	float re, im, tmp;
	float zx = xcomp(z), zy = ycomp(z);
	float mag = cx_abs( z );

	if ( mag > 0.0 ) {
		if ( zx > 0.0 ) {
			tmp = sqrt( 0.5 * (mag + zx));
			re = tmp;
			im = 0.5 * zy / tmp;
		} else {
			tmp = sqrt( 0.5 * (mag - zx));
			if ( zy < 0.0) tmp = -tmp;
			re = 0.5 * zy / tmp;
			im = tmp;
		}
	} else {
		re = im = 0.0;
	}
	return cx_set( re, im );
}

complex cx_sqrt( float x; ) {
	complex z;
	if (x >= 0.0) {
		z = x == 0.0 ? complex(0) : cx_setR( sqrt(x));
	} else {
		z = cx_sqrt( cx_setR(x));
	}
	return z;
}

////////////////////////////////////////////////////////////////////////////////
// Nth Root
complex cx_root( complex z; float n; ) {
	float p = 1.0 / n;
	float th_n = cx_arg(z) * p;
	return cx_set( pow( cx_mod(z), p) * cos(th_n), sin(th_n));
}

complex cx_root( float x, n; ) {
	return cx_root( cx_setR(x), n);
}

////////////////////////////////////////////////////////////////////////////////
// Sine
complex cx_sin( complex z; ) {
	float zx = xcomp(z), zy = ycomp(z);
	return cx_set( cosh(zy) * sin(zx), sinh(zy) * cos(zx));
}

complex cx_sin( float x; ) {
	return cx_setR( sin(x));
}

////////////////////////////////////////////////////////////////////////////////
// Cosine
complex cx_cos( complex z; ) {
	float zx = xcomp(z), zy = ycomp(z);
	return cx_set( cosh(zy) * cos(zx), -sinh(zy) * sin(zx));
}

complex cx_cos( float x; ) {
	return cx_setR( cos(x));
}

////////////////////////////////////////////////////////////////////////////////
// Tangent
complex cx_tan( complex z; ) {
	return cx_div( cx_sin(z), cx_cos(z));
}

complex cx_tan( float x; ) {
	return cx_setR( tan(x));
}

////////////////////////////////////////////////////////////////////////////////
// Hyperbolic Sine
complex cx_sinh( complex z; ) {
	float zx = xcomp(z), zy = ycomp(z);
	return cx_set( sinh(zx) * cos(zy), cosh(zx) * sin(zy));
}

complex cx_sinh( float x; ) {
	return cx_setR( sinh(x));
}

////////////////////////////////////////////////////////////////////////////////
// Hyperbolic Cosine
complex cx_cosh( complex z; ) {
	float zx = xcomp(z), zy = ycomp(z);
	return cx_set( cosh(zx) * cos(zy), sinh(zx) * sin(zy));
}

complex cx_cosh( float x; ) {
	return cx_setR( cosh(x));
}

////////////////////////////////////////////////////////////////////////////////
// Fresnel factors (Kr and Kt) using full formula for unpolarized light
float fresnel_kr(	float cosalpha;	complex ior; )
{
	float kr = 1.0;
	if (cosalpha > 0.0) {
		float c2 = cosalpha * cosalpha;
		complex g2 = cx_add( cx_sub( cx_mul( ior, ior ), 1), c2);
		float ag2 = cx_abs( g2 ),
			  a = sqrt( .5 * (ag2 + cx_re( g2 ))),
			  b = sqrt( .5 * (ag2 - cx_re( g2 ))),
			  b2 = b * b,
			  ss = (1.0 - c2) / cosalpha,
			  amc = a - cosalpha,
			  apc = a + cosalpha,
			  ams = a - ss,
			  aps = a + ss;

		kr = .5 * ((amc * amc + b2) / (apc * apc + b2)) *
			(1.0 + ((ams * ams + b2) / (aps * aps + b2)));
	}
	return kr;
}

float fresnel_kt(	float cosalpha;	complex ior; )
{
	return max( 0.0, 1.0 - fresnel_kr( cosalpha, ior ));
}

////////////////////////////////////////////////////////////////////////////////
#endif /* ODWIKI_COMPLEX_H */

