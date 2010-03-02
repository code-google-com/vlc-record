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

// -----------------------------------------------------------------------------
// macro definition for state changes ...
// -----------------------------------------------------------------------------
#define mCaseStateChg(__state__) case __state__: emit pPlayer->sendStateMsg(#__state__); break

// log file functions ...
extern CLogFile VlcLog;

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
   bIsPlaying   = false;
   pMedia       = NULL;
   pMediaPlayer = NULL;
   pVlcInstance = NULL;
   pEMMedia     = NULL;
   pEMPlay      = NULL;
   pLibVlcLog   = NULL;

#ifndef QT_NO_DEBUG
   uiVerboseLevel = 3;
#else
   uiVerboseLevel = 1;
#endif /* QT_NO_DEBUG */

   // init exception structure ...
   libvlc_exception_init(&vlcExcpt);

   // connect volume slider with volume change function ...
   connect(ui->volSlider, SIGNAL(sliderMoved(int)), this, SLOT(slotChangeVolume(int)));

   // connect state change signal with state label in dialog ...
   connect(this, SIGNAL(sigStateChg(QString)), ui->labState, SLOT(setText(QString)));

   // do periodical logging ...
   connect(&poller, SIGNAL(timeout()), this, SLOT(slotLibVLCLog()));

   poller.start(500);
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
   sPlugInPath = sPath;
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
   mutex.lock();

   // release player ...
   if (pMediaPlayer)
   {
      libvlc_media_player_stop (pMediaPlayer, &vlcExcpt);
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
      libvlc_log_close (pLibVlcLog, &vlcExcpt);
      pLibVlcLog = NULL;
   }

   // release vlc instance ...
   if (pVlcInstance)
   {
      libvlc_release (pVlcInstance);
      pVlcInstance = NULL;
   }

   bIsPlaying = false;

   mutex.unlock();
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
         args.argArray[i] = new char[lArgs[i].size() + 1];

         if (args.argArray[i])
         {
            strcpy (args.argArray[i], lArgs[i].toAscii().constData());
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
         libvlc_set_log_verbosity (pVlcInstance, uiVerboseLevel, &vlcExcpt);
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
         /*
         libvlc_event_attach(pEMPlay, libvlc_MediaPlayerOpening, CPlayer::eventCallback,
                             (void *)this, &vlcExcpt);
         iRV = raise(&vlcExcpt);
         */
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
      if (bIsPlaying)
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
   pMedia = libvlc_media_new (pVlcInstance, sMrl.toAscii().constData(), &vlcExcpt);
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

      if (!iRV)
      {
         bIsPlaying = true;
      }
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

   if (pMediaPlayer && bIsPlaying)
   {
      libvlc_exception_clear(&vlcExcpt);
      libvlc_media_player_stop (pMediaPlayer, &vlcExcpt);
      bIsPlaying = false;
      iRV = raise(&vlcExcpt);
   }

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

      if (bIsPlaying && libvlc_media_player_can_pause(pMediaPlayer, &vlcExcpt))
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
   switch (e->type()) {
   case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
   default:
      break;
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
                            .arg(libvlc_exception_get_message(ex)));
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
   case libvlc_MediaStateChanged:
      {
         // media state changed ... what is the new state ...
         switch (ev->u.media_state_changed.new_state)
         {
         mCaseStateChg(libvlc_NothingSpecial);
         mCaseStateChg(libvlc_Opening);
         mCaseStateChg(libvlc_Buffering);
         mCaseStateChg(libvlc_Playing);
         mCaseStateChg(libvlc_Paused);
         mCaseStateChg(libvlc_Stopped);
         mCaseStateChg(libvlc_Ended);
         mCaseStateChg(libvlc_Error);
         default:
            break;
         }
      }
      break;
   default:
      break;
   }
}

/* -----------------------------------------------------------------\
|  Method: sendStateMsg
|  Begin: 01.03.2010 / 14:00:10
|  Author: Jo2003
|  Description: short state string and emit signal ...
|
|  Parameters: ref. to state message ...
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::sendStateMsg (const QString &msg)
{
   // remove "libvlc_" from string ...
   QString sMsg = msg;
   sMsg.remove("libvlc_", Qt::CaseInsensitive);
   sMsg = sMsg.toUpper();
   emit sigStateChg(sMsg);
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
   int     iRV;

   // make sure log handle will not be deleted while
   // we collect the log entries ...
   mutex.lock();

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

         // while there are entries ...
         while (!iRV)
         {
            // get log message presented by log iterator ...
            pLogMsg = libvlc_log_iterator_next (it, &logMsg, &vlcExcpt);
            iRV     = raise(&vlcExcpt);

            if (!iRV)
            {
               // build log message ...
               mInfo(tr("Name: \"%1\", Type: \"%2\", Severity: %3\n --> %4")
                      .arg(pLogMsg->psz_name).arg(pLogMsg->psz_type)
                      .arg(pLogMsg->i_severity).arg(pLogMsg->psz_message));

               // is there a next entry ... ?
               if (!libvlc_log_iterator_has_next(it, &vlcExcpt))
               {
                  // no --> break while ...
                  iRV = 1;
               }
            }
         }

         // delete all log entries ...
         libvlc_log_clear(pLibVlcLog, &vlcExcpt);

         // free log iterator ...
         libvlc_log_iterator_free (it, &vlcExcpt);
      }
   }

   mutex.unlock();
}

/************************* History ***************************\
| $Log$
\*************************************************************/
