#ifndef RSL_SHRIMP_SAMPLING
#define RSL_SHRIMP_SAMPLING 1

#include "rsl_shrimp_helpers.h" // useful quantities

/* Note: WIP */
////////////////////////////////////////////////////////////////////////////////
// Blinn PDF
void
sample_phong(
				float xi1, xi2, exponent;
				output varying float cosphi, sinphi, costheta, sintheta;
		)
{
	float phi = S_2PI * xi2;
	costheta = pow( xi1, 1/(exponent+1));
	sintheta = sqrt( max(0, 1-SQR(costheta)));
	cosphi = cos(phi);
	sinphi = sin(phi);
}

////////////////////////////////////////////////////////////////////////////////
// Beckmann microfacet distribution. Also see paper by I.T.Dimov, T.V.Gurov,
// and A.A.Penzov, "A Monte Carlo Approach for the Cook-Torrance Model".
void
sample_beckmann(
					float xi1, xi2, m;
					output varying float cosphi, sinphi, costheta, sintheta;
		)
{
	float m2 = SQR(m), phi = S_2PI * xi2;
	float tantheta = sqrt( -m2 * log(1-xi1) );
	costheta = 1 / sqrt(1 + SQR(tantheta));
	sintheta = costheta * tantheta;
	cosphi = cos(phi);
	sinphi = sin(phi);
}

////////////////////////////////////////////////////////////////////////////////
// Ward isotropic PDF
void
sample_ward_iso(
					float xi1, xi2, m;
					output varying float cosphi, sinphi, costheta, sintheta;
		)
{
	float phi = S_2PI * xi2;
	float tantheta = m * sqrt( -log(xi1) );
	cosphi = cos(phi);
	sinphi = sin(phi);
	costheta = 1 / sqrt( 1 + SQR(tantheta) );
	sintheta = costheta * tantheta;
}

////////////////////////////////////////////////////////////////////////////////
// Ward anisotropic PDF
void
sample_ward_aniso(
					float xi1, xi2, mratio, nu2, nv2;
					output varying float cosphi, sinphi, costheta, sintheta;
		)
{
	float tantheta, tanphi;
	if (xi2 < 0.25) {
		tanphi = mratio * tan( S_PI_2 * 4 * xi2);
		cosphi = 1 / sqrt(1 + SQR(tanphi));
		sinphi = tanphi * cosphi;
	} else if (xi2 < 0.5) {
		tanphi = mratio * tan( S_PI_2 * (1 - 4 * (0.5 - xi2)) );
		cosphi = -1 / sqrt( 1 + SQR(tanphi));
		sinphi = -tanphi * cosphi;
	} else if (xi2 < 0.75) {
		tanphi = mratio * tan( S_2PI * (4 * (xi2 - 0.5)) );
		cosphi = -1 / sqrt( 1 + SQR(tanphi));
		sinphi = tanphi * cosphi;
	} else {
		tanphi = mratio * tan( S_2PI * (1 - 4 * (1 - xi2)) );
		cosphi = 1 / sqrt( 1 + SQR(tanphi));
		sinphi = -tanphi * cosphi;
	}
	tantheta = -log(1 - xi1) / (SQR(cosphi) / nu2 + SQR(sinphi) / nv2);
	costheta = 1 / sqrt(1 + tantheta);
	sintheta = costheta * sqrt(tantheta);
}

////////////////////////////////////////////////////////////////////////////////
// Ashikhmin-Shirley PDF
void
sample_ashikhmin_shirley(
					float xi1, xi2, nunvratio, nu, nv;
					output varying float cosphi, sinphi, costheta, sintheta;
		)
{
	float phi, theta, xi;
	if (xi2 < 0.25) {
		phi = atan( nunvratio * tan(S_PI_2 * 4 * xi2));
	} else if (xi2 < 0.5) {
		phi = atan( nunvratio * tan(S_PI_2 * (1 - 4 * (0.5 - xi2)) ));
		phi = S_PI - phi;
	} else if (xi2 < 0.75) {
		phi = atan( nunvratio * tan(S_PI_2 * 4 * (xi2 - 0.5)));
		phi += S_PI;
	} else {
		phi = atan( nunvratio * tan(S_PI_2 * (1 - 4 * (1 - xi2)) ));
		phi = S_2PI - phi;
	}
	cosphi = cos(phi);
	sinphi = sin(phi);
	costheta = pow( 1-xi1, 1/(nu*SQR(cosphi) + nv*SQR(sinphi) + 1));
	sintheta = sqrt( max(0, 1-SQR(costheta)));
}

