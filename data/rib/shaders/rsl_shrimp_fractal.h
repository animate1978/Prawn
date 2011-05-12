#ifndef SHRIMP_FRACTAL_H
#define SHRIMP_FRACTAL_H 1

/* Some noise and Fractal functions.
 * These are based on the functions given in noises.h written by
 * Larry Gritz and distributed with BMRT. see www.bmrt.org
 * Used with permission. */

/* Note, some modifications were made to give shrimp some extra blocks */

#ifndef snoise

/* Signed noise -- the original Perlin kind with range [-1,1]. We prefer
 * signed noise to regular noise mostly because its average is zero.
 * We define three simple macros:
 *   snoise(p) - Perlin noise on either a 1-D (float) or 3-D (point) domain.
 *   snoisexy(x,y) - Perlin noise on a 2-D(float, float), or 4-D(point, float)
 *   domain.
 *   vsnoise(p) - vector-valued Perlin noise on either 1-D or 3-D domain.
 *   vsnoisexy(x,y) - vector-valued Perlin noise on a 2D or 4D domain.
 */

#define snoise(p) (2 * (float noise(p)) - 1)
#define snoisexy(x,y) (2 * (float noise(x,y)) - 1)
#define vsnoise(p) (2 * (vector noise(p)) - 1)
#define vsnoisexy(x,y) (2 * (vector noise(x,y)) - 1)
#endif

/* Filtered noise */
#define filterednoise(p,width) (float noise(p) * \
		(1-smoothstep (0.2,0.75,width)))

#define filterednoisexy(x,y,width) (float noise(x,y) * \
		(1-smoothstep (0.2,0.75,width)))

#define filteredvnoise(p,width) (vector noise(p) * \
		(1-smoothstep (0.2,0.75,width)))

#define filteredvnoisexy(x,y,width) (vector noise(x,y) * \
		(1-smoothstep (0.2,0.75,width)))

/* If we know the filter size, we can crudely antialias snoise by fading
 * to its average value at approximately the Nyquist limit.
 */
#define filteredsnoise(p,width) (snoise(p) * \
		(1-smoothstep (0.2,0.75,width)))

#define filteredsnoisexy(x,y,width) (snoisexy(x,y) * \
		(1-smoothstep (0.2,0.75,width)))

#define filteredvsnoise(p,width) (vsnoise(p) * \
		(1-smoothstep (0.2,0.75,width)))

#define filteredvsnoisexy(x,y,width) (vsnoisexy(x,y) * \
		(1-smoothstep (0.2,0.75,width)))

