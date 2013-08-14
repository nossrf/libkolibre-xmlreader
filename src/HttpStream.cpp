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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <string>

#include "DataStreamHandler.h"
#include "CacheObject.h"
#include "HttpStream.h"
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.xmlreader
log4cxx::LoggerPtr xmlHttpStreamLog(
        log4cxx::Logger::getLogger("kolibre.xmlreader.httpstream"));

using namespace std;

HttpStream::HttpStream(const std::string url, CURL *curlHandle,
        CURLM *curlMultiHandle, CacheObject *pCache) :
        fMulti(curlMultiHandle), fEasy(curlHandle), fTotalBytesRead(0), fTotalBytesWrite(
                0), fWritePtr(0), fBytesRead(0), fBytesToRead(0), fDataAvailable(
                false), fBuffer(0), fBufferSize(0), fBufferHead(0), fBufferTail(
                0), hContent_length_response(0), bStoreCache(false), bTransferFinished(
                false), bStreamFromCache(false), bDeleteCache(false), cacheObject(
                0)
{
    bUseCache = true;
    mErrorMsg = "unknown error";
    mErrorCode = NONE;

    sURL = url;
    LOG4CXX_DEBUG(xmlHttpStreamLog, "constructor for '" << sURL << "'");
    setupConnection(pCache);
}

HttpStream::~HttpStream()
{
    LOG4CXX_DEBUG(xmlHttpStreamLog, "destructor for '" << sURL << "'");
    LOG4CXX_DEBUG(xmlHttpStreamLog,
            "read: " << fTotalBytesRead << " bufsize: " << fBufferSize);
    destroyConnection();

    // Check if we should store the cacheobject
    if (USE_CACHE && cacheObject != NULL)
    {
        if (bStoreCache && bTransferFinished && bUseCache)
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Finishing compression phase for " << sURL);
            cacheObject->writeBytes(NULL, 0);
            cacheObject->setContentLength(fTotalBytesRead);
            cacheObject->resetState();
            DataStreamHandler::Instance()->addCacheObject(sURL, cacheObject);
        }
        else if (bUseCache && bStreamFromCache)
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Resetting cacheObject " << sURL);
            cacheObject->resetState();
        }
        else
        {
            //LOG4CXX(xmlHttpStreamLog, "Deleting cacheObject " << sURL);
            if (bDeleteCache)
                delete cacheObject;
        }
    }
    else
    {
        //LOG4CXX_DEBUG(xmlHttpStreamLog, "Not storing cacheObject for " << sURL);
    }

    DataStreamHandler::Instance()->releaseHandle(fEasy);
}

inline unsigned int HttpStream::curPos() const
{
    return fTotalBytesRead;
}

bool HttpStream::setupConnection(CacheObject *pCache)
{
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Connection: keep-alive");
    headers = curl_slist_append(headers, "Keep-Alive: 300");

    fLocation = NULL;

    if (USE_CACHE && bUseCache)
    {
        if (pCache == NULL)
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Creating new cacheObject for " << sURL);
            cacheObject = new CacheObject(sURL.c_str());
            bDeleteCache = true;
        }
        else
        {
            string headerstr = "";
            cacheObject = pCache;
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Have cache object for url " << sURL << " of size " << cacheObject->getContentLength());

            // Append these if we have cached data and want to verify it's freshness
            if (cacheObject->getLastModified() != NULL)
            {
                headerstr = "If-Modified-Since: ";
                headerstr.append(cacheObject->getLastModified());
                headers = curl_slist_append(headers, headerstr.c_str());
            }

            if (cacheObject->getEtag() != NULL)
            {
                headerstr = "If-None-Match: \"";
                headerstr.append(cacheObject->getEtag());
                headerstr.append("\"");
                headers = curl_slist_append(headers, headerstr.c_str());
            }

            if (cacheObject->getLocation() != NULL)
            {
                fLocation = strdup(cacheObject->getLocation());
            }
        }
    }

    curl_easy_setopt(fEasy, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(fEasy, CURLOPT_WRITEHEADER, this);
    // Pass this pointer to header function
    curl_easy_setopt(fEasy, CURLOPT_HEADERFUNCTION, staticHeaderCallback);
    // Our static header function

    curl_easy_setopt(fEasy, CURLOPT_WRITEDATA, this);
    // Pass this pointer to write function
    curl_easy_setopt(fEasy, CURLOPT_WRITEFUNCTION, staticWriteCallback);
    // Our static write function

    curl_easy_setopt(fEasy, CURLOPT_DEBUGDATA, this);
    // Pass this pointer to debug function
    curl_easy_setopt(fEasy, CURLOPT_DEBUGFUNCTION, staticDebugCallback);
    // Our static debug function

    curl_easy_setopt(fEasy, CURLOPT_URL, sURL.c_str());

    return true;
}

