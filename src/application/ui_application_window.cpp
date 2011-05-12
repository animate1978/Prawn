
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


#include "ui_application_window.h"
#include "ui_code_preview.h"
#include "ui_shader_properties.h"
#include "ui_preferences.h"
#include "ui_edit_code.h"

#include "../services.h"

#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"
#include "../miscellaneous/misc_system_functions.h"
#include "../miscellaneous/misc_xml.h"

#include <fltk/Color.h>
#include <fltk/Divider.h>
#include <fltk/Input.h>
#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/MenuBar.h>
#include <fltk/ReturnButton.h>
#include <fltk/TextEditor.h>
#include <fltk/ToggleItem.h>

#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/file_chooser.h>
#include <fltk/filename.h>

#include <fltk/DoubleBufferWindow.h>

#include <fstream>
#include <iostream>
using namespace std;

application_window::application_window(services* services_instance, opengl_view* opengl_view_instance) :
	Window (fltk::USEDEFAULT, fltk::USEDEFAULT, 800, 600, "Scene", true),
	m_services (services_instance),
	m_opengl_view (opengl_view_instance),
	m_zoom_slider (80, 575, 400, 19, "Zoom"),
	m_block_menu (20, 22, 90, 24, "Add block")
{
	log() << aspect << "Application window constructor" << std::endl;

	application_pointer = this;

	// set main window's callback
	callback(cb_window);

	// create UI
	log() << aspect << "creating UI" << std::endl;

	begin();

		// main menu bar
		fltk::MenuBar* left_menu_bar = new fltk::MenuBar (0, 0, 800, 20, "Left menu");
		left_menu_bar->begin();

			// file menu
			fltk::ItemGroup* menu_file = new fltk::ItemGroup ("&File");
			menu_file->begin();

				fltk::Item* menu_file_new = new fltk::Item ("New");
				menu_file_new->shortcut (fltk::CTRL + 'n');
				menu_file_new->callback ((fltk::Callback*)cb_menu_file_new, this);

				fltk::Item* menu_file_open = new fltk::Item ("Open");
				menu_file_open->shortcut (fltk::CTRL + 'o');
				menu_file_open->callback ((fltk::Callback*)cb_menu_file_open, this);

				new fltk::Divider();

				fltk::Item* menu_file_save = new fltk::Item ("Save");
				menu_file_save->shortcut (fltk::CTRL + 's');
				menu_file_save->callback ((fltk::Callback*)cb_menu_file_save, this);

				fltk::Item* menu_file_save_as = new fltk::Item ("Save as...");
				menu_file_save_as->shortcut (fltk::SHIFT + fltk::CTRL + 's');
				menu_file_save_as->callback ((fltk::Callback*)cb_menu_file_save_as, this);

				new fltk::Divider();

				fltk::Item* menu_file_shader_properties = new fltk::Item ("Shader Properties");
				menu_file_shader_properties->shortcut (fltk::CTRL + 'p');
				menu_file_shader_properties->callback ((fltk::Callback*)cb_menu_shader_properties, this);

				fltk::Item* menu_file_code_preview = new fltk::Item ("Code Preview");
				menu_file_code_preview->shortcut (fltk::CTRL + 'w');
				menu_file_code_preview->callback ((fltk::Callback*)cb_menu_code_preview, this);

				fltk::Item* menu_file_export_scene = new fltk::Item ("Export Scene");
				menu_file_export_scene->shortcut (fltk::CTRL + 'e');
				menu_file_export_scene->callback ((fltk::Callback*)cb_menu_file_export_scene, this);

				new fltk::Divider();

				fltk::Item* menu_file_options = new fltk::Item ("Options");
				menu_file_options->shortcut (fltk::CTRL + 't');
				menu_file_options->callback ((fltk::Callback*)cb_menu_file_options, this);

				new fltk::Divider();

				fltk::Item* menu_file_quit = new fltk::Item ("Quit");
				menu_file_quit->shortcut (fltk::CTRL + 'q');
				menu_file_quit->callback ((fltk::Callback*)on_menu_file_quit);

			menu_file->end();

			// view edit
			fltk::ItemGroup* menu_edit = new fltk::ItemGroup ("&Edit");
			menu_edit->begin();

				fltk::Item* menu_edit_copy = new fltk::Item ("Copy");
				menu_edit_copy->shortcut (fltk::CTRL + 'c');
				menu_edit_copy->callback ((fltk::Callback*)cb_menu_edit_copy, this);

				fltk::Item* menu_edit_paste = new fltk::Item ("Paste");
				menu_edit_paste->shortcut (fltk::CTRL + 'v');
				menu_edit_paste->callback ((fltk::Callback*)cb_menu_edit_paste, this);

				fltk::Item* menu_edit_cut = new fltk::Item ("Cut");
				menu_edit_cut->shortcut (fltk::CTRL + 'x');
				menu_edit_cut->callback ((fltk::Callback*)cb_menu_edit_cut, this);

				fltk::Item* menu_edit_group = new fltk::Item ("Group");
				menu_edit_group->shortcut ('g');
				menu_edit_group->callback ((fltk::Callback*)cb_menu_edit_group, this);

				fltk::Item* menu_edit_ungroup = new fltk::Item ("Ungroup");
				menu_edit_ungroup->shortcut ('u');
				menu_edit_ungroup->callback ((fltk::Callback*)cb_menu_edit_ungroup, this);

				fltk::Item* menu_edit_edit = new fltk::Item ("Edit block");
				menu_edit_edit->shortcut ('e');
				menu_edit_edit->callback ((fltk::Callback*)cb_menu_edit_edit, this);

				fltk::Item* menu_edit_delete = new fltk::Item ("Delete blocks");
				menu_edit_delete->shortcut (fltk::DeleteKey);
				menu_edit_delete->callback ((fltk::Callback*)cb_menu_edit_delete, this);



			menu_edit->end();

			// view menu
			fltk::ItemGroup* menu_view = new fltk::ItemGroup ("&View");
			menu_view->begin();

				m_menu_show_grid = new fltk::ToggleItem ("Show grid");
				m_menu_show_grid->shortcut (fltk::CTRL + 'g');
				m_menu_show_grid->callback ((fltk::Callback*)cb_menu_view_toggle_grid, this);

				m_menu_snap_to_grid = new fltk::ToggleItem ("Snap blocks");
				m_menu_snap_to_grid->shortcut (fltk::CTRL + 'a');
				m_menu_snap_to_grid->callback ((fltk::Callback*)cb_menu_view_toggle_grid_snap, this);

/*
				m_menu_overview = new fltk::ToggleItem ("Overview");
				m_menu_overview->shortcut (fltk::CTRL + 'v');
				m_menu_overview->callback ((fltk::Callback*)cb_menu_view_toggle_overview, this);
*/

			menu_view->end();

		left_menu_bar->end();

		// help menu
		fltk::MenuBar* right_menu_bar = new fltk::MenuBar (750, 0, 39, 20);
		right_menu_bar->begin();

			fltk::ItemGroup* menu_help = new fltk::ItemGroup ("&Help");
			menu_help->begin();

				fltk::Item* menu_help_help = new fltk::Item ("Help");
				menu_help_help->callback ((fltk::Callback*)on_menu_help_help);

				fltk::Item* menu_help_about = new fltk::Item ("About");
				menu_help_about->callback ((fltk::Callback*)on_menu_help_about);


			menu_help->end();

		right_menu_bar->end();

		// block menu
		m_block_menu.tooltip ("Add a predefined block to the scene");
		m_block_menu.callback ((fltk::Callback*)block_menu_callback);
		m_block_menu.begin();

		ofstream out("./doc/index.xml");
		ofstream outc("./doc/content.xml");

		if(out && outc) {
					out << "<?xml-stylesheet type=\"text/xsl\" href=\"block.xsl\"?>" << endl;
					out << "<!-- Copyright 2009, Romain Behar <romainbehar@users.sourceforge.net>; " <<"\n" <<
							"This file is part of Shrimp 2." <<"\n" <<
							"Shrimp 2 is free software: you can redistribute it and/or modify " <<"\n" <<
							"it under the terms of the GNU General Public License as published by " <<"\n" <<
							"the Free Software Foundation, either version 3 of the License, or " <<"\n" <<
							"(at your option) any later version. " <<"\n" <<
							"Shrimp 2 is distributed in the hope that it will be useful, " <<"\n" <<
							"but WITHOUT ANY WARRANTY; without even the implied warranty of " <<"\n" <<
							"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the " <<"\n" <<
							"GNU General Public License for more details. " <<"\n" <<
							"You should have received a copy of the GNU General Public License " <<"\n" <<
							"along with Shrimp 2.  If not, see <http://www.gnu.org/licenses/>.-->" << endl;
					out << "<block>" << endl;
					outc << "<?xml-stylesheet type=\"text/xsl\" href=\"content.xsl\"?>" << endl;
					outc << "<!-- Copyright 2009, Romain Behar <romainbehar@users.sourceforge.net> " <<"\n" <<
							"This file is part of Shrimp 2." <<"\n" <<
							"Shrimp 2 is free software: you can redistribute it and/or modify " <<"\n" <<
							"it under the terms of the GNU General Public License as published by " <<"\n" <<
							"the Free Software Foundation, either version 3 of the License, or " <<"\n" <<
							"(at your option) any later version. " <<"\n" <<
							"Shrimp 2 is distributed in the hope that it will be useful, " <<"\n" <<
							"but WITHOUT ANY WARRANTY; without even the implied warranty of " <<"\n" <<
							"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the " <<"\n" <<
							"GNU General Public License for more details. " <<"\n" <<
							"You should have received a copy of the GNU General Public License " <<"\n" <<
							"along with Shrimp 2.  If not, see <http://www.gnu.org/licenses/>.-->" << endl;
					outc << "<block>" << endl;
					}

			block_tree_node_t root = m_services->get_block_hierarchy();
			for (block_tree_node_list_t::const_iterator tree_node = root.child_nodes.begin();
				tree_node != root.child_nodes.end(); ++tree_node) {

				build_menu (*tree_node);
				if(!out) {
							cout << "Cannot open file.\n";
							}
						else {
							build_help (*tree_node,out,outc);
						}
			}
			out << "</block>" << endl;
			out.close();
			outc << "</block>" << endl;
			outc.close();
			// add custom block entry
			new fltk::Item ("Custom block");

		m_block_menu.end();

			// help file generation

		// load data from preferences
		general_options prefs;
		prefs.load();
		m_renderers = prefs.get_renderer_list();
		m_scenes = prefs.get_scene_list();

		// renderer chooser
		m_renderer_chooser = new fltk::Choice (250, 22, 100, 24, "Renderer");
		m_renderer_chooser->tooltip ("Choose an installed RenderMan engine");
		set_renderer_chooser_value (prefs.m_renderer_code);

		// display chooser
		m_renderer_display_chooser = new fltk::Choice (410, 22, 100, 24, "Display");
		m_renderer_display_chooser->tooltip ("Select one of the renderer's displays");

		// set preferences values
		on_renderer_choice (this, (void*)prefs.m_renderer_code.c_str());

		// scene chooser
		m_scene_chooser = new fltk::Choice (570, 22, 100, 24, "Scene");
		m_scene_chooser->tooltip ("Choose a 3D scene for preview");
		set_scene_chooser_value (prefs.m_scene);

		// preview button
		fltk::Button* preview_button = new fltk::Button (690, 22, 100, 24, "Render");
		preview_button->callback ((fltk::Callback*)cb_preview, this);
		preview_button->tooltip ("Click to preview the shader using selected renderer and display");



		// zoom slider
		m_zoom_slider.labelfont (fltk::HELVETICA_BOLD);
		m_zoom_slider.labelcolor (fltk::Color(0x7486A200));
		m_zoom_slider.minimum (0);
		m_zoom_slider.maximum (5);
		m_zoom_slider.step (0.02);
		m_zoom_slider.value (3);
		m_zoom_slider.callback ((fltk::Callback*)cb_zoom_slider, this);
		m_zoom_slider.align (fltk::ALIGN_LEFT);
		m_zoom_slider.type (fltk::Slider::TICK_ABOVE);
		m_zoom_slider.tooltip ("Drag to zoom in or out");

		// fit scene button
		fltk::Button* fit_button = new fltk::Button (490, 570, 100, 24, "Fit scene");
		fit_button->callback ((fltk::Callback*)cb_button_fit_scene, this);
		fit_button->tooltip ("Click to fit the block scene to the current view");

		// OpenGL view
				fltk::Group* main_view = new fltk::Group (2, 48, 796, 520);
				main_view->begin();

					fltk::InvisibleBox* frame = new fltk::InvisibleBox (0, 0, 796, 520);
					frame->box (fltk::DOWN_BOX);
					frame->color ((fltk::Color) (56));
					frame->selection_color ((fltk::Color) (69));

					m_scene_view = new scene_view (m_services, m_opengl_view, 2, 2, 792, 518);

				main_view->end();
				resizable (main_view);

	end();
}

