
/*
    Copyright 2008-2009, Romain Behar <romainbehar@users.sourceforge.net>

    This file is part of Shrimp 2.

    Shrimp 2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Shrimp 2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Shrimp 2.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _preferences_h_
#define _preferences_h_

#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"
#include "../miscellaneous/misc_system_functions.h"
#include "../miscellaneous/misc_xml.h"

#include <fstream>
#include <string>

// all renderers must be referenced here, they're used by FLTK's callbacks
// and serve as renderer identifiers through the -DRENDERER=... option
static const char _3delight[] = "_3delight";
static const char air[] = "air";
static const char aqsis[] = "aqsis";
static const char entropy[] = "entropy";
static const char pixie[] = "pixie";
static const char prman[] = "prman";
static const char renderdotc[] = "renderdotc";

class general_options
{
	const std::string m_preferences_file;
	const std::string m_rib_renderer_file;
	const std::string m_rib_scene_dir;

public:
	// renderer list
	typedef std::vector<std::string> displays_t;
	typedef std::vector<std::string> filters_t;
	struct renderer_t {

		std::string name;
		std::string shader_compiler;
		std::string compiled_shader_extension;
		std::string renderer_command;
		displays_t displays;
		filters_t filter_type;
		std::string filter_size;


		std::string texture_command;
		std::string texture_default;
		std::string texture_extension;
	};
	typedef std::map<std::string, renderer_t> renderers_t;
	renderers_t m_renderers;

	// scene list
	struct scene_t {

		std::string name;
		std::string file;
	};
	typedef std::vector<scene_t> scenes_t;
	scenes_t m_scenes;

	std::string m_help_reader;
	bool m_splash_screen;

	std::string m_renderer_code;
	std::string m_shader_compiler;
	std::string m_compiled_shader_extension;
	std::string m_renderer;
	std::string m_renderer_display;
	std::string m_pixel_filter;


	double m_output_width;
	double m_output_height;
	double m_shading_rate;
	double m_samples_x;
	double m_samples_y;
	double m_filter_width_s;
	double m_filter_width_t;

	std::string m_scene;

public:
	general_options();

	const std::string renderer_file() const;
	const std::string rib_scene_dir() const;

	bool load();
	bool save();
	void set_defaults();

	std::string get_RIB_scene();

	void load_renderer_list();
	renderers_t get_renderer_list();

	void load_scene_list();
	scenes_t get_scene_list();

	void set_renderer (const std::string& RendererCode);
	void set_display (const std::string& RendererDisplay);
	void set_scene (const std::string& Scene);
	void set_pixelfilter (const std::string& Pixel_filter);
	void set_help (const std::string& Help);

private:
	const std::string preferences_file();
};

#endif // _preferences_h_

