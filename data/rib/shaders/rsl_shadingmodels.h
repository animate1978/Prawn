#ifndef RSL_SHADING_MODELS_H
#define RSL_SHADING_MODELS_H 1

#include "rsl_helpers.h"	/* useful constants & others */
#include "rsl_extras.h"	/* for PDFs and Geometric attenuation, for the
								   Cook-Torrance model */
#include "rsl_aov.h"		/* for the AOVs macros. Note that we initialize
								   all the AOVs to zero in the first statement
								   of the preview shader with
								   INIT_AOV_PARAMETERS*/
#include "rsl_odwikicomplex.h"	/* Complex math, from the Odforce.net's Odwiki,
								   including full formula for complex Fresnel */

#include "rsl_color.h"	/* spec6 functions */
#include "rsl_data.h"	/* glass, substrate, coating data */

////////////////////////////////////////////////////////////////////////////////
// Wrapped diffuse illumination model, based on ZJ's wrapped diffuse model /////
////////////////////////////////////////////////////////////////////////////////

color
wrappeddiffuse(
				float wangle, atten;
				normal Nn;
				vector In;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Ln;
	
	extern point P;
	color C = color(0);
	uniform float nondiff;
	float gamma = 5;

	illuminance( category, P, Nf, S_PI )
	{
		extern vector L;
		extern color Cl;
		
		nondiff = 0;
		// besides on/off switch, nondiff can be a scale factor
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			Ln = normalize(L);
			C += Cl * (1-nondiff) * pow( max( (1-acos(Ln.Nf) /
							(S_PI * wangle)), 0), atten);
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Lambertian diffuse model with control over the falloff between lit and //////
// unlit regions in the surface. From the SIGGRAPH 2002 course 16 //////////////
// RenderMan in Production, chapter 3, "Light/Surface Interactions", by ////////
// Matt Pharr. /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color
diffusefalloff(
				float falloff; // [0,1]
				normal Nn;
				vector In;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Ln;
	
	extern point P;
	color C = color(0);
	uniform float nondiff;

	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;

		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {	
	
			Ln = normalize(L);
			C += Cl * (1-nondiff) * gain( Nf.Ln, falloff);
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Lommel-Seeliger BRDF, tau = [0,1], 0 = infinite optical depth, 1 = layer ////
// thickness - isotropical scattering (forward/backscattering) /////////////////////////////////////////////////////////////////////////////////////////////////

color
LommelSeeliger(
				normal Nn;
				vector In;
				float tau; // thickness of layer, for non spherical surface
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln;

	float costheta = Nf.Vf, cospsi;

	uniform float nondiff;
	color C = color(0);
	extern point P;

	illuminance( category, P, Nf, S_PI_2 )
	{
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			extern vector L;
			extern color Cl;
			
			Ln = normalize(L);
			cospsi = Ln.Nf;

			C += Cl * (1-nondiff) * (1 / (cospsi + costheta)) *
					(1 - exp( -tau * (1/cospsi + 1/costheta))) * cospsi;
		}
	}
	return clamp(C, color(0), color(1));
}

////////////////////////////////////////////////////////////////////////////////
// Buratti photometric model, based on "Photometry and Surface Structure of ////
// the Icy Galilean Satellites", by Bonnie J. Buratti, Jet Propulsion //////////
// Laboratory, in "Journal of Geophysical Research", "special issue on the /////
// Icy Galilean Satellites". ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* A combination of the "Lommel-Seelieger" (or lunar scattering law) and 
 * Lambert's law, where the parameter k describes the fraction of light that
 * is multiply scattered, with 1 corresponding to single scattering. */

color
Buratti(
			normal Nn;
			vector In;
			float k, sden;
			uniform string category;
	   )
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln;

	float costheta = Nf.Vf, cospsi;
	float cosalpha, tanalpha, alpha, R;

	uniform float nondiff;
	color C = color(0);
	extern point P;

	illuminance( category, P, Nf, S_PI_2 )
	{
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			extern vector L;
			extern color Cl;

			Ln = normalize(L);
			cospsi = Ln.Nf;
			cosalpha = clamp( Vf.Ln, EPS, 1 - EPS);
			tanalpha = sqrt( max(0, 1 - SQR(cosalpha))) / cosalpha;
			alpha = acos(cosalpha);
			
			// macroscopic roughness
			R = (alpha < S_PI_2 - EPS) ?
				2 - tanalpha / 2 / sden * (1 - exp(-sden/tanalpha)) *
				(3 - exp(-sden/tanalpha)) : 1;

			C += Cl * (1-nondiff) * (k * (cospsi/(cospsi+costheta)) * sden
					+ (1-k) * cospsi) * R;
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Rim lighting shader /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

float
rimlighting(
				float rwidth;
				float dx;
				normal Nn;
				vector In; // usually viewer vector,can be user supplied vector
		)
{
	normal Nf = faceforward( Nn, In);
	vector Vf = -In;
	float C = 0;
	
	if (dx == 0) {
		C = smoothstep(1 - rwidth, 1, 1 - Vf.Nf);
	} else {
		C = filteredsmoothstep(1 - rwidth, 1, 1 - Vf.Nf, dx);
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Minnaert limb darkening term (Note: Nvidia uses a different approach   //////
// in which the limb coefficient is in the 0,1 range, this one is in the 12 ///
// range. //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color
Minnaert(
            float k;
            normal Nn;
            vector In;
			uniform string category;
        )
{
    normal Nf = faceforward(Nn, In);
	vector Vf = -In, Ln;
	
	float ndotv = max( 0, Nf.Vf ), ndotl;
	float thetak = pow( ndotv, k-1 );
	uniform float nondiff;
	
    extern point P;
	color Cminnaert = color(0);
	
    illuminance( category, P, Nf, S_PI_2 )
    {
        extern vector L;
        extern color Cl;
		
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
	        Ln = normalize(L);
    	    ndotl = max( 0, Ln.Nf);
        	Cminnaert += Cl * (1-nondiff) * pow( ndotl, k);
		}
    }
    Cminnaert *= thetak;

    return Cminnaert;
}

////////////////////////////////////////////////////////////////////////////////
// Hapke-Lommel-Seeliger lunar surface BRDF ////////////////////////////////////
// Szymon Rusinkiewicz's implementation, based on "A Theoretical photometric ///
// function for the Lunar Surface", in Journal of Geophysical Research,vol.68 //
// no.15, 1963 /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/* slightly tweaked to fit Shrimp's structure */

/*
 * Szymon Rusinkiewicz
 *
 * brdf_hls.cc
 * The Hapke/Lommel-Seeliger Lunar-surface BRDF with
 * Sch?nberg backscattering and the Hapke retrodirective
 * function.
 *
 * Hapke, Bruce. "A Theoretical Photometric Function for
 * the Lunar Surface", in
 * Journal of Geophysical Research, Vol.68, No.15, 1963
 * */

float Eval(
            vector vin, vout;
            normal vn;
            float g, f, r;
        )
{
    float cosalpha = clamp( vin.vout, EPS, 1.0-EPS);
    float sinalpha = sqrt( 1.0 - SQR( cosalpha) );
    float tanalpha = sinalpha/cosalpha;
    float alpha = acos( cosalpha );

	// effects of macroscopic roughness
	float R = (alpha < S_PI_2 - EPS) ?
		2 - tanalpha/2/g * (1-exp(-g/tanalpha)) * (3-exp(-g/tanalpha)) : 1;
	// opposition effect
	float S = 2 / 3 / S_PI * ( (sinalpha + (S_PI - alpha) * cosalpha) / S_PI
			+ f * SQR(1 - cosalpha));

	return r*R*S/(vin.vn + vout.vn);
}

color Hapke(
                float sden, fscat, refl;
                normal Nn;
                vector In;
				uniform string category;
        )
{
    normal Nf = faceforward( Nn, In);
    vector Vf = -In, Ln;

    float cos_theta_r = clamp(Nf.Vf, EPS, 1);
	uniform float nondiff;

    extern point P;
	color Ct = color(0);
	
    illuminance( category, P, Nf, S_PI_2 )
	{
        extern vector L;
        extern color Cl;
		
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {

        	Ln = normalize(L);
        	Ct += Cl * (1-nondiff) * Eval( Ln, Vf, Nf, sden, fscat, refl) 
					* Ln.Nf * S_PI;
		}
    }
    return Ct;
}

////////////////////////////////////////////////////////////////////////////////
// Lawrence B. Wolff diffuse model, based in "Improved Diffuse Reflection //////
// Models for Computer Vision", by Lawrence B. Wolff, Shree K. Nayar, and //////
// Michael Oren. ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef AIR
#define AIR 1.000293 // ior for air
#endif

// formula for the Fresnel reflection factor for unpolarized light
float nfresnel( float cos_theta, eta; )
{
	float g2 = sqrt( SQR(eta) - 1 + SQR(cos_theta) );
	float A = g2 - cos_theta;
	float B = g2 + cos_theta;
	float AA = B * cos_theta - 1;
	float BB = A * cos_theta + 1;
	return .5 * (SQR(A) / SQR(B)) * (1 + SQR(AA)/SQR(BB) );
}

/* "The diffuse reflection model proposed by Wolff, models reflection from
 * smooth dielectric materials as a combination of subsurface light scattering
 * distribution produced from internal inhomogeneities coupled with the 
 * refraction of externally incidend and internally scattered light at the 
 * air/surface dielectric boundary.", from "Improved Diffuse Reflection Models
 * for Computer Vision", by Lawrence B. Wolff, Shree K. Nayar, and
 * Michael Oren. */

color Wolff(
				normal Nn;
				vector In;
				float ior;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln;
	uniform float nondiff;

	// store preset quantities whenever possible
	float cos_theta_r = Vf.Nf, cos_theta_i, F;

	extern point P;
	color C = color(0);

	illuminance( category, P, Nf, S_PI_2 )
	{
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			extern vector L;
			extern color Cl;
			Ln = normalize(L);
			cos_theta_i = Ln.Nf;
			/* first Fresnel term accounting for refraction of externally
			 * incident light. */
			F = 1 - nfresnel( cos_theta_i, ior );
			C += Cl * (1-nondiff) * F * cos_theta_i;
		}
	}
	/* getting cosine of transmitted angle, for the second Fresnel term,
	 * accounting for the refraction of internally scattered light. */
	float eta = AIR/ior;
	float cos_theta_t = sqrt( max( 0, 1 - SQR(eta) * (1-SQR(cos_theta_r))));
	// 2nd Fresnel term
	float F2 = 1 - nfresnel( cos_theta_t, eta );

	return C * F2;
}

////////////////////////////////////////////////////////////////////////////////
// Oren-Nayar generalization of Lambert's reflection model /////////////////////
// implementation by Szymon Rusinkiewicz  //////////////////////////////////////
// from http://unix.math2.us.edu.pl/~perry/mp/wyklBRDF.pdf /////////////////////
////////////////////////////////////////////////////////////////////////////////
/* slightly tweaked to better fit Shrimp's structure */

/*
 * Oren and Nayar's generalization of Lambert's reflection
 * model. The roughness parameter gives the standard deviation
 * of angle orientatations of the presumed surface grooves.
 * When roughness = 0, the model is Lambertian.
 * */

// NOTE: this is the "qualitative" model, without inter-reflections

color
OrenNayar(
            float roughness;
            normal Nn;
            vector In;
			uniform string category;
        )
{
	vector Nf = faceforward (Nn, In);
	vector Vf = -In, Ln;
	
	// surface roughness coefficients
	float sigma2 = SQR(roughness);
	float A = 1 - 0.5 * sigma2 / (sigma2 + 0.33);
	float B = 0.45 * sigma2 / (sigma2 + 0.09);
	float ndotv = Vf.Nf;

	// useful precomputed quantities
	float theta_r = acos( ndotv ); // angle between V and N
	vector V_perp_N = normalize(Vf-Nf * (ndotv) ); // part of V perpend to N

	float cos_theta_i, cos_phi_diff;
	float theta_i, alpha, beta;
	uniform float nondiff;

	color C = color(0);
	extern point P;
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
        extern color Cl;
		
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			Ln = normalize(L);
			cos_theta_i = Ln.Nf;
			cos_phi_diff = V_perp_N . normalize(Ln - Nf * cos_theta_i);
			theta_i = acos( cos_theta_i );
			alpha = max( theta_i, theta_r );
			beta = min( theta_i, theta_r );
			
			C += Cl * (1-nondiff) * cos_theta_i * (A + B * max( 0,
						cos_phi_diff ) * sin( alpha ) * tan( beta ) );
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Oren-Nayar diffuse model, implementation by Larry Gritz, from ///////////////
// The RenderMan Repository - http://www.renderman.org /////////////////////////
// This implementation is the 3 coefficients implementation with ///////////////
// inter-reflections ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* renamed LG_orennayar.sl -- tal AT renderman DOT org */
/*
 * orennayar.sl - rough diffuse surface
 *
 * 
 * DESCRIPTION:
 *   Makes a rough surface using a BRDF which is more accurate than
 *   Lambert.  Based on Oren & Nayar's model (see Proc. SIGGRAPH 94).
 *
 *   Lambertian (isotropic) BRDF is a simple approximation, but not
 *   an especially accurate one for rough surfaces.  Truly rough surfacs
 *   tend to act more like retroreflectors than like isotropic scatterers.
 * 
 * PARAMETERS:
 *   Ka, Kd - just like matte.sl
 *   sigma - roughness (0 is lambertian, larger values are rougher)
 *
 * AUTHOR:  Larry Gritz
 *
 * REFERENCES:
 *   Oren, Michael and Shree K. Nayar.  "Generalization of Lambert's
 *         Reflectance Model," Computer Graphics Annual Conference
 *         Series 1994 (Proceedings of SIGGRAPH '94), pp. 239-246.
 *
 * NOTES:
 *   1. Note that this is really just an illuminance loop that gathers
 *      light from the sources and applies Oren & Nayar's local reflectance
 *      model to the result.  It could easily be packaged up as a macro
 *      or a function and used in any other shader, in place of diffuse().
 *   2. Examination of why rough surfaces are not Lambertian will lead
 *      you to the solution to the famous "flat full moon" problem.
 *
 * HISTORY:
 *   14 June 1994 -- written by Larry Gritz
 *
 * */

// tweaked a bit to fit Shrimp's structure.

color
LG_OrenNayar(
				color Cdiff;
				float sigma;
				normal Nn;
				vector In;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln = vector(0);
	// store preset quantites whenever possible
	float cos_theta_r = Vf.Nf, cos_theta_i, cos_phi_diff;
	float theta_i, alpha, beta;
	float theta_r = acos( cos_theta_r );
	float sigma2 = SQR(sigma);
	float C1 = 1 - .5 * sigma2 / (sigma2 + .33), C2, C3;

	uniform float nondiff;

	color C = color(0), L1, L2;
	extern point P;

	illuminance( category, P, Nf, S_PI_2 )
	{
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			extern vector L;
			extern color Cl;
			Ln = normalize(L);
			cos_theta_i = Ln.Nf;
			cos_phi_diff = normalize(Vf - Nf * cos_theta_r) .
								 normalize(Ln - Nf * cos_theta_i);
			theta_i = acos( cos_theta_i );
			alpha = max( theta_i, theta_r );
			beta = min( theta_i, theta_r );
			
			C2 = .45 * sigma2 / (sigma2 + .09);
			
			if (cos_phi_diff >= 0) {
				C2 *= sin(alpha);
			} else {
				C2 *= (sin(alpha) - pow( 2 * beta / S_PI, 3));
			}
				C3 = .125 * sigma2 / (sigma2 + .09) * pow( 4 * alpha
						* beta / S_2PI, 2);
			
			L1 = Cdiff * (cos_theta_i * (C1 + cos_phi_diff * C2 * tan(beta)
						+ (1 - abs( cos_phi_diff)) * C3 * tan( (alpha
								+ beta) / 2 )));
			L2 = SQR(Cdiff) * (.17 * cos_theta_i * sigma2 / (sigma2 + .13)
					* (1 - cos_phi_diff * 4 * SQR(beta) / S_2PI ));
			
			C += Cl * (1-nondiff) * (L1 + L2);
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Oren-Nayar-Wolff diffuse model, based on the paper "Improved Diffuse ////////
// Reflection Models for Computer Vision", by Lawrence B. Wolff, Shree K. //////
// Nayar, Michael Oren, and on Larry Gritz's implementation of the full ////////
// Oren-Nayar model, with inter-reflections, from The RenderMan Repository /////
// http://www.renderman.org ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* "The diffuse reflection model proposed by Wolff, models reflection from
 * smooth dielectric materials as a combination of a subsurface light 
 * scattering distribution produced from internal inhomogeneities coupled
 * with the refraction of externally incident, and internally scattered
 * light at the air-surface dielectric boundary." */
/* The Oren-Nayar-Wolff implementation is based on the full model considering
 * inter-reflections. */

color
OrenNayarWolff(
				float sigma, ior;
				color Cdiff;
				normal Nn;
				vector In;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln;

	// store preset quantities whenever possible
	float cos_theta_r = Vf.Nf, cos_theta_i, cos_theta_tl;
	float theta_r = acos( cos_theta_r ), theta_i, cos_phi_diff;
	float sigma2 = SQR(sigma), alpha, beta;
	float C1, C2, C3;

	float eta = AIR/ior;
	uniform float nondiff;
	color C = color(0), L1, L2;
	extern point P;
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {

			extern vector L;
			extern color Cl;
		
			Ln = normalize(L);
			cos_theta_i = Ln.Nf;
			cos_phi_diff = normalize(Vf-Nf * cos_theta_r) .
								normalize(Ln - Nf * cos_theta_i );
			theta_i = acos( cos_theta_i );
			alpha = max( theta_i, theta_r );
			beta = min( theta_i, theta_r );

			// transmitted angle
			cos_theta_tl = sqrt( max( 0, 1 - SQR(eta) * (1-SQR(cos_theta_i))));

			// 1st Oren-Nayar coefficient
			C1 = (1 - .5 * sigma2 / (sigma2 + .33)) * (1 - nfresnel(
					cos_theta_i, ior )) * (1 - nfresnel( cos_theta_tl, eta ));
			// 2nd coefficient
			C2 = .45 * sigma2 / (sigma2 + .09);

			if (cos_phi_diff >= 0) {
				C2 *= sin(alpha);
			} else {
				C2 *= (sin(alpha) - pow( 2 * beta / S_PI, 3));
			}
			// 3rd coefficient (Qualitative model only uses C1+C2)
			C3 = .125 * sigma2 / (sigma2 + .09) * pow( 4 * alpha *
							beta / S_2PI, 2);

			// Final L1 term
			L1 = Cdiff * (cos_theta_i * (C1 + cos_phi_diff * C2 * tan(beta)
						+ (1 - abs( cos_phi_diff )) * C3 * tan( (alpha
								+ beta) / 2 )));

			//> Final L2 term
			L2 = (Cdiff * Cdiff) * (.17 * cos_theta_i * sigma2 / (sigma2
						+ .13) * (1 - cos_phi_diff * 4 * SQR(beta) / S_2PI ));

			C += Cl * (1-nondiff) * (L1 + L2);
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Note: below follows the Ashikhmin-Shirley diffuse term, based on the ////////
// implementation by Peter Stuart, as well as further below, the specular //////
// term, for those wanting to experiment with things. The complete model is ////
// further below. Also note that the diffuse term and specular terms are ///////
// inter-dependent, but for the sake of giving the user the possibility to /////
// experiment a bit, i thought we could include the separate terms as well. ////
// So things were tweaked a bit, in order to fit shrimp's structure. ///////////
////////////////////////////////////////////////////////////////////////////////
// based on "An anisotropic Phong BRDF Model", by Michael Ashikhmin and ////////
// Peter Shirley ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* 
 * ashshir.sl
 * Simple surface using Ashikmin-Shirley BRDF
 * Peter Stuart
 * $Id: ashshir.sl,v 1.4 2003/05/05 05:22:41 ophi Exp $
 * description: Ashikmin and Shirley BRDF lighting model as described
 * in "An Anisotropic Phong Light Reflection Model." Most of the work
 * is done in ashShirModel.h by two functions - one for dealing with
 * direct lighting, and the other for dealing with environment lighting.
 *
 * ---- surface parameters ----------
 * rd: diffuse reflectance
 * rs: specular reflectance
 * nu: specular exponent along xdir
 * nv: specular exponent perpendicular to xdir
 * refl: amount of reflection to use
 * envname: name of environment map to use. If this is "" the shader will
 *   try to raytrace the environment if there are samples (envsamp > 0)
 * envsamp: number of samples to use when sampling the environment
 * */

color
anisophongdiff(
				normal Nn;
				vector In;
				float Kd, Ks;
				uniform string category;
				)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln;

	float ndotv = Nf.Vf, ndotl;
	
    uniform float nondiff;
    color C = color(0);
	extern point P;

    illuminance( category, P, Nf, S_PI_2 )
    {
        extern vector L;
        extern color Cl;
		
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
	        Ln = normalize(L);

       		ndotl = Nf.Ln;

			C += Cl * (1-nondiff) * (28 * Kd / (23*S_PI)) *
				(1-Ks) * (1-pow(1-ndotl/2,5)) * (1-pow(1-ndotv/2,5)) * ndotl;
		}
    }
    return C;
}

////////////////////////////////////////////////////////////////////////////////
// James F. Blinn specular term (Phong-Blinn) //////////////////////////////////
// from "Models of Light Reflection for Computer Synthesized Pictures", ////////
// Computer Graphics, Vol.11, No.2, July 1977, page 192-198 (?) ////////////////
////////////////////////////////////////////////////////////////////////////////

color phong_blinn(
					normal Nn;
					vector In;
					float size;
					uniform string category;
					)
{
	normal Nf = faceforward(Nn, In);
	vector Vf = -In, Ln, Hn;
	
    uniform float nonspec;
	float ndoth;
	color C = color(0);
	extern point P;
	
    illuminance( category, P, Nf, S_PI_2 )
    {
        extern vector L;
        extern color Cl;
		
		nonspec = 0;
		// besides on/off switch, nonspec can be a scale factor
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
	        Ln = normalize(L);
    	    Hn = normalize(Ln + Vf);
			
        	ndoth = Nf.Hn;
        	C += Cl * (1-nonspec) * pow( max( 0.0, ndoth), size) * (Nf.Ln);
		}
    }
    return C;
}

////////////////////////////////////////////////////////////////////////////////
// Larry Gritz's LocIllumGlossy ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked to fit shrimp's structure //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* LocIllumGlossy - possible replacement for specular(), with a more 
 * uniformly bright core and sharper falloff. Good specular function for
 * something made of glass or liquid, in which roughness = related to the
 * size of the highlight, larger is bigger, and sharpness = 1 is infinitely
 * sharp, 0 is very dull
 * */

color
LocIllumGlossy(
				normal Nn;
				vector In;
				float roughness, sharpness;
				uniform string category;
				)
{
	normal Nf = faceforward( Nn, In);
	vector Vf = -In, Ln, Hn;
    
	float w = .18 * (1-sharpness);
	
	uniform float nonspec;
    color C = color(0);
	extern point P;
	
	illuminance (P, Nf, S_PI_2 )
	{
		extern vector L;
        extern color Cl;
        
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			Ln = normalize(L);
			Hn = normalize(Vf+Ln);

			C += Cl * (1-nonspec) * smoothstep( .72-w, .72+w,
					pow( max( 0.0, Nf.Hn), 1/roughness ) );
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Variation on Greg Ward's isotropic specular /////////////////////////////////
// Note that you can just pass identical values to x and y anisotropy, in the //
// Ward anisotropic shader, and get the same results, altough at a slight //////
// extra expense. //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color
Wardisotropy(
				normal Nn;
				vector In;
				float roughness;
				uniform string category;
		)
{
    normal Nf = faceforward( Nn, In);
    vector Vf = -In, Ln, Hn;
    float ndotv = Nf.Vf, ndotl, ndoth, tandelta2;
	float m2 = roughness * roughness;

	uniform float nonspec;
	color C = color(0);
	extern point P;
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
	        Ln = normalize(L);
    	    ndotl = Nf.Ln;

			Hn = normalize(Ln + Vf);
			ndoth = Nf.Hn;

			tandelta2 = SQR( sqrt( max(0, 1 - SQR(ndoth))) / ndoth );

			C += Cl * (1-nonspec) * ndotl * ( exp(-tandelta2/m2) /
							(4 * m2 * sqrt( ndotl * ndotv )) );
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Greg Ward Larson's anisotropic model ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked to fit shrimp's structure //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Greg Ward Larson's anisotropic specular local illumination model.
 * The derivation and formulae can be found in:  Ward, Gregory J.
 * "Measuring and Modeling Anisotropic Reflection," ACM Computer
 * Graphics 26(2) (Proceedings of Siggraph '92), pp. 265-272, July, 1992.
 * Notice that compared to the paper, the implementation below appears to be
 * missing an factor of 1/PI, and to hav an extra L.N term.
 * This is not an error! It is because the paper's formula is for the BRDF,
 * which is only part of the kernel of the light integral, whereas shaders
 * must compute the result of the integral.
 * 
 * Inputs:
 *  N - unit surface normal
 *  V - unit viewing direction (from P toward the camera)
 *  xdir - a unit tangent of the surface which defines the reference
 *  direction for the anisotropy.
 *  roughness - the apparent roughness of the surface in xdir.
 *  yroughness - the roughness for the direction of the surface
 *  tangent which is perpendicular to xdir.
 *  */

color
LocIllumWardAnisotropic(
							normal Nn;
							vector In, xdir;
							float xroughness, yroughness;
							uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln, Hn;

	float cos_theta_r = clamp( Nf.Vf, EPS, 1), cos_theta_i, rho;
	uniform float nonspec;
	vector X = xdir / xroughness;
	vector Y = (Nf ^ xdir) / yroughness;

	extern point P;
	color C = color(0);
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			Ln = normalize(L);
			cos_theta_i = Ln.Nf;
			
			if (cos_theta_i > 0) {
				
				Hn = normalize(Vf + Ln);
				
				rho = exp( -2 * ( SQR( X.Hn ) + SQR( Y.Hn )) /
						(1 + Hn.Nf)) / sqrt( cos_theta_i * cos_theta_r );
				
				C += Cl * (1-nonspec) * cos_theta_i * rho;
			}
		}
	}
	return C / (4 * xroughness * yroughness);
}

////////////////////////////////////////////////////////////////////////////////
// Christophe Schlick isotropic BRDF ///////////////////////////////////////////// based on his paper, 'An inexpensive BRDF model for physically based /////////
// rendering, Eurographics'94, published in Computer Graphics Forum, v13, n3 ///
// p199-162, September 1994. ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color schlickspec(
                    normal Nn;
                    vector In;
                    float roughness;
					uniform string category;
        )
{
	normal Nf = faceforward( Nn, In);
	vector Vf = -In, Ln, Rn;
	float Nr = 1 / (roughness * roughness);
	float rdotv, coeff;
	uniform float nonspec;
	
    extern point P;
	color C = color(0);
    
    illuminance( category, P, Nf, S_PI_2 )
    {
        extern vector L;
        extern color Cl;
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			Ln = normalize(L);
			Rn = normalize( reflect( -Ln, Nf));	
			
        	rdotv = max( 0, Rn.Vf);
        	coeff = rdotv / (Nr - Nr * rdotv + rdotv);
			
        	C += Cl * (1-nonspec) * coeff * (Nf.Ln);
		}
    }
    return C;
}

////////////////////////////////////////////////////////////////////////////////
// Anisotropic version of Cristophe Schlick's BRDF, with geometric /////////////
// attenuation, based on his paper: 'An inexpensive BRDF model for /////////////
// physically based rendering', Eurographics'94, published in Computer /////////
// Graphics Forum, v13, n3, p199-162, September 1994. //////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Approximation of the Smith geometric attenuation function
float sgeoattenuation(
						float costheta, roughness;
		)
{
	float g1 = roughness - roughness * costheta + costheta;
	return costheta / g1;
}

// Angle dependence
float angledependence(
						float cosbeta, isotropy;
		)
{
	float i2 = SQR(isotropy);
	float cosbeta2 = SQR(cosbeta);
	return sqrt( isotropy / (i2 - i2 * cosbeta2 + cosbeta2));
}

// Zenith dependence
float zenithdependence(
						float cosbeta, roughness;
		)
{
	float cosbeta2 = SQR(cosbeta);
	float zz = 1 - (1 - roughness) * cosbeta2;
	return roughness / SQR(zz) ;
}

// NOTE: better plot it
color aschlick(
				float roughness, isotropy, ior;
				normal Nn;
				vector In, dir;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln, Hn;
	vector xdir = normalize(Nf ^ dir);

	float costheta = Vf.Nf, cospsi, cosbeta;
	float G, A, Z, D;
	uniform float nonspec;

	color C = color(0);
	extern point P;

	illuminance( category, P, Nf, S_PI_2 )
	{
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			extern vector L;
			extern color Cl;

			Ln = normalize(L);
			Hn = normalize(Vf + Ln);

			cospsi = Ln.Nf;
			cosbeta = Hn.xdir;

			// self-shadowing, without reeemission
			G = sgeoattenuation( costheta, roughness) *
						sgeoattenuation( cospsi, roughness );
			// anisotropy
			A = angledependence( cosbeta, isotropy );
			Z = zenithdependence( Nf.Hn, roughness );

			D = Z * A * G / (4 * cospsi * costheta) * cospsi;

			C += Cl * (1-nonspec) * D;
		}
	}
	float F = schlickfresnel( Nf, Vf, ior );

	return clamp(F*C, color(0), color(1));
}				

////////////////////////////////////////////////////////////////////////////////
// Cook-Torrance specular term. With Beckmann, Trowbridge-Reitz, Ward, and /////
// Heidrich-Seidel microfacet distribution functions, and the default //////////
// Torrance-Sparrow geometric attenuation, Smith attenuation, and He-Torrance //
// attenuation. ////////////////////////////////////////////////////////////////
// TODO: other distributions? (Bennett-Porteus, check Kelemen's paper) /////////
////////////////////////////////////////////////////////////////////////////////

color
cooktorrance(
				normal Nn;
				vector In, anisodir;
				float ior, k, roughness;
				uniform float distmodel, geomodel;
				uniform string category;
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -normalize(In), Ln, Hn;

	float costheta = Nf.Vf, cosalpha, cospsi, cospsi2;
	float D, G, F;
	uniform float nonspec;
	
	color Ccook = color(0);
	extern point P;
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			extern vector L;
			extern color Cl;
			Ln = normalize(L);
			
			/* The Fresnel term accounts for the amount of reflected light
			 * from the microfacet normal H, the half-angle direction, not
			 * the mean surface normal N */
			Hn = normalize(Ln+Vf);

			// store preset quantities
			cosalpha = Nf.Hn;
			cospsi = Nf.Ln;
			cospsi2 = Ln.Hn;

			// Microfacet distribution functions
			if (distmodel == 0) {
				// Cook-Torrance model, refinement of Blinn adaptations of
				// the Torrance-Sparrow model
				D = beckmann( cosalpha, roughness );
			} else if (distmodel == 1) {
				// Suggested by Kelemen and Szirmay-Kalos, which have further
				// simplify the Cook-Torrance model
				D = ward( cosalpha, roughness );
			} else if (distmodel == 2) {
				// Suggested by Blinn to the Torrance-Sparrow model, in which
				// facets are modelled as ellipsoids of varying eccentricity,
				// with m (roughness) = 0 for shiny surfaces, 1 for rough
				// surfaces. This distribution needs a normalization factor
				// not included. The Blinn model in Maya is the Torrance-Sparrow
				// model with Blinn's suggested adaptations, and fast
				// approximations to both the geometric masking/shadowing term
				// and the Fresnel term.
				D = trowbridge_reitz( cosalpha, roughness );
			} else {
				// Anisotropic distribution suggested by Heidrich and Seidel.
				D = heidrich_seidel( Nf, Vf, Ln, anisodir, roughness );
			}

			// Geometric attenuation term
			if (geomodel == 0) {
				// Original masking/shadowing term suggested by Blinn to the
				// Torrance-Sparrow model
				G = blinn_ts( costheta, cosalpha, cospsi, cospsi2 );
			} else if (geomodel == 1) {
				// Schlick's approximation of the first Smith equation for
				// a known microfacet normal
				G = schlick_smith( cospsi, costheta, roughness );
			} else {
				// Smith second equation for an averaged microfacet normal
				G = smith( costheta, cospsi, roughness );
			}

			// Using the full formula fresnel for unpolarized light, from
			// the Odforce.net's Odwiki, by Mario Marengo and the Odforce
			// community.
			F = fresnel_kr( Ln.Hn, cx_set( ior, k ) );

			// Aqsis produced some strange artifacs when getting the amount
			// of reflected light from the microfacets. Apparent solution
			// seems to clamp the value.
#if RENDERER == aqsis
			F = clamp( F, 0, 1 );
#endif

			// TODO: Re-read Heidrich & Seidel's paper, iirc, the anisotropic
			// specular term was meant to be coupled with an isotropic
			// specular term - confirm this - for the time being the
			// isotropic specular will be added via specularbrdf() shadeop

#if RENDERER == aqsis // cannot find a suitable cast for the expression
			if (distmodel == 3) {
				Ccook += Cl * (1-nonspec) * (D*G*F / costheta) +
					specularbrdf( Ln, Nf, Vf, roughness*.5);
			} else {
				Ccook += Cl * (1-nonspec) * (D*G*F / costheta);
			}
#else
			Ccook += Cl * (1-nonspec) * (D*G*F / costheta) +
				( (distmodel == 3) ?
				  specularbrdf( Ln, Nf, Vf, roughness*.5) : color(0));
#endif // Aqsis doesn't likes ternary operator?
		}
	}
	// NOTE: the values at high grazing angles seem to go up a lot
	return clamp(Ccook, color(0), color(1));
}

////////////////////////////////////////////////////////////////////////////////
// Ashikhmin-Shirley specular term /////////////////////////////////////////////
// based on Peter Stuart's implementation, original implementation further  ////
// below. Read notes about this model's diffuse term earlier in this header ////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Model from "An anisotropic Phong BRDF model", by Michael Ashikhmin and //////
// Peter Shirley. //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* 
 * ashshir.sl
 * Simple surface using Ashikmin-Shirley BRDF
 * Peter Stuart
 * $Id: ashshir.sl,v 1.4 2003/05/05 05:22:41 ophi Exp $
 * description: Ashikmin and Shirley BRDF lighting model as described
 * in "An Anisotropic Phong Light Reflection Model." Most of the work
 * is done in ashShirModel.h by two functions - one for dealing with
 * direct lighting, and the other for dealing with environment lighting.
 *
 * ---- surface parameters ----------
 * rd: diffuse reflectance
 * rs: specular reflectance
 * nu: specular exponent along xdir
 * nv: specular exponent perpendicular to xdir
 * refl: amount of reflection to use
 * envname: name of environment map to use. If this is "" the shader will
 *   try to raytrace the environment if there are samples (envsamp > 0)
 * envsamp: number of samples to use when sampling the environment
 * */

color
anisophongspec(
				normal Nn;
				vector In;
				float nu, nv, ior;
				uniform string category;
				)
{
	normal Nf = faceforward( Nn, In);
	vector Vf = -In, Ln, Hv;
	
	extern vector dPdu, dPdv;
	vector vu = normalize(dPdu), vv = normalize(dPdv);
	
	// preset quantities
	float ndotv = Nf.Vf, ndoth, ndotl, speccont;
	float nu2, nv2, nunv, nh, nhmax;

	uniform float nonspec;
	color C = color(0);
	extern point P;

    illuminance( category, P, Nf, S_PI_2 )
    {
        extern vector L;
        extern color Cl;
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			Ln = normalize(L);
    	    Hv = normalize(Ln+Vf);
			
			ndoth = Nf.Hv;
			ndotl = Nf.Ln;

        	speccont = sqrt( (nu+1) * (nv+1) / (8*PI));
			nu2 = nu * SQR( Hv.vu );
			nv2 = nv * SQR( Hv.vv );
			nunv = (nu2 + nv2) / (1 - SQR(ndoth));
			nh = pow( ndoth, nunv );
			nhmax = ndoth * max( ndotl, ndotv);
			
			C += Cl * (1-nonspec) * ndotl * (speccont * (nh / nhmax));
		}
    }
    return C * schlickfresnel( Nn, Vf, ior );
}

////////////////////////////////////////////////////////////////////////////////
// Stephen Westin's brushed metal //////////////////////////////////////////////
// from The RenderMan Repository / http://www.renderman.org ////////////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked for shrimp use, and we need the specular term only /////////
////////////////////////////////////////////////////////////////////////////////

// renamed to SHW_brushedmetal for RMR -- tal@SpamSucks_cs.caltech.edu

/*
 * brushdemetal.sl -- brushed metal (anisotropic)
 *
 * DESCRIPTION:
 *   An attempt at an anisotropic surface.
 *   The surface is idealized as a bed of slightly rough parallel
 *   cylinders. For the cylinders,
 *   I use Kajiya's Phong-like highlight from a cylinder (SIGGRAPH '89),
 *   coupled with an arbitrary shadowing/masking function. Direction
 *   of anisotropy (i.e. the axis of the cylinders) is simply the "U"
 *   direction of patch parameter.
 *
 * PARAMETERS:
 *   Ka, Kd, Ks, roughness, specularcolor - work just like the plastic
 *
 * ANTIALIASING: should antialias itself fairly well
 *
 * AUTHOR: written by Stephen H. Westin, Ford Motor Company Corporate Design
 *
 * last modified  02 December 1995 S. H. Westin
 *
 *
 * HISTORY:
 *   12/24/98 tal -- Updated to newer rman syntax
 *
 * */

color
shw_brushed_metal(
					float roughness;
					normal Nn;
					vector In, vdir;
					uniform string category;
					)
{
    float aniso;                   // Anisotropic scale factor
    float shad;                    // Phong-like shadow/masking function
    
    float sin_light, sin_eye;      // Sines of angles from tangent vector
    float cos_light, cos_eye;      // Cosines of angles from tangent vector
    
    normal Nf = faceforward (Nn, In);
    vector Vf = -In, Ln, Hn;
	float costheta = max( Nf.Vf, 0 );
    
    // "Specular" highlight in the Phong sense: directional-diffuse
    cos_eye = -vdir.Vf;
    sin_eye = sqrt( max(0, 1.0 - SQR(cos_eye)));
    
    uniform float nonspec;
	color C = color(0);
	extern point P;
    
    illuminance( category, P, Nf, S_PI_2 )
    {
        extern vector L;
        extern color Cl;
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			Ln = normalize(L);
			Hn = 0.5 * ( Vf + Ln );
			
	        cos_light = vdir.Ln;
			sin_light = sqrt( max( 0, 1 - SQR(cos_light)));
        	aniso = max( cos_light*cos_eye + sin_light*sin_eye, 0.0 );
       		shad = costheta * max( Nf.Ln, 0.0 );
			
        	C += Cl * (1-nonspec) * pow( aniso, 1.0/roughness ) * shad;
		}
    }
    return C;
}

////////////////////////////////////////////////////////////////////////////////
// Peter Stuart's implementation of the Ashikhmin-Shirley BRDF. ////////////////
// includes environment sampling. //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked to fit shrimp's structure //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Ashkihmin-Shirley BRDF
 * Peter Stuart
 * $Id: ash-shir.sl,v 1.9 2003/05/05 05:22:41 ophi Exp $
 * description: Ashikmin and Shirley BRDF lighting model as described
 * in "An Anisotropic Phong Light Reflection Model." A local illumination
 * version and an environment map version are both included. The environment
 * map version uses the importance sampling as described in the paper to reduce
 * variance.
 * 
 * ---- local illumination surface parameters ----------
 * N: normal direction
 * V: view direction
 * xdir: direction of anisotropy - can be primary or secondary
 * rd: diffuse reflectance
 * rs: specular reflectance
 * nu: specular exponent along xdir
 * nv: specular exponent perpendicular to xdir
 *
 * ---- environment illumination surface parameters ----------
 * N: normal direction
 * V: view direction
 * xdir: direction of anisotropy - can be primary or secondary
 * rd: diffuse reflectance
 * rs: specular reflectance
 * nu: specular exponent along xdir
 * nv: specular exponent perpendicular to xdir
 * samples: number of samples to take of the environment map
 * envmap: name of environment map
 * */

/* ---- fast fresnel --------
 * H: vector halfway between V and L ((V+L) / 2)
 * V: view vector (-I/|I|)
 * f0: reflectance at normal incidence
 *
 * Schlick's approximation to the fresnel function
 * */
float fastFresnel(vector Hn, Vf; float f0;)
{
    return f0 + (1-f0)*pow(1 - (Hn.Vf), 5);
}

// local illumination model

color LocIllumAshShir(
                        normal Nn;
                        vector In, xdir;
						color Cdiff, Cspec;
		                float Kd, Ks, nu, nv;
						uniform string category;
						DECLARE_AOV_OUTPUT_PARAMETERS
                        )
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -normalize(In), Ln, Hn;
	
	// store preset quantities
	float ndotv = Nf.Vf, ndotl, ndoth, nunv2;
	float maxndotl = 0;
	
    vector u_dir = xdir;
    vector v_dir = Nf ^ xdir;

    // calculations that can be done one time ...
	float diffcont = (28*Kd / 23) * (1-Ks) * (1-pow(1-ndotv/2,5));
	float speccont = sqrt( (nu+1) * (nv+1)) / 8;

	uniform float nondiff, nonspec;
	float rho_d = 0, rho_s = 0;
    extern point P;
    
    illuminance( category, P, Nf, S_PI_2 )
	{
        extern vector L;
        extern color Cl;

        Ln = normalize(L);
		// store preset quantities
		ndotl = Nf.Ln;

        nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			rho_d = diffcont * (1 - pow( 1-ndotv/2, 5));
			aov_diffuse += Cl * (1-nondiff) * rho_d * maxndotl;
        }

		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			Hn = normalize(Vf + Ln);
			// store preset quantities
			ndoth = Nf.Hn;
		
			nunv2 = ( nu * SQR(Hn.u_dir) + (nv * SQR(Hn.v_dir))) /
					(1 - SQR(ndoth));
			rho_s = speccont * pow( ndoth, nunv2) / (ndoth * max( ndotv,
						ndotl)) * fastFresnel( Hn, Vf, Ks );
			
			maxndotl = max( 0, ndotl);
			aov_specular += Cl * (1-nonspec) * rho_s * maxndotl;
        }
    }
    return Cdiff * aov_diffuse + Cspec * aov_specular;
}