application_window::~application_window()
{
	log() << aspect << "ui_application_window: destructor" << std::endl;
	delete m_services;
}


// Main window's callback, prevents closing the application when hitting the Escape key
void application_window::on_window_callback (fltk::Widget*)
{
	if (fltk::event() == fltk::KEY && fltk::event_key() == fltk::EscapeKey)
	{
		// Don't do anything
	}
	else
	{
		// Exit
		//TODO: check whether there's something to save
		exit(0);
	}
}


// File menu : New
void application_window::on_menu_file_new (fltk::Widget*)
{
	m_services->reset_scene();

	// fit scene
	const double new_size = m_scene_view->fit_scene();
	m_zoom_slider.value (new_size);

	// refresh
	m_scene_view->redraw();
}


// File menu : Open
void application_window::on_menu_file_open (fltk::Widget*) {

	// get current directory
	char result[2048];
	fltk::filename_absolute (result, 2048, ".");

	// choose shader file
	const char* file = fltk::file_chooser ("Open Shader", "*.xml", result);
	if (!file)
		return;

	load_scene (file);
}


bool application_window::load_scene (const std::string& File)
{
	if (!m_services->load (File))
	{
		const std::string alert = "Couldn't open '" + File + "'.";
		fltk::alert (alert.c_str());

		return false;
	}

	const double new_size = m_scene_view->fit_scene();

	m_zoom_slider.value (new_size);

	// Set scene name as window's title
	label (m_services->get_scene_name().c_str());

	return true;
}


