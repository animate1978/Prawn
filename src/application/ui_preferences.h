
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


#ifndef _ui_preferences_h_
#define _ui_preferences_h_

#include "../shading/preferences.h"
#include "../miscellaneous/misc_xml.h"
#include "../miscellaneous/misc_string_functions.h"

#include <fltk/Choice.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/LightButton.h>
#include <fltk/Output.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextEditor.h>
#include <fltk/Window.h>

#include <string>

namespace preferences
{

static bool answer;

static fltk::LightButton* s_splash_screen;
static fltk::Choice* s_renderer;
static fltk::Output* s_renderer_code;
static fltk::Input* s_compilation;
static fltk::Input* s_shader_extension;
static fltk::Input* s_rendering;
static fltk::Choice* s_renderer_display;

static fltk::Input* s_render_width;
static fltk::Input* s_render_height;
static fltk::Input* s_shading_rate;
static fltk::Input* s_pixelsamples_x;
static fltk::Input* s_pixelsamples_y;
static fltk::Choice* s_pixel_filter;
static fltk::Input* s_filter_width_s;
static fltk::Input* s_filter_width_t;
static fltk::Choice* s_scene;
static fltk::Input* s_help_reader;

static std::vector<std::string> s_renderers;

// trick for FLTK's callback
class dialog;
dialog* preferences_dialog_instance;

class dialog :
	general_options
{

private:
	fltk::Window* w;

	std::string m_hidden_renderer_code;

public:
	dialog() {

		// load preferences
		load();

		// find shape number
		int shape_number = 0;
		int current_shape = 0;
		for (scenes_t::iterator s_i = m_scenes.begin(); s_i != m_scenes.end(); ++s_i, ++current_shape) {
			if (s_i->name == m_scene) {
				shape_number = current_shape;
			}
		}

		// warn the user if no scene file is found
		if (!m_scenes.size()) {

			log() << error << "couldn't find any scene file in '" << rib_scene_dir() << "' directory." << std::endl;
		}


		// build dialog window
		w = new fltk::Window (400, 500, "preferences");
		w->begin();

			s_splash_screen = new fltk::LightButton (250,10, 130,23, "splash screen");
			w->add (s_splash_screen);
			s_splash_screen->tooltip ("show the splash screen when launching shrimp");

			const int start = 30;

			s_renderer = new fltk::Choice (90,start, 120,25, "renderer");
			s_renderer->begin();
				new fltk::Item ("set defaults...");

				for (renderers_t::iterator r_i = m_renderers.begin(); r_i != m_renderers.end(); ++r_i) {

					if (r_i->first == _3delight)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)_3delight);
					else if (r_i->first == air)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)air);
					else if (r_i->first == aqsis)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)aqsis);
					else if (r_i->first == entropy)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)entropy);
					else if (r_i->first == pixie)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)pixie);
					else if (r_i->first == prman)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)prman);
					else if (r_i->first == renderdotc)
						new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)renderdotc);
				}
			s_renderer->end();
			w->add (s_renderer);
			s_renderer->tooltip ("overwrite current rendering settings with another renderer");

			s_renderer_code = new fltk::Output (120,start + 30, 200,23,"renderer code");
			w->add (s_renderer_code);
			s_renderer_code->tooltip ("renderer code (not editable, it's an option for the shader compiler)");

			s_compilation = new fltk::Input (120,start + 60, 250,23,"compilation");
			w->add (s_compilation);
			s_compilation->tooltip ("shader compilation command");

			s_shader_extension = new fltk::Input (120,start + 90, 50,23,"shader extension");
			w->add (s_shader_extension);
			s_shader_extension->tooltip ("compiled shader extension");

			s_rendering = new fltk::Input (120,start + 120, 250,23,"rendering");
			w->add (s_rendering);
			s_rendering->tooltip ("scene rendering command");

			s_renderer_display = new fltk::Choice (120,start + 150, 250,23,"display");
			w->add (s_renderer_display);
			s_renderer_display->tooltip ("renderer display for preview output");


			s_scene = new fltk::Choice (110,start + 200, 120,25, "scene");
			s_scene->begin();
				for (scenes_t::iterator s_i = m_scenes.begin(); s_i != m_scenes.end(); ++s_i) {

					new fltk::Item (s_i->name.c_str());
				}
			s_scene->end();
			w->add (s_scene);
			s_scene->tooltip ("object(s) rendered in the output scene");
			s_scene->value (shape_number);

			s_render_width = new fltk::Input (110,start + 230, 50,23,"render width");
			w->add (s_render_width);
			s_render_width->tooltip ("render test picture width in pixels");

			s_render_height = new fltk::Input (260,start + 230, 50,23,"render height");
			w->add (s_render_height);
			s_render_height->tooltip ("render test picture height in pixels");

			s_shading_rate = new fltk::Input (110,start + 260, 50,23,"shading rate");
			w->add (s_shading_rate);
			s_shading_rate->tooltip ("shading rate (the lower the longer the rendering time)");

			s_pixelsamples_x = new fltk::Input (110,start + 290, 50,23,"pixel samples x");
			w->add (s_pixelsamples_x);
			s_pixelsamples_x->tooltip ("renderman pixel samples on x");

			s_pixelsamples_y = new fltk::Input (260,start + 290, 50,23,"pixel samples y");
			w->add (s_pixelsamples_y);
			s_pixelsamples_y->tooltip ("renderMan pixel samples on Y");

			s_pixel_filter = new fltk::Choice (110,start +330, 120,25,"pixel filter");
			w->add (s_pixel_filter);
			s_pixel_filter->tooltip ("select current filter used to filter the Pixel Samples into the final color in s and t");

			s_filter_width_s = new fltk::Input (110,start + 360, 50,23,"filter width s");
			w->add (s_filter_width_s);
			s_filter_width_s->tooltip ("Suggested values , Box: 1.0 Triangle: 2.0 Blackman-Harris: 2.0 Gaussian: 2.5 Catmull-Rom: 4.0 Separable Catmull-Rom: 4.0 Mitchell: 4.0 Lanczos: 6.0 Bessel: 6.49 Sinc: 8.0");

			s_filter_width_t = new fltk::Input (260,start + 360, 50,23,"filter width t");
			w->add (s_filter_width_t);
			s_filter_width_t->tooltip ("Suggested values , Box: 1.0 Triangle: 2.0 Blackman-Harris: 2.0 Gaussian: 2.5 Catmull-Rom: 4.0 Separable Catmull-Rom: 4.0 Mitchell: 4.0 Lanczos: 6.0 Bessel: 6.49 Sinc: 8.0");

			s_help_reader = new fltk::Input (120,start + 390, 250,23,"help reader");
			w->add (s_help_reader);
			s_compilation->tooltip ("help reader command");


			fltk::ReturnButton* rb = new fltk::ReturnButton (150, start + 420, 70, 25, "OK");
			rb->label ("Ok");
			rb->callback (cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button (250, start + 420, 70, 25, "Cancel");
			cb->label ("Cancel");
			cb->callback (cb_cancel, (void*)this);

		w->end();

		// initialize renderer code
		m_hidden_renderer_code = m_renderer_code;
	}

	~dialog() {

		delete w;
	}

	bool pref_dialog() {

		// save instance for callback
		preferences_dialog_instance = this;

		// set values
		s_compilation->text (m_shader_compiler.c_str());
		s_shader_extension->text (m_compiled_shader_extension.c_str());
		s_renderer_code->text (m_renderer_code.c_str());
		s_rendering->text (m_renderer.c_str());
		set_display_chooser (m_renderer_code.c_str(), m_renderer_display);

		std::string render_width = string_cast (m_output_width);
		std::string render_height = string_cast (m_output_height);
		s_render_width->text (render_width.c_str());
		s_render_height->text (render_height.c_str());

		std::string shading_rate = string_cast (m_shading_rate);
		s_shading_rate->text (shading_rate.c_str());

		std::string samples_x = string_cast (m_samples_x);
		std::string samples_y = string_cast (m_samples_y);
		s_pixelsamples_x->text (samples_x.c_str());
		s_pixelsamples_y->text (samples_y.c_str());

		set_pixelfilter_chooser (m_renderer_code.c_str(), m_pixel_filter);

		std::string filter_width_s = string_cast (m_filter_width_s);
		std::string filter_width_t = string_cast (m_filter_width_t);
		s_filter_width_s->text (filter_width_s.c_str());
		s_filter_width_t->text (filter_width_t.c_str());
		s_help_reader->text(m_help_reader.c_str());
		s_splash_screen->value (m_splash_screen);

		// show it
		w->exec();

		return answer;
	}

	void on_ok (fltk::Widget* W) {

		// save values
		m_renderer_code = m_hidden_renderer_code;
		m_shader_compiler = trim (s_compilation->value());
		m_compiled_shader_extension = trim (s_shader_extension->value());
		m_renderer = trim (s_rendering->value());
		m_renderer_display = get_display_chooser_value (m_hidden_renderer_code);
		m_pixel_filter = get_pixelfilter_chooser_value(m_hidden_renderer_code);

		std::string render_width = trim (s_render_width->value());
		std::string render_height = trim (s_render_height->value());
		m_output_width = from_string (render_width, 256);
		m_output_height = from_string (render_height, 256);

		std::string shading_rate = trim (s_shading_rate->value());
		m_shading_rate = from_string (shading_rate, 1.0); // keep 1.0 (not 1) to convert to double

		std::string samples_x = trim (s_pixelsamples_x->value());
		std::string samples_y = trim (s_pixelsamples_y->value());
		m_samples_x = from_string (samples_x, 1.0); // keep 1.0 (not 1) to convert to double
		m_samples_y = from_string (samples_y, 1.0); // keep 1.0 (not 1) to convert to double

		std::string filter_width_s = trim (s_filter_width_s->value());
		std::string filter_width_t = trim (s_filter_width_t->value());
		m_filter_width_s = from_string (filter_width_s, 1.0);
		m_filter_width_t = from_string (filter_width_t, 1.0);

		// get scene name
		const unsigned int scene_number = s_scene->value();
		if (scene_number >= 0 && scene_number < m_scenes.size()) {

			m_scene = m_scenes[scene_number].name;
		}

		m_help_reader = trim (s_help_reader->value());
		m_splash_screen = s_splash_screen->value();

		save();

		answer = true;
		W->window()->make_exec_return (false);
	}
	void on_cancel (fltk::Widget* W) {

		answer = false;
		W->window()->make_exec_return(false);
	}
	static void cb_ok (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
	static void cb_cancel (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_cancel(W); }

	void on_renderer_choice (fltk::Widget* W, void* Data) {

		std::string renderer_code ("");
		std::string shader_compiler ("");
		std::string compiled_shader_extension ("");
		std::string renderer ("");
		std::string renderer_display ("");
		std::string filter_type ("");

		std::string old_renderer_code = m_hidden_renderer_code;

		m_hidden_renderer_code = std::string ((const char*)Data);
		renderers_t::const_iterator r = m_renderers.find (m_hidden_renderer_code);
		if (r != m_renderers.end()) {

			renderer_code = m_hidden_renderer_code;
			shader_compiler = r->second.shader_compiler;
			compiled_shader_extension = r->second.compiled_shader_extension;
			renderer = r->second.renderer_command;
			renderer_display = get_display_chooser_value (old_renderer_code);
			filter_type = get_pixelfilter_chooser_value (old_renderer_code);
		}

		s_renderer_code->text (renderer_code.c_str());
		s_compilation->text (shader_compiler.c_str());
		s_shader_extension->text (compiled_shader_extension.c_str());
		s_rendering->text (renderer.c_str());
		set_display_chooser (renderer_code, renderer_display);
		set_pixelfilter_chooser (renderer_code, filter_type);
	}

	static void cb_renderer (fltk::Widget* W, void* Data) {

		preferences_dialog_instance->on_renderer_choice (W, Data);
	}

	void set_display_chooser (const std::string RendererCode, const std::string Value = "") {

		unsigned long display_number = 0;

		general_options::renderers_t::const_iterator r = m_renderers.find (RendererCode);
		if (m_renderers.end() == r) {
			log() << error << "unknown renderer (set_display_chooser): '" << RendererCode << "'." << std::endl;
		}

		s_renderer_display->remove_all();
		s_renderer_display->begin();
		unsigned long current_display_number = 0;
		for (general_options::displays_t::const_iterator current_display = r->second.displays.begin(); current_display != r->second.displays.end(); ++current_display, ++current_display_number) {
			new fltk::Item (current_display->c_str());

			if (*current_display == Value) {
				display_number = current_display_number;
			}
		}
		s_renderer_display->end();

		// set value
		if (r->second.displays.size() > 0) {

			s_renderer_display->value (display_number);
		}
	}

	std::string get_display_chooser_value (const std::string RendererCode) {

		const unsigned long list_number = s_renderer_display->value();

		general_options::renderers_t::const_iterator r = m_renderers.find (RendererCode);
		if (r->second.displays.size() > list_number) {

			return r->second.displays[list_number];
		}

		return "";
	}

	void set_pixelfilter_chooser (const std::string RendererCode, const std::string Value = "") {

			unsigned long filter_number = 0;

			general_options::renderers_t::const_iterator r = m_renderers.find (RendererCode);
			if ( m_renderers.end() == r) {
				log() << error << "unknown renderer (set_pixelfilter_chooser): '" << RendererCode << "'." << std::endl;
			}

			s_pixel_filter->remove_all();
			s_pixel_filter->begin();
			unsigned long current_filter_number = 0;
			for (general_options::filters_t::const_iterator current_filter = r->second.filter_type.begin(); current_filter != r->second.filter_type.end(); ++current_filter, ++current_filter_number) {
				new fltk::Item (current_filter->c_str());

				if (*current_filter == Value) {
					filter_number = current_filter_number;
				}
			}
			s_pixel_filter->end();

			// set value
			if (r->second.filter_type.size() > 0) {

				s_pixel_filter->value (filter_number);
			}
		}

		std::string get_pixelfilter_chooser_value (const std::string RendererCode) {

			const unsigned long list_number = s_pixel_filter->value();

			general_options::renderers_t::const_iterator r = m_renderers.find (RendererCode);
			if (r->second.filter_type.size() > list_number) {

				return r->second.filter_type[list_number];
			}

			return "";
		}

};

}

#endif // _ui_preferences_h_

