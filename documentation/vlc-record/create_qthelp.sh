#!/bin/bash
TMPFILE=/tmp/_0815.tmp
HIDDENFEATURES=0

case $1 in 
   vlc-record)
      PROGRAM="VLC-Record"
      SERVICE="Kartina.TV"
      APISERVER="iptv.kartina.tv"
      HIDDENFEATURES=1
      ;;
   
   kartina_tv)
      PROGRAM="Kartina.TV"
      SERVICE=$PROGRAM
      APISERVER="iptv.kartina.tv"
      ;;
   
   polsky_tv)
      PROGRAM="Polsky.TV"
      SERVIVE=$PROGRAM
      APISERVER="iptv.polsky.tv"
      ;;
   *)
      PROGRAM="VLC-Record"
      SERVICE="Kartina.TV"
      APISERVER="iptv.kartina.tv"
      ;;
   
esac


LANGUAGES="en de ru pl"


for i in $LANGUAGES ; do
   # patch html files ...
   sed -e "s/\[%PROGRAM%\]/$PROGRAM/g" -e "s/\[%SERVICE%\]/$SERVICE/g" -e "s/\[%APISERVER%\]/$APISERVER/g" help_$i.tmpl > help_$i.html
   
   if [ $HIDDENFEATURES -gt 0 ]; then
      sed -e "s/^.*\[%HIDDEN%\].*$//g" help_$i.html > $TMPFILE
      mv $TMPFILE help_$i.html
   fi

	qhelpgenerator help_$i.qhp -o help_$i.qch
	qcollectiongenerator help_$i.qhcp -o help_$i.qhc
done

