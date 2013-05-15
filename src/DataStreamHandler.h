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

#ifndef DATASTREAMHANDLER_H
#define DATASTREAMHANDLER_H

#include <curl/curl.h>
#include <curl/multi.h>
#include <curl/easy.h>
#include <pthread.h>
#include <string>
#include <queue>
#include <map>

#include "InputStream.h"

#define USE_PIPELINEING 1
#define USE_CACHE 1

#ifdef WIN32
#ifdef KOLIBRE_DLL
#define KOLIBRE_API __declspec(dllexport)
#else
#define KOLIBRE_API __declspec(dllimport)
#endif
#else
#define KOLIBRE_API
#endif

// Forward declaration of class CacheObject, keeps interface clean
class CacheObject;

//
// This class acts as a handler for all the active DataStreams
//

class KOLIBRE_API DataStreamHandler
{
public:
    static DataStreamHandler* Instance();
    void DestroyInstance();
    ~DataStreamHandler();

    InputStream* newStream(std::string url, bool tidy = false, bool useCache = true);

    void setUseragent(std::string useragent); // Useragent string to use
    void setTimeout(unsigned int timeout); // Timeout in seconds
    void setDebugmode(bool setting); // Will make transfers verbose (LOG_DEBUG)

    // only used internally by xmlreader
    bool addCacheObject(std::string, CacheObject *);
    CacheObject *getCacheObject(const std::string &);
    void releaseHandle(CURL *fEasy);

private:

    //singleton instance
    static DataStreamHandler* pinstance;

    CURLM* fMulti;
    CURLSH* fShare;

    // Lock/unlock functions for shared curl data
    static void staticLockCallback(CURL *handle, curl_lock_data data,
            curl_lock_access access, void *handler);
    void lockCallback(CURL *handle, curl_lock_data data,
            curl_lock_access access);

    static void staticUnlockCallback(CURL *handle, curl_lock_data data,
            void *handler);
    void unlockCallback(CURL *handle, curl_lock_data data);

    // Curl data mutexes
    pthread_mutex_t CURL_LOCK_DATA_SHARE_MUTEX;
    pthread_mutex_t CURL_LOCK_DATA_COOKIE_MUTEX;
    pthread_mutex_t CURL_LOCK_DATA_DNS_MUTEX;
    pthread_mutex_t CURL_LOCK_DATA_SSL_SESSION_MUTEX;
    pthread_mutex_t CURL_LOCK_DATA_CONNECT_MUTEX;

    // Debug functions
    static size_t staticDebugCallback(CURL *handle, curl_infotype type,
            char *msg, size_t msgsize, void *outstream);
    size_t debugCallback(CURL *handle, curl_infotype type, char *msg,
            size_t msgsize);

    DataStreamHandler();

    std::queue<CURL *> freeHandles;

    // Http Cache variables
    std::map<std::string, CacheObject*> HttpCache;
    std::map<std::string, CacheObject*>::iterator itHttpCache;

    std::string mUseragent;
    unsigned int mTimeout;
    bool bDebugmode;

    void checkCacheSize(CacheObject *);
};

#endif
