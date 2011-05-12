
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


#ifndef _shrimp_public_structures_h_
#define _shrimp_public_structures_h_

// Structures that need to be accessed from outside the core

#include "shader_block.h"

#include <string>
#include <vector>


namespace shrimp
{
	// definition of a pad, as a pair of strings:
	// the first is the block name, the second is the pad name
	typedef std::pair <std::string, std::string> io_t;
	// store connections as a directed acyclic graph, Output -> Input,
	// in a map as <input, output>, an input receives only one output
	typedef std::map <io_t, io_t> dag_t;

	typedef std::set<shader_block*> shader_blocks_t;
	// group structures
	typedef std::set<int> group_set_t;
	typedef std::map<std::string, int> groups_t;
}


// Shrimp block list
struct default_block_t
{
	std::string name;
	std::string path;
};
typedef std::vector<default_block_t> default_block_list_t;


// Shrimp block hierarchy
struct block_tree_node_t;
typedef std::vector<block_tree_node_t> block_tree_node_list_t;

struct block_tree_node_t
{
	std::string node_name;
	std::string node_path;
	block_tree_node_list_t child_nodes;

	default_block_list_t blocks;
};


#endif // _shrimp_public_structures_h_

