#!/bin/sh

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result -batch -x ${srcdir:-.}/run --args"
fi

$PREFIX ./parsexmlbom ${srcdir:-.}/testdata/utf8-bom.html
$PREFIX ./parsexmlbom ${srcdir:-.}/testdata/utf8-no-bom.html
$PREFIX ./parsexmlbom ${srcdir:-.}/testdata/utf8-bom.xml
$PREFIX ./parsexmlbom ${srcdir:-.}/testdata/utf8-no-bom.xml
