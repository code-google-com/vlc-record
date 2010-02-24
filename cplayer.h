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
   #define connect_to_wnd(a, b, c) libvlc_media_player_set_hwnd (a, b, c)
#elif defined Q_OS_MAC // on MAC OS
   #define connect_to_wnd(a, b, c) libvlc_media_player_set_agl (a, b, c)
#else                  // on Linux
   #define connect_to_wnd(a, b, c) libvlc_media_player_set_xwindow (a, b, c)
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
   void initPlayer (QStringList &slArgs);
   void setMedia (const QString &sMrl);
   void play();
   void stop();
   void pause();

protected:
   void changeEvent(QEvent *e);
   int  raise(libvlc_exception_t * ex);
   void releasePlayer ();
   int  createArgs (const QStringList &lArgs, Ui::vlcArgs& args);
   void freeArgs (Ui::vlcArgs& args);

private:
   Ui::CPlayer           *ui;
   bool                   bIsPlaying;
   libvlc_exception_t     vlcExcpt;
   libvlc_instance_t     *pVlcInstance;
   libvlc_media_player_t *pMediaPlayer;
   libvlc_media_t        *pMedia;
};

#endif /* __022410__CPLAYER_H */
/************************* History ***************************\
| $Log$
\*************************************************************/
