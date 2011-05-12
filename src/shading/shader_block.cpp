
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


#include "shader_block.h"

#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"
#include "../miscellaneous/misc_xml.h"

#include <iostream>
#include <vector>


property::property (const std::string& Name, const std::string& Description) :
	m_name (Name),
	m_description (Description),
	m_multi_operator (""),
	m_multi_operator_parent_name (""),
	m_shader_output (false),
	m_current_storage (VARYING),
	m_type_parent (""),
	m_value ("")
{
}


int property::add_possible_types (const std::string& Types) {

	int type_count = 0;

	// initialize types from list
	if (Types.size()) {

		std::string type_list = Types;
		std::istringstream stream (type_list);

		std::string new_type;
		stream >> new_type;
		m_possible_types.insert (convert_type (new_type));
		m_current_type = convert_type (new_type);

		while (!stream.eof()) {

			stream >> new_type;
			m_possible_types.insert (convert_type (new_type));
		}
	}

	if (m_possible_types.empty()) {

		m_possible_types.insert (FLOAT);
		m_current_type = FLOAT;
	}

	// ?
	return type_count;
}


bool property::set_type (const std::string& Type) {

	variable_t new_type = UNKNOWN;

	if (Type.size()) {

		std::string token_list = Type;
		std::istringstream stream (token_list);

		// strtolower ?
		do {
			std::string new_token;
			stream >> new_token;

			variable_t nt = convert_type (new_token);
			if (nt != UNKNOWN) {
				new_type = nt;
			}

		} while (!stream.eof());

		if (new_type != UNKNOWN) {

			m_current_type = new_type;

			return true;
		}

		log() << error << "unknown type '" << Type << "'" << std::endl;
	} else {

		log() << warning << "empty type (set_type)" << std::endl;
	}

	return false;
}


std::string property::get_type() const {

	return convert_type (m_current_type);
}


bool property::set_type_extension (const std::string& TypeExtension) {

	m_current_type_extension = UNKNOWN;
	if (TypeExtension == "" && get_type() != "array") {
		return true;
	}

	if (get_type() == "array") {
		// Type extension can be "vector", "vector:5"
		// (where 'vector' is the array type, 5 its size)
		std::string extension_name = TypeExtension;
		m_current_type_extension_size = 0;

		int colon = TypeExtension.find (':', 0);
		if (colon > 0) {

			extension_name = TypeExtension.substr (0, colon);

			int colon2 = TypeExtension.find (':', colon + 1);
			if (colon2 < 0) {
				std::string size_value = TypeExtension.substr (colon + 1);
				m_current_type_extension_size = from_string (size_value, 0);
			} else {
				m_current_type_extension_size = 0;
			}
		}

		variable_t new_type = convert_type (extension_name);
		if (new_type == UNKNOWN) {
			log() << error << "unknown type extension '" << m_current_type_extension << "'" << std::endl;
			return false;
		}

		m_current_type_extension = new_type;

	}

	return false;
}


std::string property::get_type_extension() const {

	if (m_current_type != ARRAY) {
		return "";
	}

	return convert_type (m_current_type_extension);
}


int property::get_type_extension_size() const
{

	return m_current_type_extension_size;
}


std::string property::get_type_for_declaration() const
{
	std::string type_declaration = get_type();
	if (type_declaration == "array")
	{
		type_declaration = get_type_extension();
	}

	return type_declaration;
}


bool property::set_storage (const std::string& Storage)
{

	m_current_storage = convert_storage (Storage);

	return true;
}


std::string property::get_storage() const {

	return convert_storage (m_current_storage);
}


void property::set_value (const std::string& Value) {

	m_value = Value;
}


std::string property::get_value() const {

	return m_value;
}


