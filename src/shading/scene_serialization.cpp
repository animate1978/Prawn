
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


#include "scene.h"
#include "rib_root_block.h"

#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"

#include <fltk/filename.h>

#include <fstream>


bool scene::load (const std::string& Scene) {

	// try opening scene
	TiXmlDocument xml_shader(Scene.c_str());
	if (!xml_shader.LoadFile()) {

		log() << error << "couldn't load shader " << Scene << ", is it a valid XML file?" << std::endl;
		return false;
	} else {

		m_file_name = Scene;
	}

	// start parsing basic shader information
	std::string scene_name ("");
	std::string scene_description ("");
	std::string scene_authors ("");
	TiXmlNode* network_xml_node = 0;

	for (TiXmlAttribute* a = xml_shader.FirstChild()->ToElement()->FirstAttribute(); a; a = a->Next()) {

		// parse attributes
		const std::string name (a->Name());
		if (name == "name") {
			scene_name = a->Value();
		}
		else if (name == "authors") {
			scene_authors = a->Value();
		}
		else {
			log() << error << "unhandled shader attribute : '" << name << "'" << std::endl;
		}
	}

	for (TiXmlNode* c = xml_shader.FirstChild()->FirstChild(); c; c = c->NextSibling()) {

		// parse child nodes
		const std::string element (c->Value());

		if(element == "about") {

			if (c->FirstChild()) {
				scene_description = c->FirstChild()->ToText()->Value();
			}
		}
		else if(element == "network") {

			network_xml_node = c;
		}
		else {
			log() << error << "unknown shader element '" << element << "' in " << Scene << std::endl;
		}
	}

	if (!network_xml_node) {

		log() << error << "no network block found in '" << Scene << "'." << std::endl;
		return false;
	}

	// scene could be loaded, initialize
	empty_scene();

	// start loading content
	m_name = scene_name;
	set_description (scene_description);
	m_authors = scene_authors;
	log() << aspect << "loading shader '" << m_name << "'" << std::endl;

	// load blocks
	for (TiXmlNode* e = network_xml_node->FirstChild(); e; e = e->NextSibling()) {

		const std::string element (e->Value());
		if(element == "block") {

			xml::attributes_t attributes;
			xml::get_attributes (*e->ToElement(), attributes);

			std::string id;
			if (!xml::get_attribute (*e->ToElement(), "id", id)) {

				log() << error << "block has undefined 'id' attribute." << std::endl;
				continue;
			}

			bool root_block = false;
			std::string root_value ("");
			if (xml::get_attribute (*e->ToElement(), "root", root_value)) {

				root_block = true;
			}

			bool rolled_block = false;
			std::string rolled_value ("");
			if (xml::get_attribute (*e->ToElement(), "rolled", rolled_value)) {

				rolled_block = true;
			}

			std::string position_sx;
			std::string position_sy;
			xml::get_attribute (*e->ToElement(), "position_x", position_sx);
			xml::get_attribute (*e->ToElement(), "position_y", position_sy);
			double position_x = 0;
			double position_y = 0;
			std::istringstream stream_x (position_sx.c_str());
			stream_x >> position_x;
			std::istringstream stream_y (position_sy.c_str());
			stream_y >> position_y;

			// create block
			shader_block* new_block = 0;
			if (!root_block) {
				new_block = new shader_block (id, "");
			} else {
				//if (root_value == "RIB") {
					m_rib_root_block = new rib_root_block (id, this);
					new_block = m_rib_root_block;

					for (TiXmlNode* c = e->FirstChild(); c; c = c->NextSibling()) {

						const std::string element (c->Value());
						if (element == "rib_statements") {

							if (c->FirstChild()) {
								const std::string statements = c->FirstChild()->ToText()->Value();
								m_rib_root_block->set_general_statements (trim (statements) + "\n");
							}
						}
						else if (element == "imager_statement") {

							if (c->FirstChild()) {
								const std::string statement = c->FirstChild()->ToText()->Value();
								m_rib_root_block->set_imager_statement (trim (statement));
							}
						}
/*
						else if (element == "AOV_preview") {

							if (c->FirstChild()) {
								const std::string AOV_preview = a->Value();
								m_rib_root_block->set_AOV (!state.empty());
							}
						}
*/
					}
				//}
			}

			add_block (id, "", new_block);
			new_block->set_name (id);
			new_block->set_position (position_x, position_y);
			if (rolled_block) {
				set_block_rolled_state (new_block, true);
			}

			// load input and output properties
			if (!root_block) {
				new_block->load_from_xml (*e);
			}

		} else if (element == "group") {

			xml::attributes_t attributes;
			xml::get_attributes (*e->ToElement(), attributes);

			int group_id = 0;
			if (attributes.find ("id") == attributes.end()) {

				log() << error << "group has no 'id' attribute!" << std::endl;
				continue;
			}

			group_id = from_string (attributes["id"], 0);
			if (!group_id) {

				log() << error << "group id is null!" << std::endl;
				continue;
			}

			if (attributes.find ("name") != attributes.end()) {

				std::string name = attributes["name"];
				if (name.size()) {

					set_group_name (group_id, name);
				}
			}

			for (TiXmlNode* c = e->FirstChild(); c; c = c->NextSibling()) {

				const std::string type = std::string(c->Value());

				xml::attributes_t attributes;
				xml::get_attributes(*c->ToElement(), attributes);

				if (attributes.find("name") == attributes.end()) {

					log() << error << type << " has no 'name' attribute!" << std::endl;
					continue;
				}

				std::string name = attributes["name"];
				if (name.size()) {

					add_to_group (name, group_id);
				}
			}

		} else {
			log() << error << "Unknown element '" << element << "' in " << Scene << "'s network." << std::endl;
			continue;
		}
	}

	// load connections
	for (TiXmlNode* e = network_xml_node->FirstChild(); e; e = e->NextSibling()) {

		const std::string element(e->Value());
		if(element != "block")
			continue;

		std::string id;
		if(!xml::get_attribute(*e->ToElement(), "id", id))
			continue;

		// load input and output properties
		for(TiXmlNode* c = e->FirstChild(); c; c = c->NextSibling())
		{
			std::string name;
			if(!xml::get_attribute(*c->ToElement(), "name", name))
				continue;

			if(c->FirstChild())
			{
				std::string output_name;
				if(!xml::get_attribute(*c->FirstChild()->ToElement(), "output", output_name))
					continue;

				std::string output_id;
				if(!xml::get_attribute(*c->FirstChild()->ToElement(), "parent", output_id))
					continue;

				connect (shrimp::io_t (id, name), shrimp::io_t (output_id, output_name));
			}
		}
	}

	return true;
}


