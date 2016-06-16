#!/bin/bash

echo "Content-type: text/html"
echo ""

echo "<h4>Before reduce,the binary file's size is :<h4>"
echo "<br>"
ls -l /tmp/hello
echo "<br>"
./sstrip.cgi /tmp/hello

echo "<br>"
echo "<br>"
echo "<h4>After reduced,the new binay's size is :<h4>"
echo "<br>"
ls -l /tmp/hello
