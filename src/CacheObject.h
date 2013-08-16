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

#ifndef CACHEOBJECT_H
#define CACHEOBJECT_H

#include <zlib.h>
#include <string>

#define Z_CHUNK_SIZE 16384

class CacheObject
{
public:
    // Initializes a cache object
    CacheObject(const char* url = "");
    ~CacheObject();

    // Return object
    CacheObject *getObject();

    enum CacheState
    {
        EMPTY, WRITE, FULL, READ, BUSY
    };

    // Setter functions
    void setEtag(const char* etag);
    void setLastModified(const char* last_modified);
    void setLocation(const char* location);
    void setContentLength(const unsigned long content_length);
    void setHttpCode(const int code);

    char *getEtag() const;
    char *getLastModified() const;
    char *getLocation() const;
    unsigned long getContentLength() const;
    unsigned long getBufferSize() const;
    int getHttpCode() const;

    // Set the tidied/untiedied flags
    void setTidyFlag(bool flag);
    bool getTidyFlag();

    // Append data to the zBuffer
    unsigned int writeBytes(const char *buffer, const size_t bytes);

    // Read data from the zBuffer
    int readBytes(const char *buffer, const size_t bytes);

    // Resets the current state without destroying buffers
    void resetState();
    CacheState getState();

    void resetBuffer();

    const std::string &getErrorMsg();

private:

    char *pSrcUrl;
    char *pEtag;
    char *pLast_modified;
    char *pLocation;
    unsigned long lContent_length;
    int iHttpCode;

    CacheState eState;

    // Flags
    bool bTidyFlag;

    // zLib stuff
    z_stream c_stream;
    char c_stream_buffer[Z_CHUNK_SIZE]; // Temp output buffer
    static const char dictionary[];

    // zBuffer
    char *zBuffer;
    int zBufferAllocCount;
    unsigned long zBufferSize;
    unsigned long zBufferPos;

    std::string mErrorMsg;
};

#endif
