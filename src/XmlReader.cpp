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
 * \class XmlReader
 *
 * \brief This is the main interface for the kolibre xmlreader. XmlReader lets you parse HMTL and XML resource (offline and online) and can even cached data for quicker access.
 */

#include "XmlReader.h"

#include "XmlAttributes.h"
#include "XmlError.h"
#include "XmlDefaultHandler.h"

#include "DataSource.h"

#include <malloc.h>
#include <libxml/encoding.h>

#include <cstring>
#include <sstream>

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.xmlreader
log4cxx::LoggerPtr xmlXmlReaderLog(
        log4cxx::Logger::getLogger("kolibre.xmlreader.xmlreader"));

static void startDocumentHandler(void *userData)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    LOG4CXX_DEBUG(xmlXmlReaderLog, "startDocumentHandler()");
    reader->contentHandler()->startDocument();
}

static void endDocumentHandler(void *userData)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    LOG4CXX_DEBUG(xmlXmlReaderLog, "endDocumentHandler()");
    reader->endDocumentHandlerCalled();
    reader->contentHandler()->endDocument();
}

static void startElementHandler(void *userData, const xmlChar *name,
        const xmlChar **libxmlAttributes)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);

    LOG4CXX_TRACE(xmlXmlReaderLog, "startElementHandler() for " << name);

    if (reader->parserStopped())
    {
        return;
    }

    XmlAttributes attributes(libxmlAttributes);

    XmlNamespace* ns = reader->pushNamespaces(attributes);
    attributes.split(ns);

    const xmlChar *qName = name;
    xmlChar *localName = NULL;
    const xmlChar *uri = NULL;
    const xmlChar *colonPtr = xmlStrstr(qName, (xmlChar *) ":");

    if (colonPtr != NULL)
    {
        localName = xmlStrdup(qName + (colonPtr - qName) + 1);
    }

    uri = reader->xmlNamespace()->uriForPrefix(qName);

    // We pass in the namespace of the element, and then the name both with and without
    // the namespace prefix.
    if (localName != NULL)
    {
        reader->contentHandler()->startElement(uri, localName, qName,
                attributes);
        free(localName);
    }
    else
    {
        reader->contentHandler()->startElement(uri, qName, qName, attributes);
    }
}

static void endElementHandler(void *userData, const xmlChar *name)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }

    LOG4CXX_TRACE(xmlXmlReaderLog, "endElementHandler() for " << name);

    const xmlChar *qName = name;
    xmlChar *localName = NULL;
    const xmlChar *uri = NULL;
    const xmlChar *colonPtr = xmlStrstr(qName, (xmlChar *) ":");

    if (colonPtr != NULL)
    {
        localName = xmlStrdup(qName + (colonPtr - qName) + 1);
    }

    uri = reader->xmlNamespace()->uriForPrefix(qName);

    if (localName != NULL)
    {
        reader->contentHandler()->endElement(uri, localName, qName);
        free(localName);
    }
    else
        reader->contentHandler()->endElement(uri, qName, qName);

    XmlNamespace* ns = reader->popNamespaces();
    if (ns)
        ns->deref();
}

static void charactersHandler(void *userData, const xmlChar *s, int len)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }

    reader->contentHandler()->characters(s, len);
}

static void processingInstructionHandler(void *userData, const xmlChar *target,
        const xmlChar *data)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }
    reader->contentHandler()->processingInstruction(target, data);
}

static void cdataBlockHandler(void *userData, const xmlChar *s, int len)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }
    reader->lexicalHandler()->startCDATA();
    reader->contentHandler()->characters(s, len);
    reader->lexicalHandler()->endCDATA();
}

static void commentHandler(void *userData, const xmlChar *comment)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    reader->lexicalHandler()->comment(comment);
}