// File menu : Save
void application_window::on_menu_file_save (fltk::Widget*)
{
	if (!m_services->save()) {
		save_scene_as();
	}
}

// File menu : Save As...
void application_window::on_menu_file_save_as (fltk::Widget*)
{
	save_scene_as();
}

// File menu : Shader Properties
void application_window::on_menu_shader_properties (fltk::Widget*) {

	std::string name (m_services->get_scene_name());
	std::string description (m_services->get_scene_description());
	std::string authors (m_services->get_scene_authors());

	if (shader_properties::sp_dialog (name, description, authors))
	{
		m_services->set_scene_name (shader_properties::name->value());
		m_services->set_scene_description (shader_properties::description->text());
		m_services->set_scene_authors (shader_properties::authors->value());

		// Set scene name as window's title
		label (m_services->get_scene_name().c_str());
	}
}

// File menu : Code Preview...
void application_window::on_menu_code_preview (fltk::Widget*) {

	std::string surface_code = m_services->show_code();

	code_preview::dialog d;
	d.open (surface_code);
}

// File menu : Render Options...
void application_window::on_menu_file_options (fltk::Widget*) {

	preferences::dialog d;
	d.pref_dialog();

	// update the renderer, display and scene choosers according to the preferences
	general_options prefs;
	prefs.load();
	const std::string renderer_code = prefs.m_renderer_code;
	const std::string display_name = prefs.m_renderer_display;
	const std::string scene = prefs.m_scene;

	set_renderer_chooser_value (renderer_code);
	set_display_chooser_value (renderer_code, display_name);
	set_scene_chooser_value (scene);
}


