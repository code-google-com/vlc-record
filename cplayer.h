/*********************** Information *************************\
| $HeadURL: https://vlc-record.googlecode.com/svn/trunk/vlc-record/cplayer.h $
|
| Author: Jo2003
|
| Begin: 24.02.2010 / 10:41:34
|
| Last edited by: $Author: Olenka.Joerg $
|
| $Id: cplayer.h 110 2010-03-04 15:46:01Z Olenka.Joerg $
\*************************************************************/
#ifndef __022410__CPLAYER_H
   #define __022410__CPLAYER_H

#include <QtGui/QApplication>
#include <QMessageBox>
#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <vlc/vlc.h>

#include "clogfile.h"
#include "playstates.h"

//===================================================================
// namespace
//===================================================================
namespace Ui
{
   class CPlayer;
   typedef struct _vlcArgs
   {
      char** argArray;
      int    argc;
   } vlcArgs;
}

//===================================================================
// macro to connect player to hardware ...
//===================================================================
#ifdef Q_OS_WIN        // on windows ...
   #define connect_to_wnd(a, b) libvlc_media_player_set_hwnd (a, b)
#elif defined Q_OS_MAC // on MAC OS
   #define connect_to_wnd(a, b) libvlc_media_player_set_agl (a, b)
#else                  // on Linux
   #define connect_to_wnd(a, b) libvlc_media_player_set_xwindow (a, b)
#endif

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
   int  initPlayer (QStringList &slArgs);
   int  setMedia (const QString &sMrl);
   bool isPlaying ();
   void setPlugInPath(const QString &sPath);
   static void eventCallback (const libvlc_event_t *ev, void *player);

protected:
   void changeEvent(QEvent *e);
   int  raise();
   void releasePlayer ();
   int  createArgs (const QStringList &lArgs, Ui::vlcArgs& args);
   void freeArgs (Ui::vlcArgs& args);

private:
   Ui::CPlayer            *ui;
   QTimer                  poller;
   libvlc_instance_t      *pVlcInstance;
   libvlc_media_player_t  *pMediaPlayer;
   libvlc_media_t         *pMedia;
   libvlc_event_manager_t *pEMPlay;
   libvlc_event_manager_t *pEMMedia;
   libvlc_log_t           *pLibVlcLog;
   uint                    uiVerboseLevel;
   QString                 sPlugInPath;

private slots:
   void slotChangeVolume(int newVolume);
   void slotLibVLCLog ();

public slots:
   int  playMedia (const QString &sCmdLine);
   int  slotToggleFullScreen ();
   int  play();
   int  stop();
   int  pause();

signals:
   void sigStateChg(const QString &str);
   void sigPlayState (int ps);
};

#endif /* __022410__CPLAYER_H */
/************************* History ***************************\
| $Log$
\*************************************************************/
