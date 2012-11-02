/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
 
 /*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-xmlreader.
 *
 * Kolibre-xmlreader is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-xmlreader is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-xmlreader. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \class XmlAttributes
 *
 * \brief Class for parsing and storing in elements
 */

#include "XmlAttributes.h"
#include "XmlReader.h"

#define _names(i) _attributes[i*2]
#define _values(i) _attributes[i*2+1]

/**
 * Constructor
 */
XmlAttributes::XmlAttributes() :
        _length(0), _attributes(0)
{
}

/**
 * Constructor
 *
 * @param aptr pointer to XML string pointer
 */
XmlAttributes::XmlAttributes(const xmlChar ** const aptr) :
        _length(0), _attributes(0)
{
    if (aptr)
    {
        //cout << "Creating attribute class" << endl;
        _attributes = aptr;
        for (const xmlChar **p = aptr; *p && p[1] != NULL; p += 2)
        {
            //cout << "name: " << _names(_length) << " value: " << _values(_length) << endl;
            _length++;
        }
    }
}

/**
 * Destructor
 */
XmlAttributes::~XmlAttributes()
{
}

/**
 * Get attribute name by index
 *
 * @param index index of the attribute
 * @return pointer to the name
 */
const xmlChar *XmlAttributes::localName(int index) const
{
    const xmlChar *colonPtr = xmlStrstr(_names(index), (xmlChar *) ":");
    if (colonPtr != NULL)
        // Peel off the prefix to return the localName.
        return _names(index) + (_names(index) - colonPtr);

    return _names(index);
}

/**
 * Get attribute value by name
 *
 * @param name pointer to the name
 * @return pointer to the value
 * @retval NULL if name not found
 */
const xmlChar *XmlAttributes::value(const xmlChar *name) const
{
    //cout << "Getting value for attribute " << name;
    for (int i = 0; i != _length; ++i)
    {
        if (xmlStrcmp(name, _names(i)) == 0)
        {
            //cout << ": " << _values(i) << endl;
            return _values(i);
        }
    }
    //cout << ": not found" << endl;
    return NULL;
}

/**
 * Get attribute value by name
 *
 * @param name pointer to the name
 * @return pointer to the value
 * @retval NULL if name not found
 */
const xmlChar *XmlAttributes::getValue(const xmlChar *name) const
{
    //cout << "Getting value for attribute " << name;
    for (int i = 0; i != _length; ++i)
    {
        if (xmlStrcmp(name, _names(i)) == 0)
        {
            //cout << ": " << _values(i) << endl;
            return _values(i);
        }
    }
    //cout << ": not found" << endl;
    return NULL;
}

/**
 * Split namespaces
 *
 * Disabled to increase performance
 *
 * @param ns pointer to namespace
 */
void XmlAttributes::split(XmlNamespace* ns)
{
    return; // disabled for speed
}

/**
 * Get attribute value by index
 *
 * @param index index of the attribute
 * @return pointer to the value
 */
const xmlChar *XmlAttributes::value(int index) const
{
    //cout << "returning value " << _values(index) << endl;;
    return _values(index);
}

/**
 * Get attribute value by index
 *
 * @param index index of the attribute
 * @return pointer to the value
 */
const xmlChar *XmlAttributes::getValue(int index) const
{
    //cout << "returning value " << _values(index) << endl;;
    return _values(index);
}

/**
 * Get attribute name by index
 *
 * @param index index of the attribute
 * @return pointer to the name
 */
const xmlChar *XmlAttributes::qName(int index) const
{
    //cout << "returning qname " << _names(index) << endl;;
    return _names(index);
}

/**
 * Get attribute name by index
 * @param index index of the attribute
 * @return pointer to the name
 */
const xmlChar *XmlAttributes::getQName(int index) const
{
    //cout << "returning qname " << _names(index) << endl;;
    return _names(index);
}
