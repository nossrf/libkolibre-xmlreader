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

#ifndef XMLINPUTSOURCE_H
#define XMLINPUTSOURCE_H

#include "InputStream.h"
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

class KOLIBRE_API XmlInputSource
{
public:
    virtual InputStream * makeStream() const = 0;
    virtual std::string getUrl() const = 0;

private:
    // Unimplemented
    XmlInputSource(const XmlInputSource&);
    XmlInputSource& operator=(const XmlInputSource&);
protected:
    XmlInputSource(){};
};

#endif
