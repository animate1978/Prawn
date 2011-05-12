#ifndef RSL_SHRIMP_LAYERING_H
#define RSL_SHRIMP_LAYERING_H	1

/* Utility functions to blend between layers */
////////////////////////////////////////////////////////////////////////////////
// Based on http://www.pegtop.net/delphi/articles/blendmodes/index.htm
// http://dunnbypaul.net/blends/
// and: http://www.nathanm.com/photoshop-blending-math/
////////////////////////////////////////////////////////////////////////////////

#include "rsl_shrimp_helpers.h" // luminance, color absolute, exp, min, max

// Clamp color function with user-set min/max per component
color clampcolors(
					color inputC;
					float minR, minG, minB, maxR, maxG, maxB;
		)
{
	color C = inputC;
	return clamp( C, color( minR, minG, minB ), color( maxR, maxG, maxB ) );
}

////////////////////////////////////////////////////////////////////////////////
// Blending modes
// Multiply, Screen, Overlay, Darken, Lighten, ColorDodge, ColorBurn,
// HardLight, SoftLight, Difference and Exclusion are separable modes -
// each component of the result color is determined by the components of the
// foreground and background colors - with the blending function applied
// separately to each component. Colors are operated on a component by
// component basis in RSL.
// "Over" - linear interpolation between C1 and C2
color color_over(	color C1, C2; float weight; )
{	return clamp( color mix( C1, C2, weight), color(0), color(1) );	}

////////////////////////////////////////////////////////////////////////////////
// "Average"
color color_avg(	color C1, C2; float weight; )
{
	color C = mix( C1, (C1 + C2) / color(2), weight );
	return clamp(C, color(0), color(1) );
}

////////////////////////////////////////////////////////////////////////////////
// Add
color color_add(	color C1, C2; float weight; )
{
	color C = C1 + (C2 * weight);
	return clamp(C, color(0), color(1) );
}

////////////////////////////////////////////////////////////////////////////////
// Subtract
color color_sub(	color C1, C2; float weight; )
{
	color C = C1 - (C2 * weight);
	return clamp(C, color(0), color(1) );
}

////////////////////////////////////////////////////////////////////////////////
// Screen
color color_screen(	color C1, C2; float weight; )
{	return mix( C1, clamp( (1 - C1) * C2 + C1, color(0), color(1)), weight); }

////////////////////////////////////////////////////////////////////////////////
// Multiply 
color color_multiply(	color C1, C2; float weight; )
{	return mix( C1, clamp( C1 * C2, color(0), color(1)), weight ); }

////////////////////////////////////////////////////////////////////////////////
// Divide
color color_divide(		color C1, C2; float weight; )
{
	return mix( C1, clamp( C1 / max( color(EPS), C2), color(0), color(1) ),
			weight );
}

////////////////////////////////////////////////////////////////////////////////
// Lighten
color color_lighten(	color C1, C2; float weight; )
{	return mix( C1, clamp( max( C1, C2 ), color(0), color(1)), weight ); }

////////////////////////////////////////////////////////////////////////////////
// Darken
color color_darken(		color C1, C2; float weight; )
{	return mix( C1, clamp( min( C1, C2 ), color(0), color(1)), weight ); }

