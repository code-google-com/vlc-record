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
#include <QTimer>
#include <QVector>

#include <QEvent>
#include <QKeyEvent>

#include <QTime>
#include <QMutex>

#include <vlc/vlc.h>

#include "cvlcrecdb.h"
#include "clogfile.h"
#include "playstates.h"
#include "cshortcutex.h"
#include "defdef.h"
#include "ctimerex.h"
#include "cshowinfo.h"
#include "csettingsdlg.h"
#include "cwaittrigger.h"

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
   void cleanExit ();
   int  initPlayer (QStringList &slArgs);
   int  setMedia (const QString &sMrl);
   bool isPlaying ();
   void setPlugInPath(const QString &sPath);
   void setShortCuts (QVector<CShortcutEx *> *pvSc);
   void startPlayTimer ();
   void pausePlayTimer ();
   void stopPlayTimer ();
   void setSettings (CSettingsDlg *pDlg);
   void setTrigger (CWaitTrigger *pTrig);
   static void eventCallback (const libvlc_event_t *ev, void *player);

protected:
   void changeEvent(QEvent *e);
   int  raise(libvlc_exception_t * ex);
   void releasePlayer ();
   int  createArgs (const QStringList &lArgs, Ui::vlcArgs& args);
   void freeArgs (Ui::vlcArgs& args);
   int  fakeShortCut (const QKeySequence &seq);

   virtual void keyPressEvent (QKeyEvent *pEvent);

private:
   Ui::CPlayer            *ui;
   QTimer                  poller;
   QTimer                  tAspectShot;
   QTimer                  sliderTimer;
   CTimerEx                timer;
   libvlc_exception_t      vlcExcpt;
   libvlc_instance_t      *pVlcInstance;
   libvlc_media_player_t  *pMediaPlayer;
   libvlc_media_t         *pMedia;
   libvlc_event_manager_t *pEMPlay;
   libvlc_event_manager_t *pEMMedia;
   libvlc_log_t           *pLibVlcLog;
   uint                    uiVerboseLevel;
   QString                 sPlugInPath;
   Qt::Key                 kModifier;
   QVector<CShortcutEx *> *pvShortcuts;
   bool                    bCtrlStream;
   CSettingsDlg           *pSettings;
   CWaitTrigger           *pTrigger;
   QMutex                  mtLogMutex;

private slots:
   void on_posSlider_valueChanged(int value);
   void on_posSlider_sliderReleased();
   void on_btnFullScreen_clicked();
   void on_cbxAspect_currentIndexChanged(QString str);
   void on_cbxCrop_currentIndexChanged(QString str);
   void slotChangeVolume(int newVolume);
   void slotLibVLCLog ();
   void slotUpdateSlider ();

public slots:
   int  playMedia (const QString &sCmdLine, bool bAllowCtrl);
   int  play();
   int  stop();
   int  pause();
   int  slotToggleFullScreen ();
   int  slotToggleAspectRatio ();
   int  slotToggleCropGeometry ();
   int  slotTimeJumpRelative (int iSeconds);
   void slotStoredAspectCrop ();
   void slotTriggerAspectChange();

signals:
   void sigPlayState (int ps);
   void sigStartAspectShot();
};

#endif /* __022410__CPLAYER_H */
/************************* History ***************************\
| $Log$
\*************************************************************/