// "global" illumination model
// NOTE: sampling is wrong, use the sampling function in the respective header
// instead.

/* ---- function to sample environment ----
 * P: location of sample
 * R: reflection vector
 * envname: name of environment map - use raytracing if ""
 * */
color SampleEnvironment(
                            point Pp;
                            vector Rn;
                            uniform string envname, envspace;
                            )
{
    color C = color(0);
    if (envname != "") {
        vector Rnn;
        Rnn = vtransform("camera", envspace, Rn);
        C = color environment (envname, Rnn );
    } else {
        C = trace(Pp, Rn);
    }
    return C;
}

/* ---- helper function -----
 * nu, nv: anisotropic specular exponents
 * e1: random variable [0,1]
 * */
float calcPhi(float nu, nv; float e1)
{
    return atan( sqrt( (nu+1)/(nv+1) ) * tan( S_PI_2 * e1) );
}

///////////////////////////////////////////////////////////////////////////////
/* NOTE: PRMan allows custom ray distributions, Moritz Moeller mentioned using
 * a for loop for a single sample gather call, for N rays, stored in a array.
 * This is the approach being used at the moment in rsl_shrimp_sampling.h.
 * TODO: use the Ashikhmin-Shirley sampling function in this header, instead
 * of the following EnvIllumAshsir and clean up things a bit. Also the code
 * in the sampling header uses stratified sampling which decreases noise
 * a bit. */
