
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

#include <memory>

#include "ui_splash.h"
#include "ui_application_window.h"

#include "../shading/preferences.h"
#include "../miscellaneous/logging.h"

#include <fltk/Style.h>
#include <fltk/run.h>
#include <fltk/visual.h>
#include <fltk/Tooltip.h>
#include <fltk/LightButton.h>


// Launch FLTK and Prawn's main window
int main(int argc, char** argv) {

	log_level_t level = ERROR;
	std::auto_ptr<std::streambuf> filter_level (new filter_by_level_buf (level, log()));

/*
	log() << error << "LOG = ERROR" << std::endl;
	log() << warning << "LOG = WARNING" << std::endl;
	log() << info << "LOG = INFO" << std::endl;
	log() << debug << "LOG = DEBUG" << std::endl;
	log() << aspect << "LOG = ASPECT" << std::endl;
*/

	log() << aspect << "Starting Prawn v2.1.1a" << std::endl;

	// theme colours
	/* color, textcolor, selection_color, buttoncolor, highlight_color */
	/* highlight_textcolor, labelcolor, selection_textcolor */
	fltk::Color bg_colour (0x44444400);
	fltk::Color bg_darkcolour (0x33333300);
	fltk::Color text_colour (0xFFFFFF00);
//	fltk::Color text_lightcolour (0xB9D3EE00);
	fltk::Color selection_colour (0x3A5FCD00);
	fltk::Color hover_colour(0xFF8C0000);
	// Window
	fltk::Window::default_style->color (bg_colour);
	fltk::Window::default_style->textcolor (text_colour);
	// Menu
	fltk::Menu::default_style->color (bg_colour);
	fltk::Menu::default_style->textcolor (text_colour);
	fltk::Menu::default_style->selection_color (selection_colour);
	fltk::Menu::default_style->buttoncolor (bg_colour);
	fltk::Menu::default_style->highlight_color (selection_colour);
	fltk::Menu::default_style->labelcolor (text_colour);
	// Button
	fltk::Button::default_style->color (bg_colour);
	fltk::Button::default_style->textcolor (text_colour);
	fltk::Button::default_style->labelcolor (text_colour);
	fltk::Button::default_style->buttoncolor (bg_colour);
	fltk::Button::default_style->highlight_color(hover_colour);
	// Widget
	fltk::Widget::default_style->color (bg_colour);
	fltk::Widget::default_style->textcolor (text_colour);
	fltk::Widget::default_style->selection_color (selection_colour);
	fltk::Widget::default_style->buttoncolor (bg_colour);
	fltk::Widget::default_style->highlight_color (hover_colour);
	fltk::Widget::default_style->labelcolor (text_colour);
	fltk::Widget::default_style->box (fltk::BORDER_BOX);
	fltk::Widget::default_style->buttonbox (fltk::BORDER_BOX);
	// PopupMenu
	fltk::PopupMenu::default_style->color (bg_colour);
	fltk::PopupMenu::default_style->textcolor (text_colour);
	// Slider
	fltk::Slider::default_style->color (bg_colour);
	fltk::Slider::default_style->textcolor (text_colour);
//	fltk::Slider::default_style->textsize (10);
	// Group
	fltk::Group::default_style->color (bg_colour);
	fltk::Group::default_style->textcolor (text_colour);
	fltk::Group::default_style->box (fltk::BORDER_BOX);
	fltk::Group::default_style->buttonbox (fltk::BORDER_BOX);
	// File preview
	fltk::InvisibleBox::default_style->color (bg_darkcolour);
	fltk::InvisibleBox::default_style->textcolor (text_colour);
	fltk::InvisibleBox::default_style->box (fltk::BORDER_BOX);
	fltk::InvisibleBox::default_style->buttonbox (fltk::BORDER_BOX);
	// Tooltip
	fltk::Tooltip::default_style->color (selection_colour);
	fltk::Tooltip::default_style->textcolor (text_colour);
	// LightButton
	fltk::LightButton::default_style->color (bg_colour);
	fltk::LightButton::default_style->textcolor (text_colour);
	fltk::LightButton::default_style->selection_color (hover_colour);

	// check preferences for splash screen
	general_options prefs;
	prefs.load();
	const bool show_splash = prefs.m_splash_screen;

	// open splash screen
	fltk::Window* splash_window = 0;
	double splash_start = fltk::get_time_secs();
	if (show_splash) {
		splash_window = splash_screen();
	}

	// double-buffered screen
	fltk::visual(fltk::DOUBLE_BUFFER);

	// create services
	services* services_instance = new services();
	opengl_view* opengl_view_instance = new opengl_view (services_instance);

	// create main
	log() << aspect << "prawn: creating main window" << std::endl;
	application_window* application = new application_window(services_instance, opengl_view_instance);

	// if a scene name was given as a command-line parameter, try to load it
	int nargs = 0;
	if (argc > 1) {
		std::string file = argv[1];
		log() << aspect << "prawn: trying to load scene from '" << argv[1] << "'" << std::endl;

		++nargs;
		application->load_scene (file);
	}

	// show the window (minus Prawn's parameters)
	if (nargs > 0) {
		argv[nargs] = argv[0];
	}
	application->show (argc - nargs, &argv[nargs]);

	if (splash_window) {

		log() << aspect << "prawn: showing splash screen" << std::endl;

		// keep splash screen on top
		splash_window->show();

		// hide splash screen after 3 seconds
		// (if still visible, that is escape key hasn't been pressed)
		while(splash_window->visible() && fltk::get_time_secs() - splash_start < 3.0)
			fltk::check();

		splash_window->hide();
		delete splash_window;
	}

	// FLTK's event loop
	return fltk::run();
}

