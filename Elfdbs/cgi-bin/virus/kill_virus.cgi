#!/bin/bash
echo "content-type:text/plain"
echo ""

#scan the directory and remove kill the virus
sudo clamscan -r /tmp
echo ""
echo ""
sudo clamscan -r --remove /tmp
