
/*
    Copyright 2008-2010, Romain Behar <romainbehar@users.sourceforge.net>

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


#ifndef _ui_edit_input_h_
#define _ui_edit_input_h_

#include "../miscellaneous/logging.h"

#include "tiffImage.h"

#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/Choice.h>
#include <fltk/ColorChooser.h>
#include <fltk/Group.h>
#include <fltk/Input.h>
#include <fltk/Output.h>
#include <fltk/ReturnButton.h>
#include <fltk/ValueInput.h>
#include <fltk/Window.h>
#include <fltk/file_chooser.h>
#include <fltk/FileChooser.h>
#include <fltk/filename.h>
#include <fltk/Image.h>
#include <fltk/SharedImage.h>
#include <fltk/pnmImage.h>
#include <tiffio.h>

#include <iostream>
#include <string>

namespace edit_input
{

static fltk::Output* s_name = 0;
static fltk::Input* s_value = 0;
static fltk::Button* s_colour_button = 0;
static fltk::Button* s_file_button = 0;
static fltk::Choice* s_storage = 0;
static fltk::Choice* s_type = 0;
static fltk::Choice* s_array_type = 0;
static fltk::ValueInput* s_array_size = 0;
static fltk::CheckButton* s_shader_parameter = 0;

static fltk::SharedImage* tiff_check(const char *, uchar *, int);
static fltk::SharedImage* preview;


static void cb_colour_chooser (fltk::Widget *w, void *v)
{
	uchar r = 0, g = 0, b = 0;
	//fltk::split_color(current_color, r, g, b);

	if (!fltk::color_chooser("New color:", r,g,b))
		return;

	// set new colour
	std::ostringstream new_colour;
	new_colour << ((float)r/255.0) << " " << ((float)g/255.0) << " " << ((float)b/255.0);
	s_value->text (new_colour.str().c_str());

	// redraw parent box
	w->parent()->redraw();
}

// image preview in the file dialog
static int readTiff(const char* filename, int& rw, int& rh, unsigned char* &rbits)
{
	// we turn off Warnings too many specific tags that confuse libtiff
	TIFFErrorHandler warn = TIFFSetWarningHandler(0);

	TIFF* tiff = TIFFOpen(filename, "r");

	// turn warnings back
	TIFFSetWarningHandler(warn);

	if (tiff)
	{
		int rc = 1;			// what to return
		uint32 w, h;
		size_t npixels;
		uint32* raster;

		TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &h);

		// notice that despite the fact that this always says orient=1 (TOPLEFT)
		// we still get upsidedown images!
		uint16 orient;
		TIFFGetField(tiff, TIFFTAG_ORIENTATION, &orient);    // set the origin of the image.


		// in case my data structures are different
		rw = w;
		rh = h;

		npixels = w * h;
		raster = (uint32*) _TIFFmalloc(npixels * sizeof (uint32));
		if (raster != NULL)
		{
			//Force the good orientation
			if (TIFFReadRGBAImageOriented(tiff, w, h, raster, 0,0))
			{
				rbits = (unsigned char*) raster;
			}
			else
			{
				rc=0;
				_TIFFfree(raster);
			}
		}

		TIFFClose(tiff);
		return rc;
	} else
		return 0;
}


fltk::SharedImage* tiff_check(const char* name, uchar* header, int headerlen)
{
	if (memcmp(header, "II*", 3) != 0)
		return 0;

	fltk::SharedImage* tiffImage = new fltk::tiffImage(name);
	if (!name || !tiffImage)
	{
		fltk::alert("Can't open TIFF file!");
		return 0;
	}

	return tiffImage;
/*
	// read the image
	int w, h;
	unsigned char* image;
	if (!name || !readTiff(name, w, h, image))
	{
		fltk::alert("Can't open TIFF file!");
		return 0;
	}

	if (preview)
	{
		preview = (fltk::SharedImage*)(new fltk::Image((const uchar*)image,fltk::RGBA,w,h));
		preview->setsize(w, h);
	}
	else
	{
		preview = (fltk::SharedImage*)(new fltk::Image((const uchar*)image,fltk::RGBA,w,h));
	}

	if (!preview)
	{
		fltk::alert("No SharedImage possible for '%s'\n",name);
		return fltk::SharedImage::get("?");
	}

	return preview;
*/
}


