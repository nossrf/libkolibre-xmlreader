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


#ifndef XMLDEFAULTHANDLER_H
#define XMLDEFAULTHANDLER_H

#ifdef WIN32
#ifdef KOLIBRE_DLL
#define KOLIBRE_API __declspec(dllexport)
#else
#define KOLIBRE_API __declspec(dllimport)
#endif
#else
#define KOLIBRE_API
#endif

#include "XmlReader.h"
#include <libxml/xmlstring.h>

class KOLIBRE_API XmlDefaultHandler: public XmlContentHandler,
        public XmlLexicalHandler,
        public XmlErrorHandler,
        public XmlDeclHandler,
        public XmlDTDHandler
{
public:
    virtual ~XmlDefaultHandler()
    {
    }
    ;
    bool startDocument();
    bool endDocument();
    bool startPrefixMapping(const xmlChar* const, const xmlChar* const);
    bool endPrefixMapping(const xmlChar* const);
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes &);
    bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool characters(const xmlChar* const, const unsigned int);
    bool ignorableWhitespace(const xmlChar* const);
    bool processingInstruction(const xmlChar* const, const xmlChar* const);
    bool skippedEntity(const xmlChar* const);

    bool startDTD(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool endDTD();
    bool startEntity(const xmlChar* const);
    bool endEntity(const xmlChar* const);
    bool startCDATA();
    bool endCDATA();
    bool comment(const xmlChar* const);

    bool warning(const XmlError &);
    bool error(const XmlError &);
    bool fatalError(const XmlError &);

    bool attributeDecl(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const xmlChar* const, const xmlChar* const);
    bool externalEntityDecl(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool internalEntityDecl(const xmlChar* const, const xmlChar* const);

    bool notationDecl(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool unparsedEntityDecl(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const xmlChar* const);

    std::string errorString();
};

#endif
