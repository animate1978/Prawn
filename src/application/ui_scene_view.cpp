
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


#include "ui_scene_view.h"

#include "ui_add_input.h"
#include "ui_add_output.h"
#include "ui_block_info.h"
#include "ui_edit_code.h"
#include "ui_edit_block_name.h"
#include "ui_edit_group_name.h"
#include "ui_edit_input.h"
#include "ui_edit_output.h"
#include "ui_edit_rib.h"

#include "../opengl_view.h"
#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"


#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/gl.h>
#include <fltk/Item.h>
#include <fltk/MenuBar.h>
#include <fltk/PopupMenu.h>

#if defined(__APPLE__) && defined (__MACH__)
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

#include <iostream>
#include <cmath>
#include <limits>


scene_view::scene_view (services* services_instance, opengl_view* opengl_view_instance, int x, int y, int w, int h, const char* l) :
	GlWindow (x,y,w,h,l),
	m_services (services_instance),
	m_opengl_view (opengl_view_instance)
{
	// connect signals
	m_opengl_view->m_shader_property_right_click_signal.connect (sigc::mem_fun(*this, &scene_view::shader_property_right_click));
	m_opengl_view->m_shader_block_right_click_signal.connect (sigc::mem_fun(*this, &scene_view::shader_block_right_click));
	m_opengl_view->m_block_group_right_click_signal.connect (sigc::mem_fun(*this, &scene_view::block_group_right_click));
	m_opengl_view->m_empty_right_click_signal.connect (sigc::mem_fun(*this, &scene_view::empty_right_click));
}


void scene_view::draw() {

	m_opengl_view->draw_scene (valid(), w(), h());
	redraw();
}


void scene_view::on_select_block (fltk::Widget* W, void* Data)
{
	m_services->set_block_selection (m_opengl_view->get_active_block(), true);
	redraw();
}


void scene_view::on_deselect_block (fltk::Widget* W, void* Data)
{
	m_services->set_block_selection (m_opengl_view->get_active_block(), false);
	redraw();
}


void scene_view::on_roll_block (fltk::Widget* W, void* Data)
{
	m_services->set_block_rolled_state (m_opengl_view->get_active_block(), true);
	redraw();
}


void scene_view::on_unroll_block (fltk::Widget* W, void* Data)
{
	m_services->set_block_rolled_state (m_opengl_view->get_active_block(), false);
	redraw();
}


void scene_view::on_group_selection (fltk::Widget* W, void* Data) {

	m_services->group_selection();
	redraw();
}


void scene_view::on_rename_group (fltk::Widget* W, void* Data)
{
	std::string name (m_services->get_group_name (m_active_group));
	if (edit_group_name::dialog (name))
	{
		m_services->set_group_name (m_active_group, edit_group_name::name->value());
	}

	redraw();
}


void scene_view::on_ungroup (fltk::Widget* W, void* Data)
{
	m_services->ungroup (m_active_group);
	redraw();
}


void scene_view::on_clear_selection (fltk::Widget* W, void* Data)
{
	m_services->clear_selection();
	redraw();
}


void scene_view::on_block_info (fltk::Widget* W, void* Data) {

	block_info::dialog d;
	d.open_dialog (m_active_block);

	redraw();
}


void scene_view::on_rename_block (fltk::Widget* W, void* Data)
{
	edit_block_name::dialog d (m_services);
	d.edit_name (m_active_block->name());

	redraw();
}


void scene_view::on_delete_block (fltk::Widget* W, void* Data)
{
	const std::string message = "Do you really want to delete '" + m_active_block->name() + "' block?";
	if (fltk::ask (message.c_str())) {
		m_services->delete_block (m_active_block->name());
	}

	redraw();
}


void scene_view::on_add_input (fltk::Widget* W, void* Data)
{
	add_input::dialog d;
	d.open_dialog (m_active_block);

	redraw();
}


void scene_view::on_add_output (fltk::Widget* W, void* Data)
{
	add_output::dialog d;
	d.open_dialog (m_active_block);

	redraw();
}


void scene_view::on_edit_code (fltk::Widget* W, void* Data) {

	edit_code::dialog d;
	d.open_dialog (m_active_block);
}


void scene_view::on_edit_pad (fltk::Widget* W, void* Data) {

	const std::string current_property_name = m_active_property.second;

	if (m_active_block && m_active_block->is_input (current_property_name)) {

		edit_input::dialog d (m_active_block);
		d.edit_input (current_property_name);
	}
	if (m_active_block && m_active_block->is_output (current_property_name)) {

		edit_output::dialog d (m_active_block);
		d.edit_output (current_property_name);
	}

	redraw();
}


void scene_view::on_edit_RIB (fltk::Widget* W, void* Data) {

	rib_root_block* root_block = dynamic_cast<rib_root_block*> (m_active_block);
	if (!root_block) {

		log() << error << "trying to edit a non-RIB block as a RIB block." << std::endl;
		return;
	}

	edit_rib::dialog d;
	d.open_dialog (root_block);
}


void scene_view::on_disconnect_pad (fltk::Widget* W, void* Data)
{
	m_services->disconnect (m_active_property);

	redraw();
}