color EnvIllumAshShir(
                        normal Nf;
                        vector Vf, xdir;
		                float rd, rs, nu, nv;
		                uniform float samples;
		                uniform string envmap, envspace;)
{
    uniform float i;
    color C = color(0);

	float ph, cosph, sinph, th, e1, e2;
	vector ydir, Hn;

	extern point P;
	
    for (i=0; i<samples; i+=1) {
        e1=random(); e2=random();

    // Split up the hemisphere into quarters and remap e1 to one quarter.
	// This helps to stratify the samples, hopefully decreasing variance...
    // FIXME this can be done better...
        if (e1 >= 0 && e1 < 0.25)
            ph = calcPhi(nu, nv, 4 * e1);
        else if (e1 >= 0.25 && e1 < 0.5)
            ph = S_PI - calcPhi(nu, nv, 1-4*(.5-e1));
        else if (e1 >= 0.5 && e1 < 0.75)
            ph = calcPhi(nu, nv, 4 * (e1-0.5)) + S_PI;
        else
            ph = S_2PI - calcPhi(nu, nv, 1-4*(1-e1));
    
        cosph = cos(ph);
        sinph = sin(ph);
		th = acos( pow( 1-e2, 1/ (nu * SQR(cosph) + nv * SQR(sinph) + 1) ) );

        if (th <= S_PI_2) { // we don't want thetas below hemisphere
    
            // transform H (halfway vector) into space where frame is
			// <xdir, ydir, N>
            ydir = Nf ^ xdir;
			Hn = cosph * sin(th) * xdir + sinph * sin(th) * ydir
						+ cos(th) * Nf;
			
            C += rs * SampleEnvironment(P, 2 * Hn - Vf, envmap, envspace)
                    * fastFresnel(Hn,Vf,rs);

            // now generate the diffuse part using cos-weighted distribution
            e1 = random();
            e2 = random();
			Hn = cos(S_2PI * e1) * sqrt(1-e2) * xdir + sin(S_2PI * e1)
					* sqrt(1-e2) * ydir + sqrt(e2) * Nf;
      
            C += rd * SampleEnvironment(P, Hn, envmap, envspace);

			// FIXME - instead of sampling over the hemisphere, get the
			// cos-weighted average value, which can be done with a
			// cos-weighted filtered call to environment. The problem is
			// that such a filter doesn't seem to exist :(

			// C += rd * color environment( envmap, xdir, ydir, -xdir, -ydir,
			// "filter", "gaussian" );
        }
    }
    if (samples > 0) C /= samples;
    return C;
}