////////////////////////////////////////////////////////////////////////////////
// Hardlight (non-commutative - a combination of multiply and screen,
// also the same as Overlay commuted:
color color_hardlight(	color C1, C2; float weight; )
{
	color C = color(0);
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == aqsis // Aqsis component access via x/y/z/comp only
		setcomp( C, i, ( comp(C2, i) < 0.5) ?
				comp(C1, i) * comp(C2, i) : // multiply, else screen: w = 1
				(1 - comp(C1, i)) * comp(C2, i) + comp(C1, i) );
#else // color_multiply or color_screen, with w = 1
		C[i] = ( C2[i] < 0.5 ) ?
			C[i] = C1[i] * C2[i] : (1 - C1[i]) * C2[i] + C1[i] ;
#endif // Aqsis component access via x/y/z/comp only
	}
	return mix( C1, clamp( C, color(0), color(1) ), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Overlay (a combination of multiply and screen, non-commutative)
// Hardlight commuted:
color color_overlay(	color C1, C2; float weight; )
{	return color_hardlight( C2, C1, weight ); }

////////////////////////////////////////////////////////////////////////////////
// Vivid light, non-commutative (combination of color burn and color dodge)
color color_vividlight(	color C1, C2; float weight; )
{
	color C = color(0);
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == aqsis // Aqsis component access via xyz/comp only
		setcomp( C, i, ( comp(C2, i) <= 0.5 ) ?
				comp(C1, i) / (1 - 2 * comp(C2, i)) :
				1 - (1 - comp(C1, i)) / (2 + ( comp(C2, i) - 0.5)) );
#else
		C[i] = ( C2[i] <= 0.5 ) ?
			C1[i] / (1 - 2 * C2[i]) : 1 - (1 - C1[i]) / (2 + (C2[i] - 0.5));
#endif // Aqsis component access via xyz/comp only
	}
	return mix( C1, clamp( C, color(0), color(1) ), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Linear light, non-commutative (combination of linear burn and linear dodge)
color color_linearlight(color C1, C2; float weight; )
{
	color C = color(0);
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == aqsis // Aqsis component access via xyz/comp only
		setcomp( C, i, ( comp(C2, i) <= 0.5 ) ?
				comp(C1, i) + 2 * comp(C2, i) :
				comp(C1, i) + 2 * ( comp(C2, i) - 0.5 ) );
#else
		C[i] = ( C2[i] <= 0.5 ) ?
			C1[i] + 2 * C2[i] - 1 : C1[i] + 2 * ( C2[i] - 0.5 );
#endif // Aqsis component access via xyz/comp only
	}
	return mix( C1, clamp( C, color(0), color(1) ), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Pin light, non-commutative (combination of darken and lighten)
color color_pinlight(	color C1, C2; float weight; )
{
	color C = color(0);
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == Aqsis // Aqsis component access via xyz/comp only
		setcomp( C, i, ( comp(C2, i) <= 0.5 ) ?
				min( comp(C1, i), 2 * comp(C2, i) ) :
				max( comp(C1, i), 2 * ( comp(C2, i) - 0.5 ) ) );
#else
		C[i] = ( C2[i] <= 0.5 ) ?
			min( C1[i], 2 * C2[i] ) : max( C1[i], 2 * ( C2[i] - 0.5 ) );
#endif // Aqsis component access via xyz/comp only
	}
	return mix( C1, clamp( C, color(0), color(1) ), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Softlight (non-commutative - a combination of multiply and screen,
// (Soft Light formula is only approximate)
// if (Blend > ½) R = 1 - (1-Base) × (1-(Blend-½))
// if (Blend <= ½) R = Base × (Blend+½) 
color color_softlight(	color C1, C2; float weight; )
{
	color C = ( ((1 - C1) * C2 * C1) +
			(C1 * color_screen( C1, C2, weight )) );
	return clamp(C, color(0), color(1) );
}

////////////////////////////////////////////////////////////////////////////////
// Softlight (PS) (non-commutative)
color color_softlightps(	color C1, C2; float weight; )
{
	color C = color(0); float dxC = 0;
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == aqsis // Aqsis component access via xyz/comp only
		if ( comp(C2, i) <= 0.5) {
			setcomp( C, i, comp(C1, i) - (1 - 2 * comp(C2, i)) *
					comp(C1, i) * (1 - comp(C1, i)) );
		} else {
			dxc = ( comp(C1, i) <= 0.25) ?
				( (16 * comp(C1, i) - 12) * comp(C1, i) + 4) * comp(C1, i) :
				sqrt( comp(C1, i) );
			setcomp( C, i, comp(C1, i) + (2 * comp(C2, i) - 1) *
					(dxc - comp(C1, i)));
		}
#else
		if ( C2[i] <= 0.5) {
			C[i] = C1[i] - (1 - 2 * C2[i]) * C1[i] * (1 - C1[i]);
		} else {
			dxc = (C1[i] <= 0.25) ?
				( (16 * C1[i] - 12) * C1[i] + 4) * C1[i] : sqrt( C1[i] );
			C[i] = C1[i] + (2 * C2[i] - 1) * (dxc - C1[i]);
		}
#endif // Aqsis component access via xyz/comp only
	}
	return mix( C1, clamp( C, color(0), color(1)), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Color dodge
color color_dodge(	color C1, C2; float weight; )
{
	color C = color(0);
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == aqsis // Aqsis component access via xyz/comp only
		setcomp( C, i, ( comp(C2, i) < 1 ) ?
				min( 1, comp(C1, i) / (1 - comp(C2, i)) ) : 1 );
#else
		C[i] = ( C2[i] < 1 ) ? min( 1, C1[i] / (1 - C2[i])) : 1;
#endif // Aqsis component access via xyz/comp only
	}
	return mix( C1, clamp( C, color(0), color(1)), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Color burn
color color_burn(	color C1, C2; float weight; )
{
	color C = color(0);
	uniform float i;
	for (i = 0; i < 3; i += 1) {
#if RENDERER == aqsis // Aqsis component access via xyz/comp only
		setcomp( C, i, ( comp(C2, i) > 0 ) ?
				1 - min( 1, (1 - comp(C1, i)) / comp(C2, i)) : 0 );
#else
		C[i] = ( C2[i] > 0 ) ? 1 - min( 1, (1 - C1[i]) / C2[i] ) : 0;
#endif // Aqsis component access via xyz/comp only
	}
	return mix( C1, clamp( C, color(0), color(1)), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Linear dodge
color color_lineardodge(color C1, C2; float weight; )
{	return mix( C1, clamp( C1 + C2, color(0), color(1) ), weight );	}

////////////////////////////////////////////////////////////////////////////////
// Linear burn
color color_linearburn(	color C1, C2; float weight; )
{	return mix( C1, clamp( C1 + C2 - 1, color(0), color(1)), weight ); }

////////////////////////////////////////////////////////////////////////////////
// Difference
color color_difference(	color C1, C2; float weight; )
{
	color C = absc( C1 - C2 ); // abs(C1[i] - C2[i] )
	return mix( C1, clamp( C, color(0), color(1) ), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Exclusion
color color_exclusion(	color C1, C2; float weight; )
{
	color C = C1 + C2 - 2 * C1 * C2;
	return mix( C1, clamp( C, color(0), color(1)), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Reflect
color color_reflect(	color C1, C2; float weight; )
{
	color C = (C1 * (C1 + 1)) / max( color(EPS), (1 - C2));
	return mix( C1, clamp( C, color(0), color(1)), weight );
}

////////////////////////////////////////////////////////////////////////////////
// Saturate
color color_saturate(	color C1, C2; float weight; )
{	return mix( C1, clamp(C1 * (1 + C2), color(0), color(1)), weight ); }

////////////////////////////////////////////////////////////////////////////////
// Desaturate
color color_desaturate(	color C1, C2; float weight; )
{	return mix( C1, clamp( C1 * (1 - C2), color(0), color(1)), weight ); }

////////////////////////////////////////////////////////////////////////////////
// Illuminate
color color_illuminate(	color C1, C2; float weight; )
{	return mix( C1, clamp( C1 * (2 * C2 + 1), color(0), color(1)), weight); }

////////////////////////////////////////////////////////////////////////////////
#endif // RSL_SHRIMP_LAYERING_H

