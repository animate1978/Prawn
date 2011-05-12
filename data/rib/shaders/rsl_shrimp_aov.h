#ifndef SHRIMP_AOV_H
#define SHRIMP_AOV_H 1

#define	DECLARE_AOV_PARAMETERS		\
	aov_surfacecolor,				\
	aov_ambient,					\
	aov_diffuse,					\
	aov_indirectdiffuse,			\
	aov_specular,					\
	aov_specularcolor,				\
	aov_reflection,					\
	aov_refraction,					\
	aov_rimlighting,				\
	aov_scattering,					\
	aov_translucence,				\
	aov_shadows,					\
	aov_occlusion,					\
	aov_caustics,					\
	aov_opacity,					\
	_color,							\
	_diffuse,						\
	_specular,						\
	_ambient,						\
	_radiosity,						\
	_reflect,						\
	_refract,						\
	_shadow,						\
	_caustic,						\
	_diffshad,						\
	_uv,							\
	_alpha,							\
	_z,								\
	_index

#define INIT_AOV_PARAMETERS			\
	aov_surfacecolor	= color(0);	\
	aov_ambient			= color(0);	\
	aov_diffuse			= color(0);	\
	aov_indirectdiffuse	= color(0);	\
	aov_specular		= color(0);	\
	aov_specularcolor	= color(0);	\
	aov_reflection		= color(0);	\
	aov_refraction		= color(0);	\
	aov_rimlighting		= color(0);	\
	aov_scattering		= color(0);	\
	aov_translucence	= color(0);	\
	aov_shadows			= color(0);	\
	aov_occlusion		= 0;		\
	aov_caustics		= color(0);	\
	aov_opacity			= color(0);	\
	_color				= color(0);	\
	_diffuse			= color(0);	\
	_specular			= color(0);	\
	_ambient			= color(0);	\
	_radiosity			= color(0);	\
	_reflect			= color(0);	\
	_refract			= color(0);	\
	_shadow				= color(0);	\
	_caustic			= color(0);	\
	_diffshad			= color(0);	\
	_uv					= normal(0);\
	_alpha				= 0;		\
	_z					= 0;		\
	_index				= 0;

#define	DECLARE_AOV_OUTPUT_PARAMETERS			\
	output varying color aov_surfacecolor;		\
	output varying color aov_ambient;			\
	output varying color aov_diffuse;			\
	output varying color aov_indirectdiffuse;	\
	output varying color aov_specular;			\
	output varying color aov_specularcolor;		\
	output varying color aov_reflection;		\
	output varying color aov_refraction;		\
	output varying color aov_rimlighting;		\
	output varying color aov_scattering;		\
	output varying color aov_translucence;		\
	output varying color aov_shadows;			\
	output varying float aov_occlusion;			\
	output varying color aov_caustics;			\
	output varying color aov_opacity;			\
	output varying color _color;				\
	output varying color _diffuse;				\
	output varying color _specular;				\
	output varying color _ambient;				\
	output varying color _radiosity;			\
	output varying color _reflect;				\
	output varying color _refract;				\
	output varying color _shadow;				\
	output varying color _caustic;				\
	output varying color _diffshad;				\
	output varying normal _uv;					\
	output varying float _alpha;				\
	output varying float _z;					\
	output varying float _index;

#define DEFAULT_AOV_OUTPUT_PARAMETERS						\
	output varying color aov_surfacecolor		= color(0);	\
	output varying color aov_ambient			= color(0);	\
	output varying color aov_diffuse			= color(0);	\
	output varying color aov_indirectdiffuse	= color(0);	\
	output varying color aov_specular			= color(0);	\
	output varying color aov_specularcolor		= color(0);	\
	output varying color aov_reflection			= color(0);	\
	output varying color aov_refraction			= color(0);	\
	output varying color aov_rimlighting		= color(0);	\
	output varying color aov_scattering			= color(0);	\
	output varying color aov_translucence		= color(0);	\
	output varying color aov_shadows			= color(0);	\
	output varying float aov_occlusion			= 0;		\
	output varying color aov_caustics			= color(0);	\
	output varying color aov_opacity			= color(0);	\
	output varying color _color					= color(0);	\
	output varying color _diffuse				= color(0);	\
	output varying color _specular				= color(0);	\
	output varying color _ambient				= color(0);	\
	output varying color _radiosity				= color(0);	\
	output varying color _reflect				= color(0);	\
	output varying color _refract				= color(0);	\
	output varying color _shadow				= color(0);	\
	output varying color _caustic				= color(0);	\
	output varying color _diffshad				= color(0);	\
	output varying normal _uv					= normal(0);\
	output varying float _alpha					= 0;		\
	output varying float _z						= 0;		\
	output varying float _index					= 0;

#define getshadows(GETSHADOWS_PARAMS) \
	color inshadowcolor = color(0); \
	illuminance(GETSHADOWS_PARAMS) { \
		if (1 == lightsource("__shadow", inshadowcolor)) { \
			aov_shadows += (1 - inshadowcolor); } } \
	float objectpass = 0; \
	attribute("user:object_pass", objectpass); \
	_index = objectpass; \
	_color = aov_surfacecolor; \
	_diffuse = aov_diffuse; \
	_specular = aov_specular * aov_specularcolor; \
	_ambient = aov_ambient + aov_occlusion; \
	_radiosity = aov_indirectdiffuse; \
	_reflect = aov_reflection; \
	_refract = aov_refraction; \
	_shadow = aov_shadows; \
	_caustic = aov_caustics; \
	_diffshad = aov_diffuse; \
	_uv = normal(s, t, 0); \
	_alpha = comp(ctransform("hsv", aov_opacity), 2); \
	_z = length(I);

////////////////////////////////////////////////////////////////////////////////
#endif /* SHRIMP_AOV_H */

