
/*
    Copyright 2008, Romain Behar <romainbehar@users.sourceforge.net>

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


#ifndef _ui_edit_code_h_
#define _ui_edit_code_h_

#include "../miscellaneous/misc_xml.h"
#include "../miscellaneous/misc_string_functions.h"

#include <fltk/Choice.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextDisplay.h>
#include <fltk/TextEditor.h>
#include <fltk/Window.h>

#include <string>

namespace edit_code
{

static fltk::TextDisplay* s_info;
static fltk::TextEditor* s_code;

class dialog {

private:
	fltk::Window* w;
	shader_block* m_block;

public:
	dialog() {

		// build dialog window
		w = new fltk::Window(600, 500, "Edit code");
		w->begin();

			s_info = new fltk::TextDisplay (70,10, 520,100, "Block Info");
			w->add (s_info);
			s_info->wrap_mode (true);
			s_info->tooltip ("Informations about the block to write the shader code");

			s_code = new fltk::TextEditor (70,112, 520,350, "Code");
			w->add (s_code);
			s_code->tooltip ("Shader code");
			s_code->wrap_mode (true);
			w->resizable (s_code);


			fltk::ReturnButton* rb = new fltk::ReturnButton (400, 470, 70, 25, "OK");
			rb->label ("Ok");
			rb->callback (cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button (500, 470, 70, 25, "Cancel");
			cb->label ("Cancel");
			cb->callback (cb_cancel, (void*)this);

		w->end();
	}

	~dialog()
	{
		delete w;
	}

	void open_dialog (shader_block* Block)
	{
		// save processed block
		m_block = Block;

		// get block content
		std::string info ("");
		std::string inputs ("");
		std::string outputs ("");

		for (shader_block::properties_t::const_iterator input = m_block->m_inputs.begin(); input != m_block->m_inputs.end(); ++input) {

			if (!inputs.empty()) {
				inputs += ", ";
			}

			inputs += input->m_name;
		}
		for (shader_block::properties_t::const_iterator output = m_block->m_outputs.begin(); output != m_block->m_outputs.end(); ++output) {

			if (!outputs.empty()) {
				outputs += ", ";
			}

			outputs += output->m_name;
		}

		info = "inputs: " + inputs + "\n"
			+ "outputs: " + outputs;

		s_info->text (info.c_str());
		s_code->text (m_block->get_code().c_str());

		// show...
		w->exec();
	}

	void on_ok (fltk::Widget* W) {

		// get user's content and save it
		const std::string code = s_code->text();
		m_block->set_code (code);

		// close the dialog
		W->window()->make_exec_return (false);
	}

	void on_cancel (fltk::Widget* W) {

		// close the dialog
		W->window()->make_exec_return (false);
	}

	static void cb_ok (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
	static void cb_cancel (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_cancel(W); }
};

}

#endif // _ui_edit_code_h_