////////////////////////////////////////////////////////////////////////////////
// Stephen H. Westin's velvet shader ///////////////////////////////////////////
// from The RenderMan Repository , http://www.renderman.org ////////////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked to fit shrimp's structure //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * velvet.sl -- velvet
 *
 * DESCRIPTION:
 *   An attempt at a velvet surface.
 *   This phenomenological model contains three compnents:
 *   - A retroreflective lobe (back toward the light source)
 *   - Scattering near the horizon, regardless of incident direction
 *   - A diffuse color
 * 
 * PARAMETERS:
 *   Ks:	controls retroreflective lobe
 *   Kd:	scales diffuse color
 *   Ka:	ambient component (affects diffuse color only)
 *   sheen:	color of retroreflective lobe and horizon scattering
 *   roughness: shininess of fabric (controls retroreflection only)
 *
 * ANTIALIASING: should antialias itself fairly well
 *
 * AUTHOR: written by Stephen H. Westin, Ford Motor Company
 *
 * HISTORY:
 * 	2001.02.01	westin AT graphics DOT cornell DOT edu 
 *			Fixed retroreflection lobe (sign error); added
 *			"backscatter" parameter to control it; added
 *			"edginess" parameter to control horizon scatter;
 *			defined SQR()
 *
 * prev modified  28 January 1997 S. H. Westin
 * */

// slightly tweaked to fit shrimp's structure

color velvet(
                float Ks, backscatter, edginess, roughness;
                color sheen;
                normal Nn;
                vector In;
				uniform string category;
				DECLARE_AOV_OUTPUT_PARAMETERS
        )

{
    normal Nf = faceforward( Nn, In);
    vector Vf = -In, Ln;

    float cosine, sine;
	float ndotv = max(Nf.Vf, 0);

    uniform float nonspec;
	color rlobe = color(0), horizonscatter = color(0);
	extern point P;
	
    illuminance( category, P, Nf, S_PI_2)
	{
        extern vector L;
        extern color Cl;
        
        nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
            Ln = normalize(L);
            // Retroreflective lobe
			cosine = max( Ln.Vf, 0);
			rlobe += Cl * sheen * backscatter
								* pow( cosine, 1/roughness);
			// Horizon scattering
			sine = sqrt( 1.0 - SQR( ndotv ));
			horizonscatter += Cl * (1-nonspec) * sheen * Ln.Nf *
								pow( sine, edginess );
		}
    }
	
	aov_scattering += Ks * rlobe;
	aov_rimlighting += Ks * horizonscatter;
    return aov_scattering + aov_rimlighting;
}

////////////////////////////////////////////////////////////////////////////////
// Matt Pharr's skin shader based on Hanrahan-Krueger model ////////////////////
////////////////////////////////////////////////////////////////////////////////
// Slightly tweaked to fit Shrimp's structure //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* 
 * skin.sl 
 * 
 * Copyright (C) 2000-2001, Matt Pharr <mmp AT Exluna DOT com > 
 * 
 * This software is placed in the public domain and is provided as is 
 * without express or implied warranty. 
 * 
 * Surface shader that implements a shading model that should have a visual 
 * appearence generall similar to that of skin.  Based on phenomenological 
 * information about skin reflectance from Hanrahan and Krueger, 
 * "Reflection from layered surfaces due to subsurface scattering", 
 * proceedings of Siggraph 1993. 
 *
 * See SIGGRAPH 2001 course notes, "Advanced RenderMan 3: Render Harder,"
 * for notes and background information.
 * */

/* Evaluate the Henyey-Greenstein phase function for two vectors with
 * an asymmetry value g.  v1 and v2 should be normalized and g should 
 * be in the range (-1, 1).  Negative values of g correspond to more
 * back-scattering and positive values correspond to more forward scattering.
 * */

// TODO: move phase function to helper headers, add the following phase
// functions: Rayleigh-Gans, delta-Eddington, Reynolds (?), see:
// http://www.nmr.mgh.harvard.edu/~adunn/papers/dissertation/node54.html
float phase(vector v1, v2; float g) {
	float costheta = -v1.v2;
	return (1 - SQR(g)) / pow(1 + SQR(g) - 2 * g * costheta, 1.5);
}

/* Compute a the single-scattering approximation to scattering from
 * a one-dimensional volumetric surface.  Given incident and outgoing
 * directions wi and wo, surface normal n, asymmetry value g (see above),
 * scattering albedo (between 0 and 1 for physically-valid volumes),
 * and the thickness of the volume, use the closed-form single-scattering
 * equation to approximate overall scattering. */

float singleScatter(vector wi, wo; normal n; float g, albedo, thickness) {
    float win = abs(wi.n);
    float won = abs(wo.n);

    return albedo * phase(wo, wi, g) / (win + won) *
		(1. - exp(-(1/win + 1/won) * thickness));
}

vector efresnel(vector II; normal NN; float eta; output float Kr, Kt;) {
    vector R, T;
    fresnel(II, NN, eta, Kr, Kt, R, T);
    Kr = smoothstep(0, .5, Kr);
    Kt = 1 - Kr;
    return normalize(T);
}

/* Implements overall skin subsurface shading model.  Takes viewing and
 * surface normal information, the base color of the skin, a
 * color for an oily surface sheen, the ratio of the indices of 
 * refraction of the incoming ray (typically ~1 for air) to the index
 * of refraction for the transmitted ray (say something like 1.4 for
 * skin), and the overall thickness of the skin layer.  Then loops
 * over light sources with illuminance() and computes the reflected
 * skin color. */

