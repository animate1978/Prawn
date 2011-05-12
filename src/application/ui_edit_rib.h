
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


#ifndef _ui_edit_rib_h_
#define _ui_edit_rib_h_

#include "../shading/rib_root_block.h"

#include "../miscellaneous/misc_string_functions.h"

#include <fltk/Choice.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/LightButton.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextEditor.h>
#include <fltk/Window.h>

#include <string>

namespace edit_rib
{

static fltk::TextEditor* s_code;
static fltk::Input* s_imager;
static fltk::LightButton* s_AOV;

class dialog {

private:
	fltk::Window* w;
	rib_root_block* m_block;

public:
	dialog() {

		// build dialog window
		w = new fltk::Window (600, 500, "Edit RIB");
		w->begin();

			s_code = new fltk::TextEditor (90,10, 500,350, "Declarations");
			w->add (s_code);
			s_code->tooltip ("RIB statements to include before the frame");
			s_code->wrap_mode (true);
			w->resizable (s_code);

			s_imager = new fltk::Input (90,362, 500,23, "Imager");
			w->add (s_imager);
			s_imager->tooltip ("Imager definition");

			s_AOV = new fltk::LightButton (70,400, 90,23, "AOV");
			w->add (s_AOV);
			s_AOV->tooltip ("Enable or disable AOV output preview");


			fltk::ReturnButton* rb = new fltk::ReturnButton (400, 470, 70, 25, "OK");
			rb->label ("Ok");
			rb->callback (cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button (500, 470, 70, 25, "Cancel");
			cb->label ("Cancel");
			cb->callback (cb_cancel, (void*)this);

		w->end();
	}

	~dialog() {

		delete w;
	}

	void open_dialog (rib_root_block* Block) {

		// save processed block
		m_block = Block;

		s_code->text (m_block->get_general_statements().c_str());
		s_imager->text (m_block->get_imager_statement().c_str());
		s_AOV->value (m_block->get_AOV());

		// show...
		w->exec();
	}

	void on_ok (fltk::Widget* W) {

		// get user's content and save it
		const std::string general_statements = s_code->text();
		m_block->set_general_statements (general_statements);

		const std::string imager_statement = s_imager->text();
		m_block->set_imager_statement (imager_statement);

		const bool AOV_preview = s_AOV->value();
		m_block->set_AOV (AOV_preview);

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

#endif // _ui_edit_rib_h_

