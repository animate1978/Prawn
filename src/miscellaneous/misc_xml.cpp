
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


#include "misc_xml.h"

#include <fstream>
#include <iterator>

namespace xml
{

void get_attributes(TiXmlElement& Element, attributes_t& Attributes)
{
	for(TiXmlAttribute* a = Element.FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		std::string value(a->Value());
		Attributes.insert(std::make_pair(name, value));
	}
}

bool get_attribute(TiXmlElement& Element, const std::string& Name, std::string& Value)
{
	for(TiXmlAttribute* a = Element.FirstAttribute(); a; a = a->Next())
	{
		if(Name != a->Name())
			continue;

		Value = a->Value();
		return true;
	}

	return false;
}

void output_tree::output(const element& Element)
{
    output_indentation();
    m_stream << "<" << Element.name;

    for(element::attributes_t::const_iterator a = Element.attributes.begin(); a != Element.attributes.end(); ++a)
        output(*a);

    if(Element.children.empty() && Element.text.empty())
    {
        m_stream << "/>" << std::endl;
        return;
    }

    m_stream << ">" << encode(Element.text);

    if(Element.children.size())
    {
        m_stream << std::endl;
        ++m_indentation;

        for(element::elements_t::const_iterator e = Element.children.begin(); e != Element.children.end(); ++e)
            output(*e);

        --m_indentation;
    }

    output_indentation();
    m_stream << "</" << Element.name << ">" << std::endl;
}

void output_tree::output(const attribute& Attribute)
{
    m_stream << " " << Attribute.name << "=\"" << encode(Attribute.value) << "\"";
}

void output_tree::output_indentation()
{
    m_stream << std::string(m_indentation, '\t');
}

const std::string output_tree::encode(const std::string& String)
{
    std::string result;
    result.reserve(String.size());

    for(std::string::size_type i = 0; i != String.size(); ++i)
    {
        if(String[i] == '&')
            result += "&amp;";
        else if(String[i] == '<')
            result += "&lt;";
        else if(String[i] == '>')
            result += "&gt;";
        else if(String[i] == '\'')
            result += "&apos;";
        else if(String[i] == '\"')
            result += "&quot;";
        else
            result += String[i];
    }

    return result;
}

} // namespace xml