color subsurfaceSkin(
						vector Vf;
						normal Nn;
						color skinColor, sheenColor;
                     	float eta, thickness;
						uniform string category;
						)
{
	normal Nf = faceforward( Nn, Vf );
	
	float Kr, Kt, Kr2, Kt2;
	float ldotn, hdotn;

	vector T = efresnel(Vf, Nf, eta, Kr, Kt);
	vector Ln, Hn, T2;

	color C = color(0);
	extern point P;
	    
	illuminance( category, P, Nf, S_PI_2 )
	{
        extern vector L;
        extern color Cl;
		
	    Ln = normalize(L);
	    Hn = normalize(Ln + Vf);

		// preset quantities
		ldotn = Ln.Nf;
		hdotn = Hn.Nf;
		
	    if ( hdotn > 0) {
			C += Kr * sheenColor * Cl * ldotn * pow( hdotn, 4);
		}
	    C += Kr * sheenColor * Cl * ldotn * .2;

	    T2 = efresnel(-Ln, Nf, eta, Kr2, Kt2);

		C += skinColor * Cl * ldotn * Kt * Kt2 *
				( singleScatter( T, T2, Nf, .8, .8, thickness) +
				  singleScatter( T, T2, Nf, .3, .5, thickness) +
				  singleScatter( T, T2, Nf, 0, .4, thickness ) );
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// Matt Pharr's implementation of the Kubelka-Munk model ///////////////////////
////////////////////////////////////////////////////////////////////////////////
// Slightly tweaked to fit Shrimp's structure - with some notes from ///////////
// Matt Pharr's presentation at Siggraph2001. See the Siggraph 2001 course /////
// notes for more details. /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* 
 * km.h
 * 
 * Copyright (C) 2001, Matt Pharr <mmp AT Exluna DOT com > 
 * 
 * This software is placed in the public domain and is provided as is 
 * without express or implied warranty. 
 *
 * Basic implementation of the Kubelka-Munk model for the composition
 * of layered surfaces.  See SIGGRAPH 2001 course notes, "Advanced 
 * RenderMan 3: Render Harder," for notes and background information.
 * */

/* note: check "sandwiched/layered models" from Neumann's "Constructions on
 * BRDFs". */

// Hyperbolic sine and cosine function
void sinhcosh( float t; output float sinh, cosh; )
{
    float t2 = t;
    if (t2 > 10) {
		t2 = 10;
	}
    float ex = exp(t2);
    float invex = 1 / ex;

    sinh = 0.5 * (ex - invex);
    cosh = 0.5 * (ex + invex);
}

/* This function estimates the attenuation coefficient sigmaA and scattering
 * coefficient sigmaS, for the Kubelka-Munk shading model, given the
 * reflectivity of an infinitely thick sample of the layer under consideration.
 * */
void KMEstimateCoeffs( color Rinf; output color sigma_a, sigma_s; )
{
	color a_over_s = color(0);
    if (Rinf == color(0) ) {
        sigma_a = color(1) ;
        sigma_s = color(0) ;
    }
    else {
		/* someone mentioned lots of float/color type errors in prman,
		 * - any reports on this would be apretiated - */
        a_over_s = (color(1) - Rinf) * (color(1) - Rinf)
			/ (color(2) * Rinf);
        sigma_a = color(1);
        sigma_s = sigma_a / a_over_s;
    }
}

/* This function gives the reflection and transmission coefficients for the
 * layer. Given the reflection value, we might then use it in place of the
 * diffuse coefficient Kd in a standard light reflection model, such as 
 * Lambertian diffuse, or Oren-Nayer diffuse. For a thin translucent object
 * we might use both of them together and gather incident light from both sides
 * of the object. */
void KM(	color sigma_a, sigma_s;
			float thickness;
			output varying color R, T; )
{
    uniform float i;
	extern uniform float ncomps;
	float ss, a, aa, b, sh = 0, ch = 0;
	
    for (i = 0; i < ncomps; i += 1) {
        ss = comp(sigma_s, i); a = comp(sigma_a, i);
        aa = (ss+a)/ss;
        b = sqrt(max(aa*aa - 1., 0.));

        sinhcosh(b*ss*thickness, sh, ch);

        setcomp(R, i, sh / (aa * sh + b * ch));
        setcomp(T, i, b  / (aa * sh + b * ch));
    }
}

/* Computing the reflectivity of an infinitely-thin layer from the KM
 * scattering and attenuation parameters. */
color KMInfinite( color sigma_a, sigma_s; )
{
	uniform float i;
	extern uniform float ncomps;
	float a, r;
	
    color R = color(0);
    for (i = 0; i < ncomps; i += 1) {
        a = comp(sigma_a, i) / comp(sigma_s, i);
        r = 1 + a - sqrt(SQR(a) + 2*a);
        setcomp(R, i, r);
    }
    return R;
}

/* Given the reflectances and transmittances of two layers, compute the new
 * aggregate reflectance and transmittance of them together. */
color KMComposeR( color R1, T1, R2, T2; )
{
    return R1 + T1*R2*T1 / (color(1) - R1*R2);
}

color KMComposeT( color R1, T1, R2, T2; )
{
    return T1*T2 / (color(1) - R1*R2);
}

/* Function that computes shading for a composite surface where a diffuse layer
 * has been placed on top of a surface with a standard diffuse-glossy specular
 * shading model. We compute the Kubelka-Munk reflection and transmission
 * coefficients given sigmaA and sigmaS, and the thickness of the added layer,
 * compute a new diffuse reflection coefficient Kdnew and then compute
 * plausible values for the specular coefficients Ksnew and roughness, that
 * model the effect of the added layer on the light passing through it. */
color KMOverGlossy(	normal Nn;
					vector V;
					color Kd, Ks, thickcolor;
                   	float thickness, roughness;
					DECLARE_AOV_OUTPUT_PARAMETERS )
{
	color R1 = color(0), T1 = color(0);
	color sigma_a = color(0), sigma_s = color(0);

    normal Nf = faceforward( Nn, V, Nn);

	KMEstimateCoeffs( thickcolor, sigma_a, sigma_s );
    KM(sigma_a, sigma_s, thickness, R1, T1);

	color Kdnew = KMComposeR( R1, T1, Kd, color(0) );
	color Ksnew = KMComposeR( color(0), T1, Ks, color(0) );

    float KsRatio = comp( Ks, 0 ) / comp( Ksnew, 0 );
    float roughnew = roughness * KsRatio;

	aov_surfacecolor = Kdnew;
	aov_specularcolor = Ksnew;
	aov_diffuse = diffuse(Nf);
	aov_specular = specular(Nf, -V, roughnew);
	
	return aov_surfacecolor * aov_diffuse + aov_specularcolor * aov_specular;
}

////////////////////////////////////////////////////////////////////////////////
// Peter Stuart's implementation of the Granier-Heidrich thin film /////////////
// interference BRDF. //////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Slightly tweaked to fit Shrimp's structure. /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/************************************************************
 * file: granier.sl
 * author:  Peter Stuart <stuart@cis.ohio-state.edu>
 * date: 12/07/2002
 * $Id: granier.sl,v 1.1 2003/01/25 16:49:45 ophi Exp $
 * description: Thin film interference BRDF as described in
 * "A Simple layered RGB BRDF model" by Granier and Heidrich.
 *
 * ---- surface parameters ----------
 * N: normal direction of substrate
 * N1: normal of surface under substrate
 * V: view direction
 * e0: specular exponent for light reflected from substrate
 * e1: specular exponent for light reflected from surface
 * d: thickness of substrate
 * ior: rgb index of refraction for substrate
 * lambda: rgb wavelengths (use [645 525 445])
 * Kd: diffuse coefficient
 * Ks: specular coefficient
 * */

color
LocIllumGranier(
					normal Nn, N1; vector Vf;
					float e0, e1, d;
					float ior[3]; float lambda[3];
					color SurfaceColor;
					float Kd, Ks;
					uniform string category;
					DECLARE_AOV_OUTPUT_PARAMETERS
					)
{

	// initialize everything
	float Kr, Kt, Kro, Kto;
	float specf, difff, td;
	float maxndotl;
	vector ti, ri, rti, to, ro, Ln;
	color Cdiff = color(0), Cspec = color(0);
	color diff = color(0), spec = color(0);
	uniform float nondiff, nonspec;

	// perform diffuse first
	uniform float i;
	for ( i = 0; i < 3; i += 1) {
		td = (80 / 21) * ( ior[i] / SQR( ior[i] + 1) );
		difff = 1 + 2 * cos( 4 * S_PI * ior[i] * d / lambda[i] )
				* sqrt( td * (1 - td) );
		setcomp( diff, i, difff/2 );
	}
	
	// specular
	extern point P;

	illuminance( category, P, Nn, S_PI_2 )
	{
		extern vector L;
		extern color Cl;

		// store preset quantities whenever possible
		Ln = normalize(L);
		maxndotl = max( 0, Nn.Ln );

		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			for ( i = 0; i < 3; i += 1) 
			{	
			// construct reflected & refracted rays from L, V and substrate
				fresnel( -Ln, Nn, 1/ior[i], Kr, Kt, ri, ti);
				fresnel( -Vf, Nn, 1/ior[i], Kro, Kto, ro, to);
				
				ri = normalize(ri);
				ti = normalize(ti);
				to = normalize(to);
				
				// construct reflected ray from L refracted in substrate
				rti = normalize( reflect( ti, N1 ) );
				
				// total internal reflection
				if ( (ti . N1) >= 0 || (to . N1) >= 0 || (rti . to) >= 0) {
					Kt = 0;
				}
				
				specf = Kr * pow( ri.Vf, e0) + Kt * Kto * pow( rti.-to, e1 );
				setcomp(spec, i, specf);
			}
			// specular
			Cspec += Cl * (1-nonspec) * spec * maxndotl;
		}

		// diffuse term
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			Cdiff += Cl * (1-nondiff) * diff * maxndotl;
		}
	}

	// build AOVs
	aov_surfacecolor += SurfaceColor;
	aov_diffuse += Kd * Cdiff;
	aov_specular += Ks * Cspec;
	
	return aov_surfacecolor * aov_diffuse + aov_specular;
}

////////////////////////////////////////////////////////////////////////////////
// Lafortune shader, with anisotropy support (Cx, Cy, Cz), and the correction //
// factor for the model suggested by Attila Neumann, in his dissertation: //////
// "Constructions of Bidirectional Reflectance Distribution Functions". ////////
// Originally based on Stephen Westin's RSL implemention, and in Ryan //////////
// Heniser's DSO implementation. ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * lafortune.sl -- Implement the BRDF representation of Lafortune et al.
 *
 * DESCRIPTION:
 *	Implement the BRDF representation described in "Non-Linear
 *	Approximation of Reflectance Functions", by Eric P. F. Lafortune,
 *	Sing-Choong Foo, Kenneth E. Torrance, and Donald P. Greenberg, in
 *	SIGGRAPH 97 Proceedings.
 *
 * PARAMETERS:
 *   coeff:	Coefficients for each lobe. There will be
 *		3 coefficients per lobe per wavelength.
 *		for each lobe of Nlobes:
 *		  for each channel of N_WAVES:
 *		    cxy, cz, n
 *		where cxy, cz are the directional and scale components
 *		n is the exponent for the cosine
 *
 *   Cs:	Diffuse color of surface. This provides the diffuse
 *		term that is an integral part of the model. This
 *		will be the diffuse component from the Lafortune
 *		model multiplied by pi, as the RenderMan definition
 *		of reflectance is different.
 * colormatrix:	3x3 color transformation matrix, if needed.
 *		This is here because measured data may be in the
 *		RGB space of some sensor (e.g. a digital camera)
 *		and may need correction for display. Since the three
 *		channels are represented independently (i.e. each
 *		channel will have different directionality), this
 *		correction must be done on a per-pixel basis.
 *		Since the correction may be different for each
 *		shader, we must do it here. Default is the identity
 *		matrix.
 *
 * ANTIALIASING: should antialias itself fairly well
 *
 * AUTHOR: written by Stephen H. Westin, Cornell University Program of
 *	Computer Graphics
 *
 * HISTORY:
 *
 * initial version 22 March 1999 S. H. Westin
 *
 * 22 March 1999 S. H. Westin
 *		Flipped sign on Cx to agree with description in SIGGRAPH paper
 * 25 October 2000 S. H. Westin
 *		Multiply lobes by incident cosine.
 * 27 October 2000 S. H. Westin
 *		Multiply lobes by incident cosine. Before, I was trying to
 *		use diffuse(), which doesn't work.
 *
 * */

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/* Added anisotropy support, so each wave of each lobe has 3 coefficients 
 * and 1 exponent : Cx, Cy, Cz, n.
 * Description of behaviour:
 * Anisotropy, Cx != Cy
 * Retro-reflection, Cx = Cy > 0
 * Specularity at grazing angles, Cx = Cy > Cz
 * Phong, -Cx = -Cy = Cz = nth_root((n+2)/(2*PI))
 * Generalized diffuse behaviour, Cx = Cy = 0, Cz = nth_root((n+2)/(2*PI))
 * */

#define LOBESIZE 4
// Number of wavelengths (wired in)
#define N_WAVES 3
// Number of lobes (wired in)
#define N_LOBES 3
// data lenght
#define COEFFLEN 36 /* (N_LOBES*N_WAVES*LOBESIZE) */
// lobe lenght
#define LOBELEN 12 /* (LOBESIZE*N_WAVES) */

////////////////////////////////////////////////////////////////////////////////
color
lafortunersl(
				float Ka, Kd, Ks;
				vector correction; // Neumann correction (for each lobe)
				color cs; // surface color, will be supplied via blocks later
				uniform float model, cmatrix; // clamp it to valid range
				normal Nn;
				vector In, dir; // normalize(I), dPdu
				uniform string category;
				DECLARE_AOV_OUTPUT_PARAMETERS
		)
{
	uniform float colormatrix[9] = {  1.063302, 0.382044, -0.445346,
									 -0.298125, 1.667665, -0.369540,
									 -1.322302,-0.446321,  2.768624 };

#if RENDERER==aqsis || RENDERER==pixie
	// Aqsis can access components via xyz/comp only
	float p[3] = { xcomp(correction), ycomp(correction), zcomp(correction) };
	// both Aqsis and Pixie can't initialize arrays with single element
	// get storage for lobe coefficients
	uniform float coeff[COEFFLEN];
#else
	float p[3] = { correction[0], correction[1], correction[2] };
	uniform float coeff[COEFFLEN] = 0;
#endif // Aqsis component access, Aqsis & Pixie array initialization

	// get lobe data (model should be in valid range)
	lafortunedata( model, coeff );

	vector local_x, local_y; // U and V direction vectors
	vector local_z;
	vector Ln;

	float x, y, z, f; // subterms
	float Cx, Cy, Cz, n; // coefficients+exponent
	float fr = 0, fg = 0, fb = 0; // RGB components of non-Lambertian term
	uniform float basepointer, j;

	// get unit vector in U parameter direction
	local_x = normalize(dir);
	local_z = faceforward( Nn, In );
	vector Vf = -In;
	// get a local coordinate system
	local_y = local_z^local_x;
	// store presets whenever possible
	float xvf = local_x.Vf, yvf = local_y.Vf, zvf = local_z.Vf;
	float ndotl; /* local_z.Ln */

	// The first term is the diffuse component
	uniform float nondiff, nonspec;
	// exponents and Neumann pump-up
	uniform float rexponent, gexponent, bexponent;
	float norm;
	color cdiff = color(0), cspec = color(0);

	extern point P;

	illuminance( category, P, local_z, S_PI_2 )
	{
		extern vector L;
		extern color Cl;

		Ln = normalize(L);
		ndotl = local_z.Ln;

		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			cdiff += Cl * (1-nondiff) * ndotl;
		}

		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {

			// compute the terms
			x = xvf * local_x.Ln;
			y = yvf * local_y.Ln;
			z = zvf * ndotl;

			/* Coefficient structure:
			 * for each lobe of N_LOBES:
			 * for each channel of N_WAVES:
			 * Cx, Cy, Cz, n
			 * where Cx, Cy, are the directional components
			 * and Cz the scale exponent
			 * with n the exponent for the cosine.
			 * */

			for ( basepointer = 0; basepointer < COEFFLEN;
					basepointer += LOBELEN )
			{
				rexponent = coeff[basepointer+3];
				gexponent = coeff[basepointer+LOBESIZE+3];
				bexponent = coeff[basepointer+2*LOBESIZE+3];

				fr = fg = fb = 0;

				// red
				Cx = coeff[basepointer];
				Cy = coeff[basepointer+1];
				Cz = coeff[basepointer+2];
				if (Cx * Cy * Cz * rexponent == 0) {
					/* 0,0,0,1, no more lobes, so exit the loop,
					 * leaving the color at last lobe's value (or 0) */
					break;
				}
				f = x * Cx + y * Cy + z * Cz;
				fr = (f > EPS * rexponent) ?
					/* beware of sign reversal with exponentiation */
					sign(f) * pow( abs(f), rexponent ) : 0;
				// green
				Cx = coeff[basepointer+LOBESIZE];
				Cy = coeff[basepointer+LOBESIZE+1];
				Cz = coeff[basepointer+LOBESIZE+2];
				f = x * Cx + y * Cy + z * Cz;
				fg = (f > EPS * gexponent) ?
					sign(f) * pow( abs(f), gexponent ) : 0;
				// blue
				Cx = coeff[basepointer+2*LOBESIZE];
				Cy = coeff[basepointer+2*LOBESIZE+1];
				Cz = coeff[basepointer+2*LOBESIZE+2];
				f = x * Cx + y * Cy + z * Cz;
				fb = (f > EPS * bexponent) ?
					sign(f) * pow( abs(f), bexponent ) : 0;
				/* Neumann "pump-up" factor, from Attila Neumann's dissertation
				 * "Constructions of Bidirectional Reflection Distribution
				 * Functions", Technische Universität Wien. */
				// beware of division by zero
				norm = ndotl / max( EPS, pow( max( ndotl, zvf),
							p[basepointer/LOBELEN]));
				cspec += Cl * (1-nonspec) * color( fr, fg, fb ) * norm;
			}
		}
	}
	// color correction from camera space
	// use dot product with rows of matrix (3x3)
	if (cmatrix == 1) { // do color correction
#if RENDERER == aqsis // Aqsis component access via xyz/comp only
		fr = colormatrix[0] * comp(cspec, 0) +
			colormatrix[1] * comp(cspec,1) + colormatrix[2] * comp(cspec,2);
		fg = colormatrix[3] * comp(cspec, 0) +
			colormatrix[4] * comp(cspec,1) + colormatrix[5] * comp(cspec,2);
		fb = colormatrix[6] * comp(cspec, 0) +
			colormatrix[7] * comp(cspec,1) + colormatrix[8] * comp(cspec,2);
#else
		fr = colormatrix[0] * cspec[0] + colormatrix[1] * cspec[1] +
			colormatrix[2] * cspec[2];
		fg = colormatrix[3] * cspec[0] + colormatrix[4] * cspec[1] +
			colormatrix[5] * cspec[2];
		fb = colormatrix[6] * cspec[0] + colormatrix[7] * cspec[1] +
			colormatrix[8] * cspec[2];
#endif // Aqsis component access via xyz/comp only
		cspec = color( fr, fg, fb );
	}

	aov_surfacecolor += cs;
	aov_ambient += Ka * ambient();
	aov_diffuse += Kd * cdiff;
	aov_specular += Ks * cspec;

	return aov_surfacecolor * (aov_ambient + aov_diffuse) + aov_specular;
}