// File chooser callback, to select TIFF or TDL files
static void cb_file_chooser (fltk::Widget *w, void *v)
{
	// get current directory
	char result[2048];
	fltk::filename_absolute (result, 2048, ".");

	// image preview in the file dialog
	fltk::SharedImage::set_cache_size (4096);
	fltk::SharedImage::add_handler (tiff_check);

	// choose shader file
	const char* file = fltk::file_chooser ("Open Texture", "*.{tif,tiff}\t*.{tdl,tex,tx}", result);
	if (!file)
	{
		// cancel
		return;
	}
	// save file name
	std::ostringstream new_file;
	new_file << "\"" << file << "\"";
	s_value->text (new_file.str().c_str());


	// crashes when selecting a texture file using the new file dialog in block attribute
	//preview->remove();
}


class dialog
{
private:
	fltk::Window* w;
	shader_block* m_shader_block;
	std::string m_edited_input;
	storages_t m_storage_types;
	types_t m_types;
	types_t m_array_types;

public:
	dialog (shader_block* Block) :
		m_shader_block (Block)
	{
		// build dialog window
		w = new fltk::Window(400, 160, "Edit input");
		w->begin();

			// name
			s_name = new fltk::Output (70,5, 120,23, "Name");
			w->add (s_name);
			s_name->tooltip ("Edited output name");

			// value edition
			s_value = new fltk::Input (70,32, 120,23, "Value");
			w->add (s_value);
			s_value->tooltip ("Input value");

			s_file_button = new fltk::Button (300,32, 60,23, "File");
			s_file_button->callback (cb_file_chooser);
			w->add (s_file_button);
			s_file_button->tooltip ("Texture file chooser");

			s_colour_button = new fltk::Button (200,32, 60,23, "Colour");
			s_colour_button->callback (cb_colour_chooser);
			w->add (s_colour_button);
			s_colour_button->tooltip ("Colour chooser");

			// storage edition
			s_storage = new fltk::Choice (90,60, 90,23, "Type");
			s_storage->begin();
				// make sure the list isn't destroyed before the dialog closes
				m_storage_types = get_property_storage_types();
				for (storages_t::const_iterator st_i = m_storage_types.begin(); st_i != m_storage_types.end(); ++st_i)
				{
					new fltk::Item (st_i->c_str());
				}
			s_storage->end();
			w->add (s_storage);
			s_storage->tooltip ("Input variable storage type");

			// type edition
			s_type = new fltk::Choice (180,60, 100,23, "");
			s_type->begin();
				// make sure the list isn't destroyed before the dialog closes
				m_types = get_property_types();
				for (types_t::const_iterator t_i = m_types.begin(); t_i != m_types.end(); ++t_i)
				{
					new fltk::Item (t_i->c_str());
				}
			s_type->end();
			s_type->callback (cb_type_change);
			s_type->when(fltk::WHEN_CHANGED);
			w->add (s_type);
			s_type->tooltip ("Input variable type");

			// array type edition
			s_array_type = new fltk::Choice (290,60, 100,23, "");
			s_array_type->begin();
				m_array_types = get_array_types();
				for (types_t::const_iterator t_i = m_array_types.begin(); t_i != m_array_types.end(); ++t_i)
				{
					new fltk::Item (t_i->c_str());
				}
			s_array_type->end();
			w->add (s_array_type);
			s_array_type->tooltip ("Array type");
			s_array_type->deactivate();

			// array size
			s_array_size = new fltk::ValueInput (290,85, 50,23, "");
			s_array_size->minimum (0);
			s_array_size->maximum (1E6);
			s_array_size->step (1);
			s_array_size->value (1);
			s_array_size->tooltip ("Array size");
			s_array_size->deactivate();

			// shader parameter checkbox
			s_shader_parameter = new fltk::CheckButton (70,86, 120,23, "Shader parameter");
			w->add (s_shader_parameter);
			s_shader_parameter->tooltip ("Make the input a shader parameter");


			// OK / Cancel
			fltk::ReturnButton* rb = new fltk::ReturnButton(180,125, 70,25, "OK");
			rb->label("Ok");
			rb->callback (cb_ok, (void*)this);

			fltk::Button* cb = new fltk::Button(280,125, 70,25, "Cancel");
			cb->label("Cancel");
			cb->callback (cb_cancel, (void*)this);

		w->end();
	}

