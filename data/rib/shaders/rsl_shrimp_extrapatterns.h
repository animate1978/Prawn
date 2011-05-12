#ifndef SHRIMP_EXTRA_PATTERNS_H
#define SHRIMP_EXTRA_PATTERNS_H 1

#include "rsl_shrimp_conversion.h"
#include "rsl_shrimp_fractal.h"
#include "rsl_shrimp_helpers.h"

////////////////////////////////////////////////////////////////////////////////
// Penrose rhomb tiling ////////////////////////////////////////////////////////
/* By Derek Ledbetter
 * based on the projection method described in: N.G. de Bruijn,
 * "Algebraic theory of Penrose's non-periodic tilings of the plane",
 * Proceedings of the Koninklijke nederlandse Akademie van Wetenschappen,
 * Series A, 84, 1981 */

/* slightly tweaked to fit Shrimp's structure */

void
find_penrose_rhomb(
                	output float outTileX,outTileY;
	                output float outTileType; /* 0 = skinny, 1 = fat */
	                float s,t;
                    )

{
	uniform float basisS[5] = { 1, -S_COS36, S_COS72, S_COS72, -S_COS36 };
	uniform float basisT[5] = { 0, -S_SIN36, S_SIN72, -S_SIN72, S_SIN36 };
	
	/* can't initialize arrays with a single element in Pixie or Aqsis */
	/* initialize  originalPt & pt just to get rid of the warnings */
#if RENDERER == pixie || RENDERER == aqsis
	float originalPt[5] = { 0, 0, 0, 0, 0 };
	float pt[5] = { 0, 0, 0, 0, 0 };
#else
	float originalPt[5] = 0;
	float pt[5] = 0;
#endif
	
	uniform float i;
	for( i = 0; i < 5; i += 1 )
	{
		float p = .4 * ( basisS[i] * s + basisT[i] * t) + S_PI;
		originalPt[i] = mod( p + 1.25, 1 ) - 1.25;
	}
	
	float bestDistance = 1000;
	
	uniform float shift, rotation, tileType;
	for( shift = 0; shift < 32; shift += 1 )
	{
		uniform float remainingShift = shift;
		for( i = 0; i < 5; i += 1 )
		{
			uniform float thisShift = mod( remainingShift, 2 );
			remainingShift = ( remainingShift - thisShift ) / 2;
			pt[i] = originalPt[i] + thisShift;
		}

		for( rotation = 0; rotation < 5; rotation += 1 )
		{
			uniform float factor = S_PHI;
			for( tileType = 0; tileType < 2; tileType += 1 )
			{
				uniform float
					index1 = 0,
					index2 = ( tileType + 1 ),
					index3 = 2 * ( tileType + 1 ),
					indexX = 5 - 2 * ( tileType + 1 ),
					indexY = 5 - ( tileType + 1 );
				
				float 
					boxPt0 = -( pt[index1] + pt[indexX] + factor*pt[indexY] ),
					boxPt1 = -pt[index2] + factor*( pt[indexX] + pt[indexY] ),
					boxPt2 = -( pt[index3] + pt[indexY] + factor*pt[indexX] );
				
				if(0 <= boxPt0 && 0 <= boxPt1 && 0 <= boxPt2
					&& 1 >= boxPt0 && 1 >= boxPt1 && 1 >= boxPt2 )
				{
					float boxPt3 = pt[indexX] - pt[index1]
						+ factor * ( pt[index2] - pt[index3] );
					float boxPt4 = pt[indexY] - pt[index3]
						+ factor * ( pt[index2] - pt[index1] );
	
					float thisDistance = max( abs(boxPt3 - 0.5),
							abs(boxPt4 - 0.5));
	
					if( bestDistance > thisDistance )
					{
						bestDistance = thisDistance;
						outTileX = boxPt3;
						outTileY = boxPt4;
						outTileType = tileType;
					}
				}
				factor = 1 - factor; /* change phi to -1/phi */
			}
			
			/* rotate pt */
			float temp = pt[0];
			
			for( i = 0; i < 4; i += 1 )
			{
				pt[i] = pt[i+1];
			}
			pt[4] = temp;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Painted Penrose rhomb tiling, by Derek Ledbetter ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color
painted_tiling(
                float sx, sy, tsize;
                color cskinny, cfat;
        )

{
    float scaledS = sx / tsize,
          scaledT = sy / tsize;

    float tileX =0 , tileY = 0, tileType = 0;

    find_penrose_rhomb( tileX, tileY, tileType, scaledS, scaledT );

    float distanceToBoundary = min( tileX, 1-tileX, tileY, 1-tileY);
    float intensity = smoothstep( 0, .1, distanceToBoundary );

	/* Workaround for ternary operator in Aqsis (cannot find a suitable
	 * cast for the expression) */
#if RENDERER == aqsis
	color tileColor = color(0);
	if (tileType == 0) {
		tileColor = intensity * cskinny;
	} else {
		tileColor = intensity * cfat;
	}
#else
    color tileColor = intensity *
        ( tileType == 0 ? cskinny : cfat );
#endif
    
    return tileColor;
}

////////////////////////////////////////////////////////////////////////////////
// Larry Gritz's LGVeinedMarble ////////////////////////////////////////////////
// from The RenderMan Repository , http://www.renderman.org ////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * greenmarble.sl -- RenderMan compatible shader for green veined marble.
 *
 * DESCRIPTION:
 *   Makes a marble-like surface using a turbulence function.
 *   
 * 
 * PARAMETERS:
 *   Ka, Kd, Ks, roughness, specularcolor - work just like the plastic
 *   txtscale - overall scaling for the texture
 *   darkcolor, lightcolor - colors of the underlying substrate
 *   veincolor - color of the bright veins
 *   veinfreq - controls the frequency of the veining effects
 *   sharpness - how sharp the veins appear
 *
 *
 * AUTHOR: Larry Gritz, the George Washington University
 *         email: gritz AT seas DOT gwu DOT edu 
 *
 *
 * last modified  11 July 1994 by Larry Gritz
 */

/* slightly tweaked for shimp */

color
veinedmarble(
                float veinfreq, veinlevels, warpfreq, warping, sharpness;
                color basecolor, veincolor;
				point ppn;
        )

{

    float turb, freq, j, turbsum;
	uniform float i;
    point PP, offset;

    PP = transform( "shader", ppn);

    freq = 1;
	offset = 0;

    for( i = 0; i < 6; i += 1) {
        offset += 2 * warping * (point noise( warpfreq * freq * PP) - 0.5) /
            freq;
        freq *= 2;
    }
    PP += offset;

    /* Now calculate the veining function for the lookup area */
    turbsum = 0;
	freq = 1;
    PP *= veinfreq;
    for (i = 0; i < veinlevels; i += 1) {
        turb = abs( snoise( PP ) );
        turb = pow( smoothstep( 0.8, 1, 1 - turb), sharpness) / freq;
        turbsum += (1-turbsum) * turb;
        freq *= 3;
        PP *= 3;
    }

    return mix( basecolor, veincolor, turbsum);
}

////////////////////////////////////////////////////////////////////////////////
// Mandelbrot fractal shader, by Michael Rivero ////////////////////////////////
// from The RenderMan Repository , http://www.renderman.org ////////////////////
////////////////////////////////////////////////////////////////////////////////

/* slightly tweaked for shrimp */

/*
 * MRmand.sl -- Mandelbrot texture
 *
 * DESCRIPTION:
 * Creates a Mandelbrot set as a texture
 *
 * PARAMETERS:
 * Ka, Kd, Ks, roughness - the usual
 *
 * HINTS:
 *
 * AUTHOR: Michael Rivero
 * rivero AT squareusa DOT com 
 *
 * History:
 * Created: 12/18/98
 */

color mrrand(
                float maxiteration, useuv;
        )
{   
    extern float u, v, s, t;
    float lt, ls;
    
    color C = color(0);
    
    /* Map the U and V values to the bounds of the Mandelbrot set. */

    if( useuv != 0.0)
    {
        lt = v;
        ls = u;
        /* printf("lt = %f   ls = %f\n, lt, ls); */
    }
    else
    {
        lt = t;
        ls = s;
        /* printf("lt = %f   ls = %f   pre correction   ", lt, ls); */
        ls = mod( ls, 1.0 );
        lt = mod( lt, 1.0 );
        /* printf("lt = %f   ls = %f   post correction \n", lt, ls); */
    }

    float real = ( ls * 3.0 ) - 2.0;
    float imag = ( lt * 3.0 ) - 1.5;

    float tmpval = 0, iter = 0,
          r2 = 0, tmpreal = 0, tmpimag = 0;
    float got_away = 0;
    float creal = real;
    float cimag = imag;

    color whitecolor = color(1);
    color blackcolor = color(0);

    for ( iter = 0; iter < maxiteration && got_away == 0; iter += 1)
    {
        /* z = z^2 + c */
        tmpreal = real;
        tmpimag = imag;
        real = (tmpreal * tmpreal) - (tmpimag * tmpimag);
        imag = 2 * tmpreal * tmpimag;
        real += creal;
        imag += cimag;
        r2 = (real * real) + (imag * imag);

        if ( r2 >= 4) {
            got_away = 1;
        }
    }
    
    if ( iter >= maxiteration ) {
        C = blackcolor;
    } else {
        tmpval = mod( (iter / 10), 1.0 );
    }
    C = tmpval;
    return C;
}

////////////////////////////////////////////////////////////////////////////////
// Larry Gritz's Hexagonal tile shader - LGHexTile.sl //////////////////////////
// from The RenderMan Repository , http://www.renderman.org ////////////////////
////////////////////////////////////////////////////////////////////////////////

/* tweaked for shrimp's needs & structure */

/*
 * hextile.sl -- surface shader for hexagonal tiles in st space
 *
 * DESCRIPTION
 *       This surface shader operates in s-t space and gives a pattern of
 *    hexagonal tiles, similar to that found as floor patterns in public
 *    places and such.
 *       The basic pattern is a hexagonal tiling, with a little bit of
 *    color variation from tile to tile.  On top of that is some staining
 *    (presumably due to water or something), which darkens the tile or
 *    mortar underneath it.  Finally, there is scuffing due to people's
 *    shoes, which really only affects the tile part not the mortar part.
 *
 *
 * PARAMTERS
 *    Ka, Kd, Ks, roughness, specularcolor - work just like plastic
 *    tilecolor - the color of the tiles
 *    mortarcolor - the color of the mortar (space between the tiles)
 *    tileradius - the "radius" (in s-t units) of a single tile
 *    mortarwidth - the width of the mortar (in s-t units)
 *    tilevary - the color variance from tile to tile
 *
 * ANTIALIASING
 *    Some rudimentary antialiasing is performed on the borders between
 *    tile and mortar.
 *
 * HINTS & APPLICATIONS
 *    If all of the default parameters are used, the tiles look just like
 *    the floors in the public areas of the Washington DC subway system.
 *
 * AUTHOR: written by Larry Gritz, 1994
 *
 * HISTORY:
 *    15 Feb 1994 -- written by lg
 *
 * last modified 15 Feb 94 by Larry Gritz
 */

void hextile(
				float tileradius, mortarwidth, tilevary;
                float xx, yy;
                output float mortar;
                output float tileindex;
		)
{
    extern float du;
    extern float dv;

	float swidth, twidth, sfuzz, tfuzz, fuzzmax, tilewidth,
		  tt, ttile, ss, stile, mw2, x, y;

	swidth = abs( Du(xx) * du) + abs( Dv(xx) * dv);
	twidth = abs( Du(yy) * du) + abs( Dv(yy) * dv);
	sfuzz = 0.5 * swidth;
	tfuzz = 0.5 * twidth;
	fuzzmax = max( sfuzz, tfuzz );

	tilewidth = tileradius * S_SQRT3;
	tt = mod( yy, 1.5 * tileradius);
	ttile = floor( yy / (1.5*tileradius) );
	
	if (mod ( ttile/2, 1) == 0.5) {
		ss = xx + tilewidth/2;
	} else {
		ss = xx;
	}
	
	stile = floor( ss / tilewidth );
	ss = mod( ss, tilewidth );
	mw2 = mortarwidth / 2;
	
	if ( tt < tileradius) {
		mortar = 1 - ( smoothstep( mw2, mw2 + sfuzz, ss ) *
					(1 - smoothstep( tilewidth-mw2-sfuzz, tilewidth-mw2, ss)));
	} else {
		x = tilewidth / 2 - abs( ss - tilewidth/2 );
		y = S_SQRT3 * (tt - tileradius);
		
		if (y > x) {
			if ( mod (ttile/2, 1 ) == 0.5) {
				stile -= 1;
			}
			ttile += 1;
			if ( ss > tilewidth/2 ) {
				stile += 1;
			}
		}
		mortar = ( smoothstep( x-1.73 * mw2 - tfuzz, x-1.73 * mw2, y) *
					(1 - smoothstep( x+1.73 * mw2, x+1.73*mw2 +tfuzz, y)));
	}
	tileindex = stile + 41 * ttile;
}

////////////////////////////////////////////////////////////////////////////////
// Larry Gritz's wood2.sl shader, from The RenderMan Repository ////////////////
// http://www.renderman.org ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked to fit our needs and shrimp's structure ////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * wood2.sl -- another surface shader for wood.
 *
 * DESCRIPTION:
 *   Makes wood solid texture.
 * 
 * PARAMETERS:
 *   Ka, Kd, Ks, specular, roughness - work just like the plastic shader
 *   txtscale - overall scaling factor for the texture
 *   ringscale - scaling for the ring spacing
 *   lightwood, darkwood - surface colors for the wood itself
 *   grainy - relative graininess (0 = no fine grain)
 *
 * AUTHOR: written by Larry Gritz
 *
 * $Revision: 1.1.1.1 $   $Date: 2002/02/10 02:35:52 $
 *
 * $Log: wood2.sl,v $
 * Revision 1.1.1.1  2002/02/10 02:35:52  tal
 * RenderMan Repository
 *
 * Revision 1.1.2.1  1998-02-06 14:02:32-08  lg
 * Converted to "modern" SL with appropriate use of vector & normal types
 *
 * Revision 1.1  1995-12-05 15:05:47-08  lg
 * Initial RCS revision
 *
 * 19 Jan 1994 -- recoded by lg in correct shading language.
 * March 1991 -- original by Larry Gritz
 */

float wood2(
                float ringscale, txtscale, grainy;
                point ip;
                )
{
    /* calculate in shader space */
    point pp = txtscale * transform("shader", ip);

    float my_t = zcomp(pp) / ringscale;
	/* shading space point to be computed */
    point pq = point( xcomp(pp) * 8, ycomp(pp) * 8, zcomp(pp) );
    my_t += noise(pq) / 16;

    pq = point( xcomp(pp), my_t, ycomp(pp) + 12.93);
    float r = ringscale * noise(pq);
    r -= floor(r);
    r = 0.2 + 0.8 * smoothstep( 0.2, 0.55, r) * (1-smoothstep( 0.75, 0.8, r));

    /* \/-- extra line added for fine grain */
    pq = point( xcomp(pp) * 128 + 5, zcomp(pp) * 8 - 3, ycomp(pp) * 128 + 1 );
    float r2 = grainy * (1.3 - noise(pq)) + (1 - grainy);
    return r * r2 * r2;
}

////////////////////////////////////////////////////////////////////////////////
// Blotches, distributed accordingly to voronoi cell noise /////////////////////
////////////////////////////////////////////////////////////////////////////////

/* radius = blotches radius
   frequency = frequency of blotches (P*freq)
   offset = blotch center offset towards white, 0 = hollow, 1 = solid
   jitter = controls voronoi cell noise jittering, which affects blotches
   distribution.
   Dtype = distance metric, when != 0, Minkowski, being dtype Minkowski's 
   P parameter, with P = 1 as Manhattan metric, P = 2 as Euclidian, and 
   as P increases, tends towards Tchebychev. 
   */

float blotches(
                float radius, frequency, offset, jitter;
				uniform float dtype;
                point pp;
        )
{
    float mixfactor = 0;
    float f1;
    point pos1;
    
    point pp1 = pp * frequency;
    
    voronoi_f1_3d( pp1, jitter, dtype, f1, pos1);
    
    if (f1 <= radius) {
        mixfactor = smoothstep( radius - offset, radius + offset, f1);
    }
    return mixfactor;
}

////////////////////////////////////////////////////////////////////////////////
// Larry Gritz's Oak shader , from The RenderMan Repository ////////////////////
// http://www.renderman.org ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* from oak.h, slightly tweaked, to fit shrimp's structure */

/****************************************************************************
 * oak.h - contains the oaktexture function, which is common to many
 *         shader which make wood grain patterns.  It looks very much
 *         like oak, and can also pass for ...
 *
 ***************************************************************************
 *
 * Author: Larry Gritz, 1999
 *
 * Contacts:  lg AT larrygritz DOT com
 *
 * $Revision: 1.2 $    $Date: 2003/12/24 06:18:06 $
 *
 ****************************************************************************/

/* oaktexture -- generate a pattern much like the rings and grain of
 *    oak wood.  Rings are centered on the z axis, so the caller should
 *    position the space of Pshad so that the grain goes the way you want.
 *    Note: this function takes derivatives, so it shouldn't be called 
 *    from within varying conditionals or loops.
 * Inputs:
 *    Pshad, dPshad - sample point & its derivative
 *    ringfreq - mean frequency of ring spacing
 *    ringunevenness - 0=equally spaced rings, larger is unequally spaced
 *    grainfreq - frequency of the fine grain
 *    ringnoise, ringnoisefreq - general warping of the domain
 *    trunkwobble, trunkwobblefreq - controls noise which wobbles the
 *        axis of the trunk so that it's not perfectly on the z axis.
 *    angularwobble, angularwobblefreq - warping indexed by angle about
 *        the z axis.
 *    ringy, grainy - overall scale on the degree to which rings and
 *        grain are weighted.  0 turns one off, 1 makes full effect.
 * Return value: 0 when in the "background" light wood, 1 when in the
 *    darkest part of a ring or grain.
 */

float
oaktexture(
            point PP;
            float ringfreq, ringunevenness, grainfreq, ringnoise;
            float ringnoisefreq, trunkwobble, trunkwobblefreq;
            float angularwobble, angularwobblefreq, ringy, grainy;
        )
{
    extern float du, dv;
    float dPP = filterwidthp(PP);
    
    /* We shade based on Pshad (PP), but we add several layers of warping: */
    /* Some general warping of the domain */
    vector offset = vfBm(PP * ringnoisefreq, dPP * ringnoisefreq, 2, 4, 0.5);
    point Pring = PP + ringnoise * offset;
    
    /* The trunk isn't totally steady xy as you up in z */
    Pring += trunkwobble * vsnoise(zcomp(PP) * trunkwobblefreq) *
                            vector(1, 1, 0);

    /* Calculate the radius from the center. */
    float r2 = SQR(xcomp(Pring)) + SQR(ycomp(Pring));
    float r = sqrt(r2) * ringfreq;
    
    /* Add some noise around the trunk */
    r += angularwobble * smoothstep( 0, 5, r)
                        * snoise( angularwobblefreq * (Pring)
                        * vector(1, 1, 0.1));

    /* Now add some noise so all rings are not equal width */
    float dr = filterwidth(r);
    r += ringunevenness * filteredsnoise(r, dr);

    float inring = smoothpulsetrain( .1, .55, .7, .95, 1, r);

    point Pgrain = PP * grainfreq * vector( 1, 1, .05);
    float dPgrain = filterwidthp( Pgrain );
    float grain = 0;
    
    float amp = 1;
	uniform float i;
    
    for( i = 0; i < 2; i += 1) {
        float grain1valid = 1 - smoothstep( .2, .6, dPgrain);
        if (grain1valid > 0) {
            float g = grain1valid * snoise( Pgrain);
            g *= (0.3 + 0.7 * inring);
            g = pow( clamp( 0.8 - (g), 0, 1), 2);
            g = grainy * smoothstep( 0.5, 1, g);
            
            if (i == 0) {
                inring *= (1 - 0.4 * grain1valid);
            }
            
            grain = max( grain, g);
        }
        Pgrain *= 2;
        dPgrain *= 2;
        amp *= 0.5;
    }

    return mix( inring * ringy, 1, grain);
}

////////////////////////////////////////////////////////////////////////////////

/* Voronoi diagram, Karlsruhe (Moscow) metric */

void
voronoi_km_f1f2_2d(
						float ss, tt;
		            	float jitter;
			            output float f1;
			            output float spos1, tpos1;
			            output float f2;
		    	        output float spos2, tpos2;
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

		/* Karlsruhe-metric (Moscow metric)
		 * (r,s) = polar coordinate of p (np)
		 * (ri,si) = polar coordinate of p(i) (npos) 
		 * q = min( r, ri)
		 * e(s, si) = min( |s-si|, 2PI-|s-si|)
		 * if s >= 0 && s <= 2
		 * distance(p, p(i)) = q * e(s, s(i)) + |r-r(i)| )
		 * else
		 * distance(p, p(i)) = r + r(i)
		 * */

		float nspos = 0, nss = 0, ntpos = 0, ntt = 0;
		cartesian2polar2d( ss, tt, nss, ntt );
		cartesian2polar2d( spos, tpos, nspos, ntpos );

		float qx = min( nss, nspos );
		float qy = min( abs(ntt - ntpos), S_2PI - abs(ntt - ntpos) );

		float dist = 0;
		if ( ntt >= 0 && ntt <= 2) {
			dist = qx * qy + abs(nss - nspos);
		} else {
			dist = nss + nspos;
		}

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
#endif /* SHRIMP_EXTRA_PATTERNS_H */