void HttpStream::useCache(bool setting)
{
    bUseCache = setting;
}

bool HttpStream::destroyConnection()
{
    if (fBuffer != NULL)
        free(fBuffer);
    if (fLocation != NULL)
        free(fLocation);

    return true;
}

bool HttpStream::resetBuffer()
{
    fTotalBytesRead = 0;
    fTotalBytesWrite = 0;

    fDataAvailable = false;
    if (fBufferSize != 0)
        free(fBuffer);
    fBuffer = NULL;
    fBufferSize = 0;
    fBufferHead = 0;
    fBufferTail = 0;

    bStoreCache = false;
    bStreamFromCache = false;
    return true;
}

size_t HttpStream::staticHeaderCallback(char *buffer, size_t size,
        size_t nitems, void *outstream)
{
    return ((HttpStream*) outstream)->headerCallback(buffer, size, nitems);
}

size_t HttpStream::headerCallback(char *buffer, size_t size, size_t nitems)
{
    char *bufPtr = NULL;

    size_t dataSize = 0;

    if (memcmp(buffer, "HTTP", 4) == 0)
    {
        bufPtr = buffer + 4;
        for (int c = 0; c < (size * nitems - 4); c++)
            if (memcmp(bufPtr + c, " ", 1) == 0)
            {
                char *bufPtr2 = bufPtr + c;
                // Check that there are characters after the space
                if (c + 4 + 4 > size * nitems)
                    break;

                if (memcmp(bufPtr2, " 200", 4) == 0)
                {
                    bStoreCache = true;
                    LOG4CXX_DEBUG(xmlHttpStreamLog, sURL << " 200 OK");
                }
                else if (memcmp(bufPtr2, " 301", 4) == 0)
                {
                    LOG4CXX_DEBUG(xmlHttpStreamLog, sURL << " 301 REDIRECT");
                }
                else if (memcmp(bufPtr2, " 302", 4) == 0)
                {
                    LOG4CXX_DEBUG(xmlHttpStreamLog, sURL << " 302 REDIRECT");
                }
                else if (memcmp(bufPtr2, " 304", 4) == 0)
                {
                    LOG4CXX_DEBUG(xmlHttpStreamLog,
                            sURL << " 304 NOT MODIFIED");
                }
            }

    }
    else if (memcmp(buffer, "ETag: ", 6) == 0)
    {
        bufPtr = buffer + 6;
        dataSize = (size * nitems) - (bufPtr - buffer);

        // Trim \r\n and possibly "
        if (memcmp(buffer + size * nitems - 1, "\"\r\n", 2))
        {
            dataSize -= 3;
            if (memcmp(bufPtr, "\"", 1) == 0)
            {
                bufPtr++;
                dataSize -= 1;
            }
        }
        else if (memcmp(buffer + size * nitems - 1, "\r\n", 2))
            dataSize -= 2;

        char *hEtag = (char *) malloc((dataSize + 1));
        if (hEtag != NULL)
        {
            memcpy(hEtag, bufPtr, dataSize);
            memset(hEtag + dataSize, 0, 1);
            if (USE_CACHE && bUseCache)
                cacheObject->setEtag(hEtag);
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Got etag len: " << dataSize << " '" << hEtag << "'");
            free(hEtag);
        }

    }
    else if (memcmp(buffer, "Last-Modified: ", 15) == 0)
    {
        bufPtr = buffer + 15;
        dataSize = (size * nitems) - (bufPtr - buffer);

        // Trim \r\n and possibly "
        if (memcmp(buffer + size * nitems - 1, "\r\n", 2))
            dataSize -= 2;

        char *hLast_modified = (char *) malloc((dataSize + 1));
        if (hLast_modified != NULL)
        {
            memcpy(hLast_modified, bufPtr, dataSize);
            memset(hLast_modified + dataSize, 0, 1);
            if (USE_CACHE && bUseCache)
                cacheObject->setLastModified(hLast_modified);
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Got last_modified len: " << dataSize << " '" << hLast_modified << "'");
            free(hLast_modified);
        }

    }
    else if (memcmp(buffer, "Location: ", 10) == 0)
    {
        bufPtr = buffer + 10;
        dataSize = (size * nitems) - (bufPtr - buffer);

        // Trim \r\n and possibly "
        if (memcmp(buffer + size * nitems - 1, "\r\n", 2))
            dataSize -= 2;

        char *hLocation = (char *) malloc((dataSize + 1));
        if (hLocation != NULL)
        {
            memcpy(hLocation, bufPtr, dataSize);
            memset(hLocation + dataSize, 0, 1);
            if (USE_CACHE && bUseCache)
                cacheObject->setLocation(hLocation);
            if (fLocation != NULL)
                free(fLocation);
            fLocation = strdup(hLocation);
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Got last_modified len: " << dataSize << " '" << hLocation << "'");
            free(hLocation);
        }

    }
    else if (memcmp(buffer, "Content-Length: ", 16) == 0)
    {
        bufPtr = buffer + 16;
        dataSize = (size * nitems) - (bufPtr - buffer);
        char *tmpBuf = NULL;

        // Trim \r\n and possibly "
        if (memcmp(buffer + size * nitems - 1, "\r\n", 2))
            dataSize -= 2;

        tmpBuf = (char *) malloc((dataSize + 1));
        if (tmpBuf != NULL)
        {
            memcpy(tmpBuf, bufPtr, dataSize);
            memset(tmpBuf + dataSize, 0, 1);
            hContent_length_response = atol(tmpBuf);
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Got Content-Length len: " << dataSize << " '" << tmpBuf << "' " << hContent_length_response);
            free(tmpBuf);
        }
    }

    // Always return size passed
    return size * nitems;
}

