#ifndef SHRIMP_TEXTURE_H
#define SHRIMP_TEXTURE_H 1

#include "rsl_shrimp_helpers.h"

////////////////////////////////////////////////////////////////////////////////
// Expanding select ranges of an (LDR) environment map /////////////////////////
// From SIGGRAPH 2002 Course 16, "Production Ready GI", by Hayden Landis ///////
////////////////////////////////////////////////////////////////////////////////
color
envexpand(
			color Cenv;
			float expandDynRange, dynRangeStartLum, dynRangeExponent;
		)
{
	color C = Cenv;
	if (expandDynRange > 1) {
		float lum = luminance(Cenv);
		if (lum > dynRangeStartLum) {
			// remap lum values 0-1
			lum = (lum - dynRangeStartLum) / (1 - dynRangeStartLum);
			float dynMix = pow( lum, dynRangeExponent);
			C = mix( Cenv, Cenv * expandDynRange, dynMix);
		}
	}
	return C;
}
// float version
float envexpand( float env, expandDynRange, dynRangeStartLum, dynRangeExponent;)
{
	float fenv = env;
	if (expandDynRange > 1) {
		if (fenv > dynRangeStartLum) {
			float lum = (fenv - dynRangeStartLum) / (1 - dynRangeStartLum);
			float dynMix = pow( lum, dynRangeExponent);
			fenv = mix( fenv, fenv * expandDynRange, dynMix);
		}
	}
	return fenv;
}

////////////////////////////////////////////////////////////////////////////////
// Spherical and cylindrical projection functions, from Advanced Renderman's ///
// project.h, taken from The RenderMan Repository: http://www.renderman.org ////
////////////////////////////////////////////////////////////////////////////////
/*
 * project.h - Routines for texture projection
 * Author: Larry Gritz (lg AT larrygritz DOT com)
 * Reference:
 * 		_Advanced RenderMan: Creating CGI for Motion Picture_,
 * 		by Anthony A. Apodaca and Larry Gritz, Morgan Kaufmann, 1999.
 * */
/* project 3D points onto a unit sphere centered at the origin */
void
spherical_projection(
						point PP;
						output varying float ss, tt, ds, dt;
		)
{
	extern float du, dv; // used by the filterwidth macro
	vector V = normalize( vector(PP) );
#if RENDERER == aqsis // component access via xyz(comp
	ss = (-atan( ycomp(V), xcomp(V)) + S_PI) / S_2PI;
	tt = 0.5 - acos(zcomp(V)) / S_PI;
#else
	ss = (-atan( V[1], V[0]) + S_PI) / S_2PI;
	tt = 0.5 - acos(V[2]) / S_PI;
#endif // Aqsis component access
	ds = filterwidth(ss);
	if (ds > 0.5) {
		ds = max(1 - ds, MINFILTWIDTH);
	}
	dt = filterwidth(tt);
	if (dt > 0.5) {
		dt = max(1 - dt, MINFILTWIDTH);
	}
}
// Project 3D points onto a cylincer about the Z-axis between Z=0 and Z=1
void
cylindrical_projection(
						point PP;
						output varying float ss, tt, ds, dt;
		)
{
	extern float du, dv; // used by the filterwidth macro
	vector V = normalize( vector(PP));
#if RENDERER == aqsis // component access via xyz/comp
	ss = (-atan( ycomp(V), xcomp(V)) + S_PI) / S_2PI;
	tt = zcomp(V);
#else
	ss = (-atan( V[1], V[0]) + S_PI) / S_2PI;
	tt = V[2];
#endif // Aqsis component access
	ds = filterwidth(ss);
	if (ds > 0.5) {
		ds = max(1 - ds, MINFILTWIDTH);
	}
	dt = filterwidth(tt);
}

