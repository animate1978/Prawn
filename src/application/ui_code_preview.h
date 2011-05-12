
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


#ifndef _ui_code_preview_h_
#define _ui_code_preview_h_

#include "../miscellaneous/misc_xml.h"
#include "../miscellaneous/misc_string_functions.h"

#include <fltk/ReturnButton.h>
#include <fltk/TextDisplay.h>
#include <fltk/Window.h>

#include <string>

namespace code_preview
{

static fltk::TextDisplay* s_code;

class dialog {

private:
	fltk::Window* w;

public:
	dialog() {

		// build dialog window
		w = new fltk::Window (600, 500, "Code preview");
		w->begin();

			s_code = new fltk::TextDisplay (50,10, 540,450, "Surface");
			w->add (s_code);
			s_code->tooltip ("Shader code");
			s_code->wrap_mode (true);
			w->resizable (s_code);

			fltk::ReturnButton* rb = new fltk::ReturnButton (500, 470, 70, 25, "OK");
			rb->label ("Ok");
			rb->callback (cb_ok, (void*)this);

		w->end();
	}

	~dialog() {
		delete w;
	}

	void open (const std::string& Code) {

		s_code->text (Code.c_str());

		// show...
		w->exec();
	}

	void on_ok(fltk::Widget* W) {

		// close the dialog
		W->window()->make_exec_return (false);
	}

	static void cb_ok (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
};

}

#endif // _ui_code_preview_h_

