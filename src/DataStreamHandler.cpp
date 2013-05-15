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
 * \class DataStreamHandler
 *
 * \brief Interface for controlling fetching of online resources or download a resource via a stream object.
 *
 * \note This class is a singleton.
 *
 * \author Kolibre (www.kolibre.org)
 *
 * \b Contact: info@kolibre.org
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE "kolibre-xmlreader"
#define VERSION "x.y.z"
#endif
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.xmlreader
log4cxx::LoggerPtr xmlDataStreamHlrLog(
        log4cxx::Logger::getLogger("kolibre.xmlreader.datastreamhandler"));

#include "DataStreamHandler.h"
#include "CacheObject.h"
#include "HttpStream.h"
#include "FileStream.h"
#ifdef HAVE_LIBTIDY
#include "TidyStream.h"
#endif

#define MAX_CACHE_SIZE 4194304 // 2048*2048

using namespace std;

DataStreamHandler* DataStreamHandler::pinstance = 0;

/**
 * Get instance
 *
 * If the instance does not already exist it is created.
 *
 * @return pointer to the instance object
 */
DataStreamHandler* DataStreamHandler::Instance()
{
    if (pinstance == 0) // is it the first call?
    {
        pinstance = new DataStreamHandler; // create sole instance
    }
    return pinstance; // address of sole instance
}

/**
 * Destroy instance
 *
 * The instance and all stored data is deleted.
 */
void DataStreamHandler::DestroyInstance()
{
    delete pinstance;
}

/**
 * Constrcutor
 */
