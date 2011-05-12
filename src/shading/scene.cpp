
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


#include "scene.h"
#include "rib_root_block.h"

#include "preferences.h"

#include <fltk/filename.h>

#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"
#include "../miscellaneous/misc_xml.h"

#include <fstream>
#include <iostream>


scene::scene() :
	m_file_name(std::string(""))
{
	unsigned long successful_blocks = 0;

	block_tree_node_t root_node;
	root_node.node_name = "blocks";
	root_node.node_path = "./blocks";
	m_block_classification = root_node;

	log() << aspect << "Loading default Prawn blocks." << std::endl;
	load_default_blocks (m_block_classification, successful_blocks);
	log() << aspect << "Successfully loaded " << successful_blocks << " blocks." << std::endl;

	new_scene();
}


scene::~scene() {

}


const std::string scene::name() const {

	return m_name;
}

void scene::set_name (const std::string& Name) {

	m_name = Name;
}

const std::string scene::description() const {

	return m_description;
}

void scene::set_description (const std::string& Description) {

	m_description = Description;
}

const std::string scene::authors() const {

	return m_authors;
}

void scene::set_authors (const std::string& Authors) {

	m_authors = Authors;
}

void scene::empty_scene() {

	log() << aspect << "Emptying current scene." << std::endl;

	// delete scene information
	m_name = "";
	m_description = "";
	m_authors = "";
	m_file_name = "";

	// delete scene structure
	m_dag.clear();
	m_groups.clear();
	m_group_names.clear();

	// delete blocks
	for (shader_blocks_t::iterator block_i = m_blocks.begin(); block_i != m_blocks.end(); ++block_i) {

		delete block_i->second;
	}
	//delete m_rib_root_block;

	m_blocks.clear();
}


void scene::new_scene() {

	log() << aspect << "New scene." << std::endl;

	empty_scene();

	// create default shader and add output block
	m_name = "new_scene";
	m_authors = "";

	// create RIB root block
	const std::string unique_name = get_unique_block_name ("Root block");
	m_rib_root_block = new rib_root_block (unique_name, this);
	m_blocks.insert (std::make_pair (unique_name, m_rib_root_block));
}


void scene::load_default_blocks (block_tree_node_t& RootNode, unsigned long& BlockCount) {

	// read directory content
	dirent** block_files;
	const int file_count = fltk::filename_list (RootNode.node_path.c_str(), &block_files);

	// skip empty directories
	if (file_count < 0) {
		log() << error << "tried to load blocks from empty directory '" << RootNode.node_path << "'." << std::endl;
		return;
	}

	// scan directory
	typedef std::vector<std::string> names_t;
	names_t file_paths;
	names_t files;
	names_t block_paths;

	shader_block_builder builder;
	int successful_block_count = 0;
	for (int f = 0; f < file_count; ++f) {

		const std::string file = std::string (block_files[f]->d_name);
		const std::string file_path = RootNode.node_path + "/" + file;
		if (fltk::filename_isdir (file_path.c_str())) {

			if (file[0] == '.') {
				// skip default directories
			}
			else {
				// save directory
				file_paths.push_back (file_path);
				files.push_back (file);
			}
		}
		else {
			const char* extension = fltk::filename_ext (file.c_str());
			if (std::string (extension) == ".xml") {

				// save XML file
				block_paths.push_back (file_path);
			}
		}

		free (block_files[f]);
	}

	free (block_files);

	// process subdirectories
	names_t::iterator path_i = file_paths.begin();
	names_t::iterator file_i = files.begin();
	for (; path_i != file_paths.end(); ++path_i, ++file_i) {

		block_tree_node_t sub_node;
		sub_node.node_name = *file_i;
		sub_node.node_path = RootNode.node_path + "/" + *file_i;
		RootNode.child_nodes.push_back (sub_node);

		// load blocks from the subdirectory
		load_default_blocks (RootNode.child_nodes.back(), BlockCount);
	}

	// process blocks
	for (names_t::iterator path_i = block_paths.begin(); path_i != block_paths.end(); ++path_i) {

		// try loading the block
		shader_block* new_block = builder.build_block (*path_i);
		if (new_block) {

			successful_block_count++;

			if (m_default_blocks.find (new_block->name()) != m_default_blocks.end()) {

				// duplicate name
				log() << error << "couldn't load " << *path_i << " : a block named '" << new_block->name() << "' already exists." << std::endl;
			}
			else {

				default_block_t block_info;
				block_info.name = new_block->name();
				block_info.path = *path_i;
				m_default_blocks.insert (std::make_pair (new_block->name(), block_info));
				++BlockCount;

				// save block
				RootNode.blocks.push_back (block_info);
			}
		}
		else {
			log() << error << "couldn't load " << *path_i << std::endl;
		}
	}

	log() << aspect << " loaded " << RootNode.blocks.size() << " blocks from " << RootNode.node_path << std::endl;
}


block_tree_node_t scene::get_block_hierarchy() const {

	return m_block_classification;
}


std::string scene::get_shader_code() {

	if (rib_root_block* rib_block = dynamic_cast<rib_root_block*>(m_rib_root_block)) {
		return rib_block->show_code();
	}

	return "";
}


void scene::show_preview (const std::string& TempDir) {

	if (rib_root_block* rib_block = dynamic_cast<rib_root_block*>(m_rib_root_block)) {
		rib_block->show_preview (TempDir);
	}

}


void scene::export_scene (const std::string& Directory) {

	if (rib_root_block* rib_block = dynamic_cast<rib_root_block*>(m_rib_root_block)) {
		rib_block->export_scene (Directory);
	}

}


shader_block* scene::get_parent (const std::string& BlockName, const std::string& Input, std::string& ParentOutput) const {

	const shrimp::io_t input (BlockName, Input);

	for (shrimp::dag_t::const_iterator connection = m_dag.begin(); connection != m_dag.end(); ++connection) {

		const shrimp::io_t to = connection->first;

		if (to == input) {

			const shrimp::io_t from = connection->second;
			shader_blocks_t::const_iterator block = m_blocks.find(from.first);
			ParentOutput = from.second;
			return block->second;
		}
	}

	return 0;
}


