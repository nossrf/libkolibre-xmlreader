#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb -return-child-result -x ${srcdir:-.}/run --args"
fi

# local resources
$PREFIX ./parsetest ${srcdir:-.}/testdata/ncc.html
$PREFIX ./parsetest ${srcdir:-.}/testdata/nstest.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample2.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample3.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample2_errors.xml fail

# online resources
$PREFIX ./parsetest http://www.w3schools.com/xml/note.xml
$PREFIX ./parsetest http://www.w3schools.com/xml/note_error.xml fail
$PREFIX ./parsetest http://www.cafeconleche.org/examples/1998validstats.xml
$PREFIX ./parsetest http://www.cafeconleche.org/examples/1998styledstatistics.xml