DataStreamHandler::DataStreamHandler()
{
    // Allocate the curl multi handle
    fMulti = curl_multi_init();

    // Allocate the curl share handle
    fShare = curl_share_init();

    // Initialize curl data share locking mechanisms
    pthread_mutex_init(&CURL_LOCK_DATA_SHARE_MUTEX, NULL);
    pthread_mutex_init(&CURL_LOCK_DATA_COOKIE_MUTEX, NULL);
    pthread_mutex_init(&CURL_LOCK_DATA_DNS_MUTEX, NULL);
    pthread_mutex_init(&CURL_LOCK_DATA_SSL_SESSION_MUTEX, NULL);
    pthread_mutex_init(&CURL_LOCK_DATA_CONNECT_MUTEX, NULL);

    curl_share_setopt(fShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(fShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    curl_share_setopt(fShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    curl_share_setopt(fShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);

    curl_share_setopt(fShare, CURLSHOPT_USERDATA, this);
    curl_share_setopt(fShare, CURLSHOPT_LOCKFUNC, staticLockCallback);
    curl_share_setopt(fShare, CURLSHOPT_UNLOCKFUNC, staticUnlockCallback);

    mUseragent = string(PACKAGE)+"/"+string(VERSION);
    mTimeout = 30;
    bDebugmode = false;
}

/**
 * Destructor
 *
 * CURL handles and cached objects are deleted.
 */
DataStreamHandler::~DataStreamHandler()
{
    // Cleanup the easy handles
    while (freeHandles.size() != 0)
    {
        CURL *fEasy = NULL;
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Destroying CURL easy handle");
        fEasy = freeHandles.front();
        freeHandles.pop();
        if (fEasy != NULL)
            curl_easy_cleanup(fEasy);
    }

    /*
     for(itHttpCache = HttpCache.begin(); itHttpCache != HttpCache.end(); itHttpCache++ ) {
     LOG4CXX_DEBUG(xmlDataStreamHlrLog, (*itHttpCache).first << " => " << (*itHttpCache).second->getContentLength() << "(" << (*itHttpCache).second->getBufferSize() << ")");
     }
     */

    while (!HttpCache.empty())
    {
        LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                "Freeing cacheObject for url: " << HttpCache.begin()->first << " => " << HttpCache.begin()->second->getContentLength() << "(" << HttpCache.begin()->second->getBufferSize() << ")");
        delete HttpCache.begin()->second;
        HttpCache.erase(HttpCache.begin());
    }

    // Cleanup the multi handle
    curl_multi_cleanup(fMulti);

    // Cleanup the share handle
    curl_share_cleanup(fShare);
}

/**
 * Set user agent string that is used when fetching online resources
 *
 * The default value for user agent is kolibe-xmlreader/x.y.z where x.y.z is the version number for xmlreader.
 *
 * @param useragent the user agent string to be set
 */
void DataStreamHandler::setUseragent(std::string useragent)
{
    mUseragent = useragent;
}

/**
 * Set the connection time-out value that is used when fetching online resources
 *
 * The default value for time-out is 30 seconds.
 *
 * @param timeout the time-out value in seconds
 */
void DataStreamHandler::setTimeout(unsigned int timeout)
{
    mTimeout = timeout;
}

/**
 * Toggle debug mode on or off
 *
 * The default value for debug mode is off.
 *
 * @param setting true for on, false for off
 */
void DataStreamHandler::setDebugmode(bool setting)
{
    bDebugmode = setting;
}

/**
 * Create data stream from an URL
 *
 * @param url the url for the online resource
 * @param tidy parse the resource with libtidy
 * @param useCache store resouce in cache
 */
InputStream* DataStreamHandler::newStream(std::string url, bool tidy, bool useCache)
{
    // cacheobject for this URL
    CacheObject *cacheObject = NULL;

    if (url.find("http") == 0)
    {
        // Create the HttpStream
        CURL *fEasy = NULL;

        // If we have already allocated handles free, use one of them
        if (freeHandles.size() != 0 && USE_PIPELINEING)
        {
            LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Reusing stream for " << url);

            fEasy = freeHandles.front();
            freeHandles.pop();

            // Set timeout and useragent strings in case they have changed
            curl_easy_setopt(fEasy, CURLOPT_USERAGENT, mUseragent.c_str());
            curl_easy_setopt(fEasy, CURLOPT_CONNECTTIMEOUT, mTimeout);
            if (bDebugmode)
                curl_easy_setopt(fEasy, CURLOPT_VERBOSE, true);
            else
                curl_easy_setopt(fEasy, CURLOPT_VERBOSE, false);
        }
        else
        {
            // Allocate the curl easy handle
            fEasy = curl_easy_init();

            // Set URL option
            curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYHOST, false);
            curl_easy_setopt(fEasy, CURLOPT_BUFFERSIZE, CURL_MAX_WRITE_SIZE);
            curl_easy_setopt(fEasy, CURLOPT_ENCODING,
                    "compress;q=0.5, gzip;q=1.0");
            curl_easy_setopt(fEasy, CURLOPT_SHARE, fShare);
            curl_easy_setopt(fEasy, CURLOPT_AUTOREFERER, true);
            //curl_easy_setopt(fEasy, CURLOPT_FOLLOWLOCATION, true);
            curl_easy_setopt(fEasy, CURLOPT_MAXREDIRS, 10);

            curl_easy_setopt(fEasy, CURLOPT_USERAGENT, mUseragent.c_str());
            curl_easy_setopt(fEasy, CURLOPT_CONNECTTIMEOUT, mTimeout);

            if (bDebugmode)
                curl_easy_setopt(fEasy, CURLOPT_VERBOSE, true);
        }

        // Check to see if we have a cached item for this URL
        if (USE_CACHE && useCache)
        {
            cacheObject = getCacheObject(url);
        }

        HttpStream *newStream = NULL;
        newStream = new HttpStream(url, fEasy, fMulti, cacheObject);
        newStream->useCache(useCache);

        // Add easy handle to the multi stack
        curl_multi_add_handle(fMulti, fEasy);
#ifdef HAVE_LIBTIDY
        if (tidy)
            return new TidyStream(url, newStream);
        else
#endif
            return newStream;
    }
    else
    {
        // Check to see if we have a cached item for this URL
        if (USE_CACHE && useCache)
        {
            cacheObject = getCacheObject(url);
        }

        // Create a filestream
        FileStream *newStream = NULL;
        newStream = new FileStream(url, cacheObject);
        newStream->useCache(useCache);
#ifdef HAVE_LIBTIDY
        if (tidy)
            return new TidyStream(url, newStream);
        else
#endif
            return newStream;
    }

    return NULL;
}

/**
 * Release CURL easy handler
 *
 * Do not invoke this method. It shall only be used internally by xmlreader.
 *
 * @param fEasy pointer to a CURL handle
 */
void DataStreamHandler::releaseHandle(CURL *fEasy)
{
    // Remove the easy handle from the multi stack
    curl_multi_remove_handle(fMulti, fEasy);

    if (USE_PIPELINEING)
    {
        // Store the easy handle
        freeHandles.push(fEasy);
    }
    else
    {
        curl_easy_cleanup(fEasy);
    }
}

/**
 * Check and free cached objects
 *
 * Cached objects are freed until current cache size is smaller then MAX_CACHE_SIZE.
 *
 * @param item
 */
void DataStreamHandler::checkCacheSize(CacheObject *item)
{
    // Check that the cache size don't exceed the maximum size allowed

    unsigned long cacheSize = 0;
    for (itHttpCache = HttpCache.begin(); itHttpCache != HttpCache.end();
            itHttpCache++)
    {
        cacheSize += (*itHttpCache).second->getBufferSize();

        //LOG4CXX_DEBUG(xmlDataStreamHlrLog, (*itHttpCache).first << " => " << (*itHttpCache).second->getContentLength() << "(" << (*itHttpCache).second->getBufferSize() << ")");
    }

    itHttpCache = HttpCache.end();
    while (cacheSize > MAX_CACHE_SIZE && --itHttpCache != HttpCache.begin())
    {
        if ((*itHttpCache).second != NULL && (*itHttpCache).second != item
                && (*itHttpCache).second->getState() != CacheObject::BUSY)
        {

            cacheSize -= (*itHttpCache).second->getBufferSize();

            LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                    "Freeing " << (*itHttpCache).second->getBufferSize() << " bytes for url '" << (*itHttpCache).first << "'");
            LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                    "Cache size after: " << cacheSize);

            delete (*itHttpCache).second;
            HttpCache.erase(itHttpCache);
        }
    }
}