// File menu : Export Scene...
void application_window::on_menu_file_export_scene (fltk::Widget*) {

	// choose directory
	char result[2048];
	fltk::filename_absolute (result, 2048, ".");
	const char* directory = fltk::dir_chooser ("Export Scene To", result);
	if (!directory)
		return;

	// export scene
	log() << aspect << "exporting scene to : " << directory << std::endl;
	m_services->export_scene (directory);
}

// File menu : Quit
void application_window::on_menu_file_quit (fltk::Widget*, void*)
{
	exit (0);
}

//Edit menu : Copy selection
void application_window::on_menu_edit_copy (fltk::Widget*)
{
	m_services->copy_selected_blocks(m_opengl_view->get_active_block());
}

//Edit menu : Paste selection
void application_window::on_menu_edit_paste (fltk::Widget*)
{
	m_services->paste(m_opengl_view->get_active_block());

	// refresh view
	m_scene_view->redraw();
}

//Edit menu : Cut selection
void application_window::on_menu_edit_cut (fltk::Widget*)
{
	if (m_services->selection_size() >= 1)
	{
		m_services->cut_selection(m_opengl_view->get_active_block());

		// refresh
		m_scene_view->redraw();
	}
}

//Edit menu : Group selection
void application_window::on_menu_edit_group (fltk::Widget*)
{
	if (m_services->selection_size() >= 1)
	{
		m_services->group_selection();

		// refresh
		m_scene_view->redraw();
	}
}

