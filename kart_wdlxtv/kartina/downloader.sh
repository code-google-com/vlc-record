#/bin/sh

# $1: url
# $2: output file
# $3: pid of php thread ...

# dump sanity check ...
if [ $# -ne 3 ]; then
  usage
  exit 1
fi

# use params ...
URL=$1
OFILE=$2
PHPPID=$3

# print options ...
usage() {
  echo "downloader.sh \"url to open\" \"output file\" \"pid for watch\"" 
}

# start wget to download stream ...
wget -O "$OFILE" "$URL" >/dev/null 2>&1 &

# get pid of wget ...
WGETPID=$!

# while go ...
GO=1
RV=0

# check if php thread still runs ... 
while [ $GO -eq 1 ]; do
  
  # try kill ...
  kill -0 ${PHPPID} >/dev/null 2>&1
  RV=$?
  
  # php script is still running ... ? 
  if [ $RV -eq 0 ]; then
    sleep 2
  else
    # stop wget download ...
    kill ${WGETPID}
    GO=0
  fi
done
 