////////////////////////////////////////////////////////////////////////////////
// based on ATI's paper 'Hair Rendering and Shading', by Thorsten Scheuermann //
// 3D application research group, ATI research Inc. ////////////////////////////
// this is a hair rendering technique originally made for polygon hair models //
// a mix of Kajiya-Kay hair shader model, and Marscher's model, presented at  //
// Siggraph2003 ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// shifting specular highlights along lenght of hair
vector shifttangent(
					vector Tn;
					normal Nf;
					float shift;
		)
{
	vector shiftedt = Tn + shift * Nf;
	return normalize(shiftedt);
}

// Specular strand lighting
float strandspecular(
						vector Tn, Vn, Ln;
						float exponent;
		)
{
	vector Hn = normalize(Ln + Vn);
	float dotth = Tn.Hn;
	float sinth = sqrt( max( 0, 1 - SQR(dotth)) );
	float diratten = smoothstep(-1, 0, dotth);
	return diratten * pow( sinth, exponent);
}

// Main
color tshair(
					float Ka, Kd, Ks; // ambient, diffuse, specular coeffs
					float shift; // primary specular highlight shift
                    float shift2; // secondary specular highlight shift
					float exponent; // primary specular exponent
                    float exponent2; // secondary specular exponenet
                    float specmask; // specular mask,to shift secondary spec
					color Cdiff; // surface color,sometimes bound to geo
					color Cbase; // hair base color
					color Ctip; // hair tip color
					color Cspec; // primary specular color
                    color Cspec2; // secondary specular color
					normal Nn;
					vector Vf;
					uniform string category;
					DECLARE_AOV_OUTPUT_PARAMETERS
		)
{
	/* dPdv is needed for direction, P for the illuminance
	 * construct, and the v coordinate for the color blend in the color
	 * contribution. */
	
	extern vector dPdv;
	extern float v;
	
	vector T = normalize(dPdv);
	normal Nf = faceforward( Nn, Vf );
	
    vector t1 = shifttangent( T, Nf, shift - .5);
    vector t2 = shifttangent( T, Nf, shift2 - .5);
	vector Ln = vector(0);
	
	float dottin = T.Vf;

	uniform float nondiff, nonspec;
	color Cdd = color(0);
	color Css = color(0);
	extern point P;
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;
		Ln = normalize(L);
		
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			/* Scaled diffuse term. Note that in the paper they use
			 * ambient occlusion to fake the hair shadowing. */
			Cdd += Cl * (1-nondiff) * clamp( mix( .25, 1.0, Nf.Ln), 0, 1);
		}
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			// Primary specular + secondary specular
			Css += Cl * (1-nonspec) * (Cspec * strandspecular(
						t1, Vf, Ln, exponent) + Cspec2 * specmask *
					strandspecular( t2, Vf, Ln, exponent2 ));
		}
	}

	aov_surfacecolor += mix( Cbase, Ctip, v);
	aov_ambient += Ka * ambient();
	aov_diffuse += Kd * Cdd;
	aov_specular += Ks * Css;

	return aov_surfacecolor * (aov_ambient + aov_diffuse) + aov_specular;
}

////////////////////////////////////////////////////////////////////////////////
// Hair shader, Kajiya-Kay model, based on Mario Marengo's work, from //////////
// http://forums.odforce.net/index.php?showtopic=2208&st=180 ///////////////////
// slightly tweaked for shrimp /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color kajiyakay(
					float Ka, Kd, Ks; // ambient, diffuse, specular coeffs
					float rough; // specular roughness
					color Cdd; // "hidden" surf color,sometimes bound to geo
					color Cbase; // hair base color
					color Ctip; // hair tip color
					color Css; // specular color
					normal Nn;
					vector Vf;
					uniform string category;
					DECLARE_AOV_OUTPUT_PARAMETERS
		)
{
	/* We're going to need dPdv for direction, P for the illuminance
	 * construct, and the v coordinate for the color blend in the color
	 * contribution. */
	extern vector dPdv;
	extern float v;
	
	vector T = normalize(dPdv), Ln;
	normal Nf = faceforward( Nn, Vf );
	
	float dottin = T.Vf, dottl, sdottl;

	uniform float nondiff, nonspec;
	color Cdiff = color(0);
	color Cspec = color(0);
	extern point P;
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;
		Ln = normalize(L);
		
		// Diffuse term
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			dottl = T.Ln;
			sdottl = sqrt( max( 0, 1 - SQR(dottl) ));
			Cdiff += Cl * (1-nondiff) * sdottl;
		}
		
		// Specular term
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			dottl = T.Ln;
			sdottl = sqrt( max( 0, 1 - SQR(dottl) ));
			Cspec += Cl * (1-nonspec) * pow( dottl * dottin + sdottl
						* sqrt( max(0, 1 - SQR(dottin))), 1.0/rough );
		}
	}
	
	// blend base color and tip color based on v coordinate
	aov_surfacecolor = Cdd * mix( Cbase, Ctip, v);
	aov_specularcolor = Css;
	aov_ambient = Ka * ambient();
	aov_diffuse = Kd * Cdiff;
	aov_specular = Ks * Cspec;
		
	return aov_surfacecolor * (aov_ambient + aov_diffuse) +
				(aov_specularcolor * aov_specular);
}

////////////////////////////////////////////////////////////////////////////////
// Gooch Non-PhotoRealistic shading model, by Mike King ////////////////////////
// taken from The RenderMan Repository, http://www.renderman.org ///////////////
////////////////////////////////////////////////////////////////////////////////
// slightly tweaked to fit shrimp's structure //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Renamed to MKgooch.sl for RMR -- tal AT renderman DOT org

/* s_gooch.sl - a simple implementation of the Gooch
 *              non-photorealistic lighting model
 *
 * DESCRIPTION   : This model is described in "A Non-Photorealistc
 *                 Lighting Model For Automatic Technical 
 *                 Illustration" in the Siggraph 1998 Proceedings.
 *                 http://www.cs.utah.edu/~gooch/SIG98/abstract.html
 *
 * INPUTS        : 
 *  Ka           : ambient factor
 *  Kd           : diffuse factor
 *  Ks           : specular factor
 *  alpha        : Gooch cool color factor
 *  beta         : Gooch warm color factor
 *  b            : used to construct the cool color
 *  y            : used to construct the warm color
 *  roughness    : specular roughness
 *  specularcolor: specular hilight color
 *
 * AUTHOR: written by Mike King
 *               
 ***************************************************/

color MKgooch(
                normal Nn;
                vector In;
                float Ka, Kd, Ks, calpha, cbeta, roughness, b, y;
                color SurfaceColor, SpecularColor;
				uniform string category;
				DECLARE_AOV_OUTPUT_PARAMETERS
                )
{
    normal Nf = faceforward( Nn, In);
    vector Vf = -In, Ln;
    
    color blue = color(0, 0, b);
    color yellow = color(y, y, 0);
    color cgooch = color(0);
    
    float ldotn, blendval;
    color kcool, kwarm;

    kcool = blue + (SurfaceColor * calpha);
    kwarm = yellow + (SurfaceColor * cbeta);
    extern point P;
	
    illuminance( category, P, Nf, S_PI)
	{
        extern vector L;
        Ln = normalize(L);
        
        ldotn = Ln.Nf;
        blendval = .5 * (1+ldotn);
        cgooch += mix( kcool, kwarm, blendval);
    }
	
	aov_surfacecolor += SurfaceColor;
	aov_specularcolor += SpecularColor;
	aov_ambient += Ka * ambient();
	aov_diffuse += Kd * cgooch;
	aov_specular += Ks * specular( Nf, Vf, roughness);

	return aov_ambient + aov_diffuse + aov_specularcolor * aov_specular;	
}

////////////////////////////////////////////////////////////////////////////////
// Stephen H. Westin's wood shader, Cornell University Program of Computer /////
// Graphics. REFERENCE: Stephen R. Marschner, Stephen H. Westin, Adam Arbree ///
// and Jonathan T. Moon, "Measuring and Modelling the Appearance of Finished ///
// Wood", SIGGRAPH 2005. ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Slightly tweaked to fit Shrimp's structure. /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* Note: Mario Marengo mentioned a problem (and a solution) regarding specifying
 * surface tangents to build the local space of the fiber vectors - we need
 * to extract texture gradients from the geometry. */

/*
 * woodreflectance.sl -- wood (anisotropic)
 *
 * DESCRIPTION:
 *   Simple wood shader, driven by four textures:
 *    RGB diffuse color
 *    RGB fiber (anisotropic highlight) color
 *    x,y,z of unit vector giving local fiber orientation in local
 *      coordinates
 *    beta, a "roughness" parameter to control width of highlight.
 *   If any of these files is not specified, we fall back on a
 *    uniform parameter. So far, every map has had uniform beta.
 *
 * PARAMETERS:
 *
 *   diffuse_texture: name of file containing 3-channel texture
 *     modulating color across the patch.
 *
 *   fiber_texture: name of file containing 3-channel texture
 *     modulating fiberColor across the patch. This map is divided by
 *     20 because it generally contains values greater than 1, which
 *     would be clamped in an integer pixel format.
 *
 *   axis_texture: name of file containing 3-channel texture
 *     modulating fiberAxis across the patch. Values are mapped
 *     [0:max] -> [-1:1] for each component, since most image file
 *     formats are unsigned.
 *
 *   beta_texture: name of file containing 1-channel texture
 *     modulating beta across the patch.
 *
 *   eta: Index of refraction of surface finish. Used to modulate
 *     highlight and subsurface with varying angle. eta=0 used to
 *     signal absence of overcoat.
 *
 *   color, fiberColor: fallback diffuse and directional colors if
 *     texture is missing
 *
 *   fiberAxis: fallback fiber direction if texture is missing
 *
 *   beta: width of subsurface highlight if texture is missing
 *
 *   roughness: width of surface (white) highlight
 *
 *   Ka, Kd, Ks: normal Renderman coefficients. Ka and Kd modulate the
 *     diffuse component (parameter "color"), and Ks is a fudge factor
 *     on the surface (white) highlight.
 *
 * ANTIALIASING: provided by texture lookup
 *
 * REFERENCE: Stephen R. Marschner, Stephen H. Westin,
 *              Adam Arbree, and Jonathan T. Moon,
 *            "Measuring and Modeling the Appearance of
 *               Finished Wood", SIGGRAPH 2005
 *
 * AUTHOR: written by Stephen H. Westin,
 *                    Cornell University Program of
 *                    Computer Graphics
 *         last modified by $Author: westin $
 *
 * VERSION: $Revision: 1.2 $
 *
 * HISTORY:
 * $Log: woodreflectance.sl,v $
 * Revision 1.2  2005/05/10 20:02:15  westin
 * Modified to use 4 TIFF text ure maps rather than a single
 *  10-channel map.
 *
 * Revision 1.1  2005/05/09 21:10:18  westin
 * Working version of shader; used for animation on Proceedings DVD-ROM.
 *
 */

color
woodreflectance(
                    float eta, beta, roughness;
                    float Ka, Kd, Ks;
                    vector fiberAxis, In;
                    normal Nn;
                    color diffuseColor, fiberColor;
					uniform string category;
					DECLARE_AOV_OUTPUT_PARAMETERS
        )
{
	uniform float sqrt2pi = 2.5066283, nondiff, nonspec;
	extern vector dPdu;
	extern float u, v;
	
	vector local_z;	// Surface normal in "world" space
	vector local_x; // unit vector in U direction
	vector local_y; // unit vector in V direction

	normal Nf = faceforward( Nn, In);
	color highlight;
	vector axis, Ln;
	vector Rdir, Hn;
	float tx_beta;

	vector ssInDir, ssOutDir; // Light and eye vector, possibly refracted
	float thInPrime, thOutPrime;
	float halfAngle, diffAngle;
	float fiberFactor, geometryFactor;

	float Kr, Kt;           // for output of fresnel()
	float ssAtten;          /* Attenuation from going through the smooth
							 * interface twice */
	float dummy, ssAttenOut, ssFactor, cosIncline;

    
	/* Get local coordinate system in terms of native parameters (u, v).
	 * * We should really use (s, t). */
	local_z = Nf;
	
	// Get unit vector in "u" parameter direction
	local_x = normalize( dPdu );
	
	// Get final local basis vector y perpendicular to x and z.
	local_y = local_z ^ local_x;

	/* If there is a clear overcoat, calculate refraction direction and
	 * attenuation. */
	if ( eta != 1 ) {
		fresnel ( In, Nf, 1/eta, Kr, Kt, Rdir, ssOutDir );
		ssOutDir = -ssOutDir;
		/* Use (1-Kr) rather than Kt, because we are dealing with power,
		 * not radiance. */
		ssAtten = 1 - Kr;
	} else {
		ssOutDir = -In;
		ssAtten = 1;
	}
	
	normalize(ssOutDir);

	color axisTemp, Cdiff = color(0), Cspec = color(0);
	highlight = fiberColor;

	axis = comp( fiberAxis, 0 ) * local_x + comp( fiberAxis, 1 )
				* local_y + comp( fiberAxis, 2 ) * local_z;
	normalize(axis);
	
	tx_beta = beta;
	thOutPrime = asin( ssOutDir.axis);

	extern point P;
	
    // Calculate anisotropic highlight for each light
	illuminance( category, P, Nf, S_PI_2 ) {
		
		extern vector L;
		extern color Cl;
		
		Ln = normalize(L);

		// Refract at smooth surface
		if ( eta != 1 ) {
			fresnel( -Ln, local_z, 1/eta, Kr, Kt, Rdir, ssInDir);
			/* Use (1-Kr) rather than Kt, because we are dealing with power,
			 * not radiance. */
			ssAttenOut = 1 - Kr;
		} else {
			ssInDir = -Ln;
			ssAttenOut = 1;
		}

		ssFactor = max( 0, -ssInDir.local_z) * ssAtten*ssAttenOut;
		thInPrime = asin( -ssInDir.axis);
		halfAngle = thOutPrime + thInPrime;
		diffAngle = thOutPrime - thInPrime;
		
		// Compute value of Gaussian at this angle
		fiberFactor = tx_beta * exp( -pow( halfAngle / tx_beta, 2)/2)/sqrt2pi;
		
		cosIncline = cos( diffAngle / 2);
		geometryFactor = 1 / pow( cosIncline, 2);
		fiberFactor *= geometryFactor;
		
        // Add in diffuse term, attenuated by surface term.
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			Cdiff += Cl * (1-nondiff) * ssFactor;
		}
		// Add in fiber highlight, also attenuated.
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			// Second Fresnel call is for strength of surface highlight.
			Hn = normalize( -In + Ln );
			fresnel( In, Hn, 1.0/eta, Kr, Kt);
			
			Cspec += Cl * (1-nonspec) * (fiberFactor * highlight * ssFactor +
					Kr * pow( max( 0, Hn.local_z), 1/roughness));
		}
	}
	
	aov_surfacecolor += diffuseColor;
	aov_ambient += Ka * ambient();
	aov_diffuse += Kd * Cdiff;
	aov_specular += Ks * Cspec;
	

	return aov_surfacecolor * (aov_ambient + aov_diffuse) + aov_specular;
}

