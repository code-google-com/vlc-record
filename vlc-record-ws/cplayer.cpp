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
#include "cplayer.h"
#include "ui_cplayer.h"

// log file functions ...
extern CLogFile VlcLog;

// storage db ...
extern CVlcRecDB *pDb;

// global showinfo class ...
extern CShowInfo showInfo;

/* -----------------------------------------------------------------\
|  Method: CPlayer / constructor
|  Begin: 24.02.2010 / 12:17:51
|  Author: Jo2003
|  Description: init values
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
CPlayer::CPlayer(QWidget *parent) : QWidget(parent), ui(new Ui::CPlayer)
{
   ui->setupUi(this);

   // nothing playing so far ...
   pMedia       = NULL;
   pMediaPlayer = NULL;
   pVlcInstance = NULL;
   pEMMedia     = NULL;
   pEMPlay      = NULL;
   pLibVlcLog   = NULL;
   pvShortcuts  = NULL;
   pSettings    = NULL;
   pTrigger     = NULL;
   bCtrlStream  = false;

   // set aspect shot as single shot timer ...
   tAspectShot.setSingleShot(true);

   // set log poller to single shot ...
   poller.setSingleShot(true);

#ifndef QT_NO_DEBUG
   uiVerboseLevel = 3;
#else
   uiVerboseLevel = 1;
#endif /* QT_NO_DEBUG */

   // init exception structure ...
   libvlc_exception_init(&vlcExcpt);

   // connect volume slider with volume change function ...
   connect(ui->volSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotChangeVolume(int)));

   // connect slider timer with slider position slot ...
   connect(&sliderTimer, SIGNAL(timeout()), this, SLOT(slotUpdateSlider()));

   // do periodical logging ...
   connect(&poller, SIGNAL(timeout()), this, SLOT(slotLibVLCLog()));

   connect(this, SIGNAL(sigStartAspectShot()), this, SLOT(slotTriggerAspectChange()));
   connect(&tAspectShot, SIGNAL(timeout()), this, SLOT(slotStoredAspectCrop()));

   poller.start(1000);
   sliderTimer.start(500);
}

