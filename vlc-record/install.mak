INSTALL=install
TARGETNW=/usr/local
TARGETCL=/usr/local
# SUDO=sudo
CHKINST=checkinstall

include version.mak
include checkinstall.mak

all: debnew

instclassic: install_base

instnew: install_libvlc

install_base:
	${INSTALL} -d ${TARGET}
	${INSTALL} -d ${TARGET}/bin
	${INSTALL} -d ${TARGET}/share
	${INSTALL} -d ${TARGET}/share/vlc-record
	${INSTALL} -d ${TARGET}/share/vlc-record/language
	${INSTALL} -d ${TARGET}/share/vlc-record/modules
	${INSTALL} -s -t ${TARGET}/bin release/vlc-record
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/language lang_*.qm
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/1_vlc-player.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/2_MPlayer.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/3_vlc-mp4.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/4_vlc-player-avi.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/7_vlc-mpeg2.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record resources/television.png

install_libvlc: install_base
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/5_libvlc.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/6_libvlc-mpeg2.mod
	${INSTALL} -m 644 -t ${TARGET}/share/vlc-record/modules modules/8_libvlc_xvid_avi.mod

debnew:
	${CHKINST} ${PKGNAMENW} ${PKGVERNW} ${MAINT} ${REQUIRES} ${PKGSRC} ${MAKE} TARGET=${TARGETNW} -f install.mak instnew

debclc:
	${CHKINST} ${PKGNAMECL} ${PKGVERCL} ${MAINT} ${REQUIRES} ${PKGSRC} ${MAKE} TARGET=${TARGETCL} -f install.mak instclassic