
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


#ifndef _ui_scene_view_h_
#define _ui_scene_view_h_

#include "../opengl_view.h"
#include "../shading/scene.h"

#include <fltk/run.h>

#include <fltk/GlWindow.h>
#include <fltk/gl.h>
#include <fltk/PopupMenu.h>


#include <stdlib.h>

class scene_view :
	public fltk::GlWindow,
	public sigc::trackable
{
public:
	scene_view (services* services_instance, opengl_view* opengl_view_instance, int x, int y, int w, int h, const char* l = 0);

	// OpenGL widget draw() override
	void draw();

	// OpenGL widget handle(int Event) override
	int handle (int Event);

	double fit_scene() { const double result = m_opengl_view->fit_scene (w(), h()); redraw(); return result; }

	// callbacks
	void shader_property_right_click (shrimp::io_t& Property);
	void shader_block_right_click (std::string& Block);
	void block_group_right_click (int Group);
	void empty_right_click();

private:
	services* m_services;
	opengl_view* m_opengl_view;

	// temporary storage
	shader_block* m_active_block;
	shrimp::io_t m_active_property;
	int m_active_group;

	// callbacks
	void on_select_block (fltk::Widget* W, void* Data);
	static void cb_select_block (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_select_block (W, Data); }
	void on_deselect_block (fltk::Widget* W, void* Data);
	static void cb_deselect_block (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_deselect_block (W, Data); }
	void on_roll_block (fltk::Widget* W, void* Data);
	static void cb_roll_block (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_roll_block (W, Data); }
	void on_unroll_block (fltk::Widget* W, void* Data);
	static void cb_unroll_block (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_unroll_block (W, Data); }
	void on_group_selection (fltk::Widget* W, void* Data);
	static void cb_group_selection(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_group_selection (W, Data); }
	void on_clear_selection (fltk::Widget* W, void* Data);
	static void cb_clear_selection(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_clear_selection (W, Data); }
	void on_rename_group (fltk::Widget* W, void* Data);
	static void cb_rename_group(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_rename_group (W, Data); }
	void on_ungroup (fltk::Widget* W, void* Data);
	static void cb_ungroup(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_ungroup (W, Data); }
	void on_block_info (fltk::Widget* W, void* Data);
	static void cb_block_info(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_block_info (W, Data); }
	void on_rename_block (fltk::Widget* W, void* Data);
	static void cb_rename_block(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_rename_block (W, Data); }
	void on_delete_block (fltk::Widget* W, void* Data);
	static void cb_delete_block(fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_delete_block (W, Data); }

	void on_add_input (fltk::Widget* W, void* Data);
	static void cb_add_input (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_add_input (W, Data); }
	void on_add_output (fltk::Widget* W, void* Data);
	static void cb_add_output (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_add_output (W, Data); }
	void on_edit_code (fltk::Widget* W, void* Data);
	static void cb_edit_code (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_edit_code (W, Data); }
	void on_edit_pad (fltk::Widget* W, void* Data);
	static void cb_edit_pad (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_edit_pad (W, Data); }
	void on_edit_RIB (fltk::Widget* W, void* Data);
	static void cb_edit_RIB (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_edit_RIB (W, Data); }
	void on_disconnect_pad (fltk::Widget* W, void* Data);
	static void cb_disconnect_pad (fltk::Widget* W, void* Data) { ((scene_view*)Data)->on_disconnect_pad (W, Data); }
};

#endif // _ui_scene_view_h_

