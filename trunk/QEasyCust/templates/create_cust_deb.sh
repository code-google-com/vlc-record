#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 [compiled program file]"
	echo "e.g. $0 vlc-record/release/oem-record"
	exit -1
fi

sudo make PROGSRC="$1" -f install.mak
