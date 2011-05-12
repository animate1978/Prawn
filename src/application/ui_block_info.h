
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


#ifndef _ui_block_info_h_
#define _ui_block_info_h_

#include "../miscellaneous/misc_xml.h"
#include "../miscellaneous/misc_string_functions.h"

#include <fltk/Output.h>
#include <fltk/Item.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextDisplay.h>
#include <fltk/TextEditor.h>
#include <fltk/Window.h>

#include <string>

namespace block_info
{

static fltk::Output* s_name;
static fltk::TextDisplay* s_description;
static fltk::TextDisplay* s_usage;
static fltk::Output* s_author;

class dialog {

private:
	fltk::Window* w;

public:
	dialog() {

		// build dialog window
		w = new fltk::Window(400, 300, "Block info");
		w->begin();

			s_name = new fltk::Output (90,5, 290, 23, "Name");
			w->add (s_name);
			s_name->tooltip ("Block name");

			s_description = new fltk::TextDisplay (90,30, 290,53, "Description");
			w->add (s_description);
			s_description->wrap_mode (true);
			s_description->tooltip ("Block description");

			s_usage = new fltk::TextDisplay (90,90, 290,133, "Usage");
			w->add (s_usage);
			s_usage->tooltip ("Block usage");
			s_usage->wrap_mode (true);
			w->resizable (s_usage);

			s_author = new fltk::Output (90,230, 290, 23, "Author");
			w->add (s_author);
			s_author->tooltip ("Block author");


			fltk::ReturnButton* rb = new fltk::ReturnButton(300,260, 70,25, "OK");
			rb->label("Ok");
			rb->callback(cb_ok, (void*)this);

		w->end();
	}

	~dialog() {

		delete w;
	}

	void open_dialog (const shader_block* Block) {

		// get block content
		s_name->text (Block->name().c_str());
		s_description->text (Block->m_description.c_str());
		s_usage->text (Block->m_usage.c_str());
		s_author->text (Block->m_author.c_str());

		// show...
		w->exec();
	}

	void on_ok (fltk::Widget* W) {

		// close the dialog
		W->window()->make_exec_return(false);
	}

	static void cb_ok(fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
};

}

#endif // _ui_block_info_h_

