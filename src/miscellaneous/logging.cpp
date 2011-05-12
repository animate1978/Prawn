
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


#include "logging.h"

#include <iostream>

namespace detail
{

int log_level_index() {

	static int index = std::ios::xalloc();
	return index;
}

long& log_level(std::ostream& Stream) {

	return Stream.iword (log_level_index());
}

} // namespace detail

std::ostream& log() {

	detail::log_level (std::cerr) = 0;
	return std::cerr;
}

std::ostream& aspect (std::ostream& Stream) {

	detail::log_level (Stream) = ASPECT;
	Stream << " Aspect : ";
	return Stream;
}

std::ostream& debug (std::ostream& Stream) {

	detail::log_level (Stream) = DEBUG;
	Stream << " Debug : ";
	return Stream;
}

std::ostream& error (std::ostream& Stream) {

	detail::log_level (Stream) = ERROR;
	Stream << " Error : ";
	return Stream;
}

std::ostream& warning (std::ostream& Stream) {

	detail::log_level (Stream) = WARNING;
	Stream << " Warning : ";
	return Stream;
}

std::ostream& info (std::ostream& Stream) {

	detail::log_level (Stream) = INFO;
	Stream << " Info : ";
	return Stream;
}

// filter_by_level_buf
filter_by_level_buf::filter_by_level_buf (const log_level_t MinimumLevel, std::ostream& Stream) :
		m_stream (Stream),
		m_streambuf (Stream.rdbuf()),
		m_minimum_level (MinimumLevel)
{
	setp(0, 0);
	m_stream.rdbuf(this);
}

filter_by_level_buf::~filter_by_level_buf() {

	m_stream.rdbuf(m_streambuf);
}

int filter_by_level_buf::overflow (int c) {

	if (detail::log_level (m_stream) <= m_minimum_level)
		return m_streambuf->sputc (c);

	return c;
}

int filter_by_level_buf::sync() {

	m_streambuf->pubsync();
	return 0;
}