////////////////////////////////////////////////////////////////////////////////
// Texture filtering for wrapped textures, to avoid wrapping seams /////////////
// From SIGGRAPH 2006 course 25 RenderMan For Everyone, chapter ////////////////
// "What the RI Spec never told you", by Dan Maas //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
color
clamped_texture(
					string texturename;
					uniform float channelbase;
					float ss, tt, width, maxwidth;
		)
{
	// calculate change in ss and tt across the micropolygon
	extern float du, dv;
	float ds_u = Du(ss) * du;
	float ds_v = Dv(ss) * dv;
	float dt_u = Du(tt) * du;
	float dt_v = Dv(tt) * dv;
	// scale coordinate deltas by filter width multiplier
	ds_u *= width; ds_v *= width;
	dt_u *= width; dt_v *= width;
	// clamp to maxwidth
	ds_u = clamp( ds_u, -maxwidth, maxwidth);
	ds_v = clamp( ds_v, -maxwidth, maxwidth);
	dt_u = clamp( dt_u, -maxwidth, maxwidth);
	dt_v = clamp( dt_v, -maxwidth, maxwidth);
	// find lower edge of filter region
	float ss2 = ss - (.5 * ds_u + .5 * ds_v);
	float tt2 = tt - (.5 * dt_u + .5 * dt_v);
	// four point texture call
	return color texture(	texturename[channelbase], ss2, tt2,
							ss2 + ds_u, tt2 + dt_u,
							ss2 + ds_u + ds_v, tt2 + dt_u + dt_v,
							ss2 + ds_v, tt2 + dt_v,
							"filter", "gaussian", "lerp", 1);
}
// float version
float
clamped_texture(
					string texturename;
					uniform float channelbase;
					float ss, tt, width, maxwidth;
		)
{
	// calculate change in ss and tt across the micropolygon
	extern varying float du, dv;
	float ds_u = Du(ss) * du;
	float ds_v = Dv(ss) * dv;
	float dt_u = Du(tt) * du;
	float dt_v = Dv(tt) * dv;
	// scale coordinate deltas by filter width multiplier
	ds_u *= width; ds_v *= width;
	dt_u *= width; dt_v *= width;
	// clamp to maxwidth
	ds_u = clamp( ds_u, -maxwidth, maxwidth);
	ds_v = clamp( ds_v, -maxwidth, maxwidth);
	dt_u = clamp( dt_u, -maxwidth, maxwidth);
	dt_v = clamp( dt_v, -maxwidth, maxwidth);
	// find lower edge of filter region
	float ss2 = ss - (.5 * ds_u + .5 * ds_v);
	float tt2 = tt - (.5 * dt_u + .5 * dt_v);
	// four point texture call
	return float texture(	texturename[channelbase], ss2, tt2,
							ss2 + ds_u, tt2 + dt_u,
							ss2 + ds_u + ds_v, tt2 + dt_u + dt_v,
							ss2 + ds_v, tt2 + dt_v,
							"filter", "gaussian", "lerp", 1);
}

////////////////////////////////////////////////////////////////////////////////
/* Number of channels present in a texture file, from Moritz Moeller's 
 * SIGGRAPH 2006 Course 25 RenderMan for Everyone, "Going Mad with Magic Lights"
 * 
 * Input:
 * 	- texturename
 * Return value:
 * 	- 0 if the given texturename was empty or the texture doesn't exists.
 * 	- number of channels otherwise
 * */
float numtexchannels(	uniform string texmap; ) {
	uniform float channels = 0;
	if (texmap != "") {
		textureinfo( texmap, "channels", channels);
	}
	return channels;
}
////////////////////////////////////////////////////////////////////////////////
/* Convert an input color to greyscale via: "maximum" or "minimum" or color
 * components; "average" of color components; "luminance" of color components;
 * or "channel" by specifying a channel to use for grayscale representation
 * */
