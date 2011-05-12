
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


#ifndef _misc_xml_h_
#define _misc_xml_h_

#include "tinyxml/tinystr.h"
#include "tinyxml/tinyxml.h"

#include <map>
#include <string>
#include <vector>

namespace xml
{

typedef std::map<std::string, std::string> attributes_t;
void get_attributes(TiXmlElement& Element, attributes_t& Attributes);
bool get_attribute(TiXmlElement& Element, const std::string& Name, std::string& Value);

// XML output
struct attribute
{
	attribute(const std::string& Name, const std::string& Value) : name(Name), value(Value) {}

	std::string name;
	std::string value;
};

class element
{
public:
	element(const std::string& Name) : name(Name) {}

	void set_text(const std::string& Value) { text = Value; }

	void push_attribute(const std::string& Name, const std::string& Value) { attributes.push_back(attribute(Name, Value)); }
	template<typename T>
	void push_attribute(const std::string& Name, const T& Value) { push_attribute(Name, string_cast(Value)); }

	void push_child(const element& Value) { children.push_back(Value); }

	std::string name;
	std::string text;

	typedef std::vector<attribute> attributes_t;
	attributes_t attributes;
	typedef std::vector<element> elements_t;
	elements_t children;
};

class output_tree
{
public:
    output_tree (element& Element, std::ostream& Stream) :
        m_stream(Stream),
	m_indentation(0)
    {
        output(Element);
    }

private:
    std::ostream& m_stream;
    unsigned int m_indentation;

    void output(const element& Element);
    void output(const attribute& Attribute);
    void output_indentation();

    const std::string encode(const std::string& String);
};

} // namespace xml

#endif // _misc_xml_h_