//Edit menu : Ungroup selection
void application_window::on_menu_edit_ungroup (fltk::Widget*)
{
	const int m_select_group = m_opengl_view->get_selected_group();
	if (m_select_group)
	{
		m_services->ungroup(m_select_group);

		// refresh
		m_scene_view->redraw();
	}
}

//Edit menu : Delete selection
void application_window::on_menu_edit_delete (fltk::Widget*)
{
	m_services->delete_selection();

	// refresh
	m_scene_view->redraw();
}

//Edit menu : Edit source of selection
void application_window::on_menu_edit_edit (fltk::Widget*)
{
	if (m_services->selection_size() == 1)
	{
		shader_block* block = m_opengl_view->get_active_block();
		if (block)
		{
			edit_code::dialog d;
			d.open_dialog (block);

			// toggle block selection
			m_services->clear_selection();
			m_services->set_block_selection (block, !m_services->is_selected (block));
		}
	}
}

void application_window::on_menu_view_toggle_grid (fltk::Widget*)
{
	const bool grid_state = m_menu_show_grid->state();
	m_opengl_view->set_grid_state (grid_state);

	m_scene_view->redraw();
}

void application_window::on_menu_view_toggle_grid_snap (fltk::Widget*)
{
	const bool snap_to_grid_state = m_menu_snap_to_grid->state();
	m_opengl_view->set_snap_to_grid_state (snap_to_grid_state);

	m_scene_view->redraw();
}

void application_window::on_menu_view_toggle_overview (fltk::Widget*)
{
	const bool overview_state = m_menu_overview->state();
	m_opengl_view->set_overview_state (overview_state);

	m_scene_view->redraw();
}

// Help menu : About
#include "ui_about.h"
#include <fltk/Monitor.h>

void application_window::on_menu_help_about (fltk::Widget*, void*) {

	fltk::Window* o = about_window();
	o->show();
}

void application_window::on_menu_help_help (fltk::Widget*, void*) {

// Open index.html file
	general_options prefs;
	prefs.load();
	std::string help_reader = prefs.m_help_reader;

//	system("firefox -url \"./doc/index.html\"&");
	system(help_reader.c_str());
}

void application_window::on_zoom (fltk::Slider* o, void*) {

	m_opengl_view->set_size ((double)o->value());
	m_scene_view->redraw();
}

void application_window::on_button_fit_scene (fltk::Widget*) {

	const double new_size = m_scene_view->fit_scene();

	m_zoom_slider.value (new_size);
}


void application_window::on_custom_block() {

	// create a custom block
	shader_block* new_block = m_services->add_custom_block();

	// put it in the middle of the view
	m_opengl_view->move_block_to_view_center (new_block);

	// refresh
	m_scene_view->redraw();
}


// Shader preview
void application_window::on_preview()
{
	std::string tempdir = system_functions::get_tmp_directory();
	m_services->show_preview (tempdir);
}


