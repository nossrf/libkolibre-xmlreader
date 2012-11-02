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

#ifndef TIDYSTREAM_H
#define TIDYSTREAM_H

#include <string>
#include <tidy.h>
#include <buffio.h>

#include "InputStream.h"

//
// This class implements the InputStream interface specified by the XML
// parser.
//

class TidyStream: public InputStream
{
public:
    TidyStream(const std::string, InputStream *);
    ~TidyStream();

    unsigned int curPos() const;
    int readBytes(char* const toFill, const unsigned int maxToRead);

    void useCache(bool);
    enum ParseMode
    {
        PASSTROUGH, TIDY, CACHED
    };

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    TidyStream(const TidyStream&);
    TidyStream& operator=(const TidyStream&);

    std::string sURL;

    unsigned long fTotalBytesRead;

    InputStream *inStream;

    ParseMode mode;

    CacheObject *cacheObject;

    // libtidy stuff
    TidyDoc tdoc;
    TidyBuffer outbuf;

    bool bUseCache;
    bool bTidied;

    bool Perform();
};

#endif
