/*********************** Information *************************\
| $HeadURL: https://vlc-record.googlecode.com/svn/trunk/vlc-record/cplayer.h $
|
| Author: Jo2003
|
| Begin: 24.02.2010 / 10:41:34
|
| Last edited by: $Author: Olenka.Joerg $
|
| $Id: cplayer.h 893 2012-09-03 10:37:14Z Olenka.Joerg $
\*************************************************************/
#ifndef __022410__CPLAYER_H
   #define __022410__CPLAYER_H

#include <QtGui/QApplication>
#include <QMessageBox>
#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <QEvent>
#include <QTime>
#include <QDesktopWidget>
#include <QComboBox>

#include <vlc/vlc.h>

#include "cvlcrecdb.h"
#include "clogfile.h"
#include "playstates.h"
#include "defdef.h"
#include "ctimerex.h"
#include "cshowinfo.h"
#include "csettingsdlg.h"
#include "cwaittrigger.h"
#include "qvlcvideowidget.h"

//===================================================================
// namespace
//===================================================================
namespace Ui
{
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
   int  initPlayer ();
   bool isPlaying ();
   void setShortCuts (QVector<CShortcutEx *> *pvSc);
   void startPlayTimer ();
   void pausePlayTimer ();
   void stopPlayTimer ();
   void setSettings (CSettingsDlg *pDlg);
   void setTrigger (CWaitTrigger *pTrig);
   static void eventCallback (const libvlc_event_t *ev, void *userdata);
   bool isPositionable();
   void initSlider ();
   uint getSilderPos();
   QVlcVideoWidget* getAndRemoveVideoWidget();
   void  addAndEmbedVideoWidget();
   ulong libvlcVersion();

   static libvlc_event_type_t _actEvent;
   static const char*         _pAspect[];
   static const char*         _pCrop[];

   QFrame* getFrameTimerInfo();
   QComboBox* getCbxAspect();
   QComboBox* getCbxCrop();

protected:
   virtual void changeEvent(QEvent *e);
   void enableDisablePlayControl (bool bEnable);
   void connectToVideoWidget ();
   int  addAd ();
   int  clearMediaList();
   QString aspectCropToString (const char *pFormat);

private:
   Ui::CPlayer                 *ui;
   QTimer                       sliderTimer;
   QTimer                       tAspectShot;
   QTimer                       tEventPoll;
   CTimerEx                     timer;
   libvlc_instance_t           *pVlcInstance;
   libvlc_media_player_t       *pMediaPlayer;
   libvlc_event_manager_t      *pEMPlay;
   libvlc_media_list_player_t  *pMedialistPlayer;
   libvlc_media_list_t         *pMediaList;
   libvlc_event_type_t          lastEvent;
   bool                         bCtrlStream;
   CSettingsDlg                *pSettings;
   CWaitTrigger                *pTrigger;
   bool                         bSpoolPending;
   uint                         uiDuration;
   ulong                        ulLibvlcVersion;
   bool                         bOmitNextEvent;

private slots:
   void on_posSlider_valueChanged(int value);
   void on_btnFullScreen_clicked();
   void on_cbxAspect_currentIndexChanged(int idx);
   void on_cbxCrop_currentIndexChanged(int idx);
   void slotChangeVolume(int newVolume);
   void slotUpdateSlider ();
   void slotChangeVolumeDelta (const bool up);
   void slotSliderPosChanged();
   void slotToggleFullscreen();
   void slotEventPoll();

   void on_btnSaveAspectCrop_clicked();

public slots:
   int  playMedia (const QString &sCmdLine);
   int  play();
   int  stop();
   int  silentStop();
   int  pause();
   int  slotToggleAspectRatio ();
   int  slotToggleCropGeometry ();
   int  slotTimeJumpRelative (int iSeconds);
   void slotStoredAspectCrop ();
   void slotMoreLoudly();
   void slotMoreQuietly();
   void slotMute();
   void slotShowInfoUpdated();
   void slotFsToggled (int on);
   void slotResetVideoFormat();

signals:
   void sigPlayState (int ps);
   void sigTriggerAspectChg ();
   void sigCheckArchProg(ulong ulArchGmt);
   void sigToggleFullscreen();

   void sigAspectToggle(int idx);
   void sigCropToggle(int idx);
};

#endif /* __022410__CPLAYER_H */
/************************* History ***************************\
| $Log$
\*************************************************************/
