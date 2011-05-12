
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


#ifndef _ui_shader_properties_h_
#define _ui_shader_properties_h_

#include <fltk/Input.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextEditor.h>
#include <fltk/Window.h>

#include <string>

namespace shader_properties
{

static fltk::Input* name = 0;
static fltk::TextEditor* description = 0;
static fltk::Input* authors = 0;

static int sp_answer;
static void set_sp_answer(fltk::Widget* w, long a)
{
	sp_answer = a;
	w->window()->make_exec_return(false);
}

static int sp_dialog(const std::string& Name, const std::string& Description, const std::string& Authors)
{
	// build dialog window
	fltk::Window window(500, 340, "Shader properties");
	window.begin();

		if(!name)
			name = new fltk::Input(70,10, 400,23, "Name");
		else
			window.add(name);
		name->tooltip("Shader name");

		if(!description)
			description = new fltk::TextEditor(70,40, 400,223, "Description");
		else
			window.add(description);
		description->tooltip("Shader description");
		description->wrap_mode(true);
		window.resizable(description);

		if(!authors)
			authors = new fltk::Input(70,270, 400,23, "Authors");
		else
			window.add(authors);
		authors->tooltip("Author(s)");

		name->text(Name.c_str());
		description->text(Description.c_str());
		authors->text(Authors.c_str());

		fltk::ReturnButton* rb = new fltk::ReturnButton(300, 310, 70, 25, "OK");
		rb->label("Ok");
		rb->callback(set_sp_answer, 1);

		fltk::Button* cb = new fltk::Button(400, 310, 70, 25, "Cancel");
		cb->label("Cancel");
		cb->callback(set_sp_answer, 0);

	window.end();

	// show it
	window.exec();

	// don't delete the dialogs with the window
	window.remove(name);
	window.remove(description);
	window.remove(authors);

	return sp_answer;
}

}

#endif // _ui_shader_properties_h_

