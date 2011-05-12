
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


#ifndef _logging_h_
#define _logging_h_

#include <ostream>

// available log levels
typedef enum
{
	ERROR = 1,
	WARNING = 2,
	INFO = 3,
	DEBUG = 4,
	ASPECT = 5
} log_level_t;

std::ostream& log();

std::ostream& aspect(std::ostream&);
std::ostream& debug(std::ostream&);
std::ostream& error(std::ostream&);
std::ostream& warning(std::ostream&);
std::ostream& info(std::ostream&);

// When attached to an output stream, filters-out messages below the given level
class filter_by_level_buf :
	public std::streambuf
{
public:
	filter_by_level_buf(const log_level_t MinimumLevel, std::ostream& Stream);
	~filter_by_level_buf();

protected:
	int overflow(int);
	int sync();

private:
	std::ostream& m_stream;
	std::streambuf* const m_streambuf;
	const log_level_t m_minimum_level;
};

#endif // _logging_h_

