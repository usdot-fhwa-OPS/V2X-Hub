#!/bin/bash

set -e

mkdir -p temp
cd temp
asn1c -gen-PER -fcompound-names -fincludes-quoted -fskeletons-copy ../J2735_R41_Source_mod.ASN
cd ..
rm -rf source include
mkdir source
mkdir include
mkdir include/asn_j2735_r41
mv temp/*.c source/.
mv temp/*.h include/asn_j2735_r41/.

rm -rf temp

rm source/converter-sample.c

find include/asn_j2735_r41 -maxdepth 1 -type f -exec sed '/^#[[:blank:]]*include/s/<\([^>]*\)>/"\1"/' -i {} \;

sed -e "17s/type/void/" -i include/asn_j2735_r41/asn_SET_OF.h
