
/*
    Copyright 2009, Romain Behar <romainbehar@users.sourceforge.net>

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


#include "services.h"

#include "miscellaneous/logging.h"

services::services() {
	log() << aspect << "services: constructor" << std::endl;

	log() << aspect << "services: initializing scene" << std::endl;
	m_scene = new scene();
	reset_scene();
}


services::~services() {
	delete m_scene;
}


void services::reset_scene() {
	log() << aspect << "services: reset scene" << std::endl;

	m_scene->new_scene();
	m_scene_file = "";

	m_block_selection.clear();
}


block_tree_node_t services::get_block_hierarchy() {
	return m_scene->get_block_hierarchy();
}


// turn current selection into a group
void services::group_selection()
{
	m_scene->group_blocks (m_block_selection);

	clear_selection();
}


bool services::is_selected (shader_block* Block)
{
	shrimp::shader_blocks_t::const_iterator i = m_block_selection.find (Block);
	if (i == m_block_selection.end())
	{
		return false;
	}

	return true;
}


int services::selection_size()
{
	return m_block_selection.size();
}


void services::clear_selection()
{
	m_block_selection.clear();
	m_group_selection.clear();
}

void services::clear_copy_selection()
{
	m_copy_selection.clear();
}


void services::set_group_selection (const int Group , const bool selection)
{
	if (selection)
	{
		m_group_selection.insert (Group);
	}
	else
	{
		m_group_selection.erase (Group);
	}
}


bool services::is_group_selected (const int Group)
{
	shrimp::group_set_t::const_iterator i = m_group_selection.find (Group);
	return (i != m_group_selection.end());
}


int services::group_selection_size()
{
	return m_group_selection.size();
}


void services::set_block_selection (shader_block* Block, const bool Selection)
{
	log() << aspect << "services: set_block_selection of " << Block->name() << " with " << Selection << std::endl;

	if (!Block)
	{
		log() << error << "no block given for selection." << std::endl;
		return;
	}

	if (Selection) {
		m_block_selection.insert (Block);
	} else {
		m_block_selection.erase (Block);
	}
}


void services::set_block_rolled_state (shader_block* Block, const bool Rolled)
{
	log() << aspect << "services: set_block_rolled_state of " << Block->name() << " with " << Rolled << std::endl;

	m_scene->set_block_rolled_state (Block, Rolled);
}


// copy block
void services::copy_blocks(const std::string& Name, const int Group)
{

	shader_block* BlockToCopy = get_block(Name);

	// temp name
	const std::string Temp = "Copy";
	std::string NewName;
	// create block
	shader_block* new_block = new shader_block (Temp, "",0);

	// set new name
	NewName = get_unique_block_name(Name);
	new_block->set_name(NewName);

	// copy description
	new_block->m_description = BlockToCopy->m_description;

	// copy author
	new_block->m_author = BlockToCopy->m_author;
	// copy usage
	new_block->set_usage(BlockToCopy->m_usage);

	// copy properties
	// input and output properties

		// input and output properties
		typedef std::vector<property> properties_t;

		if (BlockToCopy->m_inputs.size()){
			for (properties_t::const_iterator input = BlockToCopy->m_inputs.begin(); input != BlockToCopy->m_inputs.end(); ++input) {

					new_block->add_input (input->m_name, input->get_type(), input->get_type_extension(), input->get_storage(), input->m_description, input->get_value(), input->m_multi_operator, input->m_shader_parameter);

			}
		}

		if (BlockToCopy->m_outputs.size()){
			for (properties_t::iterator output = BlockToCopy->m_outputs.begin(); output != BlockToCopy->m_outputs.end(); ++output) {

				new_block->add_output (output->m_name, output->get_type(), output->get_type_extension(), output->get_storage(), output->m_description, output->m_shader_output);

			}
		}


	// copy block position, size and rolled state in scene
		new_block->m_position_x = BlockToCopy->m_position_x+0.5;
		new_block->m_position_y = BlockToCopy->m_position_y+0.5;
		new_block->m_width = BlockToCopy->m_width ;
		new_block->m_height = BlockToCopy->m_height ;
		new_block->m_rolled = BlockToCopy->m_rolled ;

	// copy shader code
		new_block->m_includes = BlockToCopy->m_includes;
		new_block->m_code = BlockToCopy->m_code;
		new_block->m_code_written = BlockToCopy->m_code_written;

		const copy_block_t New (NewName, new_block);
		const copy_block_t Org (Name,BlockToCopy);

	// copy new block to buffer and selection
		m_copy_buffer.insert (std::make_pair (Org,New));
		m_copy_selection.insert (std::make_pair (Org,New));

	if (Group){
		int num_groups =0;

		shrimp::group_set_t groups = m_scene->group_list();
		for (shrimp::group_set_t::const_iterator g = groups.begin(); g != groups.end(); ++g)
		{
			if(*g > num_groups)
				num_groups = *g;
		}

	// add group if block part of a group
		m_groups_buffer.insert(std::make_pair(NewName,(Group+num_groups)));
	}
}

void services::copy_connections()
{
	// copy connections
	for (shrimp::dag_t::const_iterator connection = m_scene->m_dag.begin(); connection != m_scene->m_dag.end(); ++connection)
	{
		const copy_block_t to = (std::make_pair (connection->first.first,get_block(connection->first.first)));
		const copy_block_t from = (std::make_pair (connection->second.first,get_block(connection->second.first)));

		// found connections to copy
		shader_blocks_copy_t::const_iterator i = m_copy_selection.find(to);
		shader_block* check_block = i->first.second;
		shader_block* check_block_end = m_copy_selection.end()->first.second;

		if (!(check_block == check_block_end))
		{
			const shrimp::io_t to_copy = (std::make_pair (i->second.first,connection->first.second));

			shader_blocks_copy_t::const_iterator j = m_copy_selection.find(from);

			shader_block* check_block2 = j->first.second;
			shader_block* check_block_end2 = m_copy_selection.end()->first.second;
			if (!(check_block2 == check_block_end2))
			{
				const shrimp::io_t from_copy = (std::make_pair (j->second.first,connection->second.second));

				// add connection to the m_dag_copy structure
				m_scene->m_dag_copy.insert (std::make_pair (to_copy,from_copy));
			}
		}
	}
}

void services::paste_blocks()
{
	// pasted blocks
	for (shader_blocks_copy_t::iterator new_block = m_copy_buffer.begin(); new_block != m_copy_buffer.end(); ++new_block)
	{
		shader_block* paste =new_block->second.second;

		const std::string paste_name = paste->name();
		m_scene->add_block (paste_name,"",paste);

		// create new_groups (pasted groups)
		shrimp::groups_t::const_iterator g = m_groups_buffer.find(paste_name);
		if(!(g == m_groups_buffer.end()))
		{
			const int Group = g->second;
			m_scene->add_to_group (paste_name, Group);
		}
	}

	// pasted connection of pasted blocks
	for (shrimp::dag_t::const_iterator connection = m_scene->m_dag_copy.begin(); connection != m_scene->m_dag_copy.end(); ++connection) {
		const shrimp::io_t to = connection->first;
		const shrimp::io_t from = connection->second;
		connect (to, from);
	}
}


//copy blocks
void services::copy_selected_blocks(shader_block* block)
{
	int total = selection_size();
	if (total < 1)
	{
		return;
	}

	int group_total = group_selection_size();
	clear_copy_selection();
	m_copy_buffer.clear();

	if (!block){
		log() << error << "no active block not found." << std::endl;
		return;
	}

	std::string block_name;

	//copy block
	//If multi selection
	if (total>1)
	{
		for (shrimp::shader_blocks_t::const_iterator block_i = m_block_selection.begin(); block_i != m_block_selection.end(); ++block_i)
		{
			shader_block* block_copy = *block_i;
			block_name = block_copy->name();

			int group = get_block_group (block_copy);
			if (!group)
			{
				copy_blocks (block_name, 0);
			}
		}
	}

	//If single selection no parsing grab m_active_block
	else if (total<2)
	{
		block_name = block->name();
		copy_blocks(block_name, 0);

	}
	//copy group as well

	if (group_total>=1)
	{
		copy_selected_groups();
	}
}

//copy groups
void services::copy_selected_groups()
{
	for (shrimp::group_set_t::const_iterator g = m_group_selection.begin(); g != m_group_selection.end(); ++g)
	{
		shrimp::shader_blocks_t blocks = m_scene->get_group_blocks (*g);
		for (shrimp::shader_blocks_t::iterator block = blocks.begin(); block != blocks.end(); ++block)
		{
			copy_blocks((*block)->name(), *g);
		}
	}
}


void services::paste(shader_block* active_block)
{
	if (m_copy_selection.size())
	{
		paste_buffered_blocks();
	}
	// paste buffer if no new copy selection
	else if (m_copy_buffer.size())
	{
		clear_selection();
		for (shader_blocks_copy_t::iterator new_block = m_copy_buffer.begin(); new_block != m_copy_buffer.end(); ++new_block)
		{
			m_block_selection.insert (new_block->first.second);
		}

		copy_selected_blocks(active_block);
		paste_buffered_blocks();
	}
}


void services::paste_buffered_blocks()
{
//paste blocks
	clear_selection();

	if (m_copy_selection.size())
	{
		copy_connections();
		paste_blocks();

		// make block copy current selection
		for (shader_blocks_copy_t::iterator new_block = m_copy_buffer.begin(); new_block != m_copy_buffer.end(); ++new_block)
		{
			// Don't select block part of a group
			if (!get_block_group(new_block->second.second))
			{
				set_block_selection(new_block->second.second,1);
			}
		}

		// make group copy current selection
		for (shrimp::groups_t::const_iterator g = m_groups_buffer.begin(); g != m_groups_buffer.end(); ++g)
		{
			set_group_selection(g->second,1);
		}
	}

	clear_copy_selection();
}


void services::cut_selection (shader_block* active_block)
{
	// copy blocks
	copy_selected_blocks(active_block);
	copy_connections();

	// delete blocks
	if (selection_size() > 1)
	{
		//Multi selection
		m_scene->delete_block(active_block->name());
		m_copy_buffer.clear();
		clear_selection();
		clear_copy_selection();
	}
	else if (selection_size() == 1)
	{
		m_scene->delete_block(active_block->name());
		m_copy_buffer.clear();
		clear_selection();
		clear_copy_selection();
	}
}


void services::delete_selection()
{
	// delete selected blocks
	for (shrimp::shader_blocks_t::const_iterator block_i = m_block_selection.begin(); block_i != m_block_selection.end(); ++block_i)
	{
		m_scene->delete_block ((*block_i)->name());
	}
	m_block_selection.clear();

	// delete selected groups
	for (shrimp::group_set_t::const_iterator group_i = m_group_selection.begin(); group_i != m_group_selection.end(); ++group_i)
	{
		m_scene->delete_group (*group_i);
	}
	m_group_selection.clear();
}


