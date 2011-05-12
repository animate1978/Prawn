
/*
    Copyright 2011, Ted Gocek, Romain Behar <romainbehar@users.sourceforge.net>

    This file is part of Prawn 2.1.1a.

    Prawn is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Prawn is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Prawn.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "ui_about.h"

#include <fltk/DoubleBufferWindow.h>
#include <fltk/ReturnButton.h>
#include <fltk/SharedImage.h>
#include <fltk/Monitor.h>


fltk::DoubleBufferWindow* about_panel = (fltk::DoubleBufferWindow*)0;

static void cb_about_ok(fltk::ReturnButton*, void*) {

	about_panel->hide();
	delete about_panel;
}

fltk::Window* about_window() {

	fltk::DoubleBufferWindow* o = about_panel = new fltk::DoubleBufferWindow(550, 400, "About Prawn");
	o->type(241);
	o->color((fltk::Color)97);
	o->selection_color((fltk::Color)47);
	o->begin();
		fltk::Group* g = new fltk::Group(0, 0, 550, 400);
		g->image(fltk::jpegImage::get("data/about.jpg"));
		g->begin();
			fltk::ReturnButton* about_ok = new fltk::ReturnButton(483, 364, 50, 16, "OK");
			about_ok->buttoncolor((fltk::Color)0x4c519000);
			about_ok->labelcolor((fltk::Color)0xffffff00);
			about_ok->highlight_color((fltk::Color)0x6a4fe800);
			about_ok->shortcut(0xff0d);
			about_ok->callback((fltk::Callback*)cb_about_ok);
		g->end();
		g->resizable(o);
	o->end();
	o->set_non_modal();
	o->clear_border();
	o->resizable();

	o->border(true);
	o->Rectangle::set(fltk::Monitor::find(0,0),o->w(),o->h(),fltk::ALIGN_CENTER);
	o->show();

	return about_panel;
}