static void warningHandler(void *userData, const char *message, ...)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }
    if (reader->errorHandler())
    {

        int domain = -1;
        int code = -1;
        const char *message = "an error occurred";

        xmlError *err = xmlGetLastError();
        if (err != NULL)
        {
            domain = err->domain;
            code = err->code;
            message = err->message;
        }

        reader->setLastError(
                new XmlError(domain, code, message, reader->columnNumber(),
                        reader->lineNumber()));

        if (!reader->errorHandler()->warning(*reader->getLastError()))
        {
            reader->stopParsing();
        }
    }
}

static void fatalErrorHandler(void *userData, const char *message, ...)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }
    if (!reader->errorHandler())
    {
        reader->stopParsing();
    }
    else
    {
        const char *message = "an error occurred";
        int domain = -1;
        int code = -1;
        xmlError *err = xmlGetLastError();
        if (err != NULL)
        {
            domain = err->domain;
            code = err->code;
            message = err->message;
        }

        reader->setLastError(
                new XmlError(domain, code, message, reader->columnNumber(),
                        reader->lineNumber()));

        if (!reader->errorHandler()->fatalError(*reader->getLastError()))
        {
            reader->stopParsing();
        }
        reader->recordError();
    }
}

static void normalErrorHandler(void *userData, const char *message, ...)
{
    XmlReader *reader = static_cast<XmlReader *>(userData);
    if (reader->parserStopped())
    {
        return;
    }
    if (!reader->errorHandler())
    {
        reader->stopParsing();
    }
    else
    {
        const char *message = "an error occurred";
        int domain = -1;
        int code = -1;
        xmlError *err = xmlGetLastError();
        if (err != NULL)
        {
            domain = err->domain;
            code = err->code;
            message = err->message;
        }

        reader->setLastError(
                new XmlError(domain, code, message, reader->columnNumber(),
                        reader->lineNumber()));

        if (!reader->errorHandler()->error(*reader->getLastError()))
        {
            reader->stopParsing();
        }
        reader->recordError();
    }
}

static xmlEntity xmlEntityUnknown =
{ NULL, XML_ENTITY_DECL, BAD_CAST "?", NULL, NULL, NULL, NULL, NULL, NULL,
        BAD_CAST "_", BAD_CAST "_", 1, XML_INTERNAL_PREDEFINED_ENTITY, NULL,
        NULL, NULL, NULL, 0 };

static xmlEntityPtr entityHandler(void *user_data, const xmlChar *name)
{
    xmlEntityPtr entity = xmlGetPredefinedEntity(name);

    if (entity == NULL)
    {
        warningHandler(user_data, "Unknown entity '%s'", BAD_CAST name);
        return &xmlEntityUnknown;
    }
    return entity;
}

/**
 * Constructor
 */
XmlReader::XmlReader() :
        _contentHandler(0), _declarationHandler(0), _DTDHandler(0), _errorHandler(
                0), _lexicalHandler(0), stackcount(0), m_doctype(DOCTYPE_XML), pLastError(
                0)

{
    static bool didInit = false;
    if (!didInit)
    {
        xmlInitParser();
        didInit = true;
    }

    nsStackCur = (nsStackItem*) malloc(sizeof(struct nsStackItem));
    nsStackCur->ns = NULL;
    nsStackCur->prev = NULL;

    bUseCache = true;
}

/**
 * Destructor
 */
XmlReader::~XmlReader()
{
    XmlNamespace *ns = popNamespaces();
    while (ns != NULL)
        ns = popNamespaces();
    free(nsStackCur);
    if (pLastError)
        delete pLastError;
}

/**
 * Specify whether to use caching or not
 *
 * The default value for caching is true.
 *
 * @param setting true for cache and false for no caching
 */
void XmlReader::useCache(bool setting)
{
    bUseCache = setting;
}

/**
 * Inform that endDocumentHandler has been called
 */
void XmlReader::endDocumentHandlerCalled()
{
    m_endDocumentHandlerCalled = true;
}

/**
 * Set last error
 *
 * @param e pointer to a XmlError object
 */
void XmlReader::setLastError(XmlError *e)
{
    if (pLastError != NULL)
        delete pLastError;
    pLastError = e;
}

/**
 * Get last error
 *
 * @return pointer to a XmlError object
 */