size_t HttpStream::staticDebugCallback(CURL *handle, curl_infotype type,
        char *message, size_t size, void *outstream)
{
    return ((HttpStream*) outstream)->debugCallback(handle, type, message, size);
}

size_t HttpStream::debugCallback(CURL *handle, curl_infotype type,
        char *message, size_t size)
{
    string tmpstr = "";
    switch (type)
    {
    case CURLINFO_TEXT:
        tmpstr = "CURLINFO_TEXT: ";
        tmpstr.append(message, size);
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_HEADER_IN:
        tmpstr = "CURLINFO_HEADER_IN: ";
        tmpstr.append(message, size);
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_HEADER_OUT:
        tmpstr = "CURLINFO_HEADER_OUT: ";
        tmpstr.append(message, size);
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_DATA_IN:
        tmpstr = "CURLINFO_DATA_IN: <see debug for data>";
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        tmpstr = "CURLINFO_DATA_IN: ";
        tmpstr.append(message, size);
        LOG4CXX_DEBUG(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_DATA_OUT:
        tmpstr = "CURLINFO_DATA_OUT: <see debug for data>";
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        tmpstr = "CURLINFO_DATA_OUT: ";
        tmpstr.append(message, size);
        LOG4CXX_DEBUG(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_SSL_DATA_IN:
        tmpstr = "CURLINFO_SSL_DATA_IN: <see debug for data>";
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        tmpstr = "CURLINFO_SSL_DATA_IN: ";
        tmpstr.append(message, size);
        LOG4CXX_DEBUG(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_SSL_DATA_OUT:
        tmpstr = "CURLINFO_SSL_DATA_OUT: <see debug for data>";
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        tmpstr = "CURLINFO_SSL_DATA_OUT: ";
        tmpstr.append(message, size);
        LOG4CXX_DEBUG(xmlHttpStreamLog, tmpstr);
        break;
    case CURLINFO_END:
        tmpstr = "CURLINFO_END: ";
        tmpstr.append(message, size);
        LOG4CXX_INFO(xmlHttpStreamLog, tmpstr);
        break;
    }

    return 0;
}

size_t HttpStream::staticWriteCallback(char *buffer, size_t size, size_t nitems,
        void *outstream)
{
    return ((HttpStream*) outstream)->writeCallback(buffer, size, nitems);
}

size_t HttpStream::writeCallback(char *buffer, size_t size, size_t nitems)
{
    //LOG4CXX_DEBUG(xmlHttpStreamLog, "writeCallback(char " << &buffer << ", size_t " << size << ", size_t " << nitems << ")");
    size_t cnt = size * nitems;
    size_t totalConsumed = 0;

    // Consume as many bytes as possible immediately into the buffer
    size_t consume = (cnt > fBytesToRead) ? fBytesToRead : cnt;
    memcpy(fWritePtr, buffer, consume);
    fWritePtr += consume;
    fBytesRead += consume;
    fTotalBytesRead += consume;
    fBytesToRead -= consume;

    //LOG4CXX_DEBUG(xmlHttpStreamLog, sURL << ": wrote " << size * nitems  << " (fTotalBytesRead: " << fTotalBytesRead << ")");

    // If we got a 200 response code, compress the blocks recieved and
    // store the the contents in zBuffer
    if (USE_CACHE && bUseCache && bStoreCache)
    {
        cacheObject->writeBytes(buffer, size * nitems);
    }

    // If bytes remain, rebuffer as many as possible into our holding buffer
    buffer += consume;
    totalConsumed += consume;
    cnt -= consume;
    if (cnt > 0)
    {
        if (fBufferSize == 0)
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Allocating " << cnt << " bytes to buffer");
            fBuffer = (char *) malloc(cnt * sizeof(char));
            if (fBuffer != NULL)
            {
                fBufferSize += cnt;
                fBufferHead = 0;
                consume = cnt;
            }
            else
                LOG4CXX_ERROR(xmlHttpStreamLog, "Failed to allocate memory");
        }
        else
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "Reallocating buffer to " << (fBufferSize + cnt + 1) * sizeof(char) << " bytes");
            fBuffer = (char *) realloc(fBuffer,
                    (fBufferSize + cnt + 1) * sizeof(char));
            if (fBuffer != NULL)
            {
                fBufferSize += cnt;
                consume = cnt;
            }
            else
                LOG4CXX_ERROR(xmlHttpStreamLog, "Failed to allocate memory");

        }

        memcpy(fBuffer + fBufferHead, buffer, consume);
        fBufferHead += consume;
        buffer += consume;
        totalConsumed += consume;
        //LOG4CXX_DEBUG(xmlHttpStreamLog, "write callback rebuffering " << consume << " bytes (total size: " << fBufferSize << ")");
    }

    // Return the total amount we've consumed. If we don't consume all the bytes
    // then an error will be generated. Since our buffer size is equal to the
    // maximum size that curl will write, this should never happen unless there
    // is a logic error somewhere here.

    return totalConsumed;
}

