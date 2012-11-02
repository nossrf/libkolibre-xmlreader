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

#ifndef XMLREADER_H
#define XMLREADER_H

#ifdef WIN32
#ifdef KOLIBRE_DLL
#define KOLIBRE_API __declspec(dllexport)
#else
#define KOLIBRE_API __declspec(dllimport)
#endif
#else
#define KOLIBRE_API
#endif

#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlstring.h>
#include <stack>
#include <string>

/**
 * Struct for storing a namespace
 */
struct XmlNamespace
{
    xmlChar *m_prefix; /**< pointer to namespace prefix*/
    xmlChar *m_uri; /**< pointer to namespace uri*/
    XmlNamespace* m_parent; /**< pointer to namespace parent namespace*/
    int m_ref; /**< indicator if the namespace is referred to*/

    /**
     * Constructor
     */
    XmlNamespace() :
            m_parent(0), m_ref(0)
    {
        m_prefix = xmlStrdup((xmlChar*) "");
        m_uri = xmlStrdup((xmlChar*) "");
    }

    /**
     * Constructor
     * @param p pointer to namespace prefix
     * @param u pointer to namespace uri
     * @param parent pointer to parent namespace
     */
    XmlNamespace(const xmlChar * p, const xmlChar * u, XmlNamespace* parent) :
            m_prefix(0), m_uri(0), m_parent(parent), m_ref(0)
    {
        if (p)
            m_prefix = xmlStrdup(p);
        else
            m_prefix = xmlStrdup((xmlChar*) "");
        if (u)
            m_uri = xmlStrdup(u);
        else
            m_uri = xmlStrdup((xmlChar*) "");
        if (m_parent)
            m_parent->ref();
    }

    /**
     * Get uri for prefix
     *
     * @param prefix pointer to prefix for which uri is requested
     * @return pointer to the namespace uri
     * @retval NULL if prefix not in namespace
     */
    const xmlChar *uriForPrefix(const xmlChar *prefix) const
    {
        if (xmlStrncmp(prefix, m_prefix, xmlStrlen(m_prefix)) == 0)
        {
            return m_uri;
        }
        if (m_parent)
            return m_parent->uriForPrefix(prefix);
        return NULL;
    }

    /**
     * Mark that this namespace is referred to
     */
    void ref()
    {
        m_ref++;
    }

    /**
     * Mark that this namespace is not referred to
     *
     * The namespace is removed along with other namespaces which are no longer referred to.
     */
    void deref()
    {
        if (--m_ref <= 0)
        {
            if (m_parent)
                m_parent->deref();
            free(m_prefix);
            free(m_uri);
            delete this;
        }
    }
};

/**
 * Struct for storing namespaces in a stack
 */
struct nsStackItem
{
    XmlNamespace *ns; /**< pointer to namespace in stack item*/
    struct nsStackItem *prev; /**< pointer to the previous stack item */
};

class XmlAttributes;
class XmlError;

/**
 * \class XmlContentHandler
 *
 * \brief Interface for XML content handler
 */
class KOLIBRE_API XmlContentHandler
{
public:
    virtual ~XmlContentHandler()
    {
    }
    ;
    virtual bool startDocument() = 0;
    virtual bool endDocument() = 0;
    virtual bool startPrefixMapping(const xmlChar* const prefix,
            const xmlChar* const URI) = 0;
    virtual bool endPrefixMapping(const xmlChar* const prefix) = 0;
    virtual bool startElement(const xmlChar* const namespaceURI,
            const xmlChar* const localName, const xmlChar* const qName,
            const XmlAttributes &attributes) = 0;
    virtual bool endElement(const xmlChar* const namespaceURI,
            const xmlChar* const localName, const xmlChar* const qName) = 0;
    virtual bool characters(const xmlChar* const characters,
            const unsigned int length) = 0;
    virtual bool ignorableWhitespace(const xmlChar* const characters) = 0;
    virtual bool processingInstruction(const xmlChar* const target,
            const xmlChar* const data) = 0;
    virtual bool skippedEntity(const xmlChar* const name) = 0;
    virtual std::string errorString() = 0;
};

/**
 * \class XmlLexicalHandler
 *
 * \brief Interface for XML lexical handler
 */
class KOLIBRE_API XmlLexicalHandler
{
public:
    virtual ~XmlLexicalHandler()
    {
    }
    ;
    virtual bool startDTD(const xmlChar* const name,
            const xmlChar* const publicId, const xmlChar* const systemId) = 0;
    virtual bool endDTD() = 0;
    virtual bool startEntity(const xmlChar* const name) = 0;
    virtual bool endEntity(const xmlChar* const name) = 0;
    virtual bool startCDATA() = 0;
    virtual bool endCDATA() = 0;
    virtual bool comment(const xmlChar* const characters) = 0;
    virtual std::string errorString() = 0;
};

/**
 * \class XmlErrorHandler
 *
 * \brief Interface for XML error handler
 */