void application_window::on_renderer_choice (fltk::Widget* W, void* Data) {

	const std::string renderer_name ((const char*)Data);

	general_options::renderers_t::const_iterator r = m_renderers.find (renderer_name);
	if (r == m_renderers.end()) {
		log() << error << "unknown renderer: '" << renderer_name << "'" << std::endl;
		return;
	}

	// load the preferences and the display name
	general_options prefs;
	prefs.load();
	std::string display_name = prefs.m_renderer_display;

	display_name = set_display_chooser_value (renderer_name, display_name);

	// save the renderer parameters
	prefs.set_renderer (renderer_name);
	prefs.set_display (display_name);
	prefs.save();

	// refresh
	redraw();
}


void application_window::on_renderer_display_choice (fltk::Widget* W, void* Data) {

	const std::string display_name ((const char*)Data);

	// save the display value
	general_options prefs;
	prefs.load();
	prefs.set_display (display_name);
	prefs.save();
}


void application_window::on_scene_choice (fltk::Widget* W, void* Data) {

	const std::string scene_name ((const char*)Data);

	bool scene_found = false;
	for (general_options::scenes_t::const_iterator s = m_scenes.begin(); s != m_scenes.end(); ++s) {
		if (s->name == scene_name) {
			scene_found = true;
			break;
		}
	}
	if (!scene_found) {
		log() << error << "unknown scene: '" << scene_name << "'" << std::endl;
		return;
	}

	// save the scene value
	general_options prefs;
	prefs.load();
	prefs.set_scene (scene_name);
	prefs.save();
}


void application_window::build_menu (const block_tree_node_t& tree_node) {

	fltk::ItemGroup* current_group = new fltk::ItemGroup (tree_node.node_name.c_str());
	current_group->begin();

	// check whether the directory has children
	for (block_tree_node_list_t::const_iterator sub_node = tree_node.child_nodes.begin();
		sub_node != tree_node.child_nodes.end(); ++sub_node) {

		build_menu (*sub_node);
	}

	for (default_block_list_t::const_iterator block = tree_node.blocks.begin(); block != tree_node.blocks.end(); ++block) {
		new fltk::Item (block->name.c_str());
	}

	current_group->end();
}

void application_window::build_help (const block_tree_node_t& tree_node,ofstream& out,ofstream& outc) {


	string line;
	string blockname;
	//fltk::ItemGroup* current_group = new fltk::ItemGroup (tree_node.node_name.c_str());
	//current_group->begin();

	// check whether the directory has children
	for (block_tree_node_list_t::const_iterator sub_node = tree_node.child_nodes.begin();
		sub_node != tree_node.child_nodes.end(); ++sub_node) {

		build_help (*sub_node,out,outc);
	}

	for (default_block_list_t::const_iterator block = tree_node.blocks.begin(); block != tree_node.blocks.end(); ++block) {
		//new fltk::Item (block->name.c_str());
		//create index.xml file
		ifstream in (block->path.c_str());
		blockname = block->name.c_str();
		if (in.is_open()){
			while (! in.eof() )
			    {
			      getline (in,line);
			      out << line << endl;

			    }
				outc << "<shrimp name=\""<< blockname <<"\""<<" path="<<"\"." << block->path.c_str() <<"\"></shrimp>" << endl;
			    in.close();
			  }
		else cout << "Unable to open file";
		//out << "<block name="<<"\""<< block->name.c_str() <<"\"" <<" path="<<"\"." << block->path.c_str() <<"\""<<">" <<"</block>" << endl;



	}
	//current_group->end();

}

void application_window::block_menu_action (fltk::Widget* w, void*) {

	fltk::PopupMenu* menu = (fltk::PopupMenu*)w;
	fltk::Widget* item = menu->get_item();
	if (!item)
		log() << error << "invalid menu item (block_menu_action)." << std::endl;
	else {
		// add block
		shader_block* new_block = m_services->add_predefined_block (item->label());
		// put it in the view center
		m_opengl_view->move_block_to_view_center (new_block);

		// refresh
		m_scene_view->redraw();
	}
}

