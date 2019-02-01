#!/bin/sh

#
# This script determines the recursive list of package dependencies
# for the installed software and where to obtain them from.
#

DIR_NAME=`dirname $0`
PROGRAM_NAME=`basename $0`
CWD=`pwd`

if [ "$1" = "-l" ]; then
	# List the dynamic dependencies to obtain the list of files
	shift
	
	set -- `ldd "${@}" | awk '{print $1}' | grep -v ":$" | sort -u` 
fi 

PKGS=`dpkg -S "${@}" | awk '{print $1}' | sed -e 's/:$//' | sort -u`
apt-cache depends --recurse -i ${PKGS} | awk '$1 ~ /Depends/ {print $2}' | sort -u

exit 0