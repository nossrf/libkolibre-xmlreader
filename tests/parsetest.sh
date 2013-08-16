#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result -batch -x ${srcdir:-.}/run --args"
fi

# local resources
$PREFIX ./parsetest ${srcdir:-.}/testdata/ncc.html
$PREFIX ./parsetest ${srcdir:-.}/testdata/nstest.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample2.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample3.xml
$PREFIX ./parsetest ${srcdir:-.}/testdata/sample2_errors.xml fail

# online resources (GDB is setup in httptester.sh)
${srcdir:-.}/httptester.sh ${bindir:-.}/parsetest note.xml
${srcdir:-.}/httptester.sh ${bindir:-.}/parsetest note.xml fail
${srcdir:-.}/httptester.sh ${bindir:-.}/parsetest 1998validstats.xml
# the last test fails due to "Partial transfer" and I don't know why
#${srcdir:-.}/httptester.sh ${bindir:-.}/parsetest 1998styledstatistics.xml