////////////////////////////////////////////////////////////////////////////////
// Schlick PDF
void
sample_schlick(
				float xi1, xi2, roughness, p; // p = isotropy [0,1]
				output varying float cosphi, sinphi, costheta, sintheta;
		)
{
	float phi, theta, xisqr, p2 = SQR(p);
	if (xi2 < 0.25) {
		xisqr = 4*xi2;
		xisqr = SQR(xisqr);
		phi = sqrt( (p2 * xisqr) / (1-xisqr+xisqr * p2)) * S_PI_2;
	} else if (xi2 < 0.5) {
		xisqr = 1 - 4 * (.5 - xi2);
		xisqr = SQR(xisqr);
		phi = sqrt( (p2 * xisqr) / (1-xisqr+xisqr * p2)) * S_PI_2;
		phi = S_PI - phi;
	} else if (xi2 < 0.75) {
		xisqr = 4 * (xi2 - 0.5);
		xisqr = SQR(xisqr);
		phi = sqrt( (p2 * xisqr) / (1-xisqr+xisqr * p2)) * S_PI_2;
		phi += S_PI;
	} else {
		xisqr = 1 - 4 * (1 - xi2);
		xisqr = SQR(xisqr);
		phi = sqrt( (p2 * xisqr) / (1-xisqr+xisqr * p2)) * S_PI_2;
		phi = S_2PI - phi;
	}
	cosphi = cos(phi);
	sinphi = sin(phi);

	costheta = sqrt( xi1 / (roughness-xi1 * roughness+xi1));
	sintheta = sqrt( max(0, 1-SQR(costheta)));
}

////////////////////////////////////////////////////////////////////////////////
// Based in RPS's notes stratified sampling example.
void
strat_sampling(
				uniform float model, samples;
				float nu, nv, roughness, exponent, isotropy;
				vector In;
				vector udir, vdir, ndir;
				output varying vector raydirs[];
			)
	{
		float nu2 = SQR(nu), nv2 = SQR(nv);
		float mratio = nv/nu, nunvratio = sqrt((nu+1)/(nv+1));
		uniform float ss = 0, thetasamples, phisamples;
		uniform float thetastrata, phistrata, stratifiedsamples,
				remainingsamples;
		float x, y, z, cosphi = 0, sinphi = 0, costheta = 0, sintheta = 0;
		float xi1, xi2;
		vector hdir;

		// compute the number of strata in theta and phi directions
		// for optical stratification there should be 3 to 4 times as many
		// theta strata as phi strata
		thetastrata = floor( 0.5 * sqrt(samples));
		phistrata = floor( samples/thetastrata);
		stratifiedsamples = thetastrata * phistrata;
		remainingsamples = samples - stratifiedsamples;

		// generate fully stratified directions
		for (thetasamples = 0; thetasamples < thetastrata; thetasamples += 1) {
			for (phisamples = 0; phisamples < phistrata; phisamples += 1) {
				// pick a point within stratum(thetasamples, phisamples)
				xi1 = (thetasamples + random()) / thetastrata;
				xi2 = (phisamples + random()) / phistrata;
				// choose PDF
				if (model == 0) { // Ward PDF
					if (nu == nv) { // isotropic
						sample_ward_iso( xi1, xi2, nu,
								cosphi, sinphi, costheta, sintheta);
					} else { // anisotropic
						sample_ward_aniso( xi1, xi2, mratio, nu2, nv2,
								cosphi, sinphi, costheta, sintheta);
					}
				} else if (model == 1) { // Ashikhmin-Shirley PDF
					sample_ashikhmin_shirley( xi1, xi2, nunvratio, nu, nv,
						cosphi, sinphi, costheta, sintheta );
			} else if (model == 2) { // Schlick PDF
				sample_schlick( xi1, xi2, roughness, isotropy,
						cosphi, sinphi, costheta, sintheta );
			} else if (model == 3) { // Phong PDF
				sample_phong( xi1, xi2, exponent,
						cosphi, sinphi, costheta, sintheta );
			} else if (model == 4) { // Beckmann PDF
				sample_beckmann( xi1, xi2, roughness,
						cosphi, sinphi, costheta, sintheta );
			} else { // TODO: other PDFs
				cosphi = 0; sinphi = 0; costheta = 0; sintheta = 0;
			}
			// compute half vector H
			x = cosphi * sintheta;
			y = sinphi * sintheta;
			z = costheta;
			// convert to direction on hemisphere defined by the normal
			hdir = x * udir + y * vdir + z * ndir;
			// convert to reflection direction
			raydirs[ss] = reflect( In, hdir );
			ss += 1;
		}
	}
	// generate less stratified directions (stratified in phi)
	for (phisamples = 0; phisamples < remainingsamples; phisamples += 1) {
		xi1 = random();
		xi2 = (phisamples + random()) / remainingsamples;
		// choose PDF
		if (model == 0) { // Ward
			if (nu == nv) { // isotropic
				sample_ward_iso( xi1, xi2, nu, cosphi, sinphi,
						costheta, sintheta);
			} else { // anisotropic
				sample_ward_aniso( xi1, xi2, mratio, nu, nv,
						cosphi, sinphi, costheta, sintheta);
			}
		} else if (model == 1) { // Ashikhmin-Shirley PDF
			sample_ashikhmin_shirley( xi1, xi2, nunvratio, nu2, nv2,
					cosphi, sinphi, costheta, sintheta );
		} else if (model == 2) { // Schlick PDF
			sample_schlick( xi1, xi2, roughness, isotropy,
					cosphi, sinphi, costheta, sintheta );
		} else if (model == 3) { // Phong PDF
			sample_phong( xi1, xi2, exponent,
					cosphi, sinphi, costheta, sintheta );
		} else if (model == 4) { // Beckmann PDF
			sample_beckmann( xi1, xi2, roughness,
					cosphi, sinphi, costheta, sintheta );
		} else { // TODO: other PDFs
			cosphi = 0; sinphi = 0; costheta = 0; sintheta = 0;
		}
		// compute half vector H
		x = cosphi * sintheta;
		y = sinphi * sintheta;
		z = costheta;
		// convert to direction on hemisphere defined by the normal
		hdir = x * udir + y * vdir + z * ndir;
		// convert to reflection direction
		raydirs[ss] = reflect( In, hdir );
		ss += 1;
	}
}

