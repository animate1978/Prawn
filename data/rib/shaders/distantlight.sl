/* distant light with shadows and attenuation */
light
distantlight(
				float intensity = 1, shadowblur = 0;
				color lightcolor = color(1);
				point from = point "shader"(0,0,0);
				point to = point "shader"(0,0,1);
				uniform string shadowname = "", shadowfilter = "gaussian";
				uniform float shadowsamples = 1;
				uniform float shadowwidth = 1, shadowbias = 0.01, decay = 0;
				uniform string __category = "";
				uniform float __nondiffuse = 0;
				uniform float __nonspecular = 0;
				output varying color __shadow = color(0);
				)
{
	float atten = 0;
	solar(to - from, 0) {

		float dist = length(L);
		if (decay == 0) { /* no decay */
			atten = intensity;
		} else if (decay == 1) { /* linear decay */
			atten = intensity / dist;
		} else if (decay == 2) { /* squared decay */
			atten = intensity / (dist*dist);
		} else {
			atten = intensity / pow( dist, decay);
		}

		if (shadowname != "") {
			__shadow = color shadow( shadowname, Ps,
										"samples", shadowsamples,
										"blur", shadowblur,
										"width", shadowwidth,
										"filter", shadowfilter,
										"bias", shadowbias );
		}

		Cl = atten * (1-__shadow) * lightcolor;
	}
}

