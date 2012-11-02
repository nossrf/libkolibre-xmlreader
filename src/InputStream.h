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
 * \class InputStream
 *
 * \brief An abstract class for reading bytes from a stream object.
 *
 * \author Kolibre (www.kolibre.org)
 *
 * \b Contact: info@kolibre.org
 */

#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <string>

class InputStream
{
public:

    enum ErrorCode
    {
        NONE = -1,
        OK = 0,
        NOT_FOUND,
        ACCESS_DENIED,
        READ_FAILED,
        CONNECT_FAILED,
        UNKNOWN_ERROR
    };

    /**
     * Destructor
     */
    virtual ~InputStream();

    /**
     * Get the current position in the stream
     *
     * @return number of bytes read in stream
     */

    virtual unsigned int curPos() const = 0;
    /**
     * Read bytes in stream
     *
     * @param toFill pointer where to store data
     * @param maxToRead number of bytes to read in stream
     * @return number of bytes read
     * @retval -1 when an error occurred
     * @retval 0 when there is no more bytes to read
     */
    virtual int readBytes(char* const toFill, const unsigned int maxToRead) = 0;

    /**
     * Specifiy whether to use caching or not
     *
     * @param setting true for cache and false for no caching
     */
    virtual void useCache(bool setting) = 0;

    /**
     * Get the error message
     *
     * @return pointer to the error message
     */
    const std::string &getErrorMsg()
    {
        return mErrorMsg;
    }
    ;

    /**
     * Get the error code
     *
     * @return error code number
     */
    ErrorCode getErrorCode()
    {
        return mErrorCode;
    }
    ;

protected:
    /**
     * Constructor
     */
    InputStream();

    /**
     * Protected variable for storing the error message
     */
    std::string mErrorMsg;

    /**
     * Protected variable for storing the error code
     */
    ErrorCode mErrorCode;

private:
    InputStream(const InputStream&);
    InputStream& operator=(const InputStream&);
};

inline InputStream::~InputStream()
{
}
;
inline InputStream::InputStream()
{
}
;

#endif
