
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


#ifndef _ui_edit_group_name_h_
#define _ui_edit_group_name_h_

#include <fltk/Input.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextEditor.h>
#include <fltk/Window.h>

#include <string>

namespace edit_group_name
{

static fltk::Input* name = 0;

static int answer;
static void set_answer(fltk::Widget* w, long a)
{
	answer = a;
	w->window()->make_exec_return(false);
}

static int dialog(const std::string& Name)
{
	// build dialog window
	fltk::Window window(400, 80, "Group name");
	window.begin();

		if(!name)
			name = new fltk::Input(70,10, 300,23, "Name");
		else
			window.add(name);
		name->tooltip("Group name");


		name->text(Name.c_str());

		fltk::ReturnButton* rb = new fltk::ReturnButton(150, 40, 75, 25, "OK");
		rb->label("Ok");
		rb->callback(set_answer, 1);

		fltk::Button* cb = new fltk::Button(250, 40, 70, 25, "Cancel");
		cb->label("Cancel");
		cb->callback(set_answer, 0);

	window.end();

	// show it
	window.exec();

	// don't delete the dialogs with the window
	window.remove(name);

	return answer;
}

}

#endif // _ui_edit_group_name_h_

