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

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "XmlInputSource.h"
#include <string>

#ifdef WIN32
#ifdef KOLIBRE_DLL
#define KOLIBRE_API __declspec(dllexport)
#else
#define KOLIBRE_API __declspec(dllimport)
#endif
#else
#define KOLIBRE_API
#endif

class KOLIBRE_API DataSource: public XmlInputSource
{
public:
    DataSource(std::string url, bool tidyFlag = false);
    virtual ~DataSource();

    InputStream* makeStream() const;

    std::string getUrl() const;

private:
    DataSource(const DataSource&);
    DataSource& operator=(const DataSource&);

    InputStream *inpstr;
    std::string sUrl;
};

#endif
