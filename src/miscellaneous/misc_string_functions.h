
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


#ifndef _misc_string_functions_h_
#define _misc_string_functions_h_

#include <limits>
#include <sstream>
#include <string>
#include <vector>

// string_cast

// Converts any serializable type to a string
template<typename type>
const std::string string_cast(const type& Value)
{
	std::ostringstream buffer;
	if(std::numeric_limits<type>::is_specialized)
	        buffer.precision(std::numeric_limits<type>::digits10);

	buffer << Value;
	return buffer.str();
}

// Specialization of string_cast for type bool
template<>
inline const std::string string_cast<bool>(const bool& Value)
{
	return Value ? "true" : "false";
}

// from_string

// Converts a string into any serializeable type
template<typename type>
const type from_string(const std::string& Value, const type& Default)
{
	type result = Default;
	std::istringstream stream(Value.c_str());
	stream >> result;

	return result;
}

// Specialization of from_string for type bool
template<>
inline const bool from_string(const std::string& Value, const bool& Default)
{
	if(Value == "true")
		return true;

	if(Value == "false")
		return false;

	return Default;
}

// Other functions

inline void replace_variable (std::string& Text, const std::string& Old, const std::string& New)
{
	size_t found;
	size_t pos = 0;

	do
	{
		found = Text.find(Old, pos);
		if(found != Text.npos)
		{
			Text.replace(found, Old.size(), New);
			pos = found + New.size();
		}
	} while(found != Text.npos);

}

inline std::string change_file_extension(const std::string& File, const std::string& NewExtension)
{
    // also works when no dot is found
    return std::string(File, 0, File.rfind('.')) + '.' + NewExtension;
}

inline std::string trim (const std::string& String)
{
	if(String.empty())
		return String;

	const char to_remove[] = " \t\n";

	std::string trimmed(String);

	std::string::size_type beginning = trimmed.find_first_not_of(to_remove);
	if(beginning != std::string::npos)
		trimmed.erase(0, beginning);

	std::string::size_type end = trimmed.find_last_not_of(to_remove);
	if(end != std::string::npos)
		trimmed.erase(end+1);

	return trimmed;
}

inline std::string remove_all_spaces (const std::string& String)
{
	if(String.empty())
		return String;

	std::string no_space (String);
	const char to_remove[] = " \t\n";

	size_t found;
	size_t pos = 0;

	do
	{
		found = no_space.find_first_of (to_remove);
		if (found != no_space.npos)
		{
			no_space.replace (found, 1, "");
			pos = found;
		}
	}
	while (found != no_space.npos);

	return no_space;
}

inline bool is_alpha (const char c)
{
	if (c == '_')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	if (c >= 'a' && c <= 'z')
		return true;

	return false;
}

inline bool is_num (const char c)
{
	return (c >= '0' && c <= '9');
}

// split a string using the separator
inline std::vector<std::string> split (char separator, const std::string to_split)
{
	size_t found;
	size_t pos = 0;

	std::vector<std::string> tokens;
	do
	{
		found = to_split.find (separator, pos);
		if (found != to_split.npos)
		{
			if (found > pos)
			{
				tokens.push_back (std::string (to_split, pos, found - pos));
			}
			pos = found + 1;
		}
	}
	while (found != to_split.npos);

	if (pos < to_split.size())
	{
		tokens.push_back (std::string (to_split, pos));
	}

	return tokens;
}

#endif // _misc_string_functions_h_

