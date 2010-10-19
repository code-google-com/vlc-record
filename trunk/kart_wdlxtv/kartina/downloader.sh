#/bin/sh

# $1: url
# $2: output file
# $3: logfile 

# dump sanity check ...
if [ $# -lt 2 ]; then
  usage
  exit 1
fi

# use params ...
URL=$1
OFILE=$2
LOGFILE=$3

# print options ...
usage() {
  echo "downloader.sh \"url to open\" \"output file\" \"optional log file\"" 
}

# start wget to download stream ...
LOGFILEOPT=
OFILEOPT="-O \"${OFILE}\""

if [ "${LOGFILE}" ]; then
  LOGFILEOPT="-o \"${LOGFILE}\""
fi

eval "wget ${OFILEOPT} ${LOGFILEOPT} \"${URL}\" >/dev/null 2>&1 &"

# get pid of wget ...
WGETPID=$!

echo ${WGETPID} >/var/run/kartrec.pid 