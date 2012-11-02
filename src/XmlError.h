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
 * \class XmlError
 *
 * \brief Class for storing parse error
 */

#ifndef XMLERROR_H
#define XMLERROR_H

#include <libxml/xmlerror.h>

class XmlError
{
public:

    /**
     * Constructor
     *
     * @param d error domain
     * @param c error code
     * @param m error message
     * @param col column at which error occurred
     * @param line line at which error occurred
     */
    XmlError(const int d, const int c, const std::string m, int col, int line) :
            m_message(m), m_domain(d), m_code(c), m_column(col), m_line(line)
    {
    }

    /**
     * Constructor
     *
     * @param d error domain
     * @param c error code
     * @param m error message
     */
    XmlError(const int d, const int c, const std::string m) :
            m_message(m), m_domain(d), m_code(c), m_column(0), m_line(0)
    {
    }

    /**
     * Constructor
     *
     * @param d error domain
     * @param c error code
     * @param m error message
     */
    XmlError(const int d, const int c, const char *m) :
            m_message(m), m_domain(d), m_code(c), m_column(0), m_line(0)
    {
    }

    /**
     * Destructor
     */
    //~XmlError() {}

    /**
     * Get error message
     *
     * @return pointer to the error message
     */
    const std::string & message() const
    {
        return m_message;
    }

    /**
     * Get error domain
     *
     * @return the error domain
     */
    int domain() const
    {
        return m_domain;
    }

    /**
     * Get error code
     *
     * @return the error code
     */
    int code() const
    {
        return m_code;
    }

    /**
     * Get error message
     *
     * @return the error message
     */
    std::string getMessage() const
    {
        return m_message;
    }

    /**
     * Get column number
     *
     * @return the column number at which the error occurred
     * @retval 0 column number is unknown
     */
    int columnNumber() const
    {
        return m_column;
    }

    /**
     * Get line number
     *
     * @return the error number at which the error occurred
     * @retval 0 line number is unknown
     */
    int lineNumber() const
    {
        return m_line;
    }

private:
    std::string m_message;
    int m_domain;
    int m_code;
    int m_column;
    int m_line;
};

#endif