std::string property::value_as_sl_string() const {

	switch (m_current_type) {

		case COLOR:
		{
			// parse a colour
			std::string v = m_value;
			replace_variable (v, " (", "");
			replace_variable (v, ",", " ");
			replace_variable (v, ")", "");

			std::istringstream str (v);
			double v1 = 1;
			double v2 = 1;
			double v3 = 1;

			if (! (str >> v1)) {
				// the value couldn't be parsed, keep it as is
				return m_value;
			}

			// convert the value to the expected type
			if (! (str >> v2 >> v3)) {
				// the value string contains a single number
				v2 = v3 = v1;
			}

			std::ostringstream buffer;
			buffer << "color (";
			buffer << v1 << ", " << v2 << ", " << v3 << ")";
			return buffer.str();
		}

		case POINT:
		case VECTOR:
		case NORMAL:
		{
			// parse a three value array
			std::string v = m_value;
			replace_variable (v, " (", "");
			replace_variable (v, ",", " ");
			replace_variable (v, ")", "");

			std::istringstream str (v);
			double v1 = 0;
			double v2 = 0;
			double v3 = 0;

			if (! (str >> v1)) {
				// the value couldn't be parsed, keep it as is
				return m_value;
			}

			// convert the value to the expected type
			if (! (str >> v2 >> v3)) {
				// the value string contains a single number
				v2 = v3 = v1;
			}

			std::ostringstream buffer;
			if (POINT == m_current_type)
				buffer << "point";
			else if (VECTOR == m_current_type)
				buffer << "vector";
			else if (NORMAL == m_current_type)
				buffer << "normal";
			buffer << '(';
			buffer << v1;
			buffer << ',';
			buffer << v2;
			buffer << ',';
			buffer << v3;
			buffer << ')';
			return buffer.str();
		}

		case MATRIX:
		{
			// parse a 4x4 matrix (VEX has both 3x3 and 4x4 matrices though)
			std::string v = m_value;
			replace_variable (v, " (", "");
			replace_variable (v, "{", "");
			replace_variable (v, ",", " ");
			replace_variable (v, ")", "");
			replace_variable (v, "}", "");

			std::istringstream str (v);
			double v1 = 1;
			double v2 = 0;
			double v3 = 0;
			double v4 = 0;
			double v5 = 0;
			double v6 = 1;
			double v7 = 0;
			double v8 = 0;
			double v9 = 0;
			double v10= 0;
			double v11= 1;
			double v12= 0;
			double v13= 0;
			double v14= 0;
			double v15= 0;
			double v16= 1;

			// try parsing the matrix
			if (! (str >> v1 >> v2 >> v3 >> v4 >> v5 >> v6 >> v7 >> v8 >> v9 >> v10 >> v11 >> v12 >> v13 >> v14 >> v15 >> v16)) {
				// the value couldn't be parsed, keep it as is
				return m_value;
			}

			std::ostringstream buffer;
			buffer << "matrix (";
			buffer << v1 << ", " << v2 << ", " << v3 << ", " << v4 << ", ";
			buffer << v5 << ", " << v6 << ", " << v7 << ", " << v8 << ", ";
			buffer << v9 << ", " << v10 << ", " << v11 << ", " << v12 << ", ";
			buffer << v13 <<", " << v14 << ", " << v15 << ", " << v16 << ")";
			return buffer.str();
		}

		default:
			return m_value;
	}

	return m_value;
}


property::variable_t property::convert_type (const std::string& Type) {

	const std::string type_l = Type; // strtolower ?
	if ("color" == type_l || "colour" == type_l)
		return COLOR;
	if ("float" == type_l)
		return FLOAT;
	if ("matrix" == type_l)
		return MATRIX;
	if ("normal" == type_l)
		return NORMAL;
	if ("point" == type_l)
		return POINT;
	if ("string" == type_l)
		return STRING;
	if ("vector" == type_l)
		return VECTOR;
	if ("array" == type_l)
		return ARRAY;

	log() << warning << "unknown type (convert_type) : '" << Type << "' for property '" << m_name << "'" << std::endl;

	//return UNKNOWN;
	return FLOAT;
}


std::string property::convert_type (variable_t Type) const {

	switch (Type) {

		case COLOR: return "color";
		case FLOAT: return "float";
		case MATRIX: return "matrix";
		case NORMAL: return "normal";
		case POINT: return "point";
		case STRING: return "string";
		case VECTOR: return "vector";
		case ARRAY: return "array";

		default:
			log() << warning << "unknown type (convert_type) for property '" << m_name << "'" << std::endl;
			return "unknown";
	}
}