	~dialog() {

		delete w;
	}

	void edit_input (const std::string& InputName) {

		m_edited_input = InputName;

		// set name
		s_name->text (InputName.c_str());

		// set value
		const std::string value = m_shader_block->get_input_value (m_edited_input);
		s_value->text (value.c_str());

		// set storage
		const std::string storage = m_shader_block->get_input_storage (m_edited_input);
		int storage_type_number = 0;
		for (storages_t::const_iterator s_i = m_storage_types.begin(); s_i != m_storage_types.end(); ++s_i, ++storage_type_number) {

			if (storage == *s_i)
				s_storage->value (storage_type_number);
		}

		// set type
		const std::string type = m_shader_block->get_input_type (m_edited_input);
		int type_number = 0;
		for (types_t::const_iterator t_i = m_types.begin(); t_i != m_types.end(); ++t_i, ++type_number) {

			if (type == *t_i) {
				s_type->value (type_number);
				type_change();
			}
		}

		// set array
		if (type == "array") {
			const std::string array_type = m_shader_block->get_input_type_extension (m_edited_input);
			int type_number = 0;
			for (types_t::const_iterator t_i = m_array_types.begin(); t_i != m_array_types.end(); ++t_i, ++type_number) {

				if (array_type == *t_i) {
					s_array_type->value (type_number);
				}
			}

			const int array_size = m_shader_block->get_input_type_extension_size (m_edited_input);
			s_array_size->value (array_size);
		}

		// set shader parameter state
		s_shader_parameter->value (m_shader_block->is_shader_parameter (m_edited_input));

		// show the dialog
		w->exec();
	}

	void on_type_change (fltk::Widget* W) {

		type_change();
	}

	void on_ok (fltk::Widget* W) {

		// save value
		const std::string new_value = s_value->value();
		m_shader_block->set_input_value (m_edited_input, new_value);

		// save storage
		types_t storage_list = get_property_storage_types();
		const unsigned int storage_type_number = s_storage->value();
		if (storage_type_number >= 0 && storage_type_number < storage_list.size()) {

			m_shader_block->set_input_storage (m_edited_input, storage_list[storage_type_number]);
		}

		// save type
		types_t list = get_property_types();
		const unsigned int type_number = s_type->value();
		if (type_number >= 0 && type_number < list.size()) {

			m_shader_block->set_input_type (m_edited_input, list[type_number]);
		}

		// save type extension
		const std::string type_extension = m_array_types[s_array_type->value()] + ":" + string_cast(s_array_size->value());
		m_shader_block->set_input_type_extension (m_edited_input, type_extension);

		// save parameter state
		const bool shader_parameter = s_shader_parameter->value();
		m_shader_block->set_shader_parameter (m_edited_input, shader_parameter);

		// close the dialog
		W->window()->make_exec_return (false);
	}

	void on_cancel (fltk::Widget* W) {

		W->window()->make_exec_return (false);
	}

	static void cb_type_change (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_type_change (W); }
	static void cb_ok (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_ok (W); }
	static void cb_cancel (fltk::Widget* W, void* Data) { ((dialog*)Data)->on_cancel (W); }

private:
	void type_change () {

		types_t list = get_property_types();
		const unsigned int type_number = s_type->value();
		if (type_number >= 0 && type_number < list.size()) {

			const std::string current_type = list[type_number];
			if (current_type == "color"){
				s_colour_button->activate();
				s_file_button->deactivate();
			}
			else if (current_type == "string"){
				s_file_button->activate();
				s_colour_button->deactivate();
			}
			else{
				s_colour_button->deactivate();
				s_file_button->deactivate();
			}
			if (current_type == "array") {
				s_array_type->activate();
				s_array_size->activate();
			} else {
				s_array_type->deactivate();
				s_array_size->deactivate();
			}
		}
	}
};

}

#endif // _ui_edit_input_h_