bool scene::save_as (const std::string& ShaderFile) {

	// serialize the shader into an XML file
	xml::element shader ("shrimp");
	shader.push_attribute ("name", m_name);
	shader.push_attribute ("authors", m_authors);

	xml::element about ("about");
	about.set_text (m_description);

	shader.push_child (about);
	shader.children.push_back (xml_network());

	// automatically add the .xml extension
	m_file_name = ShaderFile;
	const char* extension = fltk::filename_ext (m_file_name.c_str());
	if (std::string (extension) != ".xml")
		m_file_name += ".xml";

	// save the file
	std::ofstream out_file;
	out_file.open (m_file_name.c_str(), std::ios::out | std::ios::trunc);
	xml::output_tree (shader, out_file);
	out_file.close();

	return true;
}


xml::element scene::xml_network() {

	xml::element network ("network");

	for (shader_blocks_t::const_iterator b = m_blocks.begin(); b != m_blocks.end(); ++b) {

		xml::element xml_block ("block");

		shader_block* block = b->second;

		// block attributes
		xml_block.push_attribute ("id", block->name());
		xml_block.push_attribute ("position_x", block->m_position_x);
		xml_block.push_attribute ("position_y", block->m_position_y);
		xml_block.push_attribute ("author", block->m_author);
		if (is_rolled (block)) {
			xml_block.push_attribute ("rolled", std::string ("1"));
		}
		if (block->m_root_block) {
			if (rib_root_block* rib_block = dynamic_cast<rib_root_block*> (block)) {
				xml_block.push_attribute ("root", rib_block->root_type);
				xml::element general_statements ("rib_statements");
					general_statements.set_text (rib_block->get_general_statements());
				xml_block.push_child (general_statements);
				xml::element imager_statement ("imager_statement");
					imager_statement.set_text (rib_block->get_imager_statement());
				xml_block.push_child (imager_statement);
				if (rib_block->get_AOV()) {
					xml_block.push_attribute ("AOV_preview", std::string ("1"));
				}
			}
		}

		// inputs and their connections
		for (shader_block::properties_t::const_iterator input = block->m_inputs.begin(); input != block->m_inputs.end(); ++input) {

			const std::string name = input->m_name;

			xml::element xml_input ("input");
			xml_input.push_attribute ("name", name);
			xml_input.push_attribute ("type", input->get_type());
			if (input->get_type() == "array") {
				xml_input.push_attribute ("type_extension", input->get_type_extension() + ":" + string_cast(input->get_type_extension_size()));
			}
			xml_input.push_attribute ("storage", input->get_storage());
			if (input->is_multi_operator()) {
				xml_input.push_attribute ("multi", input->m_multi_operator);
			}
			if (!input->m_multi_operator_parent_name.empty()) {
				xml_input.push_attribute ("multi_parent", input->m_multi_operator_parent_name);
			}
			if (input->m_shader_parameter) {
				xml_input.push_attribute ("shader_parameter", std::string ("1"));
			}

			// store value, if any
			const std::string value = input->get_value();
			if (!value.empty()) {

				xml_input.push_attribute ("value", value);
			}

			xml_input.push_attribute ("description", input->m_description);

			// store type parent, if any
			const std::string type_parent = input->get_type_parent();
			if (!type_parent.empty()) {

				xml_input.push_attribute ("type_parent", type_parent);
			}

			// store connection, if any
			shrimp::dag_t::const_iterator connection = m_dag.find (shrimp::io_t (block->name(), input->m_name));
			if (connection != m_dag.end()) {

				xml::element xml_connection ("connection");
				xml_connection.push_attribute ("parent", connection->second.first);
				xml_connection.push_attribute ("output", connection->second.second);
				xml_input.push_child (xml_connection);
			}

			xml_block.push_child (xml_input);
		}

		// outputs
		for (shader_block::properties_t::const_iterator output = block->m_outputs.begin(); output != block->m_outputs.end(); ++output) {

			const std::string name = output->m_name;
			xml::element xml_output ("output");
			xml_output.push_attribute ("name", name);
			xml_output.push_attribute ("type", output->get_type());
			if (output->get_type() == "array") {
				xml_output.push_attribute ("type_extension", output->get_type_extension() + ":" + string_cast(output->get_type_extension_size()));
			}
			xml_output.push_attribute ("storage", output->get_storage());
			xml_output.push_attribute ("description", output->m_description);
			if (output->m_shader_output) {
				xml_output.push_attribute ("shader_output", std::string ("1"));
			}

			xml_block.push_child (xml_output);
		}

		// code
		xml::element code ("rsl_code");
		code.set_text (block->m_code);

		xml_block.push_child (code);

		// includes
		xml::element includes ("rsl_include");
		includes.set_text (block->m_includes);

		xml_block.push_child (includes);

		// usage
		xml::element usage ("usage");
		usage.set_text (block->m_usage);

		xml_block.push_child (usage);

		// add complete block to the list
		network.push_child (xml_block);
	}

	// groups
	shrimp::group_set_t groups = group_list();
	for (shrimp::group_set_t::const_iterator g = groups.begin(); g != groups.end(); ++g) {

		xml::element xml_group ("group");
		xml_group.push_attribute ("id", *g);
		xml_group.push_attribute ("name", get_group_name(*g));
		for (shrimp::groups_t::const_iterator i = m_groups.begin(); i != m_groups.end(); ++i) {

			if (i->second == *g) {

				xml::element block ("block");
				block.push_attribute ("name", i->first);

				xml_group.push_child (block);
			}
		}

		network.push_child (xml_group);
	}

	return network;
}


