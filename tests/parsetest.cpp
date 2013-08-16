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

#include <locale.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>

#include "XmlDefaultHandler.h"
#include "XmlAttributes.h"
#include "XmlError.h"
#include "setup_logging.h"

using namespace std;

class SaxTest: public XmlDefaultHandler
{

public:
    //!default constructor
    SaxTest() :
            m_elementcount(0), m_charcount(0), m_level(0)
    {
        cout << "SaxTest constructor" << endl;
    }
    ;
    //!destructor
    ~SaxTest()
    {
        cout << "SaxTest destructor" << endl;
    }
    ;

    bool openUri(string, string);

    //SAX METHODS
    bool startElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const, const XmlAttributes &);
    bool endElement(const xmlChar* const, const xmlChar* const,
            const xmlChar* const);
    bool characters(const xmlChar* const, const unsigned int);

    bool error(const XmlError&);
    bool fatalError(const XmlError&);
    bool warning(const XmlError&);

private:
    //!utility function
    const char *getAttributeValue(const char *);
    long m_elementcount;
    long m_charcount;
    long m_level;
};

bool SaxTest::openUri(string uri, string type)
{
    XmlReader parser;
    parser.setContentHandler(this);
    parser.setErrorHandler(this);

    cout << "Opening: " << uri << endl;

    // parser begins parsing; expect SAX Events soon
    bool ret = false;
    if (type == "html")
        ret = parser.parseHtml(uri.c_str());
    else
        ret = parser.parseXml(uri.c_str());

    if (!ret)
    {
        const XmlError *e = parser.getLastError();
        if (e)
        {
            cout << "An error occurred in d:" << e->domain() << " code:"
                    << e->code() << ": " << e->message() << " on line "
                    << e->lineNumber() << endl;

            switch (e->domain())
            {
            case XML_FROM_PARSER:
                cout << "Parser error '" << e->message() << "' on line "
                        << e->lineNumber() << endl;
                break;
            case XML_FROM_IO:
                switch (e->code())
                {
                case XML_IO_EACCES:
                    cout << "Permission denied '" << e->message() << "'" << endl;
                    break;
                case XML_IO_ENOENT:
                case XML_IO_EIO:
                default:
                    cout << "File not found '" << e->message() << "'" << endl;
                    break;
                }
                break;
            case XML_FROM_HTTP:
                cout << "Not found " << e->message() << endl;
                break;
            default:
                cout << "Unknown error " << e->message() << "(" << e->code() << ")" << endl;
                break;
            }
        }
        return false;
    }

    cout << "Parsed " << m_elementcount << " elements and " << m_charcount << " characters" << endl;
    return true;
}

bool SaxTest::startElement(const xmlChar* const namespaceURI,
        const xmlChar* const localname, const xmlChar* const qname,
        const XmlAttributes &attributes)
{

    const char* element_name = NULL;
    element_name = XmlReader::transcode(localname);

    if (m_level != 0) cout << endl;
    for (int i = 0; i < m_level; i++)
        cout << " ";
    cout << localname << "(";
    m_level++;
    /*cout << "<" << element_name;
     for(int i = 0; i < attributes.length(); i++) {
     cout << " " << attributes.qName(i) << "=\""<< attributes.value(i) << "\"";
     }
     cout << ">";*/
    m_elementcount++;

    return true;
}

bool SaxTest::endElement(const xmlChar* const namespaceURI,
        const xmlChar* const localName, const xmlChar* const qName)
{
    const char* element_name = NULL;
    element_name = XmlReader::transcode(localName);
    m_level--;
    cout << ")";
    //cout << "</" << element_name << ">";
    if (m_level == 0) cout << endl;
    return true;
}

bool SaxTest::characters(const xmlChar* const characters,
        const unsigned int length)
{
    const char *tmpchars = XmlReader::transcode(characters);
    static string mTempChars;
    mTempChars.clear();
    mTempChars.append(tmpchars, length);
    for (unsigned int i = 0; i < length; i++)
        cout << ".";
    m_charcount += length;

    return true;
}

bool SaxTest::fatalError(const XmlError& e)
{
    //char* xerces_msg;

    //  xerces_msg = XmlSimpleReader::transcode(e.getMessage());

    cout << endl << "SaxTest::fatalError: " << e.getMessage() << " at line: "
            << e.lineNumber() << " columnNumber: " << e.columnNumber() << endl;

    //XmlSimpleReader::release(xerces_msg);
    return false;
}

bool SaxTest::error(const XmlError& e)
{
    cout << endl << "SaxTest::error: " << e.getMessage() << " at line: "
            << e.lineNumber() << " columnNumber: " << e.columnNumber() << endl;
    return true;
}

bool SaxTest::warning(const XmlError& e)
{
    cout << endl << "SaxTest::warning: " << e.getMessage() << " at line: "
            << e.lineNumber() << " columnNumber: " << e.columnNumber() << endl;
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Please specify an input file on the command line" << endl;
        return 1;
    }

    setup_logging();

    bool parseResult = false;
    bool expectSuccess = true;
    string type;

    if (argc > 2 && strcmp(argv[2], "fail") == 0) expectSuccess = false;

    if (strstr(argv[1], ".html"))
        type = "html";
    else
        type = "xml";

    cout << "Test 1: opening " << argv[1] << " of type " << type << endl;
    SaxTest *parser1 = new SaxTest();
    parseResult = parser1->openUri(argv[1], type);
    delete parser1;
    if (expectSuccess && not parseResult) {
        cout << "Test 1: parsing of file " << argv[1] << " FAILED" << endl;
        return 1;
    }
    cout << endl;

    cout << "Test 2: opening " << argv[1] << " of type " << type << endl;
    SaxTest *parser2 = new SaxTest();
    parseResult = parser2->openUri(argv[1], type);
    delete parser2;
    if (expectSuccess && not parseResult) {
        cout << "Test 2: parsing of file " << argv[1] << " FAILED" << endl;
        return 1;
    }
    cout << endl;

    return 0;
}