/**
 * Add resource as a cached object
 *
 * Do not invoke this method. It shall only be used internally by xmlreader.
 *
 * @param url the url of the resource
 * @param item pointer to the cached object
 * @return boolean of the result
 * @retval true when the object was added or already cached
 * @retval false when the object was not added
 */
bool DataStreamHandler::addCacheObject(std::string url, CacheObject *item)
{
    // Add an object to a cache, identify it by the URL
    LOG4CXX_DEBUG(xmlDataStreamHlrLog, "adding object " << url);
    if (USE_CACHE)
    {
        // Check if we already have a cache item for this url
        itHttpCache = HttpCache.find(url);
        if (itHttpCache != HttpCache.end() && (*itHttpCache).second != item)
        {

            if ((*itHttpCache).second->getState() == CacheObject::BUSY)
            {
                LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                        "Not replacing busy cacheObject for url '" << (*itHttpCache).first << "'");
                delete item;
                return true;
            }

            LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                    "Replacing cacheObject for url '" << (*itHttpCache).first << "'");
            LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                    "Freeing cacheObject for url '" << (*itHttpCache).first << "' addr: " << (*itHttpCache).second << "/" << item);
            delete (*itHttpCache).second;
            HttpCache.erase(itHttpCache);
        }
        else
        {
            LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                    "Adding cacheObject for url '" << url << "', size " << item->getBufferSize());
        }

        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Adding cache object for " << url);

        HttpCache.insert(pair<std::string, CacheObject*>(url, item));

        checkCacheSize(item);

        return true;
    }

    return false;
}

/**
 * Retrieve resource as a cached object based on url
 *
 * Do not invoke this method. It shall only be used internally by xmlreader.
 *
 * @param url the url of the resource
 * @return pointer to the cached object
 * @retval NULL if the object not was found
 */
CacheObject *DataStreamHandler::getCacheObject(const std::string &url)
{
    CacheObject *cacheObject = NULL;

    LOG4CXX_DEBUG(xmlDataStreamHlrLog,
            "Getting cacheobject for " << url << " cache size: " << HttpCache.size());

    /*
     for(itHttpCache = HttpCache.begin(); itHttpCache != HttpCache.end(); itHttpCache++ ) {
     LOG4CXX_DEBUG(xmlDataStreamHlrLog, *(itHttpCache).first << " => " << (*itHttpCache).second->getContentLength() << "(" << (*itHttpCache).second->getBufferSize() << ")");
     }
     */

    itHttpCache = HttpCache.find(url);
    if (itHttpCache != HttpCache.end())
    {
        if ((*itHttpCache).second->getState() != CacheObject::BUSY)
            cacheObject = (*itHttpCache).second;
        else
            LOG4CXX_WARN(xmlDataStreamHlrLog,
                    "cacheObject for '" << url << "' BUSY");
    }

    return cacheObject;
}