////////////////////////////////////////////////////////////////////////////////
/* Having filteredsnoise leads easily to an antialiased version of fBm. */
float fBm(	point p; float filtwidth;
			uniform float maxoctaves;
			float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, sum = 0, fw = filtwidth;
    varying point pp = p;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredsnoise( pp, fw );
		amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

float fBm(	float x, filtwidth;
			uniform float maxoctaves;
			float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, sum = 0, xx = x, fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredsnoise( xx, fw );
		amp *= gain;  xx *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

float fBm(	float x, y, filtwidth;
			uniform float maxoctaves;
			float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, xx = x, yy = y, sum = 0, fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredsnoisexy( xx, yy, fw );
		amp *= gain;  
		xx *= lacunarity;  yy *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

float fBm(	point p; float t, filtwidth;
			uniform float maxoctaves;
			float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, tt = t, sum = 0, fw = filtwidth;
    varying point pp = p;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredsnoisexy (pp, tt, fw);
		amp *= gain;  
		pp *= lacunarity;  tt *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

vector vfBm(	point p; float filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, fw = filtwidth;
    varying point pp = p;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoise( pp, fw );
		amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

vector vfBm(	float x, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, xx = x, fw = filtwidth;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoise( xx, fw );
		amp *= gain;  xx *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

vector vfBm(	float x, y, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, xx = x, yy = y, fw = filtwidth;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoisexy( xx, yy, fw );
		amp *= gain;  
		xx *= lacunarity; yy *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

vector vfBm(	point p; float t, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain; )
{
    uniform float i;
    varying float amp = 1, tt = t, fw = filtwidth;
    varying point pp = p;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoisexy( pp, tt, fw );
		amp *= gain;  
		pp *= lacunarity;  tt *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

/* Typical use of vfBm: 
#define vfBm_default(p)  vfBm (p, sqrt(area(p)), 4, 2, 0.5)
*/

////////////////////////////////////////////////////////////////////////////////
/* Ken Musgrave's Variable Lacunarity noise */

#define VLNoise(Pt,scale) ( snoise( vsnoise(Pt) * scale + Pt) )

#define VLNoisexy(x,y,scale) ( snoisexy( snoise(x)*scale+x,snoise(y)*scale+y))

#define filteredVLNoise(Pt,fwidth,scale) \
            ( filteredsnoise( filteredvsnoise( Pt,fwidth)*scale+Pt,fwidth))

#define filteredVLNoisexy(x,y,fwidth,scale) \
			( filteredsnoisexy( filteredsnoise( x,fwidth)*scale+x,\
			filteredsnoise( y,fwidth)*scale+y,fwidth))

////////////////////////////////////////////////////////////////////////////////
/* Variable Lacunarity fractional Brownian motion */
float VLfBm(	point p; float filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain, scale; )
{
    uniform float i;
    varying float amp = 1, sum = 0, fw = filtwidth;
    varying point pp = p;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredVLNoise (pp, fw, scale);
		amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

float VLfBm(	float x, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain, scale; )
{
    uniform float i;
    varying float amp = 1, xx = x, sum = 0, fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredVLNoise( xx, fw, scale );
		amp *= gain;  xx *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

float VLfBm(	float x, y, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain, scale; )
{
    uniform float i;
    varying float amp = 1, xx = x, yy = y, sum = 0, fw = filtwidth;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredVLNoisexy( xx, yy, fw, scale );
		amp *= gain;  
		xx *= lacunarity; yy *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

float VLfBm(	point p; float filtwidth;
				uniform float t, maxoctaves;
				float lacunarity,gain,scale;)
{
    uniform float i;
    varying float amp = 1, tt = t, sum = 0, fw = filtwidth;
    varying point pp = p;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredVLNoisexy( pp, tt, fw, scale );
		amp *= gain;  
		pp *= lacunarity; tt *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

////////////////////////////////////////////////////////////////////////////////
/* extra VLfBm's for color/vector types */
vector VLvfBm(	point p; float filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain, scale;)
{
    uniform float i;
    varying float amp = 1, fw = filtwidth;
    varying point pp = p;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoise( filteredvsnoise( pp, fw )
				* scale + pp, fw );
		amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

vector VLvfBm(	float x, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain, scale; )
{
    uniform float i;
    varying float amp = 1, xx = x, fw = filtwidth;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoise( filteredvsnoise( xx, fw)
				* scale + xx, fw);
		amp *= gain;  xx *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

vector VLvfBm(	float x, y, filtwidth;
				uniform float maxoctaves;
				float lacunarity, gain, scale;)
{
    uniform float i;
    varying float amp = 1, xx = x, yy = y, fw = filtwidth;
    varying vector sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoisexy( filteredsnoise( xx, fw) * scale + xx,
                                    filteredsnoise( yy, fw) * scale + yy, fw);
		amp *= gain;  
		xx *= lacunarity; yy *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

vector VLvfBm(	point p; float filtwidth;
				uniform float t, maxoctaves;
				float lacunarity,gain,scale;)
{
    uniform float i;
    varying float amp = 1, tt = t, fw = filtwidth;
    varying point pp = p;
	varying vector ppv = ( xcomp(pp), ycomp(pp), zcomp(pp) ), sum = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * filteredvsnoisexy( point( filteredsnoise( pp, fw) * scale
					+ ppv ), filteredsnoise( tt, fw ) * scale + tt, fw );
		amp *= gain;  
		pp *= lacunarity; tt *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

/* Typical use of vfBm: 
#define VLfBm_default(p)      VLfBm (p, sqrt(area(p)), 4, 2, 0.5, 1.0)
*/

////////////////////////////////////////////////////////////////////////////////
/* Antialiased turbulence.  Watch out -- the abs() call introduces infinite
 * frequency content, which makes our antialiasing efforts much trickier!
 */

float turbulence(	point p; float filtwidth;
					uniform float maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, fw = filtwidth, sum = 0;
    point pp = p;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * abs( filteredsnoise (pp, fw));
		amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

float turbulence(	float x, filtwidth;
					uniform float maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, xx = x, fw = filtwidth, sum = 0;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * abs( filteredsnoise (xx, fw));
		amp *= gain;  xx *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

float turbulence(	float x, y, filtwidth;
					uniform float maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, xx = x, yy = y, fw = filtwidth, sum = 0;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * abs( filteredsnoisexy (xx, yy, fw));
		amp *= gain;  
		xx *= lacunarity; yy *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

float turbulence(	point p; float filtwidth;
					uniform float t, maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, tt = t, fw = filtwidth, sum = 0;
    point pp = p;

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		sum += amp * abs( filteredsnoisexy (pp, tt, fw));
		amp *= gain;
		pp *= lacunarity; tt *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

/* vector version of turbulence */

vector vturbulence( point p; float filtwidth;
					uniform float maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, fw = filtwidth;
    point pp = p;
    vector sum = vector(0), store = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		store = filteredvsnoise( pp, fw );
		store = ( abs(xcomp(store)), abs(ycomp(store)), abs(zcomp(store)));
		sum += amp * store;
		amp *= gain;  pp *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

vector vturbulence(	float x, filtwidth;
					uniform float maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, xx = x, fw = filtwidth;
    vector sum = vector(0), store = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		store = filteredvsnoise( xx, fw );
		store = ( abs(xcomp(store)), abs(ycomp(store)), abs(zcomp(store)));
		sum += amp * store;
		amp *= gain;  xx *= lacunarity;  fw *= lacunarity;
    }
    return sum;
}

vector vturbulence(	float x, y, filtwidth;
					uniform float maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, xx = x, yy = y, fw = filtwidth;
    vector sum = vector(0), store = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		store = filteredvsnoisexy( xx, yy, fw );
		store = ( abs(xcomp(store)), abs(ycomp(store)), abs(zcomp(store)));
		sum += amp * store;
		amp *= gain;
		xx *= lacunarity; yy *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

vector vturbulence( point p; float filtwidth;
					uniform float t, maxoctaves;
					float lacunarity, gain;)
{
    uniform float i;
    varying float amp = 1, tt = t, fw = filtwidth;
    point pp = p;
    vector sum = vector(0), store = vector(0);

    for (i = 0;  i < maxoctaves && fw < 1.0;  i += 1) {
		store = filteredvsnoisexy( pp, tt, fw );
		store = ( abs(xcomp(store)), abs(ycomp(store)), abs(zcomp(store)));
		sum += amp * store;
		amp *= gain;
		pp *= lacunarity; tt *= lacunarity; fw *= lacunarity;
    }
    return sum;
}

/* Typical use of turbulence: 
#define turbulence_default(p)  turbulence (p, sqrt(area(p)), 4, 2, 0.5)
*/

////////////////////////////////////////////////////////////////////////////////
/* Ken Musgrave's Ridged MultiFractal
 * For an explanation see "Texturing and Modelling" chap 9
 * */ 
float Ridged( 	point p; 
				float filtwidth;
				uniform float maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1.0, value = 0, signal = 0, fw = filtwidth, weight = 1;
	varying point pp = p;

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = abs( filteredsnoise( pp, fw ) );
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		pp *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, 0.0, 1.0 );
	}
	return value;
}

float Ridged( 	float x, filtwidth;
				uniform float maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1, value = 0, signal = 0, xx = x, weight = 1;
	varying float fw = filtwidth;

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = abs( filteredsnoise( xx, fw ) );
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		xx *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, 0.0, 1.0 );
	}
	return value;
}

float Ridged( 	float x, y, filtwidth;
				uniform float maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1, value = 0, signal = 0, xx = x, yy = y, weight = 1;
	varying float fw = filtwidth;

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = abs( filteredsnoisexy( xx, yy, fw ) );
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		xx *= lacunarity; yy *= lacunarity;
		fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, 0.0, 1.0 );
	}
	return value;
}

float Ridged( 	point p; float filtwidth;
				uniform float t, maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1, value = 0, signal = 0, tt = t, weight = 1;
	varying float fw = filtwidth;
	varying point pp = p;

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = abs( filteredsnoisexy( pp, tt, fw ) );
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		pp *= lacunarity; tt *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, 0.0, 1.0 );
	}
	return value;
}

/* vector version of ridged */
vector vRidged( point p; float filtwidth;
				uniform float maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1, fw = filtwidth;
	vector value = vector(0), signal = vector(0), weight = vector(1);
	point pp = p;

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
        signal = filteredvsnoise( pp, fw );
        signal = ( abs(xcomp(signal)), abs(ycomp(signal)), abs(zcomp(signal)));
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		pp *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, vector(0), vector(1) );
	}
	return value;
}

vector vRidged( float x, filtwidth;
				uniform float maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1, xx = x, fw = filtwidth;
	vector value = vector(0), signal = vector(0), weight = vector(1);

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = filteredvsnoise( xx, fw );
        signal = ( abs(xcomp(signal)), abs(ycomp(signal)), abs(zcomp(signal)));
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		xx *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, vector(0), vector(1) );
	}
	return value;
}

vector vRidged(	float x, y, filtwidth;
				uniform float maxoctaves;
				float lacunarity, offset, gain; )
{
	uniform float i;
	varying float amp = 1, xx = x, yy = y, fw = filtwidth;
	vector value = vector(0), signal = vector(0), weight = vector(1);

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = filteredvsnoisexy( xx, yy, fw );
        signal = ( abs(xcomp(signal)), abs(ycomp(signal)), abs(zcomp(signal)));
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		xx *= lacunarity; yy *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, vector(0), vector(1) );
	}
	return value;
}

vector vRidged( point p; float filtwidth;
				uniform float t, maxoctaves;
				float lacunarity, offset, gain )
{
	uniform float i;
	varying float amp = 1, tt = t, fw = filtwidth;
	vector value = vector(0), signal = vector(0), weight = vector(1);
	point pp = p;

	for( i = 0; i < maxoctaves && fw < 1.0; i += 1 )
	{
		signal = filteredvsnoisexy( pp, tt, fw );
        signal = ( abs(xcomp(signal)), abs(ycomp(signal)), abs(zcomp(signal)));
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		value += signal * amp;
		amp *= gain;
		pp *= lacunarity; tt *= lacunarity; fw *= lacunarity;
		weight = signal * 2.0;
		weight = clamp( weight, vector(0), vector(1) );
	}
	return value;
}

/* Typical use of ridged: 
#define ridged_default(p)  ridged (p, sqrt(area(p)), 4, 2, 0.7, 0.5)
*/

////////////////////////////////////////////////////////////////////////////////
// Voronoi (Worley) cell noise tweaked, with Minkowski distance added //////////
////////////////////////////////////////////////////////////////////////////////

/* Voronoi cell noise (a.k.a. Worley noise) -- 3-D, 1-feature version. */
void
voronoi_f1_3d(  point P;
	            float jitter;
				uniform float dtype; /* 0=tchebychev,1=manhattan,2=euclidian*/
	            output varying float f1;
	            output varying point pos1;
    )
{
    point thiscell = point( floor(xcomp(P))+0.5, floor(ycomp(P))+0.5,
			floor(zcomp(P))+0.5);
    f1 = 1000;
    uniform float i, j, k;
	float dx, dy, dz, dist = 0;

    for (i = -1;  i <= 1;  i += 1) {
        for (j = -1;  j <= 1;  j += 1) {
            for (k = -1;  k <= 1;  k += 1) {
		        point testcell = thiscell + vector(i,j,k);
                point pos = testcell + 
		            jitter * (vector cellnoise (testcell) - 0.5);
			
				/* absolute delta components */
                vector offset = pos-P;
				dx = abs(xcomp(offset));
				dy = abs(ycomp(offset));
				dz = abs(zcomp(offset));
				
				/* dtype = 0 = Tchebychev distanc metric, non 0 = Minkowski
				 * distance metric, dtype being Minkowski's P parameter, for 
				 * P = 1 = Euclidian distance metric, P = 2 = Manhattan
				 * distance metric, and allowing higher orders of P, increasing
				 * towards Tchebychev results. */
				if (dtype == 0) {
					dist = max( dx, dy, dz );
				}
				if (dtype == 1) {
					dist = dx + dy + dz;
				}			
				if (dtype >= 2) {
					dist = pow( pow( dx, dtype) + pow( dy, dtype) +
								pow( dz, dtype), 1 / dtype);
				}
                dist *= dist;

				if (dist < f1 ) {
                    f1 = dist;  pos1 = pos;
                }
            }
	    }
    }
    f1 = sqrt(f1);
}

/* Voronoi cell noise (a.k.a. Worley noise) -- 3-D, 2-feature version. */
void
voronoi_f1f2_3d(    point P;
		            float jitter;
                    uniform float dtype; /* metric type, see notes */
					output varying float f1, f2;
					output varying point pos1, pos2;
    )
{
    point thiscell = point( floor(xcomp(P))+0.5, floor(ycomp(P))+0.5,
			floor(zcomp(P))+0.5);

	pos1 = pos2 = 0;
    f1 = f2 = 1000;
    uniform float i, j, k;
    float dx, dy, dz, dist = 0;

    for (i = -1;  i <= 1;  i += 1) {
        for (j = -1;  j <= 1;  j += 1) {
            for (k = -1;  k <= 1;  k += 1) {
        		point testcell = thiscell + vector(i,j,k);
                point pos = testcell + 
		            jitter * (vector cellnoise (testcell) - 0.5);

                /* absolute delta components */
		        vector offset = pos - P;
                dx = abs(xcomp(offset));
                dy = abs(ycomp(offset));
                dz = abs(zcomp(offset));
				
				/* dtype = 0 = Tchebychev distance metric, non 0 = Minkowski,
				 * dtype being Minkowski's P parameter, for P = 1 = Euclidian
				 * distance metric, P = 2 = Manhattan distance metric, and
				 * allowing higher orders of P, increasing towards Tchebychev
				 * results */
				if (dtype == 0) {
					dist = max( dx, dy, dz );
				}
				if (dtype == 1) {
					dist = dx + dy + dz;
				}
				if (dtype >= 2) {
					dist = pow( pow( dx, dtype) + pow( dy, dtype) +
							pow( dz, dtype), 1 / dtype);
				}
				dist *= dist;

                if (dist < f1) {
                    f2 = f1;  pos2 = pos1;
                    f1 = dist;  pos1 = pos;
                } else if (dist < f2) {
                    f2 = dist;  pos2 = pos;
		        }
            }
	    }
    }
    f1 = sqrt(f1);  f2 = sqrt(f2);
}

/* Voronoi cell noise (a.k.a. Worley noise) -- 2-D, 1-feature version. */
void
voronoi_f1_2d(  float ss, tt;
	            float jitter;
                uniform float dtype; /* metric type, see notes */
	            output varying float f1;
	            output varying float spos1, tpos1;
    )
{
    float sthiscell = floor(ss)+0.5, tthiscell = floor(tt)+0.5;
    f1 = 1000;
    uniform float i, j;

    for (i = -1;  i <= 1;  i += 1) {
	float stestcell = sthiscell + i;
        for (j = -1;  j <= 1;  j += 1) {
	    float ttestcell = tthiscell + j;
	    float spos = stestcell +
		     jitter * (float cellnoise(stestcell, ttestcell) - 0.5);
	    float tpos = ttestcell +
		 jitter * (float cellnoise(stestcell+23, ttestcell-87) - 0.5);

        /* absolute delta components */
	    float dx = abs(spos - ss);
	    float dy = abs(tpos - tt);
		float dist = 0;

		/* dtype = 0 = Tchebychev distance metric, non 0 dtype = Minkowski
		 * distance metric, dtype being Minkowski's P parameter, for P = 1 =
		 * Euclidian distance metric, P = 2 = Manhattan distance metric, and
		 * allowing higher orders of P, increasing towards Tchebychev. */
		if (dtype == 0) {
			dist = max( dx, dy );
		}
		if (dtype == 1) {
			dist = dx + dy;
		}
		if (dtype >= 2) {
			dist = pow( pow( dx, dtype) + pow( dy, dtype), 1 / dtype);
		}
		dist *= dist;
        
	    if (dist < f1) {
		f1 = dist;
		spos1 = spos;  tpos1 = tpos;
    	    }
	    }
    }
    f1 = sqrt(f1);
}

/* Voronoi cell noise (a.k.a. Worley noise) -- 2-D, 2-feature version. */
void
voronoi_f1f2_2d(    float ss, tt;
		            float jitter;
					uniform float dtype; /* metric type, see notes */
					output varying float f1, f2;
					output varying float spos1, tpos1, spos2, tpos2;
    )
{
    float sthiscell = floor(ss)+0.5, tthiscell = floor(tt)+0.5;
	spos1 = tpos1 = spos2 = tpos2 = 0;
    f1 = f2 = 1000;
    uniform float i, j;

    for (i = -1;  i <= 1;  i += 1) {
	float stestcell = sthiscell + i;
        for (j = -1;  j <= 1;  j += 1) {
	    float ttestcell = tthiscell + j;
	    float spos = stestcell +
		   jitter * (cellnoise(stestcell, ttestcell) - 0.5);
	    float tpos = ttestcell +
		   jitter * (cellnoise(stestcell+23, ttestcell-87) - 0.5);

        /* absolute delta components */
	    float dx = abs(spos - ss);
	    float dy = abs(tpos - tt);
		float dist = 0;

		/* dtype = 0 = Tchebychev distance metric, non 0 = Minkowski, dtype
		 * being Minkowski's P parameter, for P = 1 = Euclidian distance
		 * metric, P = 2 = Manhattan distance metric, and allowing higher
		 * orders of P, increasing towards Tchebychev results. */
		if (dtype == 0) {
			dist = max( dx, dy );
		}
		if (dtype == 1) {
			dist = dx + dy;
		}
		if (dtype >= 2) {
			dist = pow( pow( dx, dtype) + pow( dy, dtype), 1 / dtype);
		}
        dist *= dist;

	    if (dist < f1) {
		f2 = f1;  spos2 = spos1;  tpos2 = tpos1;
		f1 = dist;  spos1 = spos;  tpos1 = tpos;
	    } else if (dist < f2) {
		f2 = dist;
		spos2 = spos;  tpos2 = tpos;
    	    }
	    }
    }
    f1 = sqrt(f1);  f2 = sqrt(f2);
}

////////////////////////////////////////////////////////////////////////////////
/* Some small, commonly needed macros and functions.
 * These are based on, and replace, the similar functions found
 * in patterns.h by Larry Gritz as distributed with BMRT.
 * Some of the macros are also based on similar ones given in
 * rmannotes.sl by Steve May,
 * http://accad.osu.edu/~smay/RManNotes/rmannotes.html
 * or taken from Darwyn Peachey's chapter
 * in "Texturing and Modelleling" by Ebert et al.
 * */

/* signed noise
 *
 */
//#define snoise(x)    (noise(x) * 2 - 1)
#define snoise2(x,y) (noise(x,y) * 2 - 1)

////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_FRACTAL_H */

