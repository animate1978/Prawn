/* shapes.h
 * this file includes functions for drawing various shapes
 * Author: A.Gill,  August 2000 */

#ifndef SHRIMP_SHAPES_H
#define SHRIMP_SHAPES_H 1

#include "rsl_shrimp_helpers.h"

////////////////////////////////////////////////////////////////////////////////
float
shDisc( float ox, oy, x, y, radius, blur)
{
    float d, opacity;
     
    d = distance( point ( ox, oy, 0 ), point ( x, y, 0 ) );
    opacity = 1 - smoothstep( radius - blur, radius, d );
    return opacity;
}

////////////////////////////////////////////////////////////////////////////////
float
shRing ( float ox, oy, x, y, radius, thickness, blur )
{
    float d, opacity;

    d = distance( point ( ox, oy, 0 ), point ( x, y, 0 ) );
    opacity = filteredpulse( radius - thickness, radius , d, blur );
    return opacity;
}

////////////////////////////////////////////////////////////////////////////////
float
shLine( float x0, y0, x1, y1, x, y, thickness, blur )
{
	float d, opacity;
	float thickby2 = thickness / 2;
	point p0 = (x0, y0, 0);
	point p1 = (x1, y1, 0);
	point p = (x, y, 0);
	
	d = ptlined( p0, p1, p );	
	opacity = 1 - smoothstep( thickby2 - blur, thickby2, d );
	return opacity;
}

////////////////////////////////////////////////////////////////////////////////
float 
shRect( float x0, y0, x, y, width, height, blur )
{
    float opacity;

    /* rectangle is described by the logical and of a vertical 
	 * and a horizontal pulse */
    opacity = filteredpulse( x0, x0 + width, x, blur ) *
                 filteredpulse( y0, y0+height, y, blur ); 
    return opacity;
}

////////////////////////////////////////////////////////////////////////////////
float
shStripe ( float width, freq, x, blur )
{
	float opacity;
	float period = 1/freq;
	float edge = period - (width * period);
	
	/* The following was taken from filtered pulse train in patterns.h
	 * by Larry Gritz. First, normalize so period == 1 and our domain
	 * of interest is > 0 */
    float w = blur / period;
    float x0 = x / period - w / 2;
    float x1 = x0 + w;
    float nedge = edge / period; /* normalized edge value */

    /* Definite integral of normalized pulsetrain from 0 to t */
    float integral (float t, nedge) { 
        return ( (1-nedge)*floor(t) + max(0,t-floor(t)-nedge) );
    }

    /* Now we want to integrate the normalized pulsetrain over [x0,x1] */
    opacity = (integral(x1,nedge) - integral(x0,nedge)) / w;
    
    return opacity;
}

////////////////////////////////////////////////////////////////////////////////
/* this function is likely to be replaced by a better one.
 * ( as soon as I can think what a better one would look like ).
 * Takes the point (x,y) and returns the coords inside the tile (xt,yt)
 * and the tile column and row. xfreq, yfreq affect the quantity ( and
 * consequently the shape ) of the tiles in the x and y directions. */
void
shTile( float x, y;
		uniform float xfreq, yfreq;
		output varying float xt, yt;
		output varying float column, row; )
{
    xt = repeat( x, xfreq );
    yt = repeat( y, yfreq );

    column = whichtile( x, xfreq );
    row = whichtile( y, yfreq );
}
////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_SHAPES_H */

