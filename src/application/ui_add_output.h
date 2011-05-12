
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


#ifndef _ui_add_output_h_
#define _ui_add_output_h_

#include "../miscellaneous/misc_xml.h"
#include "../miscellaneous/misc_string_functions.h"

#include <fltk/CheckButton.h>
#include <fltk/Choice.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextEditor.h>
#include <fltk/ValueInput.h>
#include <fltk/Window.h>

#include <string>

namespace add_output
{

static fltk::Input* s_name;
static fltk::Choice* s_storage = 0;
static fltk::Choice* s_type;
static fltk::Choice* s_array_type = 0;
static fltk::ValueInput* s_array_size = 0;
static fltk::CheckButton* s_shader_output;
static fltk::TextEditor* s_description;

class dialog {

private:
	fltk::Window* w;
	shader_block* m_block;

	// make sure the list isn't destroyed before the dialog closes
	types_t m_storage_types;
	types_t m_types;
	types_t m_array_types;

public:
	dialog() {

		// build type array
		m_types = get_property_types();

		// build dialog window
		w = new fltk::Window (400, 300, "Add output");
		w->begin();

			// name
			s_name = new fltk::Input (90,10, 250,23, "Name");
			w->add (s_name);
			s_name->tooltip ("Output name");

			// storage
			s_storage = new fltk::Choice (90,40, 90,23, "Type");
			s_storage->begin();
				m_storage_types = get_property_storage_types();
				for (types_t::const_iterator st_i = m_storage_types.begin(); st_i != m_storage_types.end(); ++st_i) {

					new fltk::Item (st_i->c_str());
				}
			s_storage->end();
			w->add (s_storage);
			s_storage->tooltip ("Input variable storage type");

			// type
			s_type = new fltk::Choice (180,40, 120,23, "");
			s_type->begin();
				for (types_t::const_iterator t = m_types.begin(); t != m_types.end(); ++t) {

					new fltk::Item (t->c_str());
				}
			s_type->end();
			w->add (s_type);
			s_type->tooltip ("Output type");
			s_type->callback(cb_type_change, (void*)this);
			s_type->when(fltk::WHEN_CHANGED);

			// array type
			s_array_type = new fltk::Choice (290,40, 100,23, "");
			s_array_type->begin();
				m_array_types = get_array_types();
				for (types_t::const_iterator t_i = m_array_types.begin(); t_i != m_array_types.end(); ++t_i) {

					new fltk::Item (t_i->c_str());
				}
			s_array_type->end();
			w->add (s_array_type);
			s_array_type->tooltip ("Array type");
			s_array_type->deactivate();

			// array size
			s_array_size = new fltk::ValueInput (290,65, 50,23, "");
			s_array_size->minimum (0);
			s_array_size->maximum (1E6);
			s_array_size->step (1);
			s_array_size->value (1);
			s_array_size->tooltip ("Array size");
			s_array_size->deactivate();

			// shader output checkbox
			s_shader_output = new fltk::CheckButton (90,70, 100,23, "Shader output");
			w->add (s_shader_output);
			s_shader_output->tooltip ("Input's default value");

			s_description = new fltk::TextEditor (90,100, 200,100, "Description");
			w->add (s_description);
			s_description->tooltip ("Output description");
			s_description->wrap_mode (true);
			w->resizable (s_description);


			// OK / Cancel
			fltk::ReturnButton* rb = new fltk::ReturnButton (150, 250, 70, 25, "OK");
			rb->label ("Ok");
			rb->callback (cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button (250, 250, 70, 25, "Cancel");
			cb->label ("Cancel");
			cb->callback (cb_cancel, (void*)this);

		w->end();
	}

	~dialog()
	{
		delete w;
	}

	void open_dialog(shader_block* Block)
	{
		// save processed block
		m_block = Block;

		// show it
		w->exec();
	}

	void on_type_change(fltk::Widget* W) {

		const std::string type = m_types[s_type->value()];
		if (type == "array") {
			s_array_type->activate();
			s_array_size->activate();
		} else {
			s_array_type->deactivate();
			s_array_size->deactivate();
		}
	}

	void on_ok (fltk::Widget* W) {

		// get user values
		const std::string name = s_name->value();
		const std::string type = m_types[s_type->value()];
		const std::string type_extension = m_array_types[s_array_type->value()] + ":" + string_cast(s_array_size->value());
		const std::string storage = m_storage_types[s_storage->value()];
		const bool shader_output = s_shader_output->value();
		const std::string description = s_description->text();

		if (name.empty()) {

			// don't allow empty names
			fltk::alert ("Output name is empty!");

		} else if (m_block->is_input (name)) {

			// an input with the same name already exists
			fltk::alert ("An input with the same name already exists!");

		} else if (m_block->is_output (name)) {

			// an output with the same name already exists
			fltk::alert ("An output with the same name already exists!");

		} else {
			// create the output
			m_block->add_output (name, type, type_extension, storage, description, shader_output);

			// close the dialog
			W->window()->make_exec_return (false);
		}
	}

	void on_cancel (fltk::Widget* W) {

		// close the dialog
		W->window()->make_exec_return(false);
	}

	static void cb_type_change(fltk::Widget* W, void* Data) { ((dialog*)Data)->on_type_change(W); }
	static void cb_ok (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
	static void cb_cancel (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_cancel(W); }
};

}

#endif // _ui_add_output_h_

