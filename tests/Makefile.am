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

check_PROGRAMS = cachecheck parsedoctype parsetest parsexmlbom urlextract
TESTS = cachecheck.sh parsedoctype.sh parsetest.sh parsexmlbom.sh urlextract

cachecheck_SOURCES = cachecheck.cpp
cachecheck_CPPFLAGS = -I$(top_srcdir)/src -g @LOG4CXX_CFLAGS@
cachecheck_LDADD = -L$(top_builddir)/src/ -lkolibre-xmlreader @LOG4CXX_LIBS@

parsedoctype_SOURCES = parsedoctype.cpp
parsedoctype_CPPFLAGS = -I$(top_srcdir)/src -g @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@
parsedoctype_LDADD = -L$(top_builddir)/src/ -lkolibre-xmlreader @LOG4CXX_LIBS@

parsetest_SOURCES = parsetest.cpp
parsetest_CPPFLAGS = -I$(top_srcdir)/src -g @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@
parsetest_LDADD = -L$(top_builddir)/src/ -lkolibre-xmlreader @LOG4CXX_LIBS@

parsexmlbom_SOURCES = parsexmlbom.cpp
parsexmlbom_CPPFLAGS = -I$(top_srcdir)/src -g @LIBXML2_CFLAGS@ @LOG4CXX_CFLAGS@
parsexmlbom_LDADD = -L$(top_builddir)/src/ -lkolibre-xmlreader @LOG4CXX_LIBS@

urlextract_SOURCES = urlextract.cpp
urlextract_CPPFLAGS = -I$(top_srcdir)/src -g @LOG4CXX_CFLAGS@
urlextract_LDADD = -L$(top_builddir)/src/ -lkolibre-xmlreader @LOG4CXX_LIBS@

EXTRA_DIST = cachecheck.sh \
			 parsedoctype.sh \
			 parsetest.sh \
			 parsexmlbom.sh \
			 setup_logging.h \
			 run \
			 testdata

clean-local: clean-local-check
.PHONY: clean-local-check

clean-local-check:
	-rm -rf *.log