void application_window::set_renderer_chooser_value (const std::string RendererCode) {

	unsigned long renderer_menu_number = 0;
	unsigned long current_menu_number = 0;
	m_renderer_chooser->begin();
	for (general_options::renderers_t::iterator r_i = m_renderers.begin(); r_i != m_renderers.end(); ++r_i, ++current_menu_number) {
		if (r_i->first == _3delight) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)_3delight);
			if (RendererCode == _3delight)
				renderer_menu_number = current_menu_number;
		}
		else if (r_i->first == air) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)air);
			if (RendererCode == air)
				renderer_menu_number = current_menu_number;
		}
		else if (r_i->first == aqsis) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)aqsis);
			if (RendererCode == aqsis)
				renderer_menu_number = current_menu_number;
		}
		else if (r_i->first == entropy) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)entropy);
			if (RendererCode == entropy)
				renderer_menu_number = current_menu_number;
		}
		else if (r_i->first == pixie) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)pixie);
			if (RendererCode == pixie)
				renderer_menu_number = current_menu_number;
		}
		else if (r_i->first == prman) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)prman);
			if (RendererCode == prman)
				renderer_menu_number = current_menu_number;
		}
		else if (r_i->first == renderdotc) {
			new fltk::Item (r_i->second.name.c_str(), 0, cb_renderer, (void*)renderdotc);
			if (RendererCode == renderdotc)
				renderer_menu_number = current_menu_number;
		}
		else
			log() << error << "unknown renderer: " << r_i->second.name << std::endl;
	}
	m_renderer_chooser->end();

	// set preferences value
	m_renderer_chooser->value (renderer_menu_number);
}


std::string application_window::set_display_chooser_value (const std::string RendererName, const std::string DisplayName) {

	unsigned long display_number = 0;
	bool found = false;
	std::string new_display = "";

	general_options::renderers_t::const_iterator r = m_renderers.find (RendererName);

	m_renderer_display_chooser->remove_all();
	m_renderer_display_chooser->begin();
	unsigned long current_display_number = 0;
	for (general_options::displays_t::const_iterator current_display = r->second.displays.begin(); current_display != r->second.displays.end(); ++current_display, ++current_display_number) {
		new fltk::Item (current_display->c_str(), 0, cb_renderer_display, (void*)current_display->c_str());

		if (*current_display == DisplayName) {
			display_number = current_display_number;
			found = true;
		}
	}
	m_renderer_display_chooser->end();

	// set value
	if (found) {
		m_renderer_display_chooser->value (display_number);

	} else if (r->second.displays.size() > 0) {
		// return first display in the list, because the given one isn't supported
		m_renderer_display_chooser->value (0);
		new_display = *r->second.displays.begin();
	}

	return new_display;
}


void application_window::set_scene_chooser_value (const std::string Scene) {

	unsigned long scene_menu_number = 0;
	unsigned long current_menu_number = 0;
	bool found = false;
	m_scene_chooser->remove_all();
	m_scene_chooser->begin();
	for (general_options::scenes_t::iterator s_i = m_scenes.begin(); s_i != m_scenes.end(); ++s_i, ++current_menu_number) {
		new fltk::Item (s_i->name.c_str(), 0, cb_scene, (void*)s_i->name.c_str());
		if (Scene == s_i->name){
			scene_menu_number = current_menu_number;
			found = true;
		}
	}
	m_scene_chooser->end();

	// set preferences value
	if (found) {m_scene_chooser->value (scene_menu_number);}
}


int application_window::handle (int event) {

	// update the zoom slider if necessary
	m_zoom_slider.value (m_opengl_view->get_size());

	// do overriden function's work
	return fltk::Window::handle (event);
}


void application_window::save_scene_as() {

	// get current directory
	char result[2048];
	fltk::filename_absolute (result, 2048, ".");

	// choose shader file
	const char* file = fltk::file_chooser ("Save Shader As", "Shrimp file (*.xml)", result);
	if (!file)
		return;

	// automatically add the .xml extension
	std::string real_file_name = file;
	const char* extension = fltk::filename_ext (file);
	if (std::string (extension) != ".xml")
		real_file_name += ".xml";

	// check whether file already exists
	if (std::ifstream (real_file_name.c_str())) {

		const std::string message = "Do you want to overwrite '" + std::string (fltk::filename_name (real_file_name.c_str())) + "'?";
		if (!fltk::ask (message.c_str())) {

			return;
		}
	}

	log() << aspect << "saving project as " << file << std::endl;
	m_services->save_as (real_file_name);
}


