
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


#ifndef _ui_application_window_h_
#define _ui_application_window_h_

#include <fltk/Choice.h>
#include <fltk/DoubleBufferWindow.h>
#include <fltk/Group.h>
#include <fltk/InvisibleBox.h>
#include <fltk/PopupMenu.h>
#include <fltk/RadioButton.h>
#include <fltk/ReturnButton.h>
#include <fltk/Slider.h>
#include <fltk/ThumbWheel.h>
#include <fltk/ToggleItem.h>
#include <fltk/Window.h>

#include "../shading/preferences.h"
#include "../shading/scene.h"
#include "ui_scene_view.h"

#include <fstream>
#include <iostream>
using namespace std;

class application_window;
static application_window* application_pointer = 0;

class application_window : public fltk::Window {

public:
	application_window(services* services_instance, opengl_view* opengl_view_instance);
	~application_window();

	bool load_scene (const std::string& File);

private:
	// shrimp services
	services* m_services;

	// OpenGL view of current scene
	opengl_view* m_opengl_view;
	scene_view* m_scene_view;

	// scene zoom widget
	fltk::Slider m_zoom_slider;
	fltk::PopupMenu m_block_menu;

	// renderer list
	general_options::renderers_t m_renderers;
	// renderer chooser
	fltk::Choice* m_renderer_chooser;
	// display chooser
	fltk::Choice* m_renderer_display_chooser;

	// scene list
	general_options::scenes_t m_scenes;
	// scene chooser
	fltk::Choice* m_scene_chooser;

	// menu items
	fltk::ToggleItem* m_menu_show_grid;
	fltk::ToggleItem* m_menu_snap_to_grid;
	fltk::ToggleItem* m_menu_overview;

	void on_window_callback (fltk::Widget*);

	void on_menu_file_new (fltk::Widget*);
	void on_menu_file_open (fltk::Widget*);
	void on_menu_file_save (fltk::Widget*);
	void on_menu_file_save_as (fltk::Widget*);
	void on_menu_file_export_scene (fltk::Widget*);
	void on_menu_file_options (fltk::Widget*);

	void on_menu_shader_properties (fltk::Widget*);
	void on_menu_code_preview (fltk::Widget*);
	void on_menu_preferences (fltk::Widget*);

	void on_menu_edit_copy (fltk::Widget*);
	void on_menu_edit_paste (fltk::Widget*);
	void on_menu_edit_cut (fltk::Widget*);
	void on_menu_edit_group (fltk::Widget*);
	void on_menu_edit_ungroup (fltk::Widget*);
	void on_menu_edit_delete (fltk::Widget*);
	void on_menu_edit_edit (fltk::Widget*);


	void on_menu_view_toggle_grid (fltk::Widget*);
	void on_menu_view_toggle_grid_snap (fltk::Widget*);
	void on_menu_view_toggle_overview (fltk::Widget*);

	void on_zoom (fltk::Slider*, void* Data);
	void on_button_fit_scene (fltk::Widget*);
	void on_custom_block();
	void on_preview();

	void on_renderer_choice (fltk::Widget* W, void* Data);
	void on_renderer_display_choice (fltk::Widget* W, void* Data);
	void on_scene_choice (fltk::Widget* W, void* Data);

	static void block_menu_callback(fltk::Widget* o, void* v) {
		((application_window*)(o->parent()))->block_menu_action (o, v);
	}
	void block_menu_action (fltk::Widget* w, void*);


	void build_menu (const block_tree_node_t& tree);
	void build_help (const block_tree_node_t& tree, std::ofstream& out, std::ofstream& outc);

	// update renderer, display and scene choosers
	void set_renderer_chooser_value (const std::string RendererCode);
	std::string set_display_chooser_value (const std::string RendererName, const std::string DisplayName);
	void set_scene_chooser_value (const std::string Scene);

	// override the window's handle() function
	int handle (int event);

	// callback functions
	static void cb_window (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_window_callback (W); }

	static void cb_menu_file_new (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_file_new (W); }
	static void cb_menu_file_open (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_file_open (W); }
	static void cb_menu_file_save (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_file_save (W); }
	static void cb_menu_file_save_as (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_file_save_as (W); }
	static void cb_menu_file_export_scene (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_file_export_scene (W); }
	static void cb_menu_file_options (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_file_options (W); }

	static void cb_menu_shader_properties (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_shader_properties (W); }
	static void cb_menu_code_preview (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_code_preview (W); }
	static void cb_menu_preferences (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_preferences (W); }

	static void cb_menu_edit_copy (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_copy (W); }
	static void cb_menu_edit_paste (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_paste (W); }
	static void cb_menu_edit_cut (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_cut (W); }

	static void cb_menu_edit_group (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_group (W); }
	static void cb_menu_edit_ungroup (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_ungroup (W); }
	static void cb_menu_edit_delete (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_delete (W); }
	static void cb_menu_edit_edit (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_edit_edit (W); }

	static void cb_menu_view_toggle_grid (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_view_toggle_grid (W); }
	static void cb_menu_view_toggle_grid_snap (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_view_toggle_grid_snap (W); }
	static void cb_menu_view_toggle_overview (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_menu_view_toggle_overview (W); }

	static void cb_button_fit_scene (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_button_fit_scene(W); }
	static void cb_zoom_slider (fltk::Slider* W, void* Data) { ((application_window*)Data)->on_zoom(W, Data); }
	static void cb_custom_block (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_custom_block(); }
	static void cb_preview (fltk::Widget* W, void* Data) { ((application_window*)Data)->on_preview(); }

	static void cb_renderer (fltk::Widget* W, void* Data) { application_pointer->on_renderer_choice (W, Data); }
	static void cb_renderer_display (fltk::Widget* W, void* Data) { application_pointer->on_renderer_display_choice (W, Data); }
	static void cb_scene (fltk::Widget* W, void* Data) { application_pointer->on_scene_choice (W, Data); }

	static void on_menu_file_close (fltk::Widget*, void*);
	static void on_menu_file_quit (fltk::Widget*, void*);
	static void on_menu_help_help (fltk::Widget*, void*);
	static void on_menu_help_about (fltk::Widget*, void*);


	// internal functions
	void save_scene_as();

};


#endif // _ui_application_window_h_