float
tograyscale(	color C;
				uniform string graymode;
				uniform float channel; )
{
	float result = 0;
	if (graymode == "minimum") {
		result = vmin( C );
	} else if (graymode == "maximum") {
		result = vmax( C );
	} else if (graymode == "average") {
#if RENDERER == aqsis // component access via xyz/comp only
		result = (comp(C, 0) + comp(C, 1) + comp(C, 2)) / 3;
#else
		result = (C[0] + C[1] + C[2]) / 3;
#endif // Aqsis component access
	} else if (graymode == "channel") {
#if RENDERER == aqsis // component access via xyz/comp only
		result = comp(C, channel);
#else
		result = C[channel];
#endif // Aqsis component access
	} else { // CIE luminance
		result = luminance(C);
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// Taken from Advanced Renderman's project.h, from The RenderMan Repository ////
// http://www.renderman.org , and from Gary Campion's myproject.h at ///////////
// http://www.flamestudios.org/free/RendermanShaders ///////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Project to 2D
void
project2d(
			uniform string projection, whichspace;
			uniform matrix xform;
			point PP;
			normal NN;
			// Box mapping parameters, from Gary Campion's box mapping
			float w_repeat, h_repeat; // width, height
			float xpatoffsetw, xpatoffseth; // x offset width/height
			float ypatoffsetw, ypatoffseth; // y offset width/height
			float zpatoffsetw, zpatoffseth; // z offset width/height
			uniform float xplanessinv, yplanessinv, zplanessinv; // invert 
			output varying float ss, tt, ds, dt;
		)
{
	// clear output
	ss = 0; tt = 0; ds = 0; dt = 0;
	
	extern varying float du, dv; // for filterwidth macro
	point Pproj;

	if (projection == "st") {
		extern float s, t;
		Pproj = point(s, t, 0);
	} else {
		Pproj = transform( "camera", whichspace, PP);
	}

#if RENDERER == aqsis // can't transform from set space to supplied matrix
	Pproj = (projection != "box") ? transform( xform, Pproj) : Pproj;
#else
	Pproj = (projection != "box") ? transform( "camera", xform, Pproj) : Pproj;
#endif // transform("from", xform_matrix, foo) isn't valid
	
	if (projection == "planar" || projection == "st") {
#if RENDERER == aqsis // component access via xyz/comp only
		ss = xcomp(Pproj); tt = ycomp(Pproj);
		ds = filterwidth(ss); tt = filterwidth(tt);
	} else if (projection == "perspective") {
		float z = max( zcomp(Pproj), 1.0e-6); // avoid zero division
		ss = xcomp(Pproj)/z; tt = ycomp(Pproj)/z;
#else	
		ss = Pproj[0]; tt = Pproj[1];
		ds = filterwidth(ss); dt = filterwidth(tt);
		
	} else if (projection == "perspective") {
		
		float z = max( Pproj[2], 1.0e-6); // avoid zero division
		ss = Pproj[0]/z; tt = Pproj[1]/z;
#endif // Aqsis component access
		ds = filterwidth(ss);
		dt = filterwidth(tt);
		
	} else if (projection == "box") {	
		// Box projection from normals, based on Gary Campion's myproject.h
		normal Nproj = ntransform( whichspace, NN);
		normalize(Nproj);
#if RENDERER == aqsis // component access via xyz/comp
		float xn = xcomp(Nproj), yn = ycomp(Nproj), zn = zcomp(Nproj);
		float xp = xcomp(Pproj), yp = ycomp(Pproj), zp = zcomp(Pproj);
#else
		float xn = Nproj[0], yn = Nproj[1], zn = Nproj[2];
		float xp = Pproj[0], yp = Pproj[1], zp = Pproj[2];
#endif // Aqsis component access
		if (zn < 0.577 && zn >-0.577 && xn < 0.577 && xn >-0.577) {
			
			ss = mod( xp * w_repeat/4, 4) - ypatoffsetw/2;
			tt = mod( zp * h_repeat/4, 4) - ypatoffseth/2;
			if (yplanessinv == 1) ss = -ss;
			
		} else if (xn < 0.577 && xn >-0.577 && yn < 1 && yn >-1) {
			
			ss = mod( xp * w_repeat/4, 4) - zpatoffsetw/2;
			tt = mod( yp * h_repeat/4, 4) - zpatoffseth/2;	
			if (zplanessinv == 1) ss = -ss;
			
		} else if (yn < 0.577 && yn >-0.577) {
			
			ss = mod( yp * w_repeat/4, 4) - xpatoffsetw/2;
			tt = mod( zp * h_repeat/4, 4) - xpatoffseth/2;
			if (xplanessinv == 1) ss = -ss;
			
		} else {
			
			ss = mod( xp * w_repeat/4, 4) - ypatoffsetw/2;
			tt = mod( zp * h_repeat/4, 4) - ypatoffsetw/2;
			if (yplanessinv == 1) ss = -ss;
		}
		ds = filterwidth(ss);
		dt = filterwidth(tt);
		if (ds > 0.5) ds = max(1 - ds, MINFILTWIDTH);
		if (dt > 0.5) dt = max(1 - dt, MINFILTWIDTH);
	}	
	// Special cases for the projections that may wrap
	else if (projection == "spherical") {
		spherical_projection( Pproj, ss, tt, ds, dt);
	} else if (projection == "cylindrical") {
		cylindrical_projection( Pproj, ss, tt, ds, dt);
	} else { // NDC
		extern point P;
		point NP = transform("camera", "NDC", P);
#if RENDERER == aqsis // component access via xyz/comp only
		ss = xcomp(NP);
		tt = ycomp(NP);
#else
		ss = NP[0];
		tt = NP[1];
#endif // Aqsis component access
		ds = filterwidth(ss);
		dt = filterwidth(tt);
		if (ds > 0.5) ds = max(1 - ds, MINFILTWIDTH);
		if (dt > 0.5) dt = max(1 - dt, MINFILTWIDTH);
	}
}

////////////////////////////////////////////////////////////////////////////////
/* Function to get greyscale, color, color+alpha, depending on number of number
 * of channels. If no alpha is provided, allow specifying alpha from one of
 * the greyscale conversion methods available: minimum, maximum, average,
 * luminance, specific channel.
 * */
/*
 * texmap		= texture map
 * projection	= projection type: st, box, planar, perspective, spherical
 * 				  cylindrical
 * whichspace	= which space to use for projection
 * xform		= matrix for projection
 * graymode		= if converting to grayscale, what method to use:
 * 				  available channel specified in "channel", minimum, maximum,
 * 				  average or luminance of components, or 'grayscale' if img
 * 				  is single channel, and alpha should be main channel
 * channel		= channel to use as grayscale
 *
 * */
void
getcolorandalpha(
					uniform string texmap, projection, whichspace, graymode;
					uniform matrix xform;
					uniform float channel;
					float blur; // blur for texture
					uniform float samples; // for the reconstruction filter
					uniform string filter; // gaussian, triangle, box
					uniform float width, maxwidth; // filter area
					point PP; // for projection
					normal NN; // normalize(N), for projection
					float w_repeat, h_repeat; // repeat for box projection
					float xpatoffsetw, xpatoffseth; // x offset for box proj
					float ypatoffsetw, ypatoffseth; // y offset for box proj
					float zpatoffsetw, zpatoffseth; // z offset for box proj
					uniform float xinverts, yinverts, zinverts; // invert S
					uniform float clamptexture; // fix texture seams
					output varying color mapcolor, mapopacity;
		)
{
	float ss = 0, tt = 0, ds = 0, dt = 0;
	// get projection coords
	project2d(	projection, whichspace, xform, PP, NN, w_repeat, h_repeat,
				xpatoffsetw, xpatoffseth, ypatoffsetw, ypatoffseth,
				zpatoffsetw, zpatoffseth, xinverts, yinverts, zinverts,
				ss, tt, ds, dt);
	ds *= 0.5; dt *= 0.5;
	// get number of channels from texmap, if exists, else 0 channels
	uniform float numchannels = (texmap != "") ? numtexchannels(texmap) : 0;
	color Ct = color(0), Ot = color(0);
	
#ifndef TEXMAP_PARAMS
#define TEXMAP_PARAMS	ss-ds, tt-dt, ss+ds, tt-dt, ss-ds, tt+dt, ss+ds, \
						tt+dt, "blur", blur, "width", width, "samples", \
						samples, "filter", filter
#endif
	// texture access, if there's 1 or 2 channels available
	if (numchannels > 0 && numchannels < 3) {
		// read texture starting at channel 0
		Ct = (clamptexture == 1) ? float clamped_texture( texmap, 0, ss, tt,
				width, maxwidth) : float texture( texmap[0], TEXMAP_PARAMS);
		if (numchannels == 1) { // 1 channel
			// opacity is copy of grayscale or opaque, according to mode
			Ot = (graymode == "grayscale") ? Ct : color(1);
		} else if (numchannels == 2) { // 2 channel, rgb from channel 0
			// opacity from channel 1
			if (clamptexture == 1) {
				Ct = float clamped_texture(texmap, 0, ss, tt, width, maxwidth);
				Ot = float clamped_texture(texmap, 1, ss, tt, width, maxwidth);
			} else {
				Ct = float texture( texmap[0], TEXMAP_PARAMS);
				Ot = float texture( texmap[1], TEXMAP_PARAMS);
			}
		}
	} else if (numchannels > 0 && numchannels >= 3) { // at least 3 channels
		// read color texture starting at channel 0
		Ct = (clamptexture == 1) ? color clamped_texture( texmap, 0, ss, tt,
					width, maxwidth) : color texture( texmap[0], TEXMAP_PARAMS);
		// RGB, if graymode is empty, alpha = opaque, else creates alpha
		// via method set in graymode
		if (numchannels == 3) {
		Ot = (graymode == "") ? color(1) : color( tograyscale( Ct,
				graymode, channel));
		} else if (numchannels == 4) { // alpha = [3]
		// 0 = basechannel for clamped_texture for rgb, 3 for alpha
			Ot = (clamptexture == 1) ? color( float clamped_texture( texmap, 3,
					ss, tt, width,maxwidth) ) :
				color( float texture( texmap[3], TEXMAP_PARAMS) );
		} else if (numchannels > 5) { // transparency = color, from
		// channels 3,4,5 for RGB
		Ot = (clamptexture ==1) ? 
			color(	float clamped_texture( texmap,3,ss,tt, width,maxwidth),
					float clamped_texture( texmap,4,ss,tt, width,maxwidth),
					float clamped_texture( texmap,5,ss,tt, width,maxwidth))
			:		
			color(	float texture( texmap[3], TEXMAP_PARAMS),
					float texture( texmap[4], TEXMAP_PARAMS),
					float texture( texmap[5], TEXMAP_PARAMS) );
		}
		mapcolor = Ct; mapopacity = Ot;
	} else {
		mapcolor = color(0); mapopacity = color(1);
	}
#undef TEXMAP_PARAMS
}
////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_TEXTURE_H */

