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
 * \class XmlDefaultHandler
 *
 * \brief A default handler implementing all types off handlers
 */

#include "XmlDefaultHandler.h"

/**
 * Implements startDocument via XmlContentHandler
 *
 * @return true
 */
bool XmlDefaultHandler::startDocument()
{
    return true;
}

/**
 * Implements endDocument via XmlContentHandler
 *
 * @return true
 */
bool XmlDefaultHandler::endDocument()
{
    return true;
}

/**
 * Implements startPrefixMapping via XmlContentHandler
 *
 * @param prefix
 * @param URI
 * @return true
 */
bool XmlDefaultHandler::startPrefixMapping(const xmlChar* const prefix,
        const xmlChar* const URI)
{
    return true;
}

/**
 * Implements endPrefixMapping via XmlContentHandler
 *
 * @param prefix
 * @return true
 */
bool XmlDefaultHandler::endPrefixMapping(const xmlChar* const prefix)
{
    return true;
}

/**
 * Implements startElement via XmlContentHandler
 *
 * @param namespaceURI
 * @param localName
 * @param qName
 * @param attributes
 * @return true
 */
bool XmlDefaultHandler::startElement(const xmlChar* const namespaceURI,
        const xmlChar* const localName, const xmlChar* const qName,
        const XmlAttributes &attributes)
{
    return true;
}

/**
 * Implements endElement via XmlContentHandler
 *
 * @param namespaceURI
 * @param localName
 * @param qName
 * @return true
 */
bool XmlDefaultHandler::endElement(const xmlChar* const namespaceURI,
        const xmlChar* const localName, const xmlChar* const qName)
{
    return true;
}

/**
 * Implements characters via XmlContentHandler
 *
 * @param characters
 * @param length
 * @return true
 */
bool XmlDefaultHandler::characters(const xmlChar* const characters,
        const unsigned int length)
{
    return true;
}

/**
 * Implements ignorableWhitespace via XmlContentHandler
 *
 * @param characters
 * @return true
 */
bool XmlDefaultHandler::ignorableWhitespace(const xmlChar* const characters)
{
    return true;
}

/**
 * Implements processingInstruction via XmlContentHandler
 *
 * @param target
 * @param data
 * @return true
 */
bool XmlDefaultHandler::processingInstruction(const xmlChar* const target,
        const xmlChar* const data)
{
    return true;
}

/**
 * Implements skippedEntity via XmlContentHandler
 *
 * @param name
 * @return true
 */
bool XmlDefaultHandler::skippedEntity(const xmlChar* const name)
{
    return true;
}

/**
 * Implements startDTD via XmlLexicalHandler
 *
 * @param name
 * @param publicId
 * @param systemId
 * @return true
 */
bool XmlDefaultHandler::startDTD(const xmlChar* const name,
        const xmlChar* const publicId, const xmlChar* const systemId)
{
    return true;
}

/**
 * Implements endDTD via XmlLexicalHandler
 *
 * @return true
 */
bool XmlDefaultHandler::endDTD()
{
    return true;
}

/**
 * Implements startEntity via XmlLexicalHandler
 *
 * @param name
 * @return true
 */
bool XmlDefaultHandler::startEntity(const xmlChar* const name)
{
    return true;
}

/**
 * Implements endEntity via XmlLexicalHandler
 *
 * @param name
 * @return true
 */
bool XmlDefaultHandler::endEntity(const xmlChar* const name)
{
    return true;
}

/**
 * Implements startCDATA via XmlLexicalHandler
 *
 * @return true
 */
bool XmlDefaultHandler::startCDATA()
{
    return true;
}

/**
 * Implements endCDATA via XmlLexicalHandler
 *
 * @return true
 */
bool XmlDefaultHandler::endCDATA()
{
    return true;
}

/**
 * Implements comment via XmlLexicalHandler
 *
 * @param characters
 * @return true
 */
bool XmlDefaultHandler::comment(const xmlChar* const characters)
{
    return true;
}

/**
 * Implements warning via XmlErrorHandler
 *
 * @param e
 * @return true
 */
bool XmlDefaultHandler::warning(const XmlError &e)
{
    return true;
}

/**
 * Implements error via XmlErrorHandler
 *
 * @param e
 * @return true
 */
bool XmlDefaultHandler::error(const XmlError &e)
{
    return true;
}

/**
 * Implements fatalError via XmlErrorHandler
 *
 * @param e
 * @return true
 */
bool XmlDefaultHandler::fatalError(const XmlError &e)
{
    return true;
}

/**
 * Implements attributeDecl via XmlDeclHandler
 *
 * @param entityName
 * @param attributeName
 * @param type
 * @param valueDefault
 * @param value
 * @return true
 */
bool XmlDefaultHandler::attributeDecl(const xmlChar* const entityName,
        const xmlChar* const attributeName, const xmlChar* const type,
        const xmlChar* const valueDefault, const xmlChar* const value)
{
    return true;
}

/**
 * Implements externalEntityDecl via XmlDeclHandler
 *
 * @param name
 * @param publicId
 * @param systemId
 * @return true
 */
bool XmlDefaultHandler::externalEntityDecl(const xmlChar* const name,
        const xmlChar* const publicId, const xmlChar* const systemId)
{
    return true;
}

/**
 * Implements internalEntityDecl via XmlDeclHandler
 *
 * @param name
 * @param value
 * @return true
 */
bool XmlDefaultHandler::internalEntityDecl(const xmlChar* const name,
        const xmlChar* const value)
{
    return true;
}

/**
 * Implements notationDecl via XmlDTDHandler
 *
 * @param name
 * @param publicId
 * @param systemId
 * @return true
 */
bool XmlDefaultHandler::notationDecl(const xmlChar* const name,
        const xmlChar* const publicId, const xmlChar* const systemId)
{
    return true;
}

/**
 * Implements unparsedEntityDecl via XmlDTDHandler
 *
 * @param name
 * @param publicId
 * @param systemId
 * @param notationName
 * @return true
 */
bool XmlDefaultHandler::unparsedEntityDecl(const xmlChar* const name,
        const xmlChar* const publicId, const xmlChar* const systemId,
        const xmlChar* const notationName)
{
    return true;
}

/**
 * Implements errorString via XmlDTDHandler
 *
 * @return empty string
 */
std::string XmlDefaultHandler::errorString()
{
    return "";
}
