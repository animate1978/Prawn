#ifndef SHRIMP_EXTRAS_H
#define SHRIMP_EXTRAS_H 1

#include "rsl_shrimp_helpers.h"

/* Microfacet distribution functions */
////////////////////////////////////////////////////////////////////////////////
/* Beckmann distribution */
float beckmann(
				float cosalpha, roughness;
		)
{
	float m2 = SQR(roughness);
	float cosalpha2 = SQR(cosalpha);
	float tanalpha2_over_m2 = max(0, 1 - cosalpha2) / (cosalpha2 * m2);
	return exp( -tanalpha2_over_m2) / (m2 * SQR(cosalpha2));
}

////////////////////////////////////////////////////////////////////////////////
/* Ward distribution */
float ward(
			float cosalpha, roughness;
		)
{
	float m2 = SQR(roughness);
	float cosalpha2 = SQR(cosalpha);
	float tanalpha2_over_m2 = max(0, 1 - cosalpha2) / (cosalpha2 * m2);
	return exp( -tanalpha2_over_m2) / (m2 * S_PI * cosalpha2 * cosalpha);
}

////////////////////////////////////////////////////////////////////////////////
/* Trowbridge-Reitz distribution, suggested by Jim Blinn to the Torrance-
 * -Sparrow model, based on the equation of an elipse, with m2 being the
 * eccentricity, 0 for shiny surfaces, 1 for rough. This distribution needs
 * an normalization factor (user-parameter), not included here. */

float
trowbridge_reitz(
					float cosalpha, roughness;
					)
{
	float m2 = SQR(roughness);
	float cosalpha2 = SQR(cosalpha);
	float d2 = m2 / (cosalpha2 * (m2-1)+1);
	return SQR(d2);
}

////////////////////////////////////////////////////////////////////////////////
/* Heidrich-Seidel anisotropic distribution, some literature refers to
 * coupling the anisotropic specular term with a isotropic specular term */
float
heidrich_seidel(
					normal Nf;
					vector Vf, Ln;
					vector xdir; // normalized anisotropy direction
					float roughness;
					)
{
	vector dir = normalize( Nf ^ xdir );
	float cosbeta = Ln.dir;
	float costheta = Vf.dir;
	float sinbeta = sqrt( max( 0, 1 - SQR(cosbeta)));
	float sintheta = sqrt( max( 0, 1 - SQR(costheta)));
	return pow( sinbeta * sintheta - (cosbeta * costheta), 1/roughness);
}

////////////////////////////////////////////////////////////////////////////////
/* Geometric shadowing/masking term, suggested by Jim Blinn to the Torrance-
 * -Sparrow model. */
float blinn_ts(
					float costheta, cosalpha, cospsi, cospsi2;
					)
{	
	float shadowing = (cosalpha * costheta) / cospsi2;
	float masking = (cosalpha * cospsi) / cospsi2;

	return min( 1, 2 * min( shadowing, masking) );
}

////////////////////////////////////////////////////////////////////////////////
/* Christophe Schlick's approximation of the first Smith geometric
 * selfshadowing/masking function (for a known microfacet normal), the product
 * of 2 attenuation terms, between incident vector and microstructure
 * normal, and viewer and microstructure normal. */

float schlick_smith(
				float cospsi, costheta, roughness;
				)
{
	float g1 = cospsi / (roughness + (1 - roughness) * cospsi);
	float g2 = costheta / (roughness + (1 - roughness) * costheta);
	return g1 * g2;
}

////////////////////////////////////////////////////////////////////////////////
/* Second Smith equation (fulL) for geometric shadowing/masking, for an
 * averaged microfacet normal. */

float smith(
					float costheta, cosalpha, roughness;
					)
{
	float cosalpha2 = SQR(cosalpha);
	float costheta2 = SQR(costheta);
	float m2 = SQR(roughness);

	float hnl = cosalpha2 / (2 * m2 * (1 - cosalpha2) );
	float hnv = costheta2 / (2 * m2 * (1 - costheta2) );

	float gnl = sqrt( S_PI * hnl * (2 - mm_erfc( sqrt( hnl ))));
	float gnv = sqrt( S_PI * hnv * (2 - mm_erfc( sqrt( hnv ))));

	return (gnl / (gnl +1) ) * (gnv / (gnv + 1) );
}

////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_EXTRAS_H */