string HttpStream::getHttpMsg(int httpstatuscode)
{
    switch (httpstatuscode)
    {
    case 100:
        return "100 Continue";
    case 101:
        return "101 Switching Protocols";
    case 102:
        return "102 Processing";
    case 200:
        return "200 OK";
    case 201:
        return "201 Created";
    case 202:
        return "202 Accepted";
    case 203:
        return "203 Non-Authoritative Information";
    case 204:
        return "204 No Content";
    case 205:
        return "205 Reset Content";
    case 206:
        return "206 Partial Content";
    case 207:
        return "207 Multi-Status";
    case 300:
        return "300 Multiple Choices";
    case 301:
        return "301 Moved Permanently";
    case 302:
        return "302 Found";
    case 303:
        return "303 See Other";
    case 304:
        return "304 Not Modified";
    case 305:
        return "305 Use Proxy";
    case 307:
        return "307 Temporary Redirect";
    case 400:
        return "400 Bad Request";
    case 401:
        return "401 Unauthorized";
    case 402:
        return "402 Payment Granted";
    case 403:
        return "403 Forbidden";
    case 404:
        return "404 File Not Found";
    case 405:
        return "405 Method Not Allowed";
    case 406:
        return "406 Not Acceptable";
    case 407:
        return "407 Proxy Authentication Required";
    case 408:
        return "408 Request Time-out";
    case 409:
        return "409 Conflict";
    case 410:
        return "410 Gone";
    case 411:
        return "411 Length Required";
    case 412:
        return "412 Precondition Failed";
    case 413:
        return "413 Request Entity Too Large";
    case 414:
        return "414 Request-URI Too Large";
    case 415:
        return "415 Unsupported Media Type";
    case 416:
        return "416 Requested range not satisfiable";
    case 417:
        return "417 Expectation Failed";
    case 422:
        return "422 Unprocessable Entity";
    case 423:
        return "423 Locked";
    case 424:
        return "424 Failed Dependency";
    case 500:
        return "500 Internal Server Error";
    case 501:
        return "501 Not Implemented";
    case 502:
        return "502 Bad Gateway";
    case 503:
        return "503 Service Unavailable";
    case 504:
        return "504 Gateway Time-out";
    case 505:
        return "505 HTTP Version not supported";
    case 507:
        return "507 Insufficient Storage";
    default:
        break;
    }
    return "unknown http status code";
}

