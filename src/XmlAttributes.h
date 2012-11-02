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

#ifndef XMLATTRIBUTES_H
#define XMLATTRIBUTES_H

#ifdef WIN32
#ifdef KOLIBRE_DLL
#define KOLIBRE_API __declspec(dllexport)
#else
#define KOLIBRE_API __declspec(dllimport)
#endif
#else
#define KOLIBRE_API
#endif

#include <libxml/xmlstring.h>
#include <malloc.h>

struct XmlNamespace;

class KOLIBRE_API XmlAttributes
{
public:
    XmlAttributes();
    XmlAttributes(const xmlChar ** const aptr);
    ~XmlAttributes();

    /**
     * Get length of XML string
     *
     * @return length of the string
     */
    int length() const
    {
        return _length;
    }

    /**
     * Get length of XML string
     *
     * @return length of the string
     */
    int getLength() const
    {
        return _length;
    }

    const xmlChar *qName(int index) const;
    const xmlChar *getQName(int index) const;

    const xmlChar *localName(int index) const;
    const xmlChar *value(int index) const;
    const xmlChar *getValue(int index) const;

    const xmlChar *value(const xmlChar *name) const;
    const xmlChar *getValue(const xmlChar *qName) const;

    void split(XmlNamespace* ns);

private:
    // unimplemented
    XmlAttributes(const XmlAttributes &);
    XmlAttributes &operator=(const XmlAttributes &);

    int _length;
    const xmlChar ** _attributes;
};

#endif
