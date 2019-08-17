#!/usr/bin/env bash
LIME_UTIL=../../lime/bin/lime
RESOURCE_MANIFEST=resources.manifest
OUTPUT_FILE=demo.dat
if [ ! -f "$LIME_UTIL" ]; then
    echo -e "\nCan't locate the Lime utility! Did you forget to build it?\n"
else
    ./$LIME_UTIL $RESOURCE_MANIFEST $OUTPUT_FILE -clevel=9 -head="Lime Demo" -chksum=adler32
fi
