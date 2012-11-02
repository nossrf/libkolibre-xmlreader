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

#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "InputStream.h"
#include "CacheObject.h"

//
// This class implements the BinInputStream interface specified by the XML
// parser.
//

class FileStream: public InputStream
{
public:
    FileStream(const std::string filename, CacheObject *co = NULL);
    ~FileStream();

    unsigned int curPos() const;
    int readBytes(char* const toFill, const unsigned int maxToRead);

    void useCache(bool);
    enum ParseMode
    {
        READ, CACHED
    };

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    FileStream(const FileStream&);
    FileStream& operator=(const FileStream&);

    // returns the libxml error code for system errors
    int getXmlErrorCode(int);

    bool openStream();

    ParseMode mode;

    CacheObject *cacheObject;

    std::string sFilename;

    unsigned long fTotalBytesRead;
    size_t fSize;
    FILE *fp;

    bool bUseCache;
    bool bIsOpen;
};

#endif