void scene_view::shader_property_right_click(shrimp::io_t& property)
{
	m_active_property = property;
	m_active_block = m_services->get_block (property.first);
	if (!m_active_block)
	{
		return;
	}

	const bool is_root = (m_active_block && m_active_block->m_root_block);

	fltk::PopupMenu mb (fltk::event_x(), fltk::event_y(), 0, 0, "Pad");
	mb.type (fltk::PopupMenu::POPUP3);
	mb.begin();
		int item_number = 0;

		// add the Edit menu item (except for the root block pads)
		if (!is_root) {
			++item_number;
			new fltk::Item ("Edit", 0, cb_edit_pad, (void*)this);
		}

		// add the Disconnect menu item if the block has a parent
		std::string foo;
		if (m_services->get_parent (property.first, property.second, foo)) {
			++item_number;
			new fltk::Item ("Disconnect", 0, cb_disconnect_pad, (void*)this);
		}
	mb.end();

	if (item_number > 0) {
		mb.popup();
	}
}


void scene_view::shader_block_right_click(std::string& block_name)
{
	m_active_block = m_services->get_block (block_name);
	if (!m_active_block)
	{
		return;
	}

	const bool is_root = m_active_block->m_root_block;

	fltk::PopupMenu mb (fltk::event_x(), fltk::event_y(), 0, 0, "Block");
	mb.type (fltk::PopupMenu::POPUP3);
	mb.begin();
		if (!is_root) {
			if (m_services->is_selected (m_active_block)) {
				new fltk::Item ("Deselect", 0, cb_deselect_block, (void*)this);
			} else {
				new fltk::Item ("Select", 0, cb_select_block, (void*)this);
			}
			if (m_services->selection_size() > 1) {
				new fltk::Item ("Group selection", 0, cb_group_selection, (void*)this);
			}

			if (m_services->is_rolled (m_active_block))
			{
				new fltk::Item ("Unroll", 0, cb_unroll_block, (void*)this);
			}
			else
			{
				new fltk::Item ("Roll", 0, cb_roll_block, (void*)this);
			}
		}

		new fltk::Item ("Info", 0, cb_block_info, (void*)this);

		new fltk::Item ("Rename", 0, cb_rename_block, (void*)this);

		if (!is_root) {

			new fltk::Item ("Add input", 0, cb_add_input, (void*)this);
			new fltk::Item ("Add output", 0, cb_add_output, (void*)this);
			new fltk::Item ("Edit code", 0, cb_edit_code, (void*)this);

			new fltk::Item ("Delete", 0, cb_delete_block, (void*)this);
		} else {

			// RIB root block functions
			rib_root_block* rib = dynamic_cast<rib_root_block*> (m_active_block);
			if (rib) {

				new fltk::Item ("Edit RIB", 0, cb_edit_RIB, (void*)this);
			}
		}

	mb.end();

	mb.popup();
}


void scene_view::block_group_right_click(int Group)
{
	m_active_group = Group;

	fltk::PopupMenu group_menu (fltk::event_x(), fltk::event_y(), 0, 0, "Group");
	group_menu.type (fltk::PopupMenu::POPUP3);
	group_menu.begin();
		new fltk::Item ("Rename group", 0, cb_rename_group, (void*)this);
		new fltk::Item ("Ungroup", 0, cb_ungroup, (void*)this);
	group_menu.end();

	group_menu.popup();
}


void scene_view::empty_right_click()
{
	fltk::PopupMenu group_menu (fltk::event_x(), fltk::event_y(), 0, 0, "Menu");
	group_menu.type (fltk::PopupMenu::POPUP3);
	group_menu.begin();
	if (m_services) {
		new fltk::Item ("Group selection", 0, cb_group_selection, (void*)this);
		new fltk::Item ("Clear selection", 0, cb_clear_selection, (void*)this);
	}
	group_menu.end();

	group_menu.popup();
}


int scene_view::handle (int Event)
{
	switch (Event)
	{
		// mouse wheel event
		case fltk::MOUSEWHEEL:
		{
			int wheel_move = fltk::event_dy();
			m_opengl_view->mouse_wheel_update(static_cast<double>(wheel_move));
			redraw();
		}
		return 1;

		// mouse move
		case fltk::MOVE:
		{
			m_opengl_view->mouse_move();
			redraw();
		}
		return 1;


		// mouse down
		case fltk::PUSH:
		{
			if ((fltk::event_button() == fltk::LeftButton) || (fltk::event_button() == fltk::RightButton)) {

				m_opengl_view->mouse_any_button_down();
				redraw();

				if (fltk::event_button() == fltk::LeftButton) {
					m_opengl_view->mouse_left_button_down();
					redraw();
				}

				if (fltk::event_button() == fltk::RightButton) {
					m_opengl_view->mouse_right_button_down();
					redraw();
				}
			}
		}
		return 1;

		// mouse drag
		case fltk::DRAG:
		{
			m_opengl_view->mouse_any_button_drag();
			redraw();

			if (fltk::event_button() == fltk::LeftButton) {
				m_opengl_view->mouse_left_button_drag(w(), h());
				redraw();
			}

		}
		return 1;

		// mouse up
		case fltk::RELEASE:
		{
			//TODO:
//			m_active_group = select_group();
			if (fltk::event_button() == fltk::LeftButton) {
				m_opengl_view->mouse_left_button_release();
				redraw();
			}

			//TODO:
			//m_box_selection=false;
			return 1;
		}

		default:
			// Let the base class handle all other events:
			return fltk::GlWindow::handle (Event);
	}
}



