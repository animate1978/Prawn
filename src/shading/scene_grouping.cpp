
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

#include "../miscellaneous/misc_string_functions.h"


shrimp::group_set_t scene::group_list() {

	shrimp::group_set_t groups;
	for (shrimp::groups_t::const_iterator g = m_groups.begin(); g != m_groups.end(); ++g) {
		groups.insert(g->second);
	}

	return groups;
}


void scene::add_to_group (const std::string& Block, const int Group) {

	if (Block.size() && Group)
		m_groups.insert (std::make_pair(Block, Group));
}


int scene::get_block_group (const shader_block* Block) {

	shrimp::groups_t::const_iterator g = m_groups.find(Block->name());
	if(g == m_groups.end())
		return 0;

	return g->second;
}


void scene::ungroup (const int Group) {

	shrimp::groups_t groups2;
	for (shrimp::groups_t::iterator g = m_groups.begin(); g != m_groups.end(); ++g) {

		if (g->second != Group) {

			groups2.insert (*g);
		}
	}

	m_groups = groups2;

	m_group_names.erase (Group);
}


void scene::group_blocks (const shrimp::shader_blocks_t& Blocks)
{
	// find the next group number
	int max = 0;
	for (shrimp::groups_t::const_iterator g = m_groups.begin(); g != m_groups.end(); ++g)
	{
		if(g->second > max)
			max = g->second;
	}
	++max;

	// store new group
	for (shrimp::shader_blocks_t::const_iterator block_i = Blocks.begin(); block_i != Blocks.end(); ++block_i)
	{
		m_groups.insert(std::make_pair((*block_i)->name(), max));
	}
}


shrimp::shader_blocks_t scene::get_group_blocks (const int Group)
{
	shrimp::shader_blocks_t blocks;

	for (shrimp::groups_t::const_iterator block_i = m_groups.begin(); block_i != m_groups.end(); ++block_i)
	{
		if (block_i->second == Group)
		{
			blocks.insert (get_block (block_i->first));
		}
	}

	return blocks;
}


const std::string scene::get_group_name (const int Group) const {

	group_names_t::const_iterator name = m_group_names.find(Group);
	if (name == m_group_names.end())
		return "Group " + string_cast(Group);

	return name->second;
}


void scene::set_group_name(const int Group, const std::string& Name) {

	group_names_t::iterator g = m_group_names.find(Group);
	if (g == m_group_names.end()) {
		m_group_names.insert (std::make_pair(Group, Name));
	} else {
		// insert() does not erase an existing record
		g->second = Name;
	}
}