const XmlError *XmlReader::getLastError()
{
    return pLastError;
}

/**
 * Search and add all namespaces to namespace stack
 * @param attrs pointer to a XmlAttributes fin which to search for namespaces
 * @return pointer to the namespace on the top of the stack
 */
XmlNamespace* XmlReader::pushNamespaces(XmlAttributes& attrs)
{
    XmlNamespace* ns = NULL;

    if (nsStackCur->ns != NULL)
        ns = nsStackCur->ns;
    else
        ns = new XmlNamespace();

    // Search for any xmlns attributes.
    for (int i = 0; i < attrs.length(); i++)
    {

        const xmlChar *qName = attrs.qName(i);

        if (xmlStrcmp(qName, (xmlChar *) "xmlns") == 0)
        {
            ns = new XmlNamespace(NULL, attrs.value(i), ns);
        }
        else if (xmlStrncmp(qName, (xmlChar *) "xmlns:", 6) == 0)
        {
            ns = new XmlNamespace(qName + 6, attrs.value(i), ns);
        }
    }

    nsStackItem *ptr = (nsStackItem *) malloc(sizeof(struct nsStackItem));
    ptr->prev = nsStackCur;
    ptr->ns = ns;
    nsStackCur = ptr;

    ns->ref();

    return ns;
}

/**
 * Remove namespace from the top of namespace stack
 *
 * @return pointer to the namespace on the top of the stack
 * @retval NULL when the stack is empty
 */
XmlNamespace* XmlReader::popNamespaces()
{
    XmlNamespace *ns = nsStackCur->ns;
    if (nsStackCur->prev != NULL)
    {
        nsStackItem *ptr = nsStackCur->prev;
        free(nsStackCur);
        nsStackCur = ptr;
        //LOG4CXX_DEBUG(xmlXmlReaderLog, "nsstack now of size " << --stackcount << "@" << ns->m_uri);
    }
    else
    {
        //LOG4CXX_DEBUG(xmlXmlReaderLog, "popping last stack item");
        nsStackCur->ns = NULL;
    }
    return ns;
}

/**
 * Get namespace on the top of stack
 * @return pointer to the namespace on the top of the stack
 * @retval NULL if stack is empty
 */
XmlNamespace* XmlReader::xmlNamespace()
{
    return nsStackCur->ns;
}

/**
 * Setup a XML SAX handler
 *
 * @param handler pointer to a XML SAX handler
 * @return boolean of the result
 * @retval false if setup failed
 * @retval true if setup succeeded
 */
bool XmlReader::setupSAXHandler(xmlSAXHandler &handler)
{
    if (_contentHandler && !_contentHandler->startDocument())
    {
        LOG4CXX_ERROR(xmlXmlReaderLog, "Invalid SAXHandler");
        return false;
    }

    std::memset(&handler, 0, sizeof(handler));

    handler.error = normalErrorHandler;
    handler.fatalError = fatalErrorHandler;
    if (_contentHandler)
    {
        handler.startDocument = startDocumentHandler;
        handler.endDocument = endDocumentHandler;
        handler.characters = charactersHandler;
        handler.endElement = endElementHandler;
        handler.processingInstruction = processingInstructionHandler;
        handler.startElement = startElementHandler;
    }
    if (_lexicalHandler)
    {
        handler.cdataBlock = cdataBlockHandler;
        handler.comment = commentHandler;
    }
    if (_errorHandler)
    {
        handler.warning = warningHandler;
    }

    handler.getEntity = entityHandler;

    return true;
}

/**
 * Parse a XML resource
 *
 * @param uri location of the XML resource
 * @return boolean of the result
 * @retval false if parsing failed
 * @retval true if parsing succeeded
 */