property::storage_t property::convert_storage (const std::string& Storage) {

	if ("varying" == Storage)
		return VARYING;
	if ("uniform" == Storage)
		return UNIFORM;

	if (Storage.size()) {
		log() << warning << "empty storage (convert_storage) for property '" << m_name << "'" << std::endl;
	}

	return VARYING;
}


std::string property::convert_storage (storage_t Storage) const {

	switch (Storage) {

		case VARYING: return "varying";
		case UNIFORM: return "uniform";

		default:
			log() << warning << "unhandled storage (convert_storage) for property '" << m_name << "'" << std::endl;
			return "varying";
	}
}


bool property::is_multi_operator() const {

	return !m_multi_operator.empty();
}


void property::set_type_parent (const std::string& Parent) {

	m_type_parent = Parent;
}


std::string property::get_type_parent() const {

	return m_type_parent;
}


types_t get_property_types() {

	types_t list;
	list.push_back ("float");
	list.push_back ("color");
	list.push_back ("string");
	list.push_back ("point");
	list.push_back ("vector");
	list.push_back ("normal");
	list.push_back ("matrix");
	list.push_back ("array");

	return list;
}


types_t get_property_storage_types() {

	types_t list;
	list.push_back ("varying");
	list.push_back ("uniform");

	return list;
}


types_t get_array_types() {

	types_t list;
	list.push_back ("float");
	list.push_back ("color");
	list.push_back ("string");
	list.push_back ("point");
	list.push_back ("vector");
	list.push_back ("normal");
	//list.push_back ("matrix");

	return list;
}


shader_block::shader_block (const std::string& Name, const std::string& Description, const bool RootBlock) :

	m_name (Name),
	m_description (Description),
	m_author (""),
	m_root_block (RootBlock),
	m_position_x (0),
	m_position_y (0),
	m_width (1.25),
	m_height (1),
	m_rolled (false) {

}


void shader_block::add_input (const std::string& Name, const std::string& Type, const std::string& TypeExtension, const std::string& Storage, const std::string& Description, const std::string& DefaultValue, const std::string& Multi, const bool ShaderParameter) {

	bool ok = true;

	property p (Name, Description);
	p.add_possible_types (Type);
	if (!p.set_type (Type)) {
		ok = false;
	}
	p.set_type_extension (TypeExtension);
	p.set_storage (Storage);
	if (!p.set_storage (Storage)) {
		ok = false;
	}
	p.set_value (DefaultValue);
	p.m_multi_operator = Multi;
	p.m_shader_parameter = ShaderParameter;

	if (!ok) {
		log() << warning << "...above warnings about the '" << Name << "' input of the '" << name() << "' block!" << std::endl;
	}

	m_inputs.push_back (p);
}


std::string shader_block::add_multi_input (const std::string& ParentName) {

	// make sure the given input exists
	if (!is_input (ParentName)) {

		log() << error << "input '" << ParentName << "' doesn't exist in the '" << name() << "' block!" << std::endl;
		return "";
	}

	// create a new one (with a parent)
	const std::string new_name = get_unique_input_name (ParentName);
	add_input (new_name, input_type (ParentName), "", "", "", "", "", false);
	set_input_parent (new_name, ParentName);

	// return the new input name
	return new_name;
}


void shader_block::add_output (const std::string& Name, const std::string& Type, const std::string& TypeExtension, const std::string& Storage, const std::string& Description, const bool ShaderOutput) {

	bool ok = true;

	property p (Name, Description);
	p.add_possible_types (Type);
	if (!p.set_type (Type)) {
		ok = false;
	}
	p.set_type_extension (TypeExtension);
	if (!p.set_storage (Storage)) {
		ok = false;
	}

	p.m_shader_output = ShaderOutput;

	if (!ok) {
		log() << warning << "...above warnings about the '" << Name << "' output of the '" << name() << "' block!" << std::endl;
	}

	m_outputs.push_back (p);
}


