#!/bin/sh
cd libstream;make clean;make;cd ..
cd testlibstream;make clean;make;cd ..
echo "OK"