////////////////////////////////////////////////////////////////////////////////
// Raytraced glass shader, based on Larry Gritz's glass shader, on Mario ///////
// Marengo's glass shader, and on Alan Warren's port of Mario Marengo's VEX ////
// glass shader. Dispersion added, in RYGCBV, ior data taken from optics ///////
// table generated by modified mktables.C. Larry Gritz's glass shader can be ///
// found at The RenderMan Repository: http://www.renderman.org /////////////////
// Mario Marengo's VEX glass shader can be found at the OdForce forums: ////////
// http://forums.odforce.net/index.php?/topic/4196-glass/  /////////////////////
// Alan Warren's RSL2.0 glass shader can be found at:    ///////////////////////
// http://www.alan-warren.com/2009/07/16/248/    ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/* Compute the color of a glass-like surface with coherent reflections
 * and refractions.
 * */

/* NOTE: Aqsis doesn't has raytracing, but BMRT's rayserver can be used,
 * although it is untested. As such, Aqsis workarounds added. Pixie, as
 * of r1209 crashes, even with array initalization workaround added.
 * Perhaps it doesn't likes arrays as function arguments? Some time is
 * needed to find out what's going on exactly.
 * NOTE: can't seem to be able to assign to arrays in any way at all in
 * rsl_shrimp_color.h, for rgb>spec6 and spec6>rgb conversion - as of 1.4.2 */

// LocIllumGlossy, but we already have Nf and Vf set in the glass body
color
locillumglassy(
				normal Nf;
				vector Vf;
				float roughness, sharpness;
				uniform string category;
		)
{
	float w = .18 * (1 - sharpness);
	uniform float nonspec;
	
	extern point P;
	color C = color(0);
	
	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;

		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
		
			vector Ln = normalize(L);
			vector Hn = normalize(Ln + Vf);
			C += Cl * (1-nonspec) * (smoothstep(.72-w, .72+w,
						pow( max( 0, Nf.Hn), 1/roughness)));
		}
	}
	return C;
}

////////////////////////////////////////////////////////////////////////////////
// sample reflection, refraction, transmission
void
glasssample(
				vector In;
				normal Nf;
				float eta, Kr, Kt, aexp, aamp, krblur, ktblur, entering;
				uniform float raydepth, rbounces, rsamples, bias;
				uniform float rmaxdist, tmaxdist, optics;
				uniform string envmap, rtype;
				color cabs, attencolor;
				uniform string category;
				output varying color crefl, crefr, ctran;				
		)
{
	crefr = color(0); crefl = color(0); ctran = color(1);
	vector rdir = vector(0), tdir = vector(0);
	float kr = 0, kt = 0;
	color acoeff = color(1);
	
	// Fresnel term
	fresnel( In, Nf, eta, kr, kt, rdir, tdir);
	kt = 1 - kr; // physically incorrect but portable

	// cut reflections at higher ray levels
	kr = (raydepth > rbounces) ? 0 : kr;
	
	// attenuation
	if (raydepth > 0 && entering == 0) {
		extern varying vector I;
		float ilen = length(I);
		float d = (aexp == 1) ? ilen : pow( ilen, aexp);
		acoeff = expc( cabs * d);
	} else {
		acoeff = color(1);
	}

	// get shadows, or reflection + refraction
	if (rtype == "transmission") { // doing shadows
		
		color opacity = mix( color(kt), kt * acoeff, Kt);
		color darkmix = mix( color(kr), kr * acoeff, Kr);
		color shad = attencolor;
		// convert to hsv
		opacity = ctransform("rgb", "hsv", opacity );
		darkmix = ctransform("rgb", "hsv", darkmix );
		
#if RENDERER == aqsis // can only access components with (xyz)comp
		// desaturate and convert to rgb
		opacity = ctransform("hsv", "rgb", color( comp(opacity,0), 0,
					comp(opacity, 2)));
		darkmix = ctransform("hsv", "rgb", color( comp(darkmix,0), 0,
					comp(darkmix, 2)));
		// convert shad to hsv, invert its hue, then back to rgb
		color component = ctransform("rgb", "hsv", shad);
		float reversehue = comp(component, 0) + 0.5;
#else
		// desaturate and convert to rgb
		opacity = ctransform("hsv", "rgb", color( opacity[0], 0, opacity[2]));
		darkmix = ctransform("hsv", "rgb", color( darkmix[0], 0, darkmix[2]));
		// convert shad to hsv, invert its hue, then back to rgb
		color component = ctransform("rgb", "hsv", shad);
		float reversehue = component[0] + 0.5;
#endif // component access

		reversehue = (reversehue > 1) ? reversehue - 1.0 : reversehue;
		setcomp( component, 0, reversehue);
		shad = ctransform("hsv", "rgb", component);
		
		ctran = (opacity * shad) + darkmix;
		
	} else { // doing reflection & refraction
		
		// reflection, note: envmap after last ray bounce ?
		if (Kr * kr > 0) {
			color k = mix( color(kr), kr * acoeff, Kr);
			float maxcont = vmax(k);
			if (maxcont > 0) {
				crefl = k * environment( envmap, rdir, "samples", rsamples,
						"blur", krblur, "maxdist", rmaxdist );
			}
		}
		// refractions, note: envmap after last ray bounce ?)
		if (Kt * kt > 0) {
			color k = mix( color(kt), kt * acoeff, Kt);
			float maxcont = vmax(k);
			if (maxcont > 0) {
				crefr = k * environment( envmap, tdir, "samples", rsamples,
						"blur", ktblur, "maxdist", tmaxdist );
			}
		}
		ctran = color(1);
	}	
}

////////////////////////////////////////////////////////////////////////////////
// Glass body - optics should be in [0,112] range
void
rtglass(
			float Ka, Kd, Ks, Kr, Kt, ior, roughness, sharpness;
			color attencolor;
			float krblur, ktblur, aexp, aamp;
			uniform float samples, rmaxdist, tmaxdist, bias;
			uniform string spectype;
			uniform float rbounces, optics, spec6;
			vector In; normal Nn;
			uniform string envmap;
			output varying color ci, oi;
			uniform string category;
			DECLARE_AOV_OUTPUT_PARAMETERS
	   )
{
	color Camb = color(0), Cdiff = color(0), Cspec = color(0);
	color crefl = color(0), crefr = color(0), ctran = color(0);

	color attenhsv = ctransform("rgb", "hsv", attencolor);
	
#if RENDERER == aqsis // can only access components with (xyz)comp
	color cabs = ctransform("hsv", "rgb", color( comp(attenhsv, 0),
				comp(attenhsv, 1), 1));
#else
	color cabs = ctransform("hsv", "rgb", color(attenhsv[0], attenhsv[1], 1));
	cabs = cabs - vmax(cabs) - vmin(cabs) * 2 * aamp;
#endif // component access

	normal Nf = Nn;
	float entering = 1;

#if RENDERER == aqsis || RENDERER == pixie
	// can't initialize arrays with single scalar in Aqsis & Pixie
	uniform float etas[6] = {0,0,0,0,0,0};
#else
	uniform float etas[6] = 0;
#endif // array initialization workaround for Aqsis & Pixie

	/* get current ray depth and scale down samples as ray depth goes up
	 * if needed. Get ray type, for transmission rays in shadows */
	uniform float raydepth = 0;
	uniform string rtype = "";
	rayinfo("depth", raydepth);
	rayinfo("type", rtype);
	uniform float rsamples = (raydepth > 1) ? max( 1, samples /
								pow( 2, raydepth)) : samples ;

	// dispersion in spec6. A workaround needs to be added for Pixie.
	// It can't initialize arrays with a single element, needs to be
	// filled with all element. Also seems to dislike arrays as function
	// arguments - this needs some testing, so for the time being, RYGCBV
	// dispersion is disabled for Pixie in the XML block.
	// NOTE: still, it crashes, as of r1209
	// 
	if (spec6 == 1) { // RYGCBV
		
		uniform float i;

		// get eta for RYGCBV wavelenghts
		etaTableRYGCBV( optics, etas );

		// temp storage
#if RENDERER == aqsis || RENDERER == pixie
		// can't initialize arrays with single scalar in Aqsis & Pixie
		varying float	frefl[6] = {0,0,0,0,0,0}, frefr[6] = {0,0,0,0,0,0},
						ftran[6] = {0,0,0,0,0,0}, srefl[6] = {0,0,0,0,0,0},
						srefr[6] = {0,0,0,0,0,0}, stran[6] = {0,0,0,0,0,0};
#else			
		varying float frefl[6] = 0, frefr[6] = 0, ftran[6] = 0;
		varying float srefl[6] = 0, srefr[6] = 0, stran[6] = 0;
#endif // array initialization workaround for Aqsis & Pixie
		
		for(i = 0; i < 6; i += 1) { // for each R,Y,G,C,B,V

			// ray is entering for each wavelenght?
			// ray lenght at each wavelenght varies enough to consider
			// it, or should it be placed outside the loop?
			float idotn = -In.Nn;
			/* if I.N>0, ray is entering the medium, else ray is exiting
			 * and eta is the reverse of the ior when ray is entering the
			 * medium. Ray lenght varies with ior > lambda */
			float eta;
			if (idotn > 0) {
				entering = 1; eta = 1/etas[i]; Nf = Nn;
			} else {
				entering = 0; eta = etas[i]; Nf = -Nn;
			}

			// sample glass, get reflection, refraction, transmission
			glasssample( In, Nf, eta,
					Kr, Kt, aexp, aamp, krblur, ktblur,
					entering, raydepth, rbounces, rsamples, bias,
					rmaxdist, tmaxdist, optics, envmap, rtype,
					cabs, attencolor, category, crefl, crefr, ctran );

			// convert crefl, crefr, ctran to spec6
			rgbToSpec6( crefl, srefl );
			rgbToSpec6( crefr, srefr );
			rgbToSpec6( ctran, stran );
			// fill R,Y,G,C,B,V
			frefl[i] = srefl[i];
			frefr[i] = srefr[i];
			ftran[i] = stran[i];
		}
		// convert spec6 to rgb		
		crefl = spec6ToRgb( frefl );
		crefr = spec6ToRgb( frefr );
		ctran = spec6ToRgb( ftran );
		
		// clear storage, or expect wouwous
		for (i = 0; i < 6; i += 1) {
			frefl[i] = 0; frefr[i] = 0; ftran[i] = 0;
			srefl[i] = 0; srefr[i] = 0; stran[i] = 0;
		}
		
	} else { // RGB
		
		float idotn = -In.Nn;
		/* if I.N>0, ray is entering the medium, else ray is exiting
		 * and eta is the reverse of the ior when ray is entering the
		 * medium. */
		float eta;
		if (idotn > 0) {
			entering = 1; eta = 1/ior; Nf = Nn;
		} else {
			entering = 0; eta = ior; Nf = -Nn;
		}

		glasssample( In, Nf,
				((entering == 1) ? 1/ior : ior),
				Kr, Kt, aexp, aamp, krblur, ktblur,
				entering, raydepth, rbounces, rsamples, bias,
				rmaxdist, tmaxdist, optics, envmap, rtype,
				cabs, attencolor, category, crefl, crefr, ctran );
	}

	// no ambient, diffuse at higher ray levels, specular max bounces
	float ka = Ka, kd = Kd, ks = Ks;
	if (raydepth > 0) {
		ka = 0; kd = 0;
		// restrict specular highlights to ray level and face orientation
		ks = (raydepth > rbounces || entering == 1) ? 0 : Ks;
	}

	/* Well, Aqsis doesn't supports raytracing yet, but we might as well
	 * just leave everything in place (there's always frankenrender,
	 * but i haven't tested it this way (w/BMRT's rayserver)).
	 * Note: the ternary operator seems to have problems with color types
	 * in Aqsis (cannot find a suitable cast for the expression) */
#if RENDERER == aqsis
	if (Ka > 0) aov_ambient += Ka * ambient();
	if (Kd > 0) aov_diffuse += Kd * diffuse(faceforward(Nn, In));
	if (Ks > 0) {
		if (spectype == "glossy")
			aov_specular += locillumglassy( faceforward(Nn, In), -In,
					roughness, sharpness, category );
		else aov_specular += specular(faceforward(Nn, In), -In, roughness);
	}
#else
	// set illumination terms if active
	aov_ambient += (ka == 0) ? color(0) : ka * ambient();
	aov_diffuse += (kd == 0) ? color(0) : kd * diffuse( faceforward(Nn, In));
	aov_specular += (ks == 0) ? color(0) : ks * ( (spectype == "glossy") ?
			locillumglassy( faceforward(Nn, In), -In, roughness,
				sharpness, category ) : specular( faceforward(Nn, In),
					-In, roughness) );
#endif
	aov_reflection += crefl;
	aov_refraction += crefr;
	
	ci = aov_ambient + aov_diffuse + aov_specular + aov_reflection
		+ aov_refraction;
	oi = ctran;
}

