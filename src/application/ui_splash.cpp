
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


#include "ui_splash.h"

#include <fltk/DoubleBufferWindow.h>
#include <fltk/SharedImage.h>
#include <fltk/Monitor.h>
#include <fltk/run.h>


fltk::Window* splash_screen() {

	fltk::DoubleBufferWindow* o = new fltk::DoubleBufferWindow(550, 400, "splash screen");
	o->type(241);
	o->color((fltk::Color)97);
	o->selection_color((fltk::Color)47);
	o->begin();
		fltk::Group* g = new fltk::Group(0, 0, 550, 400);
		g->image(fltk::jpegImage::get("data/splash.jpg"));
		g->resizable(o);
	o->end();
	o->set_non_modal();
	o->clear_border();
	o->resizable();

	o->border(false);
	o->Rectangle::set(fltk::Monitor::find(0,0),o->w(),o->h(),fltk::ALIGN_CENTER);
	o->show();
	o->flush();

	do {
		fltk::check();

	} while (!o->visible());

	return o;
}


