#!/bin/bash
TMPFILE=/tmp/_0815.tmp
HIDDENFEATURES=0
QHCFOLDER="../qhc"

if [ ${#} -gt 0 ]; then
   CUSTS=${1}
else
   CUSTS="moidom_tv polsky_tv kartina_tv vlc-record"
fi

for CUST in $CUSTS ; do

    svn-clean >/dev/null 2>&1

    case $CUST in
       vlc-record)
          PROGRAM="VLC-Record"
          SERVICE="Kartina.TV"
          APISERVER="iptv.kartina.tv"
          LANGUAGES="en de ru pl"
          HIDDENFEATURES=1
          ;;

       kartina_tv)
          PROGRAM="Kartina.TV"
          SERVICE=$PROGRAM
          APISERVER="iptv.kartina.tv"
          LANGUAGES="en de ru"
          ;;

       polsky_tv)
          PROGRAM="Polsky.TV"
          SERVICE=$PROGRAM
          APISERVER="iptv.polsky.tv"
          LANGUAGES="en de pl"
          ;;
       moidom_tv)
          PROGRAM="Moi-Dom.TV"
          SERVICE=$PROGRAM
          APISERVER="iptv.moi-dom.tv"
          LANGUAGES="en ru"
          ;;
       *)
          PROGRAM="VLC-Record"
          SERVICE="Kartina.TV"
          APISERVER="iptv.kartina.tv"
          LANGUAGES="en de ru"
          ;;

    esac

    echo -e "======================================\nBuilding help for \"$PROGRAM\" ...\n======================================"

    # delete qch and qhc files ...
    rm -f *.qch *.qhc

    for i in $LANGUAGES ; do
       # patch files ...
       sed -e "s/\[%PROGRAM%\]/$PROGRAM/g" -e "s/\[%SERVICE%\]/$SERVICE/g" -e "s/\[%APISERVER%\]/$APISERVER/g" help_$i.tmpl > help_$i.html
       sed -e "s/\[%PROGRAM%\]/$PROGRAM/g" help_$i.qhp_tmpl > help_$i.qhp

       if [ $HIDDENFEATURES -gt 0 ]; then
          sed -e "s/^.*\[%HIDDEN%\].*$//g" help_$i.html > $TMPFILE
          mv $TMPFILE help_$i.html
          sed -e "s/^.*\[%HIDDEN%\].*$//g" help_$i.qhp > $TMPFILE
          mv $TMPFILE help_$i.qhp
       fi

        qhelpgenerator help_$i.qhp -o help_$i.qch
        qcollectiongenerator help_$i.qhcp -o help_$i.qhc
    done

    echo -n "Copy help files to \"$QHCFOLDER/$PROGRAM/\" ..."
    cp *.qhc *.qch $QHCFOLDER/$PROGRAM/

    echo " Done!"

done

