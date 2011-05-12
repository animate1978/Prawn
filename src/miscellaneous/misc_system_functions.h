
/*
    Copyright 2008, Romain Behar <romainbehar@users.sourceforge.net>

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


#ifndef _misc_system_functions_h_
#define _misc_system_functions_h_

#include <fltk/filename.h>

#include <string>

namespace system_functions
{

// return user's directory for Shrimp
const std::string get_shrimp_user_directory();

// return user's temporary directory
const std::string get_tmp_directory();

// return absolute path from given one
const std::string get_absolute_path(const std::string& Path);

// execute a command
bool execute_command(const std::string& Command);

} // namespace system_functions

#endif // _misc_system_functions_h_