int HttpStream::readBytes(char* const toFill, const unsigned int maxToRead)
{
    fBytesRead = 0;
    fBytesToRead = maxToRead;
    fWritePtr = toFill;

    //LOG4CXX_DEBUG(xmlHttpStreamLog, "trying to read " << maxToRead << " bytes @ " << fTotalBytesRead);

    for (bool tryAgain = true;
            fBytesToRead > 0 && (tryAgain || fBytesRead == 0)
                    && !bStreamFromCache;)
    {
        // First, any buffered data we have available
        size_t bufCnt = fBufferSize;
        bufCnt = (bufCnt > fBytesToRead) ? fBytesToRead : bufCnt;
        if (bufCnt > 0)
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "consuming " << bufCnt << " buffered bytes");

            memcpy(fWritePtr, fBuffer, bufCnt);
            fWritePtr += bufCnt;
            fBytesRead += bufCnt;
            fTotalBytesRead += bufCnt;
            fBytesToRead -= bufCnt;

            if (fBufferSize - bufCnt > 0)
            {
                memcpy(fBuffer, fBuffer + bufCnt, fBufferSize - bufCnt);
                fTotalBytesWrite += bufCnt;
                fBufferSize -= bufCnt;
                fBufferHead -= bufCnt;

                //LOG4CXX_DEBUG(xmlHttpStreamLog, "Reallocating buffer to " << fBufferSize * sizeof(char) << " bytes");
                fBuffer = (char *) realloc(fBuffer, fBufferSize * sizeof(char));
                if (fBuffer == NULL)
                {
                    LOG4CXX_ERROR(xmlHttpStreamLog,
                            "Failed to allocate memory");
                    fBufferSize = 0;
                }
            }
            else
            {
                free(fBuffer);
                fBuffer = NULL;
                fBufferSize = 0;
                fBufferHead = 0;
            }
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "consuming " << bufCnt << " buffered bytes");

            tryAgain = true;
            continue;
        }

        // Ask the curl to do some work
        int runningHandles = 0;
        CURLMcode curlResult = curl_multi_perform(fMulti, &runningHandles);
        tryAgain = (curlResult == CURLM_CALL_MULTI_PERFORM);

        // Process messages from curl
        int msgsInQueue = 0;
        for (CURLMsg* msg = NULL;
                (msg = curl_multi_info_read(fMulti, &msgsInQueue)) != NULL;)
        {
            int httpcode = 0;

            //LOG4CXX_DEBUG(xmlHttpStreamLog, "msg " << msg->msg << ", " << msg->data.result << " from curl (" << runningHandles << " handles)");

            if (msg->msg != CURLMSG_DONE)
                continue;

            switch (msg->data.result)
            {
            case CURLE_OK:
                // We completed, now check the response code of the document
                curl_easy_getinfo(fEasy, CURLINFO_RESPONSE_CODE, &httpcode);
                //LOG4CXX_DEBUG(xmlHttpStreamLog, httpcode << " read " << fTotalBytesRead << " bytes from " << sURL);
                switch (httpcode)
                {
                case 0:
                case 200:
                    // Tell the destructor to store the cache entry since everything is ok
                    if (USE_CACHE && bUseCache)
                        cacheObject->setHttpCode(httpcode);
                    tryAgain = false;
                    break;

                case 301:
                case 302:
                    if (USE_CACHE && bUseCache)
                        cacheObject->setHttpCode(httpcode);
                    if (fLocation != NULL)
                    {
                        LOG4CXX_WARN(xmlHttpStreamLog,
                                "Got redirect, using '" << fLocation << "' as new location");
                        string newlocation = fLocation;
                        destroyConnection();
                        curl_multi_remove_handle(fMulti, fEasy);
                        string oldLocation = sURL;
                        sURL = newlocation;

                        resetBuffer();
                        CacheObject *pCache = NULL;
                        if (USE_CACHE && bUseCache)
                            pCache =
                                    DataStreamHandler::Instance()->getCacheObject(
                                            sURL);
                        setupConnection(pCache);

                        string username = url2username(oldLocation);
                        string password = url2password(oldLocation);
                        if (username != "" && password != "")
                        {
                            if (url2hostname(oldLocation)
                                    == url2hostname(newlocation))
                            { //redirect auth allowed to the same server only
                                string userpwd = username + ":" + password;
                                curl_easy_setopt(fEasy, CURLOPT_USERPWD,
                                        userpwd.c_str());
                                LOG4CXX_WARN(xmlHttpStreamLog,
                                        "Got redirect, using '" << username << "' as username");
                            }
                            else
                            {
                                LOG4CXX_ERROR(xmlHttpStreamLog,
                                        "Got redirect to different server (" << newlocation << "), refusing to send authentication");
                            }
                        }

                        curl_multi_add_handle(fMulti, fEasy);

                        runningHandles = 1;
                        fBytesRead = 0;
                        fBytesToRead = maxToRead;
                        fWritePtr = toFill;

                        tryAgain = true;
                    }
                    else
                    {
                        tryAgain = false;
                        mErrorMsg = "Got redirect but no new location";
                        mErrorCode = CONNECT_FAILED;
                        LOG4CXX_ERROR(xmlHttpStreamLog,
                                "%Got redirect but no new location");
                        //throw(XmlError(XML_FROM_HTTP, httpcode, mErrorMsg));
                    }
                    break;

                case 304:
                    LOG4CXX_DEBUG(xmlHttpStreamLog, "HTTP Code " << httpcode);
                    bStreamFromCache = true;
                    tryAgain = false;
                    break;

                case 401: // Unauthorized
                case 403: // Forbidden
                case 407: // Proxy authentication required
                    tryAgain = false;
                    mErrorMsg = getHttpMsg(httpcode);
                    mErrorCode = ACCESS_DENIED;
                    LOG4CXX_ERROR(xmlHttpStreamLog, "HTTP Error " << httpcode);
                    //throw(XmlError(XML_FROM_IO, XML_IO_EACCES, mErrorMsg));
                    break;

                case 400: // Bad request
                case 404: // Not found
                    tryAgain = false;
                    mErrorMsg = getHttpMsg(httpcode);
                    mErrorCode = NOT_FOUND;
                    LOG4CXX_ERROR(xmlHttpStreamLog, "HTTP Error " << httpcode);
                    //throw(XmlError(XML_FROM_IO, XML_IO_ENOENT, mErrorMsg));
                    break;

                default:
                    tryAgain = false;
                    LOG4CXX_ERROR(xmlHttpStreamLog, "HTTP Error " << httpcode);
                    mErrorMsg = getHttpMsg(httpcode);
                    mErrorCode = NOT_FOUND;
                    //throw(XmlError(XML_FROM_IO, XML_IO_EIO, mErrorMsg));
                    break;

                }
                break;

            case CURLE_REMOTE_FILE_NOT_FOUND:
                tryAgain = false;
                mErrorMsg = curl_easy_strerror(msg->data.result);
                mErrorCode = NOT_FOUND;
                LOG4CXX_ERROR(xmlHttpStreamLog, "CURL Error " << mErrorMsg);
                //throw(XmlError(XML_FROM_IO, XML_IO_ENOENT, mErrorMsg));
                break;

            case CURLE_UNSUPPORTED_PROTOCOL:
                tryAgain = false;
                mErrorMsg = curl_easy_strerror(msg->data.result);
                mErrorCode = CONNECT_FAILED;
                LOG4CXX_ERROR(xmlHttpStreamLog, "CURL Error " << mErrorMsg);
                //throw(XmlError(XML_FROM_IO, XML_IO_EFAULT, mErrorMsg));
                break;

            case CURLE_COULDNT_RESOLVE_HOST:
            case CURLE_COULDNT_RESOLVE_PROXY:
                tryAgain = false;
                mErrorMsg = curl_easy_strerror(msg->data.result);
                mErrorCode = CONNECT_FAILED;
                LOG4CXX_ERROR(xmlHttpStreamLog, "CURL Error " << mErrorMsg);
                //throw(XmlError(XML_FROM_IO, XML_IO_EFAULT, mErrorMsg));
                break;

            case CURLE_COULDNT_CONNECT:
                tryAgain = false;
                mErrorMsg = curl_easy_strerror(msg->data.result);
                mErrorCode = CONNECT_FAILED;
                LOG4CXX_ERROR(xmlHttpStreamLog, "CURL Error " << mErrorMsg);
                //throw(XmlError(XML_FROM_IO, XML_IO_EFAULT, mErrorMsg));
                break;

            case CURLE_RECV_ERROR:
                tryAgain = false;
                mErrorMsg = curl_easy_strerror(msg->data.result);
                mErrorCode = READ_FAILED;
                LOG4CXX_ERROR(xmlHttpStreamLog, "CURL Error " << mErrorMsg);
                //throw(XmlError(XML_FROM_IO, XML_IO_EIO, mErrorMsg));
                break;

            default:
                tryAgain = false;
                mErrorMsg = curl_easy_strerror(msg->data.result);
                mErrorCode = READ_FAILED;
                LOG4CXX_ERROR(xmlHttpStreamLog, "CURL Error " << mErrorMsg);
                //throw(XmlError(XML_FROM_HTTP, msg->data.result, mErrorMsg));
                break;
            }
        }

        // If nothing is running any longer, bail out
        if (runningHandles == 0)
        {
            //LOG4CXX_DEBUG(xmlHttpStreamLog, "No more running handles.. bailing out");
            break;
        }
        // If there is no further data to read, and we haven't
        // read any yet on this invocation, call select to wait for data
        if (!tryAgain && fBytesRead == 0)
        {
            fd_set readSet[16];
            fd_set writeSet[16];
            fd_set exceptSet[16];
            int fdcnt = 16;

            memset(&readSet, 0, sizeof(fd_set) * 16);
            memset(&writeSet, 0, sizeof(fd_set) * 16);
            memset(&exceptSet, 0, sizeof(fd_set) * 16);

            // As curl for the file descriptors to wait on
            (void) curl_multi_fdset(fMulti, readSet, writeSet, exceptSet,
                    &fdcnt);

            // Wait on the file descriptors
            timeval tv;
            memset(&tv, 0, sizeof(struct timeval));
            tv.tv_sec = 0;
            tv.tv_usec = 200;
            (void) select(fdcnt, readSet, writeSet, exceptSet, &tv);

            //LOG4CXX_DEBUG(xmlHttpStreamLog, "fBytesRead = " << fBytesRead << ", fBytesToRead " << fBytesToRead);
        }
    }

    // If we are done with the CURL part, start streaming from cache in case we got a 304 response
    if (USE_CACHE && bUseCache && bStreamFromCache)
    {
        //LOG4CXX_DEBUG(xmlHttpStreamLog, "Trying to read " << fBytesToRead << " bytes from cache for " << sURL);

        fBytesRead = cacheObject->readBytes((char *) fWritePtr, fBytesToRead);
        if (fBytesRead == 0)
            cacheObject->resetState();
        else if (fBytesRead < 0)
        {
            mErrorMsg = cacheObject->getErrorMsg();
            mErrorCode = READ_FAILED;
        }

        fTotalBytesRead += fBytesRead;
    }

    //LOG4CXX_DEBUG(xmlHttpStreamLog, "Read " << fBytesRead << " (wanted: " << maxToRead << ") (fTotalBytesRead: " << fTotalBytesRead << " buffered: " << fBufferSize * sizeof(char));

    // Reset the fBytesToRead so that the following data will be buffered instead
    fBytesToRead = 0;

    if (hContent_length_response == fTotalBytesRead)
        bTransferFinished = true;

    // If we have an error return -1
    if (mErrorCode != NONE)
        return -1;

    return fBytesRead;
}

