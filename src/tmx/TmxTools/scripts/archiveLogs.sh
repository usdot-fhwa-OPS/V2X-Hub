#!/bin/bash

# 
# Script to archive the TMX logs
#

DIR_NAME=`dirname $0`
PROGRAM_NAME=`basename $0`
CWD=`pwd`

PCAP_LOG=0
EVENT_LOG=0
MESSAGE_LOG=0
DEBUG_LOG=0
REMOVE=0
OVERWRITE=0
DATE=`date +"%d-%b-%Y"`

function Usage {
	echo "Usage: $PROGRAM_NAME [ -a ] [ -c ] [ -e ] [ -l ] [ -m ] [ -r ] [ -O ]" >&2
	echo "                     [ -d <date> ] [ -o <outputfile > ]" >&2
	echo >&2
	echo "-a => Grab all the logs" >&2
	echo >&2
	echo "-c => Grab the pcap logs from the Cohda directory" >&2
	echo "-e => Grab the event log from the database." >&2
	echo "-l => Grab the debug log from upstart" >&2
	echo "-m => Grab the message log from the database." >&2
	echo "-r => Remove the log files after archival.  Default is false" >&2
	echo "-O => Overwrite existing archive file if one exists.  Normally, the archive will be updated." >&2
	echo "-d <date> => Specifies which date to pull from (dd-Mon-YYYY).  Default is today (e.g. ${DATE})" >&2
	echo "-o <outputfile> => Specifies what archive file to build" >&2
}

# Gather arguments
while getopts :acd:elmro: opt; do
	case "$opt" in
		a)	PCAP_LOG=1; 
			EVENT_LOG=1;
			MESSAGE_LOG=1;
			DEBUG_LOG=1;;
		c)	PCAP_LOG=1;;
		d)	DATE="$OPTARG";;
		e)  EVENT_LOG=1;;
		l)	DEBUG_LOG=1;;
		m)  MESSAGE_LOG=1;;
		r)	REMOVE=1;;
		O)  OVERWRITE=1;;
		o)  OUT_FILE="$OPTARG";;
		*)	Usage; exit 1;;
	esac
done

# Check that the date is correct
TODAY=`date --date="${DATE}"`
if [ -z "${TODAY}" ]; then
	Usage
	exit 1
fi

TOMORROW=$((`date --date="${TODAY}" +%s` + 24*60*60))
TOMORROW=`date --date="@${TOMORROW}"`


if [ -z "${OUT_FILE}" ]; then
	OUT_FILE="${CWD}/TMXlogs-${DATE}.zip"
fi

if [ -f "${OUT_FILE}" ]; then
i	if [ "${OVERWRITE} -gt 0 ]; then
		rm -f "${OUT_FILE}"
	else
		ZIP_ARG="-u"
	fi
fi

ZIP_CMD="zip ${ZIP_ARG} ${OUT_FILE}"

if [ ${PCAP_LOG} -gt 0 && -d /mnt/ubi/log ]; then
	cd /mnt/ubi/log
	${ZIP_CMD} -r `date --date="${TODAY}" +"%Y.%m%d.*"`
	
	[ $? -eq 0 -a ${REMOVE} -gt 0 ] && \
		sudo rm -rf `date --date="${TODAY}" +"%Y.%m%d.*"`
fi

if [ ${DEBUG_LOG} -gt 0 ]; then
	if which journalctl >/dev/null 2>&1; then
		journalctl -t tmxcore -S "${TODAY}" -U "${TOMORROW}" >> /tmp/tmxcore.log
		cd /tmp
		
		sudo ${ZIP_CMD} tmxcore.log
		[ $? -eq 0 -a ${REMOVE} -gt 0 ] && sudo rm -f tmxcore.log
			
		sudo chown `id -u`:`id -g` "${OUT_FILE}"
	elif [ -d /var/log/upstart ]; then
		cd /var/log/upstart
	
		sudo ${ZIP_CMD} tmxcore.log
		[ $? -eq 0 -a ${REMOVE} -gt 0 ] && sudo rm -f tmxcore.log
		
		sudo chown `id -u`:`id -g` "${OUT_FILE}"
	fi
fi

TODAY_YRFIRST=`date --date="${TODAY}" +"%Y-%m-%d"`
TOMORROW_YRFIRST=`date --date="${TOMORROW}" +"%Y-%m-%d"`
MYSQL_CMD="mysql -uIVP -p$MYSQL_ROOT_PASSWORD -s"

if [ ${EVENT_LOG} -gt 0 ]; then
	cd /tmp
	rm -f messageLog.out
	${MYSQL_CMD} -e "SELECT * FROM messageLog WHERE ENTRYTIME >= '${TODAY_YRFIRST}' AND ENTRYTIME < '${TOMORROW_YRFIRST}'" IVP > messageLog.out

	${ZIP_CMD} messageLog.out
	
	[ $? -eq 0 -a ${REMOVE} -gt 0 ] && \
		${MYSQL_CMD} -e "DELETE FROM messageLog WHERE ENTRYTIME >= '${TODAY_YRFIRST}' AND ENTRYTIME < '${TOMORROW_YRFIRST}'" IVP >/dev/null
	rm -f messageLog.out	
fi

if [ ${MESSAGE_LOG} -gt 0 ]; then
	cd /tmp
	rm -f eventLog.out
	${MYSQL_CMD} -e "SELECT * FROM eventLog WHERE TIMESTAMP >= '${TODAY_YRFIRST}' AND TIMESTAMP < '${TOMORROW_YRFIRST}'" IVP > eventLog.out

	${ZIP_CMD} eventLog.out

	[ $? -eq 0 -a ${REMOVE} -gt 0 ] && \
		${MYSQL_CMD} -e "DELETE FROM eventLog WHERE TIMESTAMP >= '${TODAY_YRFIRST}' AND TIMESTAMP < '${TOMORROW_YRFIRST}'" IVP >/dev/null
	rm -f eventLog.out
fi