bool XmlReader::parseXml(const char* uri)
{
    m_doctype = DOCTYPE_XML;
    bool ret = false;

    xmlSAXHandler handler;
    ret = setupSAXHandler(handler);

    if (!ret)
    {
        setLastError(
                new XmlError(XML_FROM_PARSER, -1,
                        "Failed to initialize SAX callbacks"));
        return false;
    }

    m_context = xmlCreatePushParserCtxt(&handler, this, NULL, 0, NULL);

    DataSource ds(uri);

    LOG4CXX_DEBUG(xmlXmlReaderLog, "Parsing '" << uri << "'");
    ret = parse(ds);

    if (ret == false)
    {
        const XmlError *e = getLastError();
        if (e)
        {
            LOG4CXX_ERROR(xmlXmlReaderLog,
                    "Document '" << uri << "' contains errors: " << e->getMessage());
        }
        else
        {
            LOG4CXX_ERROR(xmlXmlReaderLog,
                    "Document '" << uri << "' contains errors: unknown");
        }
    }

    if (handler.endDocument != NULL && m_endDocumentHandlerCalled == false)
    {
        contentHandler()->endDocument();
    }

    xmlFreeParserCtxt(m_context);
    m_context = NULL;

    return ret;
}

/**
 * Parse a HTML resource
 *
 * @param uri location of the HTML resource
 * @return boolean of the result
 * @retval false if parsing failed
 * @retval true if parsing succeeded
 */
bool XmlReader::parseHtml(const char *uri)
{
    m_doctype = DOCTYPE_HTML;
    bool ret = false;
    DataSource *ds = NULL;

    ds = new DataSource(uri);

    const XmlError *e = NULL;

    // Setup a null handler
    xmlSAXHandler nullhandler;
    std::memset(&nullhandler, 0, sizeof(nullhandler));

    // First check if the HTML document is valid
    m_context = htmlCreatePushParserCtxt(&nullhandler, this, NULL, 0, NULL,
            (xmlCharEncoding) 0);

    LOG4CXX_DEBUG(xmlXmlReaderLog, "Preparsing '" << uri << "'");
    ret = parse(*ds);

    if (ret == false)
    {
        e = getLastError();

        if (e)
        {
            LOG4CXX_WARN(xmlXmlReaderLog,
                    "Document '" << uri << "' contains errors: " << e->getMessage());
        }
        else
        {
            LOG4CXX_WARN(xmlXmlReaderLog,
                    "Document '" << uri << "' contains errors: unknown");
        }

        delete ds;
        ds = new DataSource(uri, true);

        // Recreate parser
        htmlFreeParserCtxt(m_context);
        m_context = htmlCreatePushParserCtxt(&nullhandler, this, NULL, 0, NULL,
                (xmlCharEncoding) 0);

        // Try parsing again
        LOG4CXX_DEBUG(xmlXmlReaderLog, "Preparsing tidied '" << uri << "'");
        ret = parse(*ds);

        if (ret == false)
        {
            e = getLastError();

            if (e)
            {
                LOG4CXX_ERROR(xmlXmlReaderLog,
                        "Error parsing '" << uri <<"': " << e->getMessage());
            }
            else
            {
                LOG4CXX_ERROR(xmlXmlReaderLog,
                        "Error parsing '" << uri << "': unknown");
            }

            htmlFreeParserCtxt(m_context);
            m_context = NULL;
            delete ds;
            return ret;
        }
    }

    delete ds;

    // If we have a valid document, setup sax handler to recieve events
    xmlSAXHandler handler;
    ret = setupSAXHandler(handler);

    if (ret == false)
    {
        setLastError(
                new XmlError(XML_FROM_PARSER, -1,
                        "Failed to initialize SAX callbacks"));
        return false;
    }

    // reCreate the datasource
    ds = new DataSource(uri);

    // Create a new context
    htmlFreeParserCtxt(m_context);
    m_context = htmlCreatePushParserCtxt(&handler, this, NULL, 0, NULL,
            (xmlCharEncoding) 0);

    LOG4CXX_DEBUG(xmlXmlReaderLog, "Parsing '" << uri << "'");
    ret = parse(*ds);

    if (ret == false)
    {
        e = getLastError();

        if (e)
        {
            LOG4CXX_ERROR(xmlXmlReaderLog,
                    "Document '" << uri << "' contains errors: " << e->getMessage());
        }
        else
        {
            LOG4CXX_ERROR(xmlXmlReaderLog,
                    "Document '" << uri << "' contains errors: unknown");
        }
    }

    if (handler.endDocument != NULL && m_endDocumentHandlerCalled == false)
    {
        contentHandler()->endDocument();
    }

    delete ds;
    htmlFreeParserCtxt(m_context);
    m_context = NULL;

    return ret;
}