/**
 * Strip the protocol, username, password, and path from an url.
 */
string url2hostname(const string& url)
{
    size_t hostname_beg = url.find("://");
    if (hostname_beg == string::npos) // protocol required
        return "";

    hostname_beg += 3;

    size_t hostname_at = url.find("@", hostname_beg);
    if (hostname_at != string::npos)
        hostname_beg = hostname_at + 1;

    size_t hostname_end = url.find_first_of(":/", hostname_beg);

    if (hostname_end == string::npos)
        return url.substr(hostname_beg);

    return url.substr(hostname_beg, hostname_end - hostname_beg);
}

string url2username(const string& url)
{
    size_t username_beg = url.find("://");
    if (username_beg == string::npos) // protocol required
        return "";

    username_beg += 3;

    size_t username_at = url.find("@", username_beg); // at required
    if (username_at == string::npos)
        return "";

    size_t username_end = url.find(":", username_beg);
    if (username_end == string::npos)
    { // separator not found, username ends with @
        size_t username_len = username_at - username_beg;
        return url.substr(username_beg, username_len);
    }

    if (username_end > username_at)
    { // : is not the separator
        size_t username_len = username_at - username_beg;
        return url.substr(username_beg, username_len);
    }

    size_t username_len = username_end - username_beg;
    return url.substr(username_beg, username_len);
}

string url2password(const string& url)
{
    size_t userpwd_beg = url.find("://");
    if (userpwd_beg == string::npos) // protocol required
        return "";

    userpwd_beg += 3;

    size_t userpwd_at = url.find("@", userpwd_beg);
    if (userpwd_at == string::npos) // url contains no credentials
        return "";

    size_t userpwd_sep = url.find(":", userpwd_beg);
    if (userpwd_at == string::npos) // url contains no passwd
        return "";
    if (userpwd_at < userpwd_sep) // url contains no passwd
        return "";

    userpwd_beg = userpwd_sep + 1;
    size_t userpwd_end = userpwd_at;
    size_t userpwd_len = userpwd_end - userpwd_beg;
    return url.substr(userpwd_beg, userpwd_len);
}
