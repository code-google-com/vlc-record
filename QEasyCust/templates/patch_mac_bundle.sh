#!/bin/bash
SRC_BUNDLE=$1
TRG_BUNDLE="[%MAC_BUNDLE%]"
DATESTR=`date +%Y%m%d`
TMP_FILE=/tmp/_patch_${DATESTR}

# create destination folder name ...
DSTFOLDER=[%INT_NAME%]-[%VERSION%]-${DATESTR}-mac

# find out which executeable to rename ...
tr -d '\n' < "${SRC_BUNDLE}/Contents/Info.plist" > ${TMP_FILE}
BIN_SRC=`sed -n 's/^.*<key>CFBundleExecutable<\/key>[^<]*<string>\([^<]*\)<\/string>.*$/\1/p' ${TMP_FILE}`
rm ${TMP_FILE}

# copy content ...
cp -R "${SRC_BUNDLE}/Contents/MacOS" "${TRG_BUNDLE}/Contents/"
cp -R "${SRC_BUNDLE}/Contents/Frameworks" "${TRG_BUNDLE}/Contents/"
cp -R "${SRC_BUNDLE}/Contents/PlugIns" "${TRG_BUNDLE}/Contents/"
cp -R "${SRC_BUNDLE}/Contents/translations" "${TRG_BUNDLE}/Contents/"
cp "${SRC_BUNDLE}/Contents/PkgInfo" "${TRG_BUNDLE}/Contents/"
cp "${SRC_BUNDLE}/Contents/Resources/qt.conf" "${SRC_BUNDLE}/Contents/Resources/empty.lproj" "${TRG_BUNDLE}/Contents/Resources/"

#rename executable file ...
mv "${TRG_BUNDLE}/Contents/MacOS/${BIN_SRC}" "${TRG_BUNDLE}/Contents/MacOS/[%INT_NAME%]"

# create dmg folder ...
mkdir -p $DSTFOLDER
mv "${TRG_BUNDLE}" $DSTFOLDER/
ln -s /Applications $DSTFOLDER/

./create_dmg.sh $DSTFOLDER
