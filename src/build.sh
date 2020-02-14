#!/bin/bash
cd tmx
cmake .
make
make install  

cd ../v2i-hub
cmake .
make