void shader_block::set_includes (const std::string& File) {

	if (!m_includes.empty()) {

		// TODO : CR/LF depending on the system
		m_includes += "\n";
	}

	m_includes += File;
}


void shader_block::set_code (const std::string& Code) {

	m_code = Code;
}


std::string shader_block::get_code() const {

	return m_code;
}


std::string shader_block::name() const {

	return m_name;
}


std::string shader_block::sl_name() const {

	std::string sl_name = m_name;
	replace_variable (sl_name, " ", "_");

	return sl_name;
}


void shader_block::set_name (const std::string& Name) {

	m_name = Name;
}


void shader_block::set_usage (const std::string& Usage) {

	m_usage = Usage;
}


std::string shader_block::input_type (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_type();
		}
	}

	log() << warning << "unknown input : " << Name << std::endl;
	return std::string ("");
}


std::string shader_block::input_storage (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_storage();
		}
	}

	log() << warning << "unknown input : " << Name << std::endl;
	return std::string ("");
}


std::string shader_block::output_type (const std::string& Name) const {

	for (properties_t::const_iterator o = m_outputs.begin(); o != m_outputs.end(); ++o) {

		if (o->m_name == Name) {

			return o->get_type();
		}
	}

	log() << warning << "unknown output : " << Name << std::endl;
	return std::string ("");
}


std::string shader_block::output_storage (const std::string& Name) const {

	for (properties_t::const_iterator o = m_outputs.begin(); o != m_outputs.end(); ++o) {

		if (o->m_name == Name) {

			return o->get_storage();
		}
	}

	log() << warning << "unknown output : " << Name << std::endl;
	return std::string ("");
}


bool shader_block::is_input (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return true;
		}
	}

	return false;
}


bool shader_block::is_output (const std::string& Name) const {

	for (properties_t::const_iterator o = m_outputs.begin(); o != m_outputs.end(); ++o) {

		if (o->m_name == Name) {

			return true;
		}
	}

	return false;
}


void shader_block::set_input_value (const std::string& Name, const std::string& Value) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			i->set_value (Value);
			return;
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
}


std::string shader_block::get_input_value (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_value();
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return "";
}


std::string shader_block::get_input_value_as_sl_string (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->value_as_sl_string();
		}
	}

	log() << error << "Unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return "";
}


bool shader_block::set_input_type (const std::string& Name, const std::string& Type) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->set_type (Type);
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
	return false;
}


std::string shader_block::get_input_type (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_type();
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return "";
}


bool shader_block::set_input_type_extension (const std::string& Name, const std::string& TypeExtension) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->set_type_extension (TypeExtension);
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
	return false;
}


std::string shader_block::get_input_type_extension (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_type_extension();
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return "";
}


int shader_block::get_input_type_extension_size (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_type_extension_size();
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return 0;
}


bool shader_block::set_input_storage (const std::string& Name, const std::string& Storage) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->set_storage (Storage);
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
	return false;
}


std::string shader_block::get_input_storage (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_storage();
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return "";
}


void shader_block::set_input_parent (const std::string& Name, const std::string& Parent) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			i->m_multi_operator_parent_name = Parent;
			return;
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
}


std::string shader_block::get_input_parent (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->m_multi_operator_parent_name;
		}
	}

	log() << error << "shader block input '" << Name << "' in " << name() << " has no parent." << std::endl;
	return "";
}


void shader_block::set_input_type_parent (const std::string& Name, const std::string& Parent) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			i->set_type_parent (Parent);
			return;
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
}


