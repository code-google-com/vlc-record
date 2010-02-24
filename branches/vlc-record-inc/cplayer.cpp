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

   // init exception structure ...
   libvlc_exception_init(&vlcExcpt);
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

   // release vlc instance ...
   if (pVlcInstance)
   {
      libvlc_release (pVlcInstance);
      pVlcInstance = NULL;
   }

   bIsPlaying = false;
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
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::initPlayer(QStringList &slArgs)
{
   Ui::vlcArgs args;

   releasePlayer();

   // reset exception stuff ...
   libvlc_exception_init(&vlcExcpt);

   if (slArgs.size() == 0)
   {
      // no arguments given ... create standard arguments ...
      slArgs << "-I";
      slArgs << "dummy";
      slArgs << "--ignore-config";
#ifndef QT_NO_DEBUG
      slArgs << "--extraintf=logger";
      slArgs << "--verbose=2";
#endif
   }

   // add plugin path ...
   slArgs << QString("--plugin-path=\"%1/plugins\"")
              .arg(QApplication::applicationDirPath());

   // fill vlcArgs struct ...
   createArgs(slArgs, args);

   // arguments there ... ?
   if (args.argArray)
   {
      //create a new libvlc instance
      pVlcInstance = libvlc_new(args.argc, args.argArray, &vlcExcpt);
      raise (&vlcExcpt);

      // Create a media player playing environement
      pMediaPlayer = libvlc_media_player_new (pVlcInstance, &vlcExcpt);
      raise (&vlcExcpt);

      // add player to window ...
      connect_to_wnd(pMediaPlayer, ui->fVideo->winId(), &vlcExcpt);

      raise(&vlcExcpt);

      freeArgs(args);
   }
}

/* -----------------------------------------------------------------\
|  Method: setMedia
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: set media
|
|  Parameters: media MRL
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::setMedia(const QString &sMrl)
{
   // is player playing ... ?
   libvlc_media_t *curMedia = libvlc_media_player_get_media (pMediaPlayer, &vlcExcpt);

   // if playing, stop and then release media ...
   if (curMedia)
   {
      if (bIsPlaying)
      {
         libvlc_media_player_stop(pMediaPlayer, &vlcExcpt);
      }

      // release media ...
      libvlc_media_release (curMedia);
      pMedia = NULL;
   }

   // reset exception stuff ...
   libvlc_exception_init(&vlcExcpt);

   // create new media ...
   pMedia = libvlc_media_new (pVlcInstance, sMrl.toAscii().constData(), &vlcExcpt);
   raise(&vlcExcpt);

   // add media ...
   libvlc_media_player_set_media (pMediaPlayer, pMedia, &vlcExcpt);
   raise(&vlcExcpt);
}

/* -----------------------------------------------------------------\
|  Method: play
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: play media
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::play()
{
   // reset exception stuff ...
   libvlc_exception_init(&vlcExcpt);

   libvlc_media_player_play (pMediaPlayer, &vlcExcpt);

   if (!raise(&vlcExcpt))
   {
      bIsPlaying = true;
   }
}

/* -----------------------------------------------------------------\
|  Method: stop
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: stop playing
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::stop()
{
   if (bIsPlaying)
   {
      libvlc_media_player_stop (pMediaPlayer, &vlcExcpt);
      bIsPlaying = false;
   }
}

/* -----------------------------------------------------------------\
|  Method: pause
|  Begin: 24.02.2010 / 16:00:51
|  Author: Jo2003
|  Description: pause / unpause playing
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::pause()
{
   // reset exception stuff ...
   libvlc_exception_init(&vlcExcpt);

   if (bIsPlaying && libvlc_media_player_can_pause(pMediaPlayer, &vlcExcpt))
   {
      if (!raise(&vlcExcpt))
      {
         libvlc_media_player_pause(pMediaPlayer, &vlcExcpt);
         raise(&vlcExcpt);
      }
   }
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

/************************* History ***************************\
| $Log$
\*************************************************************/
