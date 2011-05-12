
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


#ifndef _ui_edit_output_h_
#define _ui_edit_output_h_

#include "../miscellaneous/logging.h"

#include <fltk/LightButton.h>
#include <fltk/Output.h>
#include <fltk/ReturnButton.h>
#include <fltk/ValueInput.h>
#include <fltk/Window.h>

#include <string>

namespace edit_output
{

static fltk::Output* s_name = 0;
static fltk::Choice* s_storage = 0;
static fltk::Choice* s_type = 0;
static fltk::Choice* s_array_type = 0;
static fltk::ValueInput* s_array_size = 0;
static fltk::LightButton* s_shader_output = 0;


class dialog {

private:
	fltk::Window* w;
	shader_block* m_shader_block;
	std::string m_edited_output;
	storages_t m_storage_types;
	types_t m_types;
	types_t m_array_types;

public:
	dialog (shader_block* Block) :
		m_shader_block (Block) {

		// build dialog window
		w = new fltk::Window (400, 160, "Edit output");
		w->begin();

			// name
			s_name = new fltk::Output (70,5, 120,23, "Name");
			w->add (s_name);
			s_name->tooltip ("Edited output name");

			// storage edition
			s_storage = new fltk::Choice (90,32, 90,23, "Type");
			s_storage->begin();
				// make sure the list isn't destroyed before the dialog closes
				m_storage_types = get_property_storage_types();
				for (types_t::const_iterator st_i = m_storage_types.begin(); st_i != m_storage_types.end(); ++st_i) {

					new fltk::Item (st_i->c_str());
				}
			s_storage->end();
			w->add (s_storage);
			s_storage->tooltip ("Input variable storage type");

			// type edition
			s_type = new fltk::Choice (180,32, 100,23, "");
			s_type->begin();
				// make sure the list isn't destroyed before the dialog closes
				m_types = get_property_types();
				for (types_t::const_iterator t_i = m_types.begin(); t_i != m_types.end(); ++t_i) {

					new fltk::Item (t_i->c_str());
				}
			s_type->end();
			s_type->callback (cb_type_change);
			s_type->when(fltk::WHEN_CHANGED);
			w->add (s_type);
			s_type->tooltip ("Output variable type");

			// array type edition
			s_array_type = new fltk::Choice (290,32, 100,23, "");
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
			s_array_size = new fltk::ValueInput (290,57, 50,23, "");
			s_array_size->minimum (0);
			s_array_size->maximum (1E6);
			s_array_size->step (1);
			s_array_size->value (1);
			s_array_size->tooltip ("Array size");
			s_array_size->deactivate();

			// shader output state (inactive)
			s_shader_output = new fltk::LightButton (70,59, 120,23, "Shader output");
			w->add (s_shader_output);
			s_shader_output->tooltip ("Make it a shader output for message passing");
			s_shader_output->deactivate();


			// OK / Cancel
			fltk::ReturnButton* rb = new fltk::ReturnButton (150,95, 70,25, "OK");
			rb->label("Ok");
			rb->callback(cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button (250,95, 70,25, "Cancel");
			cb->label("Cancel");
			cb->callback(cb_cancel, (void*)this);

		w->end();
	}

	~dialog() {

		delete w;
	}

	void edit_output (const std::string& OutputName) {

		m_edited_output = OutputName;

		// set name
		s_name->text (OutputName.c_str());

		// set storage
		const std::string storage = m_shader_block->get_output_storage (m_edited_output);
		int storage_type_number = 0;
		for (storages_t::const_iterator s_i = m_storage_types.begin(); s_i != m_storage_types.end(); ++s_i, ++storage_type_number) {

			if (storage == *s_i)
				s_storage->value (storage_type_number);
		}

		// set type
		const std::string type = m_shader_block->get_output_type (m_edited_output);
		int type_number = 0;
		for (types_t::const_iterator t_i = m_types.begin(); t_i != m_types.end(); ++t_i, type_number++) {

			if (type == *t_i)
				s_type->value (type_number);
				type_change();
		}

		// set array
		if (type == "array") {
			const std::string array_type = m_shader_block->get_output_type_extension (m_edited_output);
			int type_number = 0;
			for (types_t::const_iterator t_i = m_array_types.begin(); t_i != m_array_types.end(); ++t_i, ++type_number) {

				if (array_type == *t_i) {
					s_array_type->value (type_number);
				}
			}

			const int array_size = m_shader_block->get_output_type_extension_size (m_edited_output);
			s_array_size->value (array_size);
		}

		// set shader output state
		s_shader_output->value (m_shader_block->is_shader_output (OutputName));

		// show the dialog
		w->exec();
	}

	void on_type_change (fltk::Widget* W) {

		type_change();
	}

	void on_ok (fltk::Widget* W) {

		// save storage
		types_t storage_list = get_property_storage_types();
		const unsigned int storage_type_number = s_storage->value();
		if (storage_type_number >= 0 && storage_type_number < storage_list.size()) {

			m_shader_block->set_output_storage (m_edited_output, storage_list[storage_type_number]);
		}

		// save type
		types_t list = get_property_types();
		const unsigned int type_number = s_type->value();
		if (type_number >= 0 && type_number < list.size()) {

			m_shader_block->set_output_type (m_edited_output, list[type_number]);
		}

		// save type extension
		const std::string type_extension = m_array_types[s_array_type->value()] + ":" + string_cast(s_array_size->value());
		m_shader_block->set_output_type_extension (m_edited_output, type_extension);

		// save shader output state
		//m_shader_block->set_shader_output (m_edited_output, s_shader_output->value());

		// close the dialog
		W->window()->make_exec_return (false);
	}

	void on_cancel (fltk::Widget* W) {

		W->window()->make_exec_return (false);
	}

	static void cb_type_change (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_type_change (W); }
	static void cb_ok (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
	static void cb_cancel (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_cancel(W); }

private:
	void type_change () {

		types_t list = get_property_types();
		const unsigned int type_number = s_type->value();
		if (type_number >= 0 && type_number < list.size()) {

			const std::string current_type = list[type_number];
			if (current_type == "array") {
				s_array_type->activate();
				s_array_size->activate();
			} else {
				s_array_type->deactivate();
				s_array_size->deactivate();
			}
		}
	}
};

}

#endif // _ui_edit_output_h_