/**
 * Implements a static lock callback function
 *
 * @param handle pointer to a CURL handle
 * @param data CURL lock data
 * @param access curl lock access
 * @param handler void pointer for type casting
 */
void DataStreamHandler::staticLockCallback(CURL *handle, curl_lock_data data,
        curl_lock_access access, void *handler)
{
    ((DataStreamHandler*) handler)->lockCallback(handle, data, access);
}

/**
 * Implements a lock callback function
 *
 * @param handle pointer to a CURL handle
 * @param data CURL lock data
 * @param access CURL access data
 */
void DataStreamHandler::lockCallback(CURL *handle, curl_lock_data data,
        curl_lock_access access)
{
    switch (data)
    {
    case CURL_LOCK_DATA_SHARE:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Locking CURL_LOCK_DATA_SHARE");
        pthread_mutex_lock(&CURL_LOCK_DATA_SHARE_MUTEX);
        break;
    case CURL_LOCK_DATA_COOKIE:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Locking CURL_LOCK_DATA_COOKIE");
        pthread_mutex_lock(&CURL_LOCK_DATA_COOKIE_MUTEX);
        break;
    case CURL_LOCK_DATA_DNS:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Locking CURL_LOCK_DATA_DNS");
        pthread_mutex_lock(&CURL_LOCK_DATA_DNS_MUTEX);
        break;
    case CURL_LOCK_DATA_SSL_SESSION:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                "Locking CURL_LOCK_DATA_SSL_SESSION");
        pthread_mutex_lock(&CURL_LOCK_DATA_SSL_SESSION_MUTEX);
        break;
    case CURL_LOCK_DATA_CONNECT:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Locking CURL_LOCK_DATA_CONNECT");
        pthread_mutex_lock(&CURL_LOCK_DATA_CONNECT_MUTEX);
        break;
    default:
        LOG4CXX_WARN(xmlDataStreamHlrLog, "Unknown data to lock " << data);
        break;
    }
}

/**
 * Implements a static unlock callback function
 *
 * @param handle pointer to a CURL handle
 * @param data CURL lock data
 * @param handler void pointer for type casting
 */
void DataStreamHandler::staticUnlockCallback(CURL *handle, curl_lock_data data,
        void *handler)
{
    ((DataStreamHandler*) handler)->unlockCallback(handle, data);
}

/**
 * Implements a unlock callback function
 *
 * @param handle pointer to a CURL handle
 * @param data CURL lock data
 */
void DataStreamHandler::unlockCallback(CURL *handle, curl_lock_data data)
{
    switch (data)
    {
    case CURL_LOCK_DATA_SHARE:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Unlocking CURL_LOCK_DATA_SHARE");
        pthread_mutex_unlock(&CURL_LOCK_DATA_SHARE_MUTEX);
        break;
    case CURL_LOCK_DATA_COOKIE:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Unlocking CURL_LOCK_DATA_COOKIE");
        pthread_mutex_unlock(&CURL_LOCK_DATA_COOKIE_MUTEX);
        break;
    case CURL_LOCK_DATA_DNS:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Unlocking CURL_LOCK_DATA_DNS");
        pthread_mutex_unlock(&CURL_LOCK_DATA_DNS_MUTEX);
        break;
    case CURL_LOCK_DATA_SSL_SESSION:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog,
                "Unlocking CURL_LOCK_DATA_SSL_SESSION");
        pthread_mutex_unlock(&CURL_LOCK_DATA_SSL_SESSION_MUTEX);
        break;
    case CURL_LOCK_DATA_CONNECT:
        LOG4CXX_DEBUG(xmlDataStreamHlrLog, "Unlocking CURL_LOCK_DATA_CONNECT");
        pthread_mutex_unlock(&CURL_LOCK_DATA_CONNECT_MUTEX);
        break;
    default:
        LOG4CXX_WARN(xmlDataStreamHlrLog, "Unknown data to unlock " << data);
        break;
    }
}