/* -----------------------------------------------------------------\
|  Method: ~CPlayer / destructor
|  Begin: 24.02.2010 / 12:17:51
|  Author: Jo2003
|  Description: clean on destruction
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
CPlayer::~CPlayer()
{
   releasePlayer();
   delete ui;
}

/* -----------------------------------------------------------------\
|  Method: cleanExit
|  Begin: 23.06.2010 / 16:17:51
|  Author: Jo2003
|  Description: pseudo destructor should be called from outside ...
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::cleanExit()
{
   timer.reset();
   poller.stop();
   sliderTimer.stop();
   releasePlayer();
}

/* -----------------------------------------------------------------\
|  Method: setPlugInPath
|  Begin: 02.03.2010 / 14:17:51
|  Author: Jo2003
|  Description: set the plugin path ...
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::setPlugInPath(const QString &sPath)
{
   mInfo(tr("Set PlugIn path to '%1'").arg(sPath));
   sPlugInPath = sPath;
}

/* -----------------------------------------------------------------\
|  Method: setShortCuts
|  Begin: 24.03.2010 / 14:17:51
|  Author: Jo2003
|  Description: store a pointer to shortcuts vector
|
|  Parameters: pointer to shortcuts vector
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::setShortCuts(QVector<CShortcutEx *> *pvSc)
{
   pvShortcuts = pvSc;
}

/* -----------------------------------------------------------------\
|  Method: setSettings
|  Begin: 16.06.2010 / 14:17:51
|  Author: Jo2003
|  Description: store a pointer to settings class
|
|  Parameters: pointer to settings class
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::setSettings(CSettingsDlg *pDlg)
{
   pSettings = pDlg;
}

/* -----------------------------------------------------------------\
|  Method: setTrigger
|  Begin: 16.06.2010 / 16:17:51
|  Author: Jo2003
|  Description: store a pointer to trigger class
|
|  Parameters: pointer to trigger class
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::setTrigger(CWaitTrigger *pTrig)
{
   pTrigger = pTrig;
}

/* -----------------------------------------------------------------\
|  Method: fakeShortCut
|  Begin: 24.03.2010 / 14:30:51
|  Author: Jo2003
|  Description: fake shortcut press if needed
|
|  Parameters: key sequence
|
|  Returns: 0 --> shortcut sent
|          -1 --> not handled
\----------------------------------------------------------------- */
int CPlayer::fakeShortCut (const QKeySequence &seq)
{
   int iRV = -1;
   QVector<CShortcutEx *>::const_iterator cit;

   if (pvShortcuts)
   {
      // test all shortcuts if one matches the now incoming ...
      for (cit = pvShortcuts->constBegin(); (cit != pvShortcuts->constEnd()) && iRV; cit ++)
      {
         // is key sequence equal ... ?
         if ((*cit)->key() == seq)
         {
            // this shortcut matches ...
            mInfo (tr("Activate shortcut: %1").arg(seq.toString()));

            // fake shortcut keypress ...
            (*cit)->activate();

            // only one shortcut should match this sequence ...
            // so we're done!
            iRV = 0;
         }
      }
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: releasePlayer
|  Begin: 24.02.2010 / 12:25:51
|  Author: Jo2003
|  Description: release all the vlc player stuff
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::releasePlayer()
{
   // stop player if needed ...
   stop();

   // release player ...
   if (pMediaPlayer)
   {
      libvlc_media_player_release(pMediaPlayer);
      pMediaPlayer = NULL;
   }

   // release media ...
   if (pMedia)
   {
      libvlc_media_release(pMedia);
      pMedia       = NULL;
   }

   // close log if opened ...
   if (pLibVlcLog)
   {
      // lock logging stuff before closing it ...
      mtLogMutex.lock();

      libvlc_log_close (pLibVlcLog, &vlcExcpt);
      pLibVlcLog = NULL;

      mtLogMutex.unlock();
   }

   // release vlc instance ...
   if (pVlcInstance)
   {
      libvlc_release (pVlcInstance);
      pVlcInstance = NULL;
   }
}

/* -----------------------------------------------------------------\
|  Method: createArgs
|  Begin: 24.02.2010 / 14:25:51
|  Author: Jo2003
|  Description: create argument array, free it with freeArgs
|
|  Parameters: ref. to argument list, ref to vlcArgs struct
|
|  Returns: number of args
|
\----------------------------------------------------------------- */
int CPlayer::createArgs (const QStringList &lArgs, Ui::vlcArgs &args)
{
   int i         = 0;
   args.argArray = new char *[lArgs.count()];

   if (args.argArray)
   {
      args.argc = lArgs.count();

      for (i = 0; i < args.argc; i++)
      {
         // Be sure to use the size of the UTF-8 String !!!
         args.argArray[i] = new char[lArgs[i].toUtf8().size() + 1];

         if (args.argArray[i])
         {
            // copy whole UTF-8 string ...
            strcpy (args.argArray[i], lArgs[i].toUtf8().constData());
         }
      }
   }

   return i;
}

/* -----------------------------------------------------------------\
|  Method: freeArgs
|  Begin: 24.02.2010 / 15:25:51
|  Author: Jo2003
|  Description: free argument array
|
|  Parameters: ref. to vlcArgs struct
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::freeArgs (Ui::vlcArgs &args)
{
   for (int i = 0; i < args.argc; i++)
   {
      delete [] args.argArray[i];
   }

   delete [] args.argArray;
   args.argArray = NULL;
   args.argc     = 0;
}

/* -----------------------------------------------------------------\
|  Method: initPlayer
|  Begin: 24.02.2010 / 14:00:51
|  Author: Jo2003
|  Description: init player with arguments
|
|  Parameters: list of arguments
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::initPlayer(QStringList &slArgs)
{
   int iRV = -1;
   Ui::vlcArgs args;

   releasePlayer();

   // reset crop and aspect cbx ... because it should show the state
   // as used ...
   ui->cbxAspect->setCurrentIndex(0);
   ui->cbxCrop->setCurrentIndex(0);

   if (slArgs.size() == 0)
   {
      // no arguments given --> create standard arguments ...
      slArgs << "-I" << "dummy" << "--ignore-config";
   }

   // is plugin path set ...
   if (sPlugInPath.length() > 0)
   {
      slArgs << QString("--plugin-path=\"%1\"").arg(sPlugInPath);
   }

#ifdef Q_OS_WIN32
   // don't catch key press events ... (should work in patched libVLC)
   slArgs << "--vout-event=3";
#endif

   // fill vlcArgs struct ...
   createArgs(slArgs, args);

   // arguments there ... ?
   if (args.argArray)
   {
      //create a new libvlc instance
      libvlc_exception_clear(&vlcExcpt);
      pVlcInstance = libvlc_new(args.argc, args.argArray, &vlcExcpt);
      iRV = raise (&vlcExcpt);

      if (!iRV)
      {
         // set verbose mode ...
         libvlc_set_log_verbosity (pVlcInstance, 3, &vlcExcpt);
         iRV = raise (&vlcExcpt);
      }

      if (!iRV)
      {
         // open logger ...
         pLibVlcLog = libvlc_log_open(pVlcInstance, &vlcExcpt);
         iRV = raise (&vlcExcpt);
      }

      if (!iRV)
      {
         // Create a media player playing environement
         pMediaPlayer = libvlc_media_player_new (pVlcInstance, &vlcExcpt);
         iRV = raise (&vlcExcpt);
      }

      if (!iRV)
      {
         // add player to window ...
         connect_to_wnd(pMediaPlayer, ui->fVideo->winId(), &vlcExcpt);
         iRV = raise(&vlcExcpt);
      }

      if (!iRV)
      {
         // get volume ...
         ui->volSlider->setSliderPosition(libvlc_audio_get_volume (pVlcInstance, &vlcExcpt));
         iRV = raise(&vlcExcpt);
      }

      if (!iRV)
      {
         // get event manager ...
         pEMPlay = libvlc_media_player_event_manager(pMediaPlayer, &vlcExcpt);
         iRV = raise(&vlcExcpt);
      }

      if (!iRV)
      {
         libvlc_event_attach(pEMPlay, libvlc_MediaPlayerEncounteredError, CPlayer::eventCallback,
                             (void *)this, &vlcExcpt);

         iRV = raise(&vlcExcpt);
      }

      freeArgs(args);
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: setMedia
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: set media
|
|  Parameters: media MRL
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::setMedia(const QString &sMrl)
{
   int iRV = -1;

   // is player playing ... ?
   libvlc_exception_clear(&vlcExcpt);
   libvlc_media_t *curMedia = libvlc_media_player_get_media (pMediaPlayer, &vlcExcpt);

   // if playing, stop and then release media ...
   if (curMedia)
   {
      if (isPlaying())
      {
         libvlc_exception_clear(&vlcExcpt);
         libvlc_media_player_stop(pMediaPlayer, &vlcExcpt);
      }

      // release media ...
      libvlc_media_release (curMedia);
      pMedia = NULL;
   }

   // create new media ...
   libvlc_exception_clear(&vlcExcpt);
   pMedia = libvlc_media_new (pVlcInstance, sMrl.toUtf8().constData(), &vlcExcpt);
   iRV = raise(&vlcExcpt);

   if (!iRV)
   {
      // get event manager ...
      pEMMedia = libvlc_media_event_manager(pMedia, &vlcExcpt);
      iRV = raise(&vlcExcpt);
   }

   if (!iRV)
   {
      // attach media state change event ...
      libvlc_event_attach(pEMMedia, libvlc_MediaStateChanged, CPlayer::eventCallback, (void *)this, &vlcExcpt);
      iRV = raise(&vlcExcpt);
   }

   if (!iRV)
   {
      // add media ...
      libvlc_media_player_set_media (pMediaPlayer, pMedia, &vlcExcpt);
      iRV = raise(&vlcExcpt);
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: slotChangeVolume
|  Begin: 28.02.2010 / 19:00:51
|  Author: Jo2003
|  Description: set volume
|
|  Parameters: new volume
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::slotChangeVolume(int newVolume)
{
   if (!newVolume)
   {
      ui->labSound->setPixmap(QPixmap(":/player/sound_off"));
   }
   else
   {
      ui->labSound->setPixmap(QPixmap(":/player/sound_on"));
   }

   if (pVlcInstance)
   {
      libvlc_exception_clear(&vlcExcpt);
      libvlc_audio_set_volume (pVlcInstance, newVolume, &vlcExcpt);
      raise(&vlcExcpt);
   }
}

/* -----------------------------------------------------------------\
|  Method: play
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: play media
|
|  Parameters: --
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::play()
{
   int iRV = 0;

   if (pMediaPlayer)
   {
      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);
      libvlc_media_player_play (pMediaPlayer, &vlcExcpt);
      iRV = raise(&vlcExcpt);
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: stop
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: stop playing
|
|  Parameters: --
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::stop()
{
   int iRV = 0;

   if (pMediaPlayer && isPlaying())
   {
      libvlc_exception_clear(&vlcExcpt);
      libvlc_media_player_stop (pMediaPlayer, &vlcExcpt);
      iRV = raise(&vlcExcpt);
   }

   stopPlayTimer();

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: pause
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: pause / unpause playing
|
|  Parameters: --
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::pause()
{
   int iRV = 0;

   if (pMediaPlayer)
   {
      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      if (isPlaying() && libvlc_media_player_can_pause(pMediaPlayer, &vlcExcpt) && bCtrlStream)
      {
         iRV = raise(&vlcExcpt);

         if (!iRV)
         {
            libvlc_media_player_pause(pMediaPlayer, &vlcExcpt);
            iRV = raise(&vlcExcpt);
         }
      }
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: playMedia
|  Begin: 03.03.2010 / 09:16:51
|  Author: Jo2003
|  Description: init player, set media, start play
|
|  Parameters: complete command line
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::playMedia(const QString &sCmdLine, bool bAllowCtrl)
{
   int iRV;

   // store control flag ...
   bCtrlStream = bAllowCtrl;

   // reset play timer stuff ...
   timer.reset();

   // enable / disable position slider ...
   ui->posSlider->setValue(0);
   ui->posSlider->setEnabled(bCtrlStream);
   ui->labPos->setEnabled(bCtrlStream);
   ui->labPos->setText("00:00:00");

   // get MRL ...
   QString     sMrl  = sCmdLine.section(";;", 0, 0);
   // QString     sMrl  = "d:/bbb.avi";
   // QString     sMrl  = "/home/joergn/Videos/bbb.avi";
   // QString     sMrl  = "d:/BR-test.ts";

   // get player arguments ...
   QStringList lArgs = sCmdLine.mid(sCmdLine.indexOf(";;", 0))
                          .split(";;", QString::SkipEmptyParts);

   mInfo(tr("starting libVLC play of:\n  --> %2\n  --> with following arguments: %1")
         .arg(lArgs.join(" ")).arg(sMrl));

   iRV = initPlayer(lArgs);

   if (!iRV)
   {
      iRV = setMedia(sMrl);
   }

   if (!iRV)
   {
      iRV = play();
   }

   if (!iRV && bCtrlStream)
   {
      // set slider to position ...
      int iSliderPos;

      // set slider range to seconds ...

      // we add 5 minutes at start and 5 minutes at end because start
      // and end time are not always accurate ...
      ui->posSlider->setRange(0, showInfo.ends() - showInfo.starts());

      iSliderPos = showInfo.lastJump();

      ui->posSlider->setValue(iSliderPos);

      ui->labPos->setText(QTime((int)iSliderPos / 3600,
                                (int)(iSliderPos % 3600) / 60,
                                iSliderPos % 60).toString("hh:mm:ss"));
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: slotUpdateSlider
|  Begin: 22.06.2010 / 17:16:51
|  Author: Jo2003
|  Description: update slider
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::slotUpdateSlider()
{
   if (pMediaPlayer)
   {
      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      if (libvlc_media_player_is_playing(pMediaPlayer, &vlcExcpt) && bCtrlStream)
      {
         int iSliderPos = timer.elapsedEx() / 1000 + showInfo.lastJump();

         if (!ui->posSlider->isSliderDown())
         {
            ui->posSlider->setValue(iSliderPos);
            ui->labPos->setText(QTime((int)iSliderPos / 3600,
                                      (int)(iSliderPos % 3600) / 60,
                                      iSliderPos % 60).toString("hh:mm:ss"));
         }
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: isPlaying
|  Begin: 03.03.2010 / 09:40:51
|  Author: Jo2003
|  Description: is player playing ?
|
|  Parameters: --
|
|  Returns: true --> playing
|          false --> not playing
\----------------------------------------------------------------- */
bool CPlayer::isPlaying()
{
   bool bRV = false;

   if (pMedia)
   {
      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      // get media state ...
      libvlc_state_t mediaState = libvlc_media_get_state (pMedia, &vlcExcpt);

      if (!raise(&vlcExcpt))
      {
         // opening, buffering, playing and paused we
         // count as playing because player is active ...
         switch (mediaState)
         {
         case libvlc_Opening:
         case libvlc_Buffering:
         case libvlc_Playing:
         case libvlc_Paused:
            bRV = true;
            break;
         default:
            break;
         }
      }
   }

   return bRV;
}

/* -----------------------------------------------------------------\
|  Method: changeEvent
|  Begin: 24.02.2010 / 11:46:10
|  Author: Jo2003
|  Description: catch event when language changes
|
|  Parameters: pointer to event
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::changeEvent(QEvent *e)
{
   QWidget::changeEvent(e);
   switch (e->type())
   {
   case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
   default:
      break;
   }
}

/* -----------------------------------------------------------------\
|  Method: keyPressEvent
|  Begin: 23.03.2010 / 22:46:10
|  Author: Jo2003
|  Description: catch keypress events to emulate shortcuts
|
|  Parameters: pointer to event
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::keyPressEvent(QKeyEvent *pEvent)
{
   QString sShortCut;
   int     iRV;

   // can we create a shortcut string for this key ... ?
   iRV = CShortcutEx::createShortcutString(pEvent->modifiers(),
                                           pEvent->text(), sShortCut);

   if (!iRV)
   {
      // check if shortcut string matches one of our shortcuts ...
      iRV = fakeShortCut(QKeySequence (sShortCut));

      if (!iRV)
      {
         pEvent->accept();
      }
   }

   // event not yet handled ... give it to base class ...
   if (iRV == -1)
   {
      QWidget::keyPressEvent(pEvent);
   }
}

/* -----------------------------------------------------------------\
|  Method: raise
|  Begin: 24.02.2010 / 11:46:10
|  Author: Jo2003
|  Description: check if there is any problem, if so display
|               error message
|  Parameters: pointer to exception
|
|  Returns: 0 ==> ok
|        else ==> any error
\----------------------------------------------------------------- */
int CPlayer::raise(libvlc_exception_t * ex)
{
   int iRV = libvlc_exception_raised (ex);
   if (iRV)
   {
      QMessageBox::critical(this, tr("LibVLC Error!"),
                            tr("LibVLC reports following error:\n%1")
                            .arg(QString::fromUtf8(libvlc_exception_get_message(ex))));
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: eventCallback
|  Begin: 01.03.2010 / 11:00:10
|  Author: Jo2003
|  Description: callback for vlc events
|
|  Parameters: pointer to event raised, pointer to player class
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::eventCallback(const libvlc_event_t *ev, void *player)
{
   CPlayer *pPlayer = (CPlayer *)player;

   switch (ev->type)
   {
   // media change events ...
   case libvlc_MediaStateChanged:
      {
         // media state changed ... what is the new state ...
         switch (ev->u.media_state_changed.new_state)
         {
         case libvlc_Opening:
            emit pPlayer->sigPlayState((int)IncPlay::PS_OPEN);
            break;

         case libvlc_Buffering:
            emit pPlayer->sigPlayState((int)IncPlay::PS_BUFFER);
            break;

         case libvlc_Playing:
            emit pPlayer->sigPlayState((int)IncPlay::PS_PLAY);
            pPlayer->startPlayTimer();
            // don't shot but whatch the log ...
            // emit pPlayer->sigStartAspectShot();
            break;

         case libvlc_Paused:
            emit pPlayer->sigPlayState((int)IncPlay::PS_PAUSE);
            pPlayer->pausePlayTimer();
            break;

         case libvlc_Stopped:
            emit pPlayer->sigPlayState((int)IncPlay::PS_STOP);
            pPlayer->stopPlayTimer();
            break;

         case libvlc_Ended:
            emit pPlayer->sigPlayState((int)IncPlay::PS_END);
            pPlayer->stopPlayTimer();
            break;

         case libvlc_Error:
            emit pPlayer->sigPlayState((int)IncPlay::PS_ERROR);
            pPlayer->stopPlayTimer();
            break;

         default:
            break;
         }
      }
      break;

   // player error event ...
   case libvlc_MediaPlayerEncounteredError:
      emit pPlayer->sigPlayState((int)IncPlay::PS_ERROR);
      pPlayer->stopPlayTimer();
      break;

   default:
      break;
   }
}

/* -----------------------------------------------------------------\
|  Method: slotDoLog
|  Begin: 02.03.2010 / 08:30:10
|  Author: Jo2003
|  Description: check libvlc_log for new entries in write into
|               log file
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::slotLibVLCLog()
{
   int iRV = 0;

   // make sure logging stuff isn't destroyed while
   // we process the log entries ...
   mtLogMutex.lock();

   // do we have a logger handle ... ?
   if (pLibVlcLog)
   {
      // how many entries in log ... ?
      uint uiEntryCount = 0;

      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      // are there entries available ... ?
      uiEntryCount = libvlc_log_count (pLibVlcLog, &vlcExcpt);
      iRV          = raise(&vlcExcpt);

      // no error and entries there ...
      if (!iRV && uiEntryCount)
      {
         // log message buffer ...
         libvlc_log_message_t   logMsg;
         libvlc_log_message_t  *pLogMsg;

         // get iterator to go through log entries ...
         libvlc_log_iterator_t *it = libvlc_log_get_iterator(pLibVlcLog, &vlcExcpt);
         iRV                       = raise (&vlcExcpt);

         if (!iRV)
         {
            // while there are entries ...
            while (libvlc_log_iterator_has_next(it, &vlcExcpt))
            {
               // get log message presented by log iterator ...
               pLogMsg = libvlc_log_iterator_next (it, &logMsg, &vlcExcpt);

               if (!raise(&vlcExcpt))
               {
                  // log this ... ?
                  if (pLogMsg->i_severity <= (int)uiVerboseLevel)
                  {
                     // build log message ...
                     mInfo(tr("Name: \"%1\", Type: \"%2\", Severity: %3\n  --> %4")
                           .arg(QString::fromUtf8(pLogMsg->psz_name))
                           .arg(QString::fromUtf8(pLogMsg->psz_type))
                           .arg(pLogMsg->i_severity)
                           .arg(QString::fromUtf8(pLogMsg->psz_message)));
                  }

                  // buffer end ... ?
                  /////////////////////////////////////////////////////////////////
                  {
                     /*
                      * Please note: This is a hack!!!
                      * The old libVLC 1.05 doesn't give the info when
                      * there is video displayed. This makes trouble cause we
                      * have to set aspect and crop ratio only, when there is video output!
                      * So we watch the log for string "Received first picture" to decide
                      * if there is video output... hopefully it works!
                      */

                     // tells log about first picture ... ?
                     if ((pLogMsg->i_severity == 3)
                        && (QString::fromUtf8(pLogMsg->psz_message) == MAGIC_LOG_VOUT))
                     {
                        mInfo(tr("libVLC / decoder reports: \"%1\"...").arg(MAGIC_LOG_VOUT));
                        slotTriggerAspectChange();
                     }
                  }
                  /////////////////////////////////////////////////////////////////
               }
               else
               {
                  // leave while loop ...
                  break;
               }
            } // --> while ...

            // free log iterator ...
            libvlc_log_iterator_free (it, &vlcExcpt);

            // delete all log entries ...
            libvlc_log_clear(pLibVlcLog, &vlcExcpt);
         }
      }
   }

   // grant access to logging stuff ...
   mtLogMutex.unlock();

   // check log again in a second ...
   poller.start(1000);
}

