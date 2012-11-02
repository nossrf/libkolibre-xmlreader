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

#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include <curl/curl.h>
#include <curl/multi.h>
#include <curl/easy.h>
#include <zlib.h>
#include <string>

#include "InputStream.h"
#include "CacheObject.h"

// Helpers for splitting urls into parts.
std::string url2hostname(const std::string& url);
std::string url2username(const std::string& url);
std::string url2password(const std::string& url);

//
// This class implements the BinInputStream interface specified by the XML
// parser.
//

class HttpStream: public InputStream
{
public:
    HttpStream(const std::string url, CURL *curlHandle, CURLM *curlMultiHandle,
            CacheObject *pCache);
    ~HttpStream();

    unsigned int curPos() const;
    int readBytes(char* const toFill, const unsigned int maxToRead);

    void useCache(bool);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    HttpStream(const HttpStream&);
    HttpStream& operator=(const HttpStream&);

    // Data function
    static size_t staticWriteCallback(char *buffer, size_t size, size_t nitems,
            void *outstream);
    size_t writeCallback(char *buffer, size_t size, size_t nitems);

    // Header functions
    static size_t staticHeaderCallback(char *buffer, size_t size, size_t nitems,
            void *outstream);
    size_t headerCallback(char *buffer, size_t size, size_t nitems);

    // Debug functions
    static size_t staticDebugCallback(CURL *handle, curl_infotype type,
            char *msg, size_t msgsize, void *outstream);
    size_t debugCallback(CURL *handle, curl_infotype type, char *msg,
            size_t msgsize);

    std::string getHttpMsg(int httpstatuscode);

    CURLM* fMulti;
    CURL* fEasy;

    bool setupConnection(CacheObject *pCache);
    bool destroyConnection();
    bool resetBuffer();

    std::string sURL;

    size_t fTotalBytesRead;
    size_t fTotalBytesWrite;
    char* fWritePtr;
    size_t fBytesRead;
    size_t fBytesToRead;
    bool fDataAvailable;

    // Overflow buffer for when curl writes more data to us
    // than we've asked for.
    char* fBuffer;
    size_t fBufferSize;
    size_t fBufferHead;
    size_t fBufferTail;

    size_t hContent_length_response;

    bool bStoreCache;
    bool bTransferFinished;
    bool bStreamFromCache;
    bool bUseCache;
    bool bDeleteCache;
    CacheObject* cacheObject;

    char * fLocation;

};

#endif
