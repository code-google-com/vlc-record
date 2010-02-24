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

   QString sPlugIns = QString("--plugin-path=\"%1/plugins\"").arg(QApplication::applicationDirPath());

   // preparation of the vlc command
   const char * const vlc_args[] = {
      "-I", "dummy",         /* Don't use any interface    */
      "--ignore-config",     /* Don't use VLC's config     */
      "--extraintf=logger",  /* log anything               */
      "--verbose=2",         /* more verbose for debugging */
      sPlugIns.toAscii().constData()};

   bIsPlaying = false;

   //Initialize an instance of vlc
   //a structure for the exception is neede for this initalization
   libvlc_exception_init(&_vlcexcep);

   //create a new libvlc instance
   _vlcinstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args,&_vlcexcep);  //tricky calculation of the char space used
   raise (&_vlcexcep);

   // Create a media player playing environement
   _mp = libvlc_media_player_new (_vlcinstance, &_vlcexcep);
   raise (&_vlcexcep);

   // connect the two sliders to the corresponding slots (uses Qt's signal / slots technology)
   // connect(&tPoller, SIGNAL(timeout()), this, SLOT(updateInterface()));

   // tPoller.start(100); //start timer to trigger every 100 ms the updateInterface slot
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
   /* Stop playing */
   libvlc_media_player_stop (_mp, &_vlcexcep);

   /* Free the media_player */
   libvlc_media_player_release (_mp);

   libvlc_release (_vlcinstance);
   raise (&_vlcexcep);

   delete ui;
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
|  Method: playFile
|  Begin: 24.02.2010 / 11:46:10
|  Author: Jo2003
|  Description: play given file in widget
|
|  Parameters: file name
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::playFile(QString file)
{
   //the file has to be in one of the following formats /perhaps a little bit outdated)
   /*
    [file://]filename              Plain media file
    http://ip:port/file            HTTP URL
    ftp://ip:port/file             FTP URL
    mms://ip:port/file             MMS URL
    screen://                      Screen capture
    [dvd://][device][@raw_device]  DVD device
    [vcd://][device]               VCD device
    [cdda://][device]              Audio CD device
    udp:[[<source address>]@[<bind address>][:<bind port>]]
    */

   /* Create a new LibVLC media descriptor */
   _m = libvlc_media_new (_vlcinstance, file.toAscii(), &_vlcexcep);
   raise(&_vlcexcep);

   libvlc_media_player_set_media (_mp, _m, &_vlcexcep);
   raise(&_vlcexcep);

   // Please note!
   //
   // passing the widget to the lib shows vlc at which position it should show up
   // vlc automatically resizes the video to the given size of the widget
   // and it even resizes it, if the size changes at the playing

   /* Get our media instance to use our window */
#if defined(Q_OS_WIN)
   libvlc_media_player_set_hwnd(_mp, ui->fVideo->winId(), &_vlcexcep ); // for vlc 1.0
#elif defined(Q_OS_MAC)
   libvlc_media_player_set_agl (_mp, ui->fVideo->winId(), &_vlcexcep); // for vlc 1.0
#else //Linux
   libvlc_media_player_set_xwindow(_mp, ui->fVideo->winId(), &_vlcexcep ); // for vlc 1.0
#endif
   raise(&_vlcexcep);

   /* Play */
   libvlc_media_player_play (_mp, &_vlcexcep );
   raise(&_vlcexcep);

   bIsPlaying = true;
}

/*
void CPlayer::updateInterface()
{
   if(!bIsPlaying)
   {
      return;
   }

   // It's possible that the vlc doesn't play anything
   // so check before
   libvlc_media_t *curMedia = libvlc_media_player_get_media (_mp, &_vlcexcep);
   libvlc_exception_clear(&_vlcexcep);

   if (!curMedia)
   {
      return;
   }

   float pos=libvlc_media_player_get_position (_mp, &_vlcexcep);
   int siderPos=(int)(pos*(float)(POSITION_RESOLUTION));
   _positionSlider->setValue(siderPos);
   int volume=libvlc_audio_get_volume (_vlcinstance,&_vlcexcep);
   _volumeSlider->setValue(volume);
}
*/

/* -----------------------------------------------------------------\
|  Method: raise
|  Begin: 24.02.2010 / 11:46:10
|  Author: Jo2003
|  Description: check if there is any problem, if so display
|               error message
|  Parameters: pointer to exception
|
|  Returns: --
\----------------------------------------------------------------- */
void CPlayer::raise(libvlc_exception_t * ex)
{
   if (libvlc_exception_raised (ex))
   {
      QMessageBox::critical(this, tr("LibVLC Error!"),
                            tr("LibVLC reports following error:\n%1")
                            .arg(libvlc_exception_get_message(ex)));
   }
}

/************************* History ***************************\
| $Log$
\*************************************************************/
