#ifndef SHRIMP_CONVERSION_H
#define SHRIMP_CONVERSION_H 1

#include "rsl_shrimp_helpers.h"

/* Utility functions to convert between coordinate systems */
////////////////////////////////////////////////////////////////////////////////
/* Cartesian to polar coordinates */
void
cartesian2polar2d(	float x, y;
					output float rho, theta;
		)
{
	rho = sqrt( SQR(x) + SQR(y) );
	theta = atan( y, x );
}

////////////////////////////////////////////////////////////////////////////////
/* Polar to Cartesian coordinates */
void
polar2cartesian2d(	float rho, theta;
					output float x, y;
		)
{
	float costheta = cos(theta);
	float sintheta = sin(theta);
	x = rho * costheta;
	y = rho * sintheta;
}

////////////////////////////////////////////////////////////////////////////////
/* Cylindrical to Cartesian */
/* Normal */
normal
cylindrical2cartesian(	normal in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float costheta = cos(theta);
	float sintheta = sin(theta);
	return normal ( rho * costheta,
					rho * sintheta,
					phi );
}

/* Vector */
vector
cylindrical2cartesian(	vector in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float costheta = cos(theta);
	float sintheta = sin(theta);
	return vector ( rho * costheta,
					rho * sintheta,
					phi );
}

/* Point */
point
cylindrical2cartesian(	point in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float costheta = cos(theta);
	float sintheta = sin(theta);
	return point (	rho * costheta,
					rho * sintheta,
					phi );
}


////////////////////////////////////////////////////////////////////////////////
/* Cartesian to cylindrical */
/* Normal */
normal
cartesian2cylindrical(	normal in;
		)
{
	float x = xcomp( in ), y = ycomp( in ), z = zcomp( in );
	return normal ( sqrt( SQR(x) + SQR(y) ),
					atan( y, x ),
					z );
}

/* Vector */
vector
cartesian2cylindrical(	vector in;
		)
{
	float x = xcomp( in ), y = ycomp( in ), z = zcomp( in );
	return vector ( sqrt( SQR(x) + SQR(y) ),
					atan( y, x ),
					z );
}
/* Point */
point
cartesian2cylindrical(	point in;
		)
{
	float x = xcomp( in ), y = ycomp( in ), z = zcomp( in );
	return point ( sqrt( SQR(x) + SQR(y) ),
					atan( y, x ),
					z );
}

////////////////////////////////////////////////////////////////////////////////
/* Spherical to Cartesian */
/* Normal */
normal
spherical2cartesian(	normal in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float costheta = cos(theta);
	float sintheta = sin(theta);
	float cosphi = cos(phi);
	float sinphi = sin(phi);
	return normal(	rho * costheta * sinphi,
					rho * sintheta * sinphi,
					rho * cosphi );
}

/* Vector */
vector
spherical2cartesian(	vector in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float costheta = cos(theta);
	float sintheta = sin(theta);
	float cosphi = cos(phi);
	float sinphi = sin(phi);
	return vector(	rho * costheta * sinphi,
					rho * sintheta * sinphi,
					rho * cosphi );
}

/* Point */
point
spherical2cartesian(	point in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float costheta = cos(theta);
	float sintheta = sin(theta);
	float cosphi = cos(phi);
	float sinphi = sin(phi);
	return point(	rho * costheta * sinphi,
					rho * sintheta * sinphi,
					rho * cosphi );
}

////////////////////////////////////////////////////////////////////////////////
/* Cartesian to Spherical */
/* Normal */
normal
cartesian2spherical(	normal in;
		)
{
	float x = xcomp( in ), y = ycomp( in ), z = zcomp( in );
	float rho = sqrt( SQR(x) + SQR(y) + SQR(z) );
	return normal ( rho,
					atan( y, x ),
					acos( z / rho) );
}

/* Vector */
vector
cartesian2spherical(	vector in;
		)
{
	float x = xcomp( in ), y = ycomp( in ), z = zcomp( in );
	float rho = sqrt( SQR(x) + SQR(y) + SQR(z) );
	return vector ( rho,
					atan( y, x ),
					acos( z / rho) );
}

/* Point */
point
cartesian2spherical(	point in;
		)
{
	float x = xcomp( in ), y = ycomp( in ), z = zcomp( in );
	float rho = sqrt( SQR(x) + SQR(y) + SQR(z) );
	return point (	rho,
					atan( y, x ),
					acos( z / rho ) );
}

////////////////////////////////////////////////////////////////////////////////
/* Spherical to cylindrical conversion */
/* Normal */
normal
spherical2cylindrical(	normal in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float cosphi = cos(phi);
	float sinphi = sin(phi);
	return normal (	rho * sinphi,
					theta,
					rho * cosphi );
}

/* Vector */
vector
spherical2cylindrical(	vector in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float cosphi = cos(phi);
	float sinphi = sin(phi);
	return vector (	rho * sinphi,
					theta,
					rho * cosphi );
}

/* Point */
point
spherical2cylindrical(	point in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	float cosphi = cos(phi);
	float sinphi = sin(phi);
	return point (	rho * sinphi,
					theta,
					rho * cosphi );
}

////////////////////////////////////////////////////////////////////////////////
/* Cylindrical to spherical */
/* Normal */
normal
cylindrical2spherical(	normal in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	return normal(	sqrt( SQR(rho) + SQR(phi) ),
					theta,
					atan( rho, phi ) );
}

/* Vector */
vector
cylindrical2spherical(	vector in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	return vector(	sqrt( SQR(rho) + SQR(phi) ),
					theta,
					atan( rho, phi ) );
}

/* Point */
point
cylindrical2spherical(	point in;
		)
{
	float rho = xcomp( in ), theta = ycomp( in ), phi = zcomp( in );
	return point(	sqrt( SQR(rho) + SQR(phi) ),
					theta,
					atan( rho, phi ) );
}

////////////////////////////////////////////////////////////////////////////////
// Read tangent space normal maps, convert them to object space, based on a ////
// shader by Larry Gritz, from:  ///////////////////////////////////////////////
// http://forums.nvidia.com/index.php?showtopic=23197&hl=tangent ///////////////
////////////////////////////////////////////////////////////////////////////////
normal
tangent2object(
				uniform string tangenttex;
				float ss, tt;
				normal Nn;
		)
{
	normal Nnew = normal(0);

	if (tangenttex != "") {
		extern vector dPdu, dPdv;
		vector lookup = vector( color texture( tangenttex, ss, tt))* 2.0 - 1.0;
		Nnew = normal( normalize(dPdu) * xcomp(lookup) +
			   normalize(dPdv) * ycomp(lookup) +
			   Nn * zcomp(lookup) );
		normalize(Nnew);
	}
	return Nnew;
}

////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_CONVERSION_H */

