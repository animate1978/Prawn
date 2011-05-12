
/*
    Copyright 2010, Romain Behar <romainbehar@users.sourceforge.net>

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


#include "rib_root_block_parsing.h"

#include "../miscellaneous/logging.h"
#include "../miscellaneous/misc_string_functions.h"

#include <map>


// poor man's parser to transform valued arrays
typedef std::string::size_type string_pos;

std::vector<std::string> new_code_lines;
std::set<std::string> new_instanciations;


string_pos previous_line_start = 0;


void new_code_line (string_pos n, std::string& new_code)
{
	if (new_instanciations.size() > 0)
	{
		for (std::set<std::string>::iterator i = new_instanciations.begin(); i != new_instanciations.end(); ++i)
		{
			new_code_lines.push_back (*i);
		}
	}

	new_code_lines.push_back (new_code);
	new_code = "";

	previous_line_start = n;
	if (n > 0)
		n--;

	new_instanciations.clear();
}

std::string get_valued_variable_name (std::string name)
{
	return "valued_variable_" + name;
}

void add_declaration (std::string name, std::set<std::string>& local_declarations)
{
	std::string declaration = "$(" + name + ":type) " + get_valued_variable_name (name);
	std::string instanciation = get_valued_variable_name (name) + " = $(" + name + ");\n";

	local_declarations.insert (declaration);
	new_instanciations.insert (instanciation);
}



// Replace:
//   $(nnn)[i]
// with
//   type nnn_i = $(nnn); // to prevent 'normalize(vvv)[i]'
//   nnn_i[i]
std::string create_array_value_variables (const std::string code, std::set<std::string>& local_declarations)
{
	new_code_lines.clear();
	new_instanciations.clear();

	bool single_line_comment = false;
	bool multi_line_comment = false;
	bool preprocessor_directive = false;
	bool in_shrimp_variable = false;
	bool in_shrimp_variable_array_index = false;

	string_pos shrimp_variable_real_start;
	string_pos shrimp_variable_start;
	string_pos shrimp_variable_end;
	std::string shrimp_variable;

	unsigned int parenthesis_level = 0;

	char previous_character = ' ';

	std::string new_code;
	for (std::string::size_type n = 0; n < code.size(); ++n)
	{
		char c = code[n];
		new_code += c;
		if (c == ' '
			|| c == '\n'
			|| c == '\t'
			|| c == '\r'
			|| single_line_comment
			|| multi_line_comment
			)
		{
			if (single_line_comment)
			{
				if (c == '\n' && previous_character != '\\')
				{
					single_line_comment = false;

					// new code line
					string_pos new_code_line_start = new_code.size();

					string_pos p = n;
					char next_c = code[p];
					while ((p < code.size()) && (next_c == '\n' || next_c == '\r'))
					{
						++p;
						++new_code_line_start;

						next_c = code[p];
					}

					new_code_line (new_code_line_start, new_code);
				}
			}

			if (multi_line_comment)
			{
				if (c == '/' && previous_character == '*')
					multi_line_comment = false;
			}

			if (preprocessor_directive)
			{
				if (c == '\n' && previous_character != '\\')
				{
					preprocessor_directive = false;

					// new code line
					string_pos new_code_line_start = new_code.size();

					string_pos p = n;
					char next_c = code[p];
					while ((p < code.size()) && (next_c == '\n' || next_c == '\r'))
					{
						++p;
						++new_code_line_start;

						next_c = code[p];
					}

					new_code_line (new_code_line_start, new_code);
				}
			}

			continue;
		}

		if (c == '/' && previous_character == '/')
		{
			single_line_comment = true;
		}
		else if (c == '*' && previous_character == '/')
		{
			multi_line_comment = true;
		}
		else if (c == '#')
		{
			preprocessor_directive = true;
		}
		else if (c == ';')
		{
			if (parenthesis_level == 0)
			{
				string_pos new_code_line_start = new_code.size();

				string_pos p = n;
				char next_c = code[p];
				while (next_c == '\n' || next_c == '\r')
				{
					++p;
					++new_code_line_start;

					next_c = code[p];
				}

				new_code_line (new_code_line_start, new_code);
			}
		}
		else if (c == '(')
		{
			parenthesis_level++;

			if (previous_character == '$' && !in_shrimp_variable_array_index)
			{
				in_shrimp_variable = true;
				shrimp_variable_real_start = n - 1;
				shrimp_variable_start = n + 1;
			}
		}
		else if (c == ')')
		{
			parenthesis_level--;

			if (in_shrimp_variable)
			{
				in_shrimp_variable = false;
				shrimp_variable_end = n - 1;
				shrimp_variable = trim (std::string (code, shrimp_variable_start, shrimp_variable_end - shrimp_variable_start + 1));
			}
		}
		else if (c == '[' && previous_character == ')' && !in_shrimp_variable_array_index)
		{
			in_shrimp_variable_array_index = true;
		}
		else if (c == ']')
		{
			if (in_shrimp_variable_array_index)
			{
				in_shrimp_variable_array_index = false;

				// replace variable in new code
				add_declaration (shrimp_variable, local_declarations);

				string_pos replacement_start = new_code.size() - (n - shrimp_variable_real_start + 1);
				string_pos replacement_size = shrimp_variable_end - shrimp_variable_real_start + 2;

				new_code.replace (replacement_start, replacement_size, get_valued_variable_name (shrimp_variable));
			}
		}

		previous_character = c;
	}

	// build shader block
	std::string shader_block = "";
	for (std::vector<std::string>::iterator i = new_code_lines.begin(); i != new_code_lines.end(); ++i)
	{
		shader_block += "\t" + *i;
	}

	return shader_block;
}



// split an array to its elements:
//   { 0.1, color (.4, .5, .6), 3, 4 }
// returns:
//   0.1
//   color (.4, .5, .6)
//   3
//   4
std::vector<std::string> split_array (const std::string array)
{
	std::vector<std::string> elements;

	unsigned int parenthesis_level = 0;

	string_pos element_start = 0;
	for (std::string::size_type n = 0; n < array.size(); ++n)
	{
		char c = array[n];

		if (c == ',')
		{
			if (parenthesis_level == 0)
			{
				string_pos element_end = n - 1;
				std::string element = std::string (array, element_start, element_end - element_start + 1);

				elements.push_back (trim (element));

				element_start = n + 1;
			}
		}
		else if (c == '(')
		{
			parenthesis_level++;
		}
		else if (c == ')')
		{
			parenthesis_level--;
		}
	}

	string_pos element_end = array.size() - 1;
	if (element_start < element_end)
	{
		std::string element = std::string (array, element_start, element_end);

		elements.push_back (trim (element));
	}

	return elements;
}


// Replace:
//   array = { v1, v2, v3 };
// with
//   array[0] = v1;
//   array[1] = v2;
//   array[2] = v3;
std::string replace_array_assignations (const std::string code, std::set<std::string>& local_declarations)
{
	new_code_lines.clear();
	new_instanciations.clear();

	bool single_line_comment = false;
	bool multi_line_comment = false;
	bool preprocessor_directive = false;
	bool in_array_initialization = false;

	string_pos lvalue_end;
	string_pos current_code_line_start;
	string_pos array_start;

	unsigned int parenthesis_level = 0;

	char previous_character = ' ';

	std::map<std::string, std::string> replacements;
	std::map<std::string, std::string> replacement_lvalues;

	std::string new_code;
	for (std::string::size_type n = 0; n < code.size(); ++n)
	{
		char c = code[n];
		new_code += c;
		if (c == ' '
			|| c == '\n'
			|| c == '\t'
			|| c == '\r'
			|| single_line_comment
			|| multi_line_comment
			)
		{
			if (single_line_comment)
			{
				if (c == '\n' && previous_character != '\\')
				{
					single_line_comment = false;

					// new code line
					string_pos new_code_line_start = new_code.size();

					string_pos p = n;
					char next_c = code[p];
					while ((p < code.size()) && (next_c == '\n' || next_c == '\r'))
					{
						++p;
						++new_code_line_start;

						next_c = code[p];
					}

					new_code_line (new_code_line_start, new_code);
				}
			}

			if (multi_line_comment)
			{
				if (c == '/' && previous_character == '*')
					multi_line_comment = false;
			}

			if (preprocessor_directive)
			{
				if (c == '\n' && previous_character != '\\')
				{
					preprocessor_directive = false;

					// new code line
					string_pos new_code_line_start = new_code.size();
					current_code_line_start = new_code_line_start;

					string_pos p = n;
					char next_c = code[p];
					while ((p < code.size()) && (next_c == '\n' || next_c == '\r'))
					{
						++p;
						++new_code_line_start;

						next_c = code[p];
					}

					new_code_line (new_code_line_start, new_code);
				}
			}

			continue;
		}

		if (c == '/' && previous_character == '/')
		{
			single_line_comment = true;
		}
		else if (c == '*' && previous_character == '/')
		{
			multi_line_comment = true;
		}
		else if (c == '#')
		{
			preprocessor_directive = true;
		}
		else if (c == ';')
		{
			if (parenthesis_level == 0)
			{
				string_pos new_code_line_start = new_code.size();
				current_code_line_start = n + 1;

				string_pos p = n;
				char next_c = code[p];
				while (next_c == '\n' || next_c == '\r')
				{
					++p;
					++new_code_line_start;

					next_c = code[p];
				}

				new_code_line (new_code_line_start, new_code);
			}
		}
		else if (c == '=')
		{
			lvalue_end = n - 1;
		}
		else if (c == '(')
		{
			parenthesis_level++;
		}
		else if (c == ')')
		{
			parenthesis_level--;
		}
		else if (c == '{')
		{
			if (previous_character == '=')
			{
				// found array inialization
				in_array_initialization = true;
				array_start = n + 1;
			}
		}
		else if (c == '}')
		{
			if (in_array_initialization)
			{
				// found array initialization's end
				in_array_initialization = false;

				// store l-value
				std::string lvalue = std::string (code, current_code_line_start, lvalue_end - current_code_line_start + 1);
				lvalue = trim (lvalue);

				// put array in a string
				string_pos array_end = n - 1;
				std::string array = std::string (code, array_start, array_end - array_start + 1);

				string_pos replacement_start = current_code_line_start;
				string_pos replacement_size = array_end - replacement_start + 2;

				std::string replacement = std::string (code, replacement_start, replacement_size);

				// save for later replacement
				replacements[replacement] = array;
				replacement_lvalues[replacement] = lvalue;
			}
		}

		previous_character = c;
	}

	// build shader block
	std::string shader_block = "";
	for (std::vector<std::string>::iterator i = new_code_lines.begin(); i != new_code_lines.end(); ++i)
	{
		std::string line = *i;

		// quick'n'dirty replacements
		for (std::map<std::string, std::string>::iterator repl = replacements.begin(); repl != replacements.end(); ++repl)
		{
			std::string orig = repl->first;
			std::string array = repl->second;
			std::string lvalue = replacement_lvalues[orig];

			std::string::size_type repl_start = line.find (orig);
			if (repl_start != std::string::npos)
			{
				// split array
				std::vector<std::string> array_values = split_array (array);

				// build replacement
				std::string new_array = "\n";
				int array_index = 0;
				for (std::vector<std::string>::const_iterator val = array_values.begin(); val != array_values.end(); ++val)
				{
					if (array_index > 0)
					{
						new_array += ";\n";
					}

					new_array += "\t" + lvalue + "[" + string_cast (array_index++) + "] = " + *val;
				}

				// replace
				line.replace (repl_start, orig.size(), new_array);
			}
		}

		// save line
		shader_block += "\t" + line;
	}

	return shader_block;
}


