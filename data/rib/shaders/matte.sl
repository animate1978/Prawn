surface matte( float Ka = .01, Kd = .8; )
{
	normal Nf = normalize( faceforward( N, I ));
	
	Oi = Os;
	Ci = Oi * Cs * (Ka * ambient() + Kd * diffuse(Nf));
}