bool shader_block::set_output_type (const std::string& Name, const std::string& Type) {

	bool answer = true;

	// look for the output and set its type
	for (properties_t::iterator i = m_outputs.begin(); i != m_outputs.end(); ++i) {

		if (i->m_name == Name) {

			// change the type children (if any)
			for (properties_t::iterator j = m_inputs.begin(); j != m_inputs.end(); ++j) {

				const std::string parent = j->get_type_parent();
				if (!parent.empty() && (parent == Name)) {

					answer &= j->set_type (Type);

					// if it's a multi-input, change the children's type
					if (j->is_multi_operator()) {

						// get list of child inputs
						std::vector<std::string> children;
						get_multi_input_child_list (j->m_name, children);

						for (std::vector<std::string>::const_iterator c = children.begin(); c != children.end(); ++c) {

							set_input_type (*c, Type);
						}
					}
				}
			}

			return answer && i->set_type (Type);
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;
	return false;
}


std::string shader_block::get_output_type (const std::string& Name) const {

	for (properties_t::const_iterator i = m_outputs.begin(); i != m_outputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_type();
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;

	return "";
}


bool shader_block::set_output_type_extension (const std::string& Name, const std::string& TypeExtension) {

	for (properties_t::iterator o = m_outputs.begin(); o != m_outputs.end(); ++o) {

		if (o->m_name == Name) {

			return o->set_type_extension (TypeExtension);
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;
	return false;
}


std::string shader_block::get_output_type_extension (const std::string& Name) const {

	for (properties_t::const_iterator o = m_outputs.begin(); o != m_outputs.end(); ++o) {

		if (o->m_name == Name) {

			return o->get_type_extension();
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;

	return "";
}


int shader_block::get_output_type_extension_size (const std::string& Name) const {

	for (properties_t::const_iterator o = m_outputs.begin(); o != m_outputs.end(); ++o) {

		if (o->m_name == Name) {

			return o->get_type_extension_size();
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;

	return 0;
}


bool shader_block::set_output_storage (const std::string& Name, const std::string& Storage) {

	// look for the output and set its storage
	for (properties_t::iterator i = m_outputs.begin(); i != m_outputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->set_storage (Storage);
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;
	return false;
}


std::string shader_block::get_output_storage (const std::string& Name) const {

	for (properties_t::const_iterator i = m_outputs.begin(); i != m_outputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->get_storage();
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;

	return "";
}
bool shader_block::is_shader_output (const std::string& Name) const {

	for (properties_t::const_iterator i = m_outputs.begin(); i != m_outputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->m_shader_output;
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;

	return false;
}


void shader_block::set_shader_output (const std::string& Name, const bool State) {

	for (properties_t::iterator i = m_outputs.begin(); i != m_outputs.end(); ++i) {

		if (i->m_name == Name) {

			i->m_shader_output = State;
			return;
		}
	}

	log() << error << "unmatched shader block output '" << Name << "' in " << name() << std::endl;
}


bool shader_block::is_shader_parameter (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->m_shader_parameter;
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return false;
}


void shader_block::set_shader_parameter (const std::string& Name, const bool State) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			i->m_shader_parameter = State;
			return;
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;
}


bool shader_block::is_input_multi_operator (const std::string& Name) const {

	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			return i->is_multi_operator();
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return false;
}


bool shader_block::set_input_multi_operator_parent (const std::string& Name, const std::string& Parent) {

	for (properties_t::iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			i->m_multi_operator_parent_name = Parent;
			return true;
		}
	}

	log() << error << "unmatched shader block input '" << Name << "' in " << name() << std::endl;

	return false;
}


void shader_block::get_multi_input_child_list (const std::string& Name, std::vector<std::string>& Children) const {

	bool found = false;
	for (properties_t::const_iterator i = m_inputs.begin(); i != m_inputs.end(); ++i) {

		if (i->m_name == Name) {

			found = true;

		} else if (i->m_multi_operator_parent_name == Name) {

			Children.push_back (i->m_name);
		}
	}

	if (!found) {

		log() << error << "input '" << Name << "' not found while listing its child inputs." << std::endl;
	}
}


std::string shader_block::get_unique_input_name (const std::string& Name) const {

	if (!is_input (Name))
		// name is already unique
		return Name;

	// append a number to make it unique
	for (int i = 2; ; i++) {

		const std::string new_name = Name + "_" + string_cast (i);

		if (!is_input (new_name))
			return new_name;
	}
}


unsigned long shader_block::input_count() const {

	unsigned long count = 0;
	for (properties_t::const_iterator input = m_inputs.begin(); input != m_inputs.end(); ++input) {

		if (input->m_multi_operator_parent_name.empty()) {

			++count;
		}
	}

	return count;
}


void shader_block::set_position (const double X, const double Y) {

	m_position_x = X;
	m_position_y = Y;
}


void shader_block::roll (const bool Roll) {

	m_rolled = Roll;
}


bool shader_block::is_rolled() const {

	return m_rolled;
}


void shader_block::get_includes (std::set<std::string>& includes) {

	if (m_includes.size()) {

		std::string include_list = m_includes;
		std::istringstream stream (include_list);

		std::string new_include;
		stream >> new_include;
		includes.insert (new_include);

		while (!stream.eof()) {

			stream >> new_include;
			includes.insert (new_include);
		}
	}
}


void shader_block::dump_structure() const {

	std::cerr << "Block dump:" << std::endl;
	std::cerr << "  name        : " << name() << std::endl;
	std::cerr << "  description : " << m_description << std::endl;
	std::cerr << "  author      : " << m_author << std::endl;
	std::cerr << "  usage       : " << m_usage << std::endl;
	std::cerr << "  root        : " << m_root_block << std::endl;
	std::cerr << "  -----------" << std::endl;
	std::cerr << "  input properties:" << std::endl;

	for (shader_block::properties_t::const_iterator input = m_inputs.begin();
		input != m_inputs.end();
		++input) {

		const std::string input_name = input->m_name;
	std::cerr << "	" << input_name << " (" << input_type (input_name) << ") = " << get_input_value (input_name) << std::endl;
		if (!input->m_multi_operator.empty())
			std::cerr << "    Multi : " << input->m_multi_operator << std::endl;
	}

	std::cerr << "  output properties:" << std::endl;
	for (shader_block::properties_t::const_iterator output = m_outputs.begin();
		output != m_outputs.end();
		++output) {

		const std::string output_name = output->m_name;
	std::cerr << "	" << output_name << " (" << output_type (output_name) << ") = " << "type" << std::endl;
	}

}


bool shader_block::load_from_xml (TiXmlNode& XML) {

	// keep end-of-lines when loading code
	XML.SetCondenseWhiteSpace (false);

	// get block information
	for (TiXmlAttribute* a = XML.ToElement()->FirstAttribute(); a; a = a->Next()) {

		const std::string name (a->Name());
		if (name == "type") {

			// skip it
		}
		else if (name == "name") {

			m_name = a->Value();
		}
		else if (name == "description") {

			m_description = a->Value();
		}
		else if (name == "author") {

			m_author = a->Value();
		}
		else if (name == "rolled") {

			const std::string rolled_value = a->Value();
			if (rolled_value == "1") {
				m_rolled = true;
			}
		}
		else if (name == "id" || name == "position_x" || name == "position_y"
			|| name == "root") {

			// ignore them
		}
		else {
			log() << warning << "unhandled block attribute : '" << name << "'." << std::endl;
		}
	}

	// check name
	if (!m_name.size()) {

		// empty name
		log() << error << "block has no name!" << std::endl;
		return false;
	}

	// load properties
	for (TiXmlNode* c = XML.FirstChild(); c; c = c->NextSibling()) {

		const std::string element (c->Value());
		if (element == "input") {

			std::string input_name ("");
			std::string input_type ("");
			std::string input_type_extension ("");
			std::string input_storage ("");
			std::string input_description ("");
			std::string input_value ("");
			std::string input_multi_operator ("");
			std::string input_multi_operator_parent ("");
			bool shader_parameter_input = false;
			std::string input_type_parent = "";

			for (TiXmlAttribute* a = c->ToElement()->FirstAttribute(); a; a = a->Next()) {

				const std::string name (a->Name());
				if (name == "name") {
					input_name = a->Value();
				}
				else if (name == "type") {
					input_type = a->Value();
				}
				else if (name == "type_extension") {
					input_type_extension = a->Value();
				}
				else if (name == "storage") {
					input_storage = a->Value();
				}
				else if (name == "shader_parameter") {
					const std::string state = a->Value();
					shader_parameter_input = !state.empty();
				}
				else if (name == "description") {
					input_description = a->Value();
				}
				else if (name == "default" || name == "value") {
					input_value = a->Value();
				}
				else if (name == "multi") {
					input_multi_operator = a->Value();
				}
				else if (name == "multi_parent") {
					input_multi_operator_parent = a->Value();
				}
				else if (name == "type_parent") {
					input_type_parent = a->Value();
				}
				else
					log() << error << "unhandled input attribute : '" << name << "'" << std::endl;
			}

			add_input (input_name, input_type, input_type_extension, input_storage, input_description, input_value, input_multi_operator, shader_parameter_input);
			if (!input_multi_operator_parent.empty()) {
				set_input_multi_operator_parent (input_name, input_multi_operator_parent);
			}
			set_input_type_parent (input_name, input_type_parent);
		}
		else if (element == "output") {

			std::string output_name = "";
			std::string output_type = "";
			std::string output_type_extension ("");
			std::string output_storage = "";
			std::string output_description = "";
			bool shader_output = false;

			for (TiXmlAttribute* a = c->ToElement()->FirstAttribute(); a; a = a->Next()) {

				const std::string name (a->Name());
				if (name == "name")
					output_name = a->Value();
				else if (name == "type") {
					output_type = a->Value();
				}
				else if (name == "type_extension") {
					output_type_extension = a->Value();
				}
				else if (name == "description") {
					output_description = a->Value();
				}
				else if (name == "storage") {
					output_storage = a->Value();
				}
				else if (name == "shader_output") {
					// DON'T WRITE 'if (a->Value() != "")' it always returns true!
					const std::string value = a->Value();
					if (value != "") {
						shader_output = true;
					}
				}
				else
					log() << error << "unhandled output attribute : '" << name << "'" << std::endl;
			}

			add_output (output_name, output_type, output_type_extension, output_storage, output_description, shader_output);
		}
		else if (element == "rsl_include") {

			if (c->FirstChild()) {
				set_includes (c->FirstChild()->ToText()->Value());
			}
		}
		else if (element == "rsl_code") {

			if (c->FirstChild()) {
				set_code (c->FirstChild()->ToText()->Value());
			}
		}
		else if (element == "vex_include") {

			//No warning on VEX block
			//if (c->FirstChild()) {
			//	set_includes (c->FirstChild()->ToText()->Value());
			//}
		}
		else if (element == "vex_code") {

			//No warning on VEX block
			//if (c->FirstChild()) {
			//	set_code (c->FirstChild()->ToText()->Value());
			//}
		}
		else if (element == "usage") {

			if (c->FirstChild()) {
				set_usage (c->FirstChild()->ToText()->Value());
			}
		}
		else {

			log() << warning << "unknown block element '" << element << "'" << std::endl;
		}
	}

	return true;
}


shader_block* shader_block_builder::build_block (const std::string& FilePath) {

	// open block file
	TiXmlDocument xml_block (FilePath.c_str());
	if (!xml_block.LoadFile()) {
		log() << error << "couldn't load block " << FilePath << ", is it a valid XML file?" << std::endl;
		return 0;
	}

	// check for block type
/*
				xml::attributes_t attributes;
				xml::get_attributes (*c->ToElement(), attributes);

				if (attributes.find ("name") == attributes.end()) {

					log() << error << type << " has no 'name' attribute!" << std::endl;
					continue;
				}

				const std::string name = attributes["name"];
*/
	std::string type ("");
	for (TiXmlAttribute* a = xml_block.FirstChild()->ToElement()->FirstAttribute(); a; a = a->Next()) {

		const std::string name (a->Name());
		if (name == "type") {

			type = a->Value();
		}
	}

	if (type != "function") {

		log() << error << "unrecognized block type : '" << type << "' in " << FilePath << std::endl;
		return 0;
	}

	// create block
	shader_block* new_block = new shader_block (FilePath, "");

	// load block information
	if (!new_block->load_from_xml (*xml_block.FirstChild())) {

		delete new_block;
		new_block = 0;
	}

	return new_block;
}


