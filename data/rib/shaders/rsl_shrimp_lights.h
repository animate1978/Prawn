#ifndef SHRIMP_LIGHT_H
#define SHRIMP_LIGHT_H 1

#include "rsl_shrimp_helpers.h"

/* Collection of useful light shaders/functions */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Siggraph 2000 shadow spot light shader, by Tal Lancaster, from //////////////
// The RenderMan Repository, http://www.renderman.org //////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* Slightly tweaked to fit shrimp's structure and needs */

/* Modified shadowspot to better handle attenuation.
 * 
 * bias -- to overide the global shadow bias value.  -1 the light will
 *   use the global shadow bias value.  Otherwise whatever value is here
 *   is what will be used for this lights shadow bias.
 * blur -- amount to soften the shadows edges.
 *
 * 
 * Tal Lancaster  talrmr@SpamSucks_pacbell.net
 * 02/12/00
 * 
 * Modification History
 * 03/25/00  Added comments.
 */

color
sig2kshadowspot(
				 float intensity;
				 color lightcolor;
				 point from, to;
				 uniform float coneangle, conedeltaangle, beamdistribution;
				 uniform float shadowsamples, shadowbias, shadowwidth;
				 uniform string shadowname, shadowfilter;
				 float shadowblur, atten;
				 output varying color shad;
		)
{
	shad = color(0);
	float attenuation, cosangle;
	vector A = (to - from) / length(to - from);
	uniform float cosoutside = cos( coneangle ),
				  cosinside = cos( coneangle - conedeltaangle );

	color C = color(0);

	illuminate( from, A, coneangle ) {
		extern vector L;
		extern point Ps;
		/* According to the RI specs both Cl and Ol are global light
		 * shader variables, accessible in the light shader, but some
		 * renderers have problems accessing Cl and Ol outside solar
		 * and illuminate constructs, when only light direction vector L
		 * is exclusively available inside solar and illuminate constructs.
		 * Workaround follows, instead of letting the shrimp blocks set
		 * Cl and Ol in the main xml statements. */
		extern color Cl;

		cosangle = L.A / length(L);
		attenuation = atten * pow( cosangle, beamdistribution) /
									( atten + length(L));
		attenuation *= smoothstep( cosoutside, cosinside, cosangle);
		
		if (shadowname != "") {
			shad = color shadow( shadowname, Ps, "samples", shadowsamples,
						"blur", shadowblur, "width", shadowwidth,
						"filter", shadowfilter, "bias", shadowbias);
		}
		Cl = attenuation * (1-shad) * lightcolor;
		C = Cl;
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Slide projector light shader, from The RenderMan Repository /////////////////
// http://www.renderman.org ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* slightly tweaked to fit shrimp's structure and needs */

/* Copyrighted Pixar 1989 */
/* From the RenderMan Companion p.373 */
/* Listing.16.28 Slide-projector light shader */

/*
 * slideprojector(): Cast a texture map into a scene as a light source
 */

// Tweaked to fit Shrimp's structure, added decay+intensity, shadows output

color
slideprojector(
				float intensity, shadowblur;
				uniform float decay, fieldofview;
				color lightcolor;
				point from, to, up;
				uniform string slidename, shadowname, shadowfilter;
				uniform float shadowsamples, shadowwidth, shadowbias;
				output varying color shad;
		)
{
	shad = color(0);
	varying vector reIT, /* normalized direction vector */
				   reIU, /* "vertical" perspective of surface point */
				   reIV; /* "horizontal perspective of surface point */

	uniform float spread = 1 / tan( fieldofview/2); /* spread of "beam" */

	float Pt, /* projection of Ps on reIT (distance of surface point along
				 light direction) */
		  Pu, /* projection of Ps on reIU */
		  Pv; /* projection of Ps on reIV */
	
	float sloc, tloc; /* perspected surface point */

	/* initialize uniform variables for perspective */
	reIT = normalize( to - from );
	reIU = reIT ^ vector( up ) ;
	reIV = normalize( reIT ^ reIU );
	reIU = reIV ^ reIT;

	color C = color(0);
	float atten = 0, dist = 0;
	
	illuminate( from, vector( reIT), atan( S_SQRT2 / spread)) /* sqrt(2) */
	{
		extern color Cl;
		extern vector L;
		extern point Ps;

		L = Ps - from; /* direction of light source from surface point */
		Pt = L.reIT;   /* direction of Ps along reIT, reIU, reIV */
		Pu = L.reIU;
		Pv = L.reIV;
		dist = length(L);
		
		sloc = spread * Pu / Pt; /* perspective divide */
		tloc = spread * Pv / Pt;
		
		sloc = sloc * .5 + .5; /* correction from [-1,1] to [0,1] */
		tloc = tloc * .5 + .5;
		
		if ( slidename != "") {
			Cl = color texture( slidename, sloc, tloc);
		} else {
			Cl = lightcolor;
		}

		if (decay == 0) { /* no decay */
			atten = intensity;
		} else if (decay == 1) { /* linear decay */
			atten = intensity / dist;
		} else if (decay == 2) { /* squared decay */
			atten = intensity / SQR(dist);
		} else { /* cubic, etc... */
			atten = intensity / pow( dist, decay);
		}
		
		if (shadowname != "") {
			shad = color shadow( shadowname, Ps, "samples", shadowsamples,
							"blur", shadowblur, "width", shadowwidth,
							"filter", shadowfilter, "bias", shadowbias );
		}
		Cl *= atten * (1-shad);
		C = Cl;
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Distant light shader, with shadows //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color
sdistantlight(
				point from, to;
				uniform string shadowname, shadowfilter;
				uniform float shadowsamples, shadowwidth;
				uniform float shadowbias, decay;
				float intensity, shadowblur;
				color lightcolor;
				output varying color shad;
		)
{
	color C = color(0);
	float atten = 0;
	shad = color(0);
	
	solar( to - from, 0) {
		extern vector L;
		extern point Ps;
		extern color Cl;

		float dist = length(L);
		if (decay == 0) { /* no decay */
			atten = intensity;
		} else if (decay == 1) { /* linear decay */
			atten = intensity / dist;
		} else if (decay == 2) { /* squared decay */
			atten = intensity / SQR(dist);
		} else {
			atten = intensity / pow( dist, decay );
		}
		
		if (shadowname != "") {
			shad = color shadow( shadowname, Ps, "samples", shadowsamples,
							"blur", shadowblur, "width", shadowwidth,
							"filter", shadowfilter, "bias", shadowbias );
		}
		Cl = atten * (1-shad) * lightcolor;
		C = Cl;
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Point light shader, with attenuation and shadows ////////////////////////////
////////////////////////////////////////////////////////////////////////////////
color
spointlight(
				point from;
				float intensity, blur;
				uniform float width, samples, bias, decay;
				uniform string shadowmap, filter;
				color lightcolor;
				output varying color shad;
		)
{
	float atten;
	color C = color(0);
	shad = color(0);

	illuminate( from ) {
		extern vector L;
		extern point Ps;
		extern color Cl;
		
		float dist = length(L);
		if (decay == 0) { /* no decay */
			atten = intensity;
		} else if (decay == 1.0) { /* linear */
			atten = intensity / dist;
		} else if (decay == 2.0) { /* squared */
			atten = intensity / SQR(dist);
		} else {
			atten = intensity / pow( dist, decay);
		}

		if (shadowmap != "") {
			shad = color shadow( shadowmap, Ps, "samples", samples,
									"blur", blur, "width", width,
									"bias", bias, "filter", filter);
		}
		Cl = atten * (1-shad) * lightcolor;
		C = Cl;
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_LIGHT_H */