////////////////////////////////////////////////////////////////////////////////
// David C. Banks fast anisotropic model, from: ////////////////////////////////
// "Illumination in Diverse Codimensions". /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

color
banksaniso(
			color cdiff, cspec;
			float Kd, Ks, roughness;
			normal Nn;
			vector In, adir;
			uniform string category;
			DECLARE_AOV_OUTPUT_PARAMETERS
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln, xdir = normalize(adir);

	float costheta = Vf.xdir, cospsi, ldotn, pd, ps;

	uniform float nondiff, nonspec;
	extern point P;

	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;

		Ln = normalize(L);
		cospsi = Ln.xdir;
		pd = 1;

		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			pd = sqrt( max(0, 1 - SQR(cospsi) ));
			ldotn = Ln.Nf;
			aov_diffuse += Cl * (1-nondiff) * ldotn * Kd * pd;
		}

		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			ps = sqrt( max(0, 1 - SQR(costheta) ));
			ldotn = Ln.Nf;
			aov_specular += Cl * (1-nonspec) * ldotn * Ks *
							pow( pd*ps - cospsi*costheta, 1/roughness);
		}
	}
	return (cdiff * aov_diffuse) + (cspec * aov_specular);
}

////////////////////////////////////////////////////////////////////////////////
// Paul S.Strauss empirical BRDF, from :  //////////////////////////////////////
// "A Realistic Lighting Model for Computer Animators" /////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Model constants
#define KF	1.12	// Fresnel adjustment constant
#define KG	1.01	// Geometric attenuation adjustment constant
#define	KJ	0.10	/* Constant for off-specular peaks for very rough
					 * surfaces [0,1] range */

// Surface white color
#define CWHITE	color( 1, 1, 1)

////////////////////////////////////////////////////////////////////////////////
// Fresnel reflectance approximation
float straussfresnel( float beta; )
{
	float x = beta / S_PI_2; // x in [0,1] range
	float A = 1 / SQR(x-KF) - (1 / SQR(KF));
	float B = 1 / SQR(1-KF) - (1 / SQR(KF));
	return A/B;
}

////////////////////////////////////////////////////////////////////////////////
// Geometric attenuation approximation
float straussgeoatten( float beta; )
{
	float x = beta / S_PI_2; // x in [0,1] range
	float A = 1 / SQR(1-KG) - (1 / SQR(x-KG));
	float B = 1 / SQR(1-KG) - (1 / SQR(KG));
	return A/B;
}

////////////////////////////////////////////////////////////////////////////////
color strauss(
				color Csurface;
				/* smoothness [0,1] = perfect diffuse/specular
				 * metalness  [0,1] = perfect non-metal, highlight = light 
				 * color, 1 = metal, highlight = based on object color */
				float smoothness, metalness;
				float transparency;
				normal Nn; vector In;
				uniform string category;
				DECLARE_AOV_OUTPUT_PARAMETERS
		)
{
	normal Nf = faceforward( Nn, In );
	vector Vf = -In, Ln, Hn;
	float F, j, rj, rs;

	// preset quantities
	float cosgamma = Vf.Nf, cosalpha, cosbeta, alpha;
	
	// Diffuse contribution factors
	/* diffuse reflectivity multiplier, attenuates diffuse reflection for
	 * metallic surfaces */
	float d = (1 - metalness * smoothness);
	// diffuse reflectivity as function of roughness and transparency
	float rd = (1 - pow( smoothness, 3)) * (1 - transparency);

	/* Specular reflectivity at normal incidence, all light not transmitted
	 * nor diffusely reflected, is reflected specularly */
	float rn = (1 - transparency) - rd;
	// specular exponent h
	float h = 3 / (1-smoothness);

	uniform float nondiff, nonspec;
	color Csw = color(0);
	color Cdiff = color(0), Cspec = color(0);
	extern point P;

	illuminance( category, P, Nf, S_PI_2 )
	{
		extern vector L;
		extern color Cl;

		Ln = normalize(L);
		// preset quantities
		cosalpha = Ln.Nf;

		/* diffuse contribution, depends on roughness and transparency
		 * with attenuation for metallic surfaces */
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			Cdiff += Cl * (1-nondiff) * d * rd * cosalpha;
		}
		
		// Specular contribution
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			/* Highlight vector H is the direction in which light along L
			 * would be reflected if the surface were a perfect mirror */
			Hn = normalize( reflect( -L, Nf ));
			cosbeta = Hn.Vf;
			alpha = acos(cosalpha);

			/* reflectivity adjustment to account for off-specular peaks */
			/* Fresnel effects */
			F = straussfresnel(alpha);
			// Geometric effects
			j = F * straussgeoatten(alpha) * straussgeoatten(acos(cosgamma));
			
			/* Adjusted reflectivity, creates an incraese in specular
			 * reflectivity near grazing incidence except for when viewer
			 * or incident angles are close to PI/2, at angles close to PI/2
			 * geometric and Fresnel equations take over */
			rj = min( 1, rn + (rn + KJ) * j);

			// specular reflectivity
			rs = pow( cosbeta, h) * rj;

			// Specular color, approaching white at grazing angles
			Csw = CWHITE + metalness * (1-F) * (Csurface - CWHITE);

			Cspec += Cl * (1-nonspec) * rs * Csw * cosalpha;
		}
	}

	aov_surfacecolor += Csurface;
	aov_diffuse += Cdiff;
	aov_specular += Cspec;
	
	return aov_surfacecolor * aov_diffuse + aov_specular;
}

////////////////////////////////////////////////////////////////////////////////
// Fur surface shader, by Clint Hanson and Armin Bruderlin, from ///////////////
// The RenderMan Repository - www.renderman.org ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// slightly tweaked to fit shrimp's structure

/* Renamed to SIG2k_srf_fur to be consistent with the RMR
 * -- tal AT renderman DOT org
 * */

/* fur shader shader, wich clumping and specular model, by Clint Hanson,
 * and Armin Bruderlin. */

color
fnc_diffuselgt(
				color Cin; // Light color
				vector Lin; // Light position
				vector Nin; // Surface normal
		)
{
	color Cout = Cin;
	vector LN, NN;
	float Atten;

	// normalize the stuff
	LN = normalize( Lin );
	NN = normalize( Nin );

	// diffuse calculation
	Atten = max( 0, LN.NN );
	Cout *= Atten;
	return (Cout);
}

// main
color
SIG2k_srf_fur(
				float Ka, Kd, Ks; // usual meaning
				float roughness1, roughness2, spec1, spec2;
				float start_spec, end_spec, spec_size_fade;
				float illum_width, var_fade_start, var_fade_end;
				float clump_dark_strength;
				// Hair color
				color rootcolor, tipcolor, specularcolor, static_ambient;
				// Variables passed from the RIB
				uniform float hair_col_var, hair_length, hair_id;
				uniform normal surface_normal;
				varying vector clump_vect;
				uniform string category;
				DECLARE_AOV_OUTPUT_PARAMETERS
		)
{
	extern vector dPdv, I; // we're in a function
	
	vector T = normalize(dPdv); // tangent along length of hair
	vector V = -normalize(I); // view vector
	float Kspec = Ks; // Specular coefficient copy
	
	vector nL;
	varying normal nSN = normalize( surface_normal );
	vector S = nSN^T; // Cross product of tangent along hair & surface normal
	vector N_hair = (T^S) ; // normal for hair oriented away from surface
	vector norm_hair;

	float l = clamp( nSN.T, 0, 1 ); // T and surface normal dot for blend
	float clump_darkening = 1.0;
	float T_Dot_nL = 0;
	float T_Dot_e = T.V;
	float sinbeta = sqrt( max( 0, 1 - SQR(T_Dot_e))), sinalpha = 0;
	float Kajiya = 0, darkening = 1.0;

	// values from light
	uniform float nondiff, nonspec;
	uniform color SpecularColor;

	/* When the hair is exactly perpendicular to the surface, use the surface
	 * normal, when the hair is exactly tangent to the surface, use the hair
	 * normal, otherwise, blend between the two normals in a linear fashion. */
	norm_hair = l * nSN + (1-l) * N_hair;
	norm_hair = normalize(norm_hair);

	/* Make the specular only hit in certain parts of the hair. V is along the
	 * length o fthe hair. */
	extern float v;
	Kspec *= min( smoothstep( start_spec, start_spec + spec_size_fade, v),
				1 - smoothstep( end_spec, end_spec - spec_size_fade, v ) );

	/* Loop over lights, catch highlights as if this was a thin cylinder.
	 * Specular illumination model from James T. Kajiya and Timothy L. Kay,
	 * (1989) "Rendering Fur with Three Dimensional Textures", Computer
	 * Graphics 23, 3, 271-280. */

	extern point P;
	color C = color(0), Cdiff = color(0), Cspec = color(0);
	
	illuminance( category, P, norm_hair, radians( illum_width ) )
	{
		extern vector L;
		extern color Cl;

		nL = normalize(L);

		// Calculate diffuse components
		if ( clump_dark_strength > 0.0 ) {
			clump_darkening = 1 - ( clump_dark_strength *
					abs( clamp( nL.normalize(-1 * clump_vect), -1, 0 )));
		} else {
			clump_darkening = 1.0;
		}
		nondiff = 0;
		if (1 == lightsource("__nondiffuse", nondiff) && nondiff < 1) {
			
			Cdiff += (1-nondiff) * clump_darkening * fnc_diffuselgt(
					Cl, L, norm_hair);
		}

		// Get light source parameters
		if ( lightsource("__SpecularColor", SpecularColor) == 0)
			SpecularColor = color(1);
		
		nonspec = 0;
		if (1 == lightsource("__nonspecular", nonspec) && nonspec < 1) {
			
			T_Dot_nL = T.nL;
			sinalpha = sqrt( max( 0, 1 - SQR(T_Dot_nL)));
			Kajiya = T_Dot_nL * T_Dot_e + sinalpha * sinbeta;

			Cspec += Cl * (1-nonspec) * clump_darkening * (spec1 * pow(Kajiya,
						1/roughness1) + spec2 * pow(Kajiya, 1/roughness2))
					* SpecularColor;
		}
	}

	aov_ambient += Ka * ambient() + static_ambient;
	aov_specularcolor += specularcolor;
	aov_diffuse += Kd * Cdiff;
	aov_specular += Kspec * v * Cspec;
	
	darkening = clamp( hair_col_var, 0, 1);
	darkening = 1 - smoothstep( var_fade_end, var_fade_start,
				abs( luminance( aov_diffuse ))) * darkening;

	aov_surfacecolor += mix( rootcolor, tipcolor, v) * darkening;

	C = aov_surfacecolor * (aov_ambient + aov_diffuse) + aov_specularcolor
				* aov_specular;
	
	return clamp( C, color(0), color(1) );
}

////////////////////////////////////////////////////////////////////////////////
// Thin film interference, based on Jens's VEX shader, at OdForce forums: //////
// http://forums.odforce.net/index.php?/topic/4788-coatings-iridescence ////////
////////////////////////////////////////////////////////////////////////////////

float
intef( float lambda, in_eps, n, d; )
{
	float out_eps = 2 * d * n * sqrt( SQR(n) + SQR(in_eps) - 1);
	return out_eps / (n * lambda);
}

/* Evaluate the intensity integral for two sine waves with relative offset
 * s and weights a, b, for value w. */
float
inten_sin(	float a, b, w, z; )
{
	float t1, t2, t3;
	t1 = SQR(a) * (w * .5 - .25 * sin(w));
	t2 = SQR(b) * (w * .5 - .25 * sin(w+z));
	t3 = 2 * a * b * (-.25 * sin(2*w*z) + w * .5 * cos(z));
	return (t1 + t2 + t3);
}

// main
void
thinfilm(
			float depth; // coating thickness in nanometers
			vector In; // normalize(I)
			normal Nn; // normalize(N)
			uniform float ior[6]; // ior data for RYGCBV
			color Csurf; // surface color
			output varying color out_ci, out_oi;
			DECLARE_AOV_OUTPUT_PARAMETERS
	   )
{
	out_ci = color(0); out_oi = color(1);
	
	vector Vf = -In; // -normalize(I)
	normal Nf = Nn;

	float costheta = Vf.Nn;

	// need to know surface orientation, reverse normals if needed
	if (costheta < 0) {
		Nf = -Nn;
		costheta = Vf.Nf;
	}

	uniform float i;
	uniform float lambda[6] = LAMBDA;
#if RENDERER == aqsis || RENDERER == pixie
	// can't initialize arrays with single element, need to explicitly
	// set all elements
	varying float	rygcbv[6] = {0,0,0,0,0,0}, ishift[6] = {0,0,0,0,0,0},
					phaseshift[6] = {0,0,0,0,0,0}, kr[6] = {0,0,0,0,0,0},
					kt[6] = {0,0,0,0,0,0}, norm[6] = {0,0,0,0,0,0},
					cout[6] = {0,0,0,0,0,0};
#else
	varying float rygcbv[6] = 0, phaseshift[6] = 0, ishift[6] = 0;
	varying float kr[6] = 0, kt[6] = 0, norm[6] = 0, cout[6] = 0;
#endif // Aqsis & Pixie array initialization
	
	rgbToSpec6( Csurf, rygcbv );

	for (i = 0; i < 6; i += 1) {
		// Calculate phaseshift for each component.
		// To get the phaseshift in nm, multiply
		// with its wavelenght.
		phaseshift[i] = intef( lambda[i], costheta, ior[i], depth) * S_2PI;
		// Fresnel term
		fresnel( In, Nf, 1/ior[i], kr[i], kt[i]);

		// Intensity change of each component due to phase shift
		ishift[i] = inten_sin( kr[i], kt[i], S_2PI, phaseshift[i]) -
					inten_sin( kr[i], kt[i], 0, phaseshift[i]);

		norm[i] = inten_sin( kr[i], kt[i], S_2PI, 0) -
					inten_sin( kr[i], kt[i], 0, 0 );

		cout[i] = ishift[i] / norm[i] * rygcbv[i];
	}
	// back to rgb
	out_oi = spec6ToRgb( kr );
	out_ci = spec6ToRgb( cout );
}

////////////////////////////////////////////////////////////////////////////////
#endif /* RSL_SHADING_MODELS_H */