/* -----------------------------------------------------------------\
|  Method: on_cbxAspect_currentIndexChanged
|  Begin: 08.03.2010 / 09:55:10
|  Author: Jo2003
|  Description: set new aspect ration ...
|
|  Parameters: new aspect ratio as string ...
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::on_cbxAspect_currentIndexChanged(QString str)
{
   if (pMediaPlayer)
   {
      QString sAspect, sCrop;

      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      // set new aspect ratio ...
      libvlc_video_set_aspect_ratio(pMediaPlayer, str.toAscii().data(), &vlcExcpt);

      raise(&vlcExcpt);

      // save aspect if changed ...
      pDb->aspect(showInfo.channelId(), sAspect, sCrop);

      if (sAspect != str)
      {
         // save to database ...
         pDb->addAspect(showInfo.channelId(), str, ui->cbxCrop->currentText());
      }

      mInfo(tr("Aspect ratio: %1")
            .arg(libvlc_video_get_aspect_ratio(pMediaPlayer, &vlcExcpt)));
   }
}

/* -----------------------------------------------------------------\
|  Method: on_cbxCrop_currentIndexChanged
|  Begin: 23.03.2010 / 09:55:10
|  Author: Jo2003
|  Description: set new crop geometry ...
|
|  Parameters: new crop geometry as string ...
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::on_cbxCrop_currentIndexChanged(QString str)
{
   if (pMediaPlayer)
   {
      QString sAspect, sCrop;

      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      // set new aspect ratio ...
      libvlc_video_set_crop_geometry(pMediaPlayer, str.toAscii().data(), &vlcExcpt);

      raise(&vlcExcpt);

      // save crop if changed ...
      pDb->aspect(showInfo.channelId(), sAspect, sCrop);

      if (sCrop != str)
      {
         // save to database ...
         pDb->addAspect(showInfo.channelId(), ui->cbxAspect->currentText(), str);
      }

      mInfo(tr("Crop ratio: %1")
            .arg(libvlc_video_get_crop_geometry(pMediaPlayer, &vlcExcpt)));
   }
}

/* -----------------------------------------------------------------\
|  Method: slotToggleFullScreen
|  Begin: 08.03.2010 / 09:55:10
|  Author: Jo2003
|  Description: toggle fullscreen mode ...
|
|  Parameters: --
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::slotToggleFullScreen()
{
   int iRV = -1;

   if (pMediaPlayer)
   {
      // reset exception stuff ...
      libvlc_exception_clear(&vlcExcpt);

      // set new aspect ratio ...
      libvlc_toggle_fullscreen (pMediaPlayer, &vlcExcpt);

      iRV = raise(&vlcExcpt);
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: slotToggleAspectRatio
|  Begin: 08.03.2010 / 15:10:10
|  Author: Jo2003
|  Description: switch aspect ratio to next one ...
|
|  Parameters: --
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::slotToggleAspectRatio()
{
   int iRV = -1;
   if (pMediaPlayer)
   {
      int idx = ui->cbxAspect->currentIndex();
      idx ++;

      // if end reached, start with index 0 ...
      if (idx >= ui->cbxAspect->count())
      {
         idx = 0;
      }

      // set new aspect ratio ...
      ui->cbxAspect->setCurrentIndex(idx);

      iRV = 0;
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: slotToggleCropGeometry
|  Begin: 08.03.2010 / 15:10:10
|  Author: Jo2003
|  Description: switch aspect ratio to next one ...
|
|  Parameters: --
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::slotToggleCropGeometry()
{
   int iRV = -1;
   if (pMediaPlayer)
   {
      int idx = ui->cbxCrop->currentIndex();
      idx ++;

      // if end reached, start with index 0 ...
      if (idx >= ui->cbxCrop->count())
      {
         idx = 0;
      }

      // set new aspect ratio ...
      ui->cbxCrop->setCurrentIndex(idx);

      iRV = 0;
   }

   return iRV;
}

/* -----------------------------------------------------------------\
|  Method: slotTimeJumpRelative
|  Begin: 18.03.2010 / 15:10:10
|  Author: Jo2003
|  Description: time jump (back to the future ;-) )
|
|  Parameters: position value (jump + / - seconds)
|
|  Returns: 0 --> ok
|          -1 --> any error
\----------------------------------------------------------------- */
int CPlayer::slotTimeJumpRelative (int iSeconds)
{
   if (isPlaying() && bCtrlStream)
   {
      int  iNewPos = ui->posSlider->value() + iSeconds;
      uint uiGmt   = 0;

      // check that we don't reach another show ...
      if (iNewPos < 0)
      {
         iNewPos = 0;
      }
      else if (iNewPos > (int)(showInfo.ends() - showInfo.starts()))
      {
         iNewPos = showInfo.ends() - showInfo.starts();
      }

      // save new position ...
      showInfo.setLastJumpTime(iNewPos);

      // add the jump value ...
      uiGmt        = (uint)(iNewPos + showInfo.starts());

      // we now have the new GMT value for the archive stream ...

      // trigger request for the new stream position ...
      QString req = QString("m=channels&act=get_stream_url&cid=%1&gmt=%2")
                       .arg(showInfo.channelId()).arg(uiGmt);

      pTrigger->TriggerRequest(Kartina::REQ_ARCHIV, req);
   }

   return 0;
}

