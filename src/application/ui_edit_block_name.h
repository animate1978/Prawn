
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


#ifndef _ui_edit_block_name_h_
#define _ui_edit_block_name_h_

#include "../shading/scene.h"

#include <fltk/Input.h>
#include <fltk/ReturnButton.h>
#include <fltk/Window.h>

#include <fltk/ask.h>

#include <string>

namespace edit_block_name
{

static fltk::Input* s_name;

class dialog {

private:
	fltk::Window* w;
	services* m_services;
	std::string m_edited_block;

public:
	dialog(services* Services) :
		m_services(Services) {

		// build dialog window
		w = new fltk::Window(400, 80, "Block name");
		w->begin();

			s_name = new fltk::Input(70,10, 300,23, "Name");
			w->add(s_name);
			s_name->tooltip("Edit block name");


			fltk::ReturnButton* rb = new fltk::ReturnButton(150, 40, 75, 25, "OK");
			rb->label("Ok");
			rb->callback(cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button(250, 40, 70, 25, "Cancel");
			cb->label("Cancel");
			cb->callback(cb_cancel, (void*)this);

		w->end();
	}

	~dialog() {

		delete w;
	}

	void edit_name (const std::string& Name) {

		// set values
		s_name->text (Name.c_str());

		// save block name for later use
		m_edited_block = Name;

		// show it
		w->exec();
	}

	void on_ok(fltk::Widget* W) {

		const std::string new_name = s_name->value();

		// save action to do
		bool quit = false;

		if (new_name == m_edited_block) {

			// the name didn't change
			quit = true;
		} else if (new_name.empty()) {

			// can't rename to an empty string
			quit = false;
			fltk::alert ("New name is empty!");
		} else {

			// check whether the name already exists
			const std::string unique_name = m_services->get_unique_block_name (new_name);
			if (unique_name == new_name) {

				// the new name is unique
				quit = true;

				shader_block* block = m_services->get_block (m_edited_block);
				if (block) {
					m_services->set_block_name (block, new_name);
				}
			} else {

				const std::string message = "This name already exists, suggested name: " + unique_name;
				fltk::alert (message.c_str());
			}
		}

		if (quit) {

			W->window()->make_exec_return(false);
		}
	}

	void on_cancel(fltk::Widget* W) {

		W->window()->make_exec_return(false);
	}

	static void cb_ok(fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok(W); }
	static void cb_cancel(fltk::Widget* W, void* Data) { ((dialog*)Data)->on_cancel(W); }
};

}

#endif // _ui_edit_block_name_h_