class KOLIBRE_API XmlErrorHandler
{
public:
    virtual ~XmlErrorHandler()
    {
    }
    ;
    virtual bool warning(const XmlError &e) = 0;
    virtual bool error(const XmlError &e) = 0;
    virtual bool fatalError(const XmlError &e) = 0;
    virtual std::string errorString() = 0;
};

/**
 * \class XmlDeclHandler
 *
 * \brief Interface for XML declaration handler
 */
class KOLIBRE_API XmlDeclHandler
{
public:
    virtual ~XmlDeclHandler()
    {
    }
    ;
    virtual bool attributeDecl(const xmlChar* const entityName,
            const xmlChar* const attributeName, const xmlChar* const type,
            const xmlChar* const valueDefault, const xmlChar* const value) = 0;
    virtual bool externalEntityDecl(const xmlChar* const name,
            const xmlChar* const publicId, const xmlChar* const systemId) = 0;
    virtual bool internalEntityDecl(const xmlChar* const name,
            const xmlChar* const value) = 0;
};

/**
 * \class XmlDTDHandler
 *
 * \brief Interface for XML DTD handler
 */
class KOLIBRE_API XmlDTDHandler
{
public:
    virtual ~XmlDTDHandler()
    {
    }
    ;
    virtual bool notationDecl(const xmlChar* const name,
            const xmlChar* const publicId, const xmlChar* const systemId) = 0;
    virtual bool unparsedEntityDecl(const xmlChar* const name,
            const xmlChar* const publicId, const xmlChar* const systemId,
            const xmlChar* const notationName) = 0;
    virtual std::string errorString() = 0;
};

class XmlInputSource;

class KOLIBRE_API XmlReader
{
public:
    XmlReader();
    ~XmlReader();

    /**
     * Set content handler
     *
     * @param handler pointer to a handler
     */
    void setContentHandler(XmlContentHandler *handler)
    {
        _contentHandler = handler;
    }

    /**
     * Set declaration handler
     *
     * @param handler pointer to a handler
     */
    void setDeclHandler(XmlDeclHandler *handler)
    {
        _declarationHandler = handler;
    }

    /**
     * Set DTD handler
     *
     * @param handler pointer to a handler
     */
    void setDTDHandler(XmlDTDHandler *handler)
    {
        _DTDHandler = handler;
    }

    /**
     * Set error handler
     *
     * @param handler pointer to a handler
     */
    void setErrorHandler(XmlErrorHandler *handler)
    {
        _errorHandler = handler;
    }

    /**
     * Set lexical handler
     *
     * @param handler pointer to a handler
     */
    void setLexicalHandler(XmlLexicalHandler *handler)
    {
        _lexicalHandler = handler;
    }

    /**
     * Get content handler
     *
     * @return pointer to handler
     * @retval NULL if handler not set
     */
    XmlContentHandler *contentHandler() const
    {
        return _contentHandler;
    }

    /**
     * Get error handler
     *
     * @return pointer to handler
     * @retval NULL if handler not set
     */
    XmlErrorHandler *errorHandler() const
    {
        return _errorHandler;
    }

    /**
     * Get lexical handler
     *
     * @return pointer to handler
     * @retval NULL if handler not set
     */
    XmlLexicalHandler *lexicalHandler() const
    {
        return _lexicalHandler;
    }

    bool parseXml(const char *);
    bool parseHtml(const char *);

    void useCache(bool setting);

    void endDocumentHandlerCalled();

    XmlNamespace* pushNamespaces(XmlAttributes& attributes);
    XmlNamespace* popNamespaces();
    XmlNamespace* xmlNamespace();

    bool parserStopped() const;
    void stopParsing();

    bool sawError() const;
    void recordError();

    int lineNumber() const;
    int columnNumber() const;

    // Conversion routines
    static const char *transcode(const xmlChar *);
    static const char *transcode(const xmlChar *, size_t);
    static const xmlChar *transcode(const char *);
    static void release(const char *);
    static void release(const xmlChar *);

    // Error routines
    void setLastError(XmlError *);
    const XmlError *getLastError();

private:
    bool setupSAXHandler(xmlSAXHandler &);
    int parseChunk(xmlParserCtxtPtr, const char *, int, int);
    bool parse(const XmlInputSource &input);

    XmlContentHandler *_contentHandler;
    XmlDeclHandler *_declarationHandler;
    XmlDTDHandler *_DTDHandler;
    XmlErrorHandler *_errorHandler;
    XmlLexicalHandler *_lexicalHandler;

    nsStackItem *nsStackCur;
    int stackcount;

    struct _xmlParserCtxt *m_context;
    static const xmlChar *currentEncoding;

    enum
    {
        DOCTYPE_XML, DOCTYPE_HTML
    } m_doctype;

    bool m_parserStopped :1;
    bool m_sawError :1;
    bool m_endDocumentHandlerCalled :1;

    bool bUseCache;

    XmlError *pLastError;
};

#endif
