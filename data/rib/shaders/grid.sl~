#ifndef filteredpulse
#define filteredpulse( a, b, x, dx) ( max( 0, (min( (x-dx/2)+dx, b) \
									- max( x-dx/2, a)) / dx ))
#endif /* filteredpulse */

#include "rsl_shrimp_aov.h"
////////////////////////////////////////////////////////////////////////////////

surface grid(
				float hfreq = 1;
				float vfreq = 1;
				float hblur = 0.01;
				float vblur = 0.01;
				DEFAULT_AOV_OUTPUT_PARAMETERS
				)
{
	INIT_AOV_PARAMETERS
	float ss = mod( s*hfreq, 1);
	float tt = mod( t*vfreq, 1);
	
	color cvert = filteredpulse( .05, .95, ss, hblur );
	color chori = filteredpulse( .05, .95, tt, vblur );

	aov_surfacecolor += (cvert * chori);
	aov_diffuse += diffuse( faceforward( normalize(N), I));

	/* get shadows pass */
	getshadows(P);
	
	Ci = aov_surfacecolor * aov_diffuse;
}