/**
 * Parse a chunk of a resource
 *
 * @param ctxt ???
 * @param chunk pointer to the resource to parse
 * @param size number of bytes to parse
 * @param terminate ???
 * @return
 */
int XmlReader::parseChunk(xmlParserCtxtPtr ctxt, const char *chunk, int size,
        int terminate)
{
    switch (m_doctype)
    {
    case DOCTYPE_HTML:
        return htmlParseChunk(ctxt, chunk, size, terminate);
    case DOCTYPE_XML:
        return xmlParseChunk(ctxt, chunk, size, terminate);
    }
    return -1;
}

/**
 * Parse a resource
 *
 * @param input pointer to a input source
 *
 * @return boolean of the result
 * @retval false if parsing failed
 * @retval true if parsing succeeded
 */
bool XmlReader::parse(const XmlInputSource &input)
{
    const size_t bufsize = 4096;

    char buffer[bufsize];
    InputStream *is = NULL;

    // Try creating the stream
    is = input.makeStream();

    if (is == NULL)
    {
        setLastError(
                new XmlError(XML_FROM_PARSER, -1,
                        "Failed to create InputStream"));
        LOG4CXX_ERROR(xmlXmlReaderLog, "Failed to create InputStream");
        return false;
    }

    m_parserStopped = false;
    m_sawError = false;
    m_endDocumentHandlerCalled = false;

    is->useCache(bUseCache);

    LOG4CXX_TRACE(xmlXmlReaderLog, "Starting parse");

    int ret = 0;
    int bytes_read;
    bool parseByteOrderMark = true;
    do
    {
        LOG4CXX_TRACE(xmlXmlReaderLog,
                "Trying to read " << bufsize << " bytes");
        try {
        bytes_read = is->readBytes(buffer, bufsize);
        } catch(XmlError e) {
            m_sawError = true;
            setLastError(new XmlError(e));
            return false;
        }

        if (bytes_read < 0)
        {
            xmlStopParser(m_context);
            m_sawError = true;
            setLastError(
                    new XmlError(XML_FROM_IO, (int) is->getErrorCode(),
                            is->getErrorMsg()));
            LOG4CXX_ERROR(xmlXmlReaderLog, "Read error " << is->getErrorMsg());
            return false;
        }

        if (bytes_read)
            LOG4CXX_TRACE(xmlXmlReaderLog, "Read " << bytes_read << " bytes");

        if (parseByteOrderMark && bytes_read >= 3)
        {
            parseByteOrderMark = false;

            char efbbbf[] =
            { 0xEF, 0xBB, 0xBF }; // UTF-8 bom is optional

            if (std::equal(&buffer[0], (&buffer[0]) + 3, efbbbf))
            {
                LOG4CXX_WARN(xmlXmlReaderLog,
                        "Removing optional UTF-8 BOM: ef bb bf ");
                std::copy(buffer + 3, buffer + bytes_read, buffer);
                bytes_read -= 3;
            }
        }

        if (bytes_read > 0)
        {
            ret = parseChunk(m_context, buffer, bytes_read, 0);

            if (ret)
            {
                xmlError *error = xmlCtxtGetLastError(m_context);
                xmlStopParser(m_context);
                m_sawError = true;
                if (error != NULL)
                {
                    LOG4CXX_ERROR(xmlXmlReaderLog,
                            "Parse error " << error->message);
                    setLastError(
                            new XmlError(error->domain, error->code,
                            		std::string(error->message),
                                    m_context->input->col,
                                    m_context->input->line));
                }
                else
                {
                    LOG4CXX_ERROR(xmlXmlReaderLog,
                            "Parse error # " << ret << " with no error message");
                    LOG4CXX_INFO(xmlXmlReaderLog,
                            "For a complete list of errors please visit: http://xmlsoft.org/html/libxml-xmlerror.html#xmlParserErrors");
                    std::ostringstream oss;
                    oss << "Error id: " << ret;
                    setLastError(
                            new XmlError(XML_FROM_PARSER, ret, oss.str(),
                                    m_context->input->col,
                                    m_context->input->line));
                }
                return false;
            }
        }

    } while (bytes_read > 0);

    if (m_sawError)
        LOG4CXX_ERROR(xmlXmlReaderLog, "Should not reach here");

    return (!m_sawError);
}