/* -----------------------------------------------------------------\
|  Method: startPlayTimer
|  Begin: 25.03.2010 / 11:10:10
|  Author: Jo2003
|  Description: set player startup time
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::startPlayTimer()
{
   timer.start();
}

/* -----------------------------------------------------------------\
|  Method: pausePlayTimer
|  Begin: 09.04.2010 / 11:10:10
|  Author: Jo2003
|  Description: pause play timer
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::pausePlayTimer()
{
   timer.pause();
}

/* -----------------------------------------------------------------\
|  Method: stopPlayTimer
|  Begin: 22.06.2010 / 11:10:10
|  Author: Jo2003
|  Description: stop play timer
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::stopPlayTimer()
{
   timer.reset();
}

/* -----------------------------------------------------------------\
|  Method: on_btnFullScreen_clicked
|  Begin: 27.05.2010 / 11:10:10
|  Author: Jo2003
|  Description: change to full screen
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::on_btnFullScreen_clicked()
{
   slotToggleFullScreen();
}

/* -----------------------------------------------------------------\
|  Method: slotStoredAspectCrop [slot]
|  Begin: 15.06.2010 / 16:10:10
|  Author: Jo2003
|  Description: use stored aspect / crop for channel
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::slotStoredAspectCrop ()
{
   QString sAspect, sCrop;

   if(!pDb->aspect(showInfo.channelId(), sAspect, sCrop))
   {
      int iIdxOld, iIdxNew;

      // change combo box value for aspect ratio ...
      iIdxOld = ui->cbxAspect->currentIndex();
      iIdxNew = ui->cbxAspect->findText(sAspect);

      if (iIdxOld != iIdxNew)
      {
         // updating combobox' actual value will also
         // trigger the libVLC call ...
         ui->cbxAspect->setCurrentIndex (iIdxNew);
      }
      else
      {
         // reset exception stuff ...
         libvlc_exception_clear(&vlcExcpt);

         // since values don't differ, updating combobox will not
         // trigger format change. So set it directly to libVLC ...
         libvlc_video_set_aspect_ratio(pMediaPlayer, sAspect.toAscii().data(), &vlcExcpt);

         raise(&vlcExcpt);
      }

      // change combo box value for crop ratio ...
      iIdxOld = ui->cbxCrop->currentIndex();
      iIdxNew = ui->cbxCrop->findText(sCrop);

      if (iIdxOld != iIdxNew)
      {
         // updating combobox' actual value will also
         // trigger the libVLC call ...
         ui->cbxCrop->setCurrentIndex (iIdxNew);
      }
      else
      {
         // reset exception stuff ...
         libvlc_exception_clear(&vlcExcpt);

         // since values don't differ, updating combobox will not
         // trigger format change. So set it directly to libVLC ...
         libvlc_video_set_crop_geometry(pMediaPlayer, sCrop.toAscii().data(), &vlcExcpt);

         raise(&vlcExcpt);
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: on_posSlider_sliderReleased
|  Begin: 23.06.2010 / 09:10:10
|  Author: Jo2003
|  Description: update position label to relect
|               slider position change
|  Parameters: actual slider position
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::on_posSlider_sliderReleased()
{
   if (isPlaying() && bCtrlStream)
   {
      uint position = (uint)ui->posSlider->value();

      // ignore slider position change of +/- 10 seconds ...
      uint uiPlayTime = showInfo.lastJump();

      // play position ...
      uiPlayTime += (uint)(timer.elapsedEx() / 1000);

      // check if slider position is in 10 sec. limit ...
      if ((position >= (uiPlayTime - 10)) && (position <= (uiPlayTime + 10)))
      {
         mInfo(tr("Ignore slightly slider position change..."));
      }
      else
      {
         // request new stream ...
         QString req = QString("m=channels&act=get_stream_url&cid=%1&gmt=%2")
                      .arg(showInfo.channelId())
                      .arg(showInfo.starts() + position);

         // update last jump time ...
         showInfo.setLastJumpTime(position);

         // trigger stream request ...
         pTrigger->TriggerRequest(Kartina::REQ_ARCHIV, req);
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: on_posSlider_valueChanged
|  Begin: 23.06.2010 / 09:10:10
|  Author: Jo2003
|  Description: update position label to relect
|               slider position change
|  Parameters: actual slider position
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::on_posSlider_valueChanged(int value)
{
   if (isPlaying() && bCtrlStream)
   {
      if (ui->posSlider->isSliderDown())
      {
         ui->labPos->setText(QTime((int)value / 3600,
                                   (int)(value % 3600) / 60,
                                   value % 60).toString("hh:mm:ss"));
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: slotTriggerAspectChange [slot]
|  Begin: 16.06.2010 / 16:10:10
|  Author: Jo2003
|  Description: start timer to change aspect ratio ...
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::slotTriggerAspectChange()
{
   // decoder has reported that there is the first picture ...
   // so wait a little and set stored aspect and crop values ...
   tAspectShot.start(100);
}

/************************* History ***************************\
| $Log$
\*************************************************************/

