
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


#ifndef _shader_block_h_
#define _shader_block_h_

#include "../miscellaneous/misc_xml.h"

#include <set>
#include <string>
#include <vector>


// a block property : input or output
class property
{
public:
	property (const std::string& Name, const std::string& Description = "");

	typedef enum {
		UNKNOWN,
		COLOR,
		FLOAT,
		MATRIX,
		NORMAL,
		POINT,
		STRING,
		VECTOR,
		ARRAY
	} variable_t;

	typedef enum {
		VARYING, // default
		UNIFORM
	} storage_t;

	std::string m_name;
	std::string m_description;
	std::string m_multi_operator;
	std::string m_multi_operator_parent_name;
	bool m_shader_parameter;
	bool m_shader_output;


	int add_possible_types (const std::string& Types);
	bool set_type (const std::string& Type);
	std::string get_type() const;

	bool set_type_extension (const std::string& TypeExtension);
	std::string get_type_extension() const;

	int get_type_extension_size() const;

	std::string get_type_for_declaration() const;

	bool set_storage (const std::string& Storage);
	std::string get_storage() const;

	bool is_multi_operator() const;

	void set_value (const std::string& Value);
	std::string get_value() const;
	std::string value_as_sl_string() const;

	void set_type_parent (const std::string& Parent);
	std::string get_type_parent() const;

private:
	// variable type
	variable_t m_current_type;
	std::set<variable_t> m_possible_types;

	variable_t convert_type (const std::string& Type);
	std::string convert_type (variable_t Type) const;

	// variable type extension
	variable_t m_current_type_extension;
	int m_current_type_extension_size;

	// storage type
	storage_t m_current_storage;
	std::set<storage_t> m_possible_storages;

	storage_t convert_storage (const std::string& Storage);
	std::string convert_storage (storage_t Storage) const;

	// name of a parent property,
	// a parent type change applies to its children
	std::string m_type_parent;

	// value
	std::string m_value;
};


typedef std::vector<std::string> types_t;
types_t get_property_types();
typedef std::vector<std::string> storages_t;
storages_t get_property_storage_types();
typedef std::vector<std::string> array_types_t;
types_t get_array_types();


// basic block class
class block {
public:
	block() {}
	virtual ~block() {}
};

// forward declaration of root blocks
class rib_root_block;

// a shader block
class shader_block : block {

public:
	shader_block (const std::string& BlockName, const std::string& Description, const bool RootBlock = false);

private:
	// name and description
	std::string m_name;

public:
	std::string m_description;
	std::string m_author;
	const bool m_root_block;

	std::string m_usage;

	std::string name() const;
	// return the block's name as a valid SL name
	std::string sl_name() const;
	void set_name (const std::string& Name);
	void set_usage (const std::string& Usage);

	// input and output properties
	typedef std::vector<property> properties_t;
	properties_t m_inputs;
	properties_t m_outputs;

	void add_input (const std::string& Name, const std::string& Type, const std::string& TypeExtension, const std::string& Storage, const std::string& Description, const std::string& Default, const std::string& Multi, const bool ShaderParameter = true);
	std::string add_multi_input (const std::string& ParentName);
	void add_output (const std::string& Name, const std::string& Type, const std::string& TypeExtension, const std::string& Storage, const std::string& Description, const bool ShaderOutput = false);

	std::string input_type (const std::string& Name) const;
	std::string input_storage (const std::string& Name) const;
	std::string output_type (const std::string& Name) const;
	std::string output_storage (const std::string& Name) const;

	bool is_input (const std::string& Name) const;
	bool is_output (const std::string& Name) const;

	void set_input_value (const std::string& Name, const std::string& Value);
	std::string get_input_value (const std::string& Name) const;
	std::string get_input_value_as_sl_string (const std::string& Name) const;

	bool set_input_type (const std::string& Name, const std::string& Type);
	std::string get_input_type (const std::string& Name) const;

	bool set_input_type_extension (const std::string& Name, const std::string& TypeExtension);
	std::string get_input_type_extension (const std::string& Name) const;
	int get_input_type_extension_size (const std::string& Name) const;

	bool set_input_storage (const std::string& Name, const std::string& Storage);
	std::string get_input_storage (const std::string& Name) const;

	void set_input_parent (const std::string& Name, const std::string& Parent);
	std::string get_input_parent (const std::string& Name) const;

	void set_input_type_parent (const std::string& Name, const std::string& Parent);
	//std::string get_input_type_parent (const std::string& Name) const;

	bool set_output_type (const std::string& Name, const std::string& Type);
	std::string get_output_type (const std::string& Name) const;

	bool set_output_type_extension (const std::string& Name, const std::string& TypeExtension);
	std::string get_output_type_extension (const std::string& Name) const;
	int get_output_type_extension_size (const std::string& Name) const;

	bool set_output_storage (const std::string& Name, const std::string& Type);
	std::string get_output_storage (const std::string& Name) const;

	bool is_shader_output (const std::string& Name) const;
	void set_shader_output (const std::string& Name, const bool State);

	bool is_shader_parameter (const std::string& Name) const;
	void set_shader_parameter (const std::string& Name, const bool State);

	bool is_input_multi_operator (const std::string& Name) const;
	bool set_input_multi_operator_parent (const std::string& Name, const std::string& Parent);

	void get_multi_input_child_list (const std::string& Name, std::vector<std::string>& Children) const;

	std::string get_unique_input_name (const std::string& Name) const;

	unsigned long input_count() const;

	// block position, size and rolled state in scene
	double m_position_x;
	double m_position_y;
	double m_width;
	double m_height;
	double m_rolled;

	void set_position (const double X, const double Y);
	void roll (const bool Roll);
	bool is_rolled() const;

	// shader code
	std::string m_includes;
	std::string m_code;
	bool m_code_written;

	void set_includes (const std::string& File);
	void set_code (const std::string& Code);
	void reset_code_written() { m_code_written = false; }
	std::string get_code() const;
	bool code_written() const { return m_code_written; }

	// return shader parameters and local values
	void get_includes (std::set<std::string>& includes);

	// dump block's structure
	void dump_structure() const;

	// load information from an XML block pointer
	bool load_from_xml (TiXmlNode& XML);
};


class shader_block_builder {

public:
	shader_block_builder() {
	}

	shader_block* build_block (const std::string& FilePath);
};


#endif // _shader_block_h_