/**
 * Check if parsing is in progress
 *
 * @return boolean
 * @retval true if parsing is not in progress
 * @retval false if parsing is in progress
 */
bool XmlReader::parserStopped() const
{
    return m_parserStopped;
}

/**
 * Stop parsing process
 */
void XmlReader::stopParsing()
{
    if (m_context != NULL)
        xmlStopParser(m_context);

    m_parserStopped = true;
}

/**
 * Check if error has occurred
 *
 * @return boolean
 * @retval true error has occurred
 * @retval false error has not occurred
 */
bool XmlReader::sawError() const
{
    return m_sawError;
}

/**
 * Inform that error has occurred
 */
void XmlReader::recordError()
{
    m_sawError = true;
}

/**
 * Get current line number
 *
 * @return current line number
 * @retval -1 no context has been set
 */
int XmlReader::lineNumber() const
{
    if (m_context == NULL)
        return -1;
    return m_context->input->line;
}

/**
 * Get current column number
 *
 * @return current column number
 * @retval -1 no context has been set
 */
int XmlReader::columnNumber() const
{
    if (m_context == NULL)
        return -1;
    return m_context->input->col;
}

/**
 * Free allocated memory for a char string
 *
 * @param charstr pointer to the char string
 */
void XmlReader::release(const char *charstr)
{
    if (charstr != NULL)
        free((void*) charstr);
}

/**
 * Free allocated memory for a XML string (UNIMPLEMENTED)
 *
 * @param xmlstr pointer to the XML string
 */
void XmlReader::release(const xmlChar *xmlstr)
{
    //xmlFree((void*)xmlstr);
}

/**
 * Return copy of the input string
 *
 * Historically this was used to convert UTF-8 to ISO-8859-1
 *
 * @param chars pointer to XML string
 * @param size number of bytes to include in new string
 * @return pointer to the new string
 * @retval NULL if input was NULL
 */
const char *XmlReader::transcode(const xmlChar *chars, size_t size)
{
    if (chars == NULL)
        return NULL;
    wchar_t wchars[size + 1];
    char buff[size * 2];
    // Cut the string using wchar array
    int wcslen = mbstowcs(wchars, (char*) chars, size);
    // Insert null at end
    wchars[wcslen] = '\0';
    // Transform it back to a char array
    int mbslen = wcstombs(buff, wchars, size * 2);
    buff[mbslen] = '\0';

    // Return a duplicate of resulting buffer
    return strdup((char*) buff);
}

/**
 * Return copy of the input string
 *
 * Historically this was used to convert UTF-8 to ISO-8859-1
 *
 * @param chars pointer to XML string
 * @return pointer to the new string
 * @retval NULL if input was NULL
 */
const char *XmlReader::transcode(const xmlChar *chars)
{
    if (chars == NULL)
        return NULL;
    //LOG4CXX_DEBUG(xmlXmlReaderLog, "converting " << chars << " (len" << xmlStrlen(chars)<< ")");
    return strdup((char*) chars);
}

/**
 * Convert char string to XML string
 *
 * Historically this was used to convert UTF-8 to ISO-8859-1
 *
 * @param chars pointer to char string
 * @return pointer to the new string
 */
const xmlChar *XmlReader::transcode(const char *chars)
{
    return reinterpret_cast<const xmlChar *>(chars);
}