////////////////////////////////////////////////////////////////////////////////
color
samplepdf(
			uniform float model, samples;
			float nu, nv, roughness, exponent, isotropy;
			vector In, xdir;
			normal Nn;
			uniform string label, subset, hitmode;
			uniform float bias, maxdist;
			uniform string envmap, envspace, filter;
			uniform float width, envsamples;
			float envblur;
		)
{
	float idotn = In.Nn;
#if RENDERER==_3delight || RENDERER==prman // expect wous if --dso
	varying vector raydirs[];
	resize( raydirs, samples);
	uniform float rsamples = samples;
#else // Pixie, Aqsis, expect fixed/constant lenght, going to clamp to 1024
	  // and hope it's enough, if the user needs more samples, he can always
	  // edit this code.
	varying vector raydirs[1024];
	uniform float rsamples = clamp(samples, 0, 1024);
#endif

	// build orthonormal basis
	vector ndir = vector(Nn);
	vector udir = normalize( xdir - (Nn.xdir) * Nn);
	vector vdir = normalize(Nn^udir);
	// get array of rays with stratified sampling according to "model" PDF
	strat_sampling( model, rsamples, nu, nv, roughness, exponent,
			isotropy, In, udir, vdir, ndir, raydirs);

	// for gather construct
	extern vector I;
	extern point P;
	color hitc = color(0), cout = color(0);
	vector R = vector(0); // store ray:direction where needed

	// choose the sampling approach to use according to renderer
	// gather() or environment() (in Aqsis's case)

#if RENDERER==prman // gather distribution
	if (idotn < 0) { // trace when visible to eye only
		gather( "illuminance", P, I, S_PI_2, rsamples, "surface:Ci", hitc,
				"ray:direction", R, "distribution", raydirs, "label", label,
				"subset", subset, "hitsides", "front", "bias", bias,
				"othreshold", color(0), "maxdist", maxdist )
		{
			cout += hitc;
		} else {
			cout += (envmap != "") ?
				color environment( envmap, vtransform( envspace,
							"object", R), "filter", filter,
							"width", width, "blur", envblur,
							"samples", envsamples ) : color(0);
		}
	}
#elif RENDERER==aqsis // environment loop
	uniform float i;
	if (envmap != "") {
		for (i = 0; i < rsamples; i += 1) {
			if (idotn < 0) {
				cout += color environment( envmap, vtransform( envspace,
						"object", raydirs[i] ), "filter", filter,
						"width", width, "blur", envblur,
						"samples", envsamples);
			} else { cout = color(0); }
	}
#else // 3delight, Pixie, Air
#define COMMON_SAMPLE_ARGUMENTS \
		"illuminance", P, raydirs[i], 0, 1, "surface:Ci", hitc, \
		"label", label, "bias", bias, "maxdist", maxdist

	uniform float i;
	for (i = 0; i < rsamples; i += 1) {
		if (idotn < 0) { // trace when visible to eye only
			gather(	COMMON_SAMPLE_ARGUMENTS
#if RENDERER==_3delight
					, "subset", subset, "hitmode", hitmode
#elif RENDERER==Air
					, "subset", subset, "othreshold", 0
#endif // common gather parameters
				  )
			{
				cout += hitc;
			} else { // no hits, lookup environment map if it exists
				cout += (envmap != "") ?
					color environment( envmap, vtransform( envspace,
							"object", raydirs[i] ),
							"filter", filter, "width", width,
							"blur", envblur, "samples", envsamples )
					: color(0);
			}
		}
	}
#endif // sampling approach to use according to renderer
#undef COMMON_SAMPLE_PARAMETERS

	cout /= rsamples;
	return cout;
}
////////////////////////////////////////////////////////////////////////////////
#endif // RSL_SHRIMP_SAMPLING

