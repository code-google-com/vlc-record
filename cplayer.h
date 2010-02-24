/*********************** Information *************************\
| $HeadURL$
|
| Author: Jo2003
|
| Begin: 24.02.2010 / 10:41:34
|
| Last edited by: $Author$
|
| $Id$
\*************************************************************/
#ifndef __022410__CPLAYER_H
   #define __022410__CPLAYER_H

#include <QtGui/QApplication>
#include <QMessageBox>
#include <QWidget>
#include <QFrame>
#include <vlc/vlc.h>

//===================================================================
// namespace
//===================================================================
namespace Ui {
    class CPlayer;
}

/********************************************************************\
|  Class: CPlayer
|  Date:  14.02.2010 / 11:42:24
|  Author: Jo2003
|  Description: widget with vlc player (using libvlc)
|
\********************************************************************/
class CPlayer : public QWidget
{
   Q_OBJECT

public:
   CPlayer(QWidget *parent = 0);
   ~CPlayer();
   void playFile(QString file);

protected:
   void changeEvent(QEvent *e);
   void raise(libvlc_exception_t * ex);

private:
   Ui::CPlayer *ui;
   bool   bIsPlaying;
   libvlc_exception_t _vlcexcep;
   libvlc_instance_t *_vlcinstance;
   libvlc_media_player_t *_mp;
   libvlc_media_t *_m;
};

#endif /* __022410__CPLAYER_H */
/************************* History ***************************\
| $Log$
\*************************************************************/
