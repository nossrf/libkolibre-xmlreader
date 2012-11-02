## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-xmlreader.
#
# Kolibre-xmlreader is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-xmlreader is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-xmlreader. If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = foreign

HDRS = DataStreamHandler.h \
	   InputStream.h \
	   XmlAttributes.h \
	   XmlDefaultHandler.h \
	   XmlError.h \
	   XmlReader.h

SRCS = CacheObject.cpp \
	   DataSource.cpp \
	   DataStreamHandler.cpp \
	   FileStream.cpp \
	   HttpStream.cpp \
	   TidyStream.cpp \
	   XmlAttributes.cpp \
	   XmlDefaultHandler.cpp \
	   XmlReader.cpp

lib_LTLIBRARIES = libkolibre-xmlreader.la

# Install the headers in a versioned directory - e.g. examplelib-1.0:
library_includedir = $(includedir)/libkolibre/xmlreader-$(PACKAGE_VERSION)
library_include_HEADERS = $(HDRS)

libkolibre_xmlreader_la_SOURCES = $(SRCS)
libkolibre_xmlreader_la_LDFLAGS = -version-info $(VERSION_INFO) @LIBXML2_LIBS@ @LIBTIDY_LIBS@ @LOG4CXX_LIBS@ @LIBCURL_LIBS@
libkolibre_xmlreader_la_CPPFLAGS = @LOG4CXX_CFLAGS@ @LIBCURL_CFLAGS@ @LIBXML2_CFLAGS@ @LIBTIDY_CFLAGS@

EXTRA_DIST = CacheObject.h \
			 DataSource.h \
			 FileStream.h \
			 HttpStream.h \
			 TidyStream.h \
			 XmlInputSource.h
