#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qchanlistdelegate.h"

// for logging ...
extern CLogFile VlcLog;

// for folders ...
extern CDirStuff *pFolders;

// global showinfo class ...
extern CShowInfo showInfo;

// storage db ...
extern CVlcRecDB *pDb;

MainWindow::MainWindow(QTranslator *trans, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   ePlayState     = IncPlay::PS_WTF;
   bLogosReady    = false;
   pTranslator    = trans;
   iDwnReqId      = -1;
   bDoInitDlg     = true;
   bOnTop         = false;
   flags          = this->windowFlags();
   bFirstInit     = true;
   bFirstConnect  = true;
   bVODLogosReady = false;

   // init account info ...
   accountInfo.bHasArchive = false;
   accountInfo.bHasVOD     = false;
   accountInfo.sExpires    = QDateTime::currentDateTime().toString(DEF_TIME_FORMAT);

   // init genre info ...
   genreInfo.iCount        = 0;
   genreInfo.iPage         = 0;
   genreInfo.iTotal        = 0;
   genreInfo.sType         = "wtf";

   bSetRecentChan = false; //default value: channel was not set to recent channel-list
   bShortCuts     = true;


   // set this dialog as parent for settings and timerRec ...
   dlgSettings.setParent(this, Qt::Dialog);
   dlgTimeRec.setParent(this, Qt::Dialog);
   vlcCtrl.setParent(this);
   trayIcon.setParent(this);

   pChannelDlg = new CChannelsEPGdlg(this);
   pChannelDlg->initDialog(true);
   pChannelDlg->setTrigger(&Trigger); // set the pointer to Trigger by CChannelsEPGdlg
   pChannelDlg->setProgressBar(ui->progressBar);
   pChannelDlg->setTextEpgShort(ui->textEpgShort);
   pChannelDlg->setSettings(&dlgSettings); // set the pointer to dlgSettings by CChannelsEPGdlg
   pChannelDlg->setGenreInfo(&genreInfo);

   //set the pointer to the StatusBar in CKartinaXMLParser, CChannelsEPGdlg, CSettingsDlg,
   //CTimerRec, CVlcCtrl, CVlcRecDB
   XMLParser.setStatusBar(ui->statusBar);
   pChannelDlg->setStatusBar(ui->statusBar);
   dlgSettings.setStatusBar(ui->statusBar);
   dlgTimeRec.setStatusBar(ui->statusBar);
   vlcCtrl.setStatusBar(ui->statusBar);
   pDb->setStatusBar(ui->statusBar);


   //remove the Exit action from the menu
   ui->menuFile->removeAction(ui->actionExit);

   for ( int i = 0; i < MAX_RECENT_CHANNELS; ++i )
   {
       // Creation recent channel actions
       RecentChansActs[i] = new QAction(this);
       RecentChansActs[i]->setVisible(false);
       connect(RecentChansActs[i], SIGNAL(triggered()), this, SLOT(slotSelectChannel()));
       // add recent channel actions to the file menu
       ui->menuFile->addAction(RecentChansActs[i]);
   }

   //add separator and the Exit action to the menu
   ui->menuFile->addSeparator();
   ui->menuFile->addAction(ui->actionExit);

   QString sAspect[] = {"std.","4:3","16:9","16:10","1:1","5:4","2.35"};
   QString sCrop[] = {"std.","4:3","16:9","16:10","1:1","5:4","2.35"};

   pAspectGroup = new QActionGroup(this);
   pCropGroup   = new QActionGroup(this);

   for ( int i = 0; i < MAX_ASPECTS; i++ )
   {
       Aspect[i] = new QAction(this);
       Aspect[i]->setVisible(true);
       Aspect[i]->setCheckable(true);
       Aspect[i]->setText(sAspect[i]);
       ui->menuAspect->addAction(Aspect[i]);
       pAspectGroup->addAction(Aspect[i]);
       connect(Aspect[i], SIGNAL(triggered()), this, SLOT(slotAspect()));
   }

   for ( int i = 0; i < MAX_CROPS; i++ )
   {
       Crop[i] = new QAction(this);
       Crop[i]->setVisible(true);
       Crop[i]->setCheckable(true);
       Crop[i]->setText(sCrop[i]);
       ui->menuCrop->addAction(Crop[i]);
       pCropGroup->addAction(Crop[i]);
       connect(Crop[i], SIGNAL(triggered()), this, SLOT(slotCrop()));
   }

   Aspect[0]->setChecked(true);
   Crop[0]->setChecked(true);

   // hide upper toolbar  and shortEPG window
   ui->player->getFrameTimerInfo()->hide();
   ui->textEpgShort->hide();

   VlcLog.SetLogFile(pFolders->getDataDir(), APP_LOG_FILE);

   // set logo dir and host for chan logo downloader ...
   dwnLogos.setHostAndFolder(dlgSettings.GetAPIServer(), pFolders->getLogoDir());
   dwnVodPics.setHostAndFolder(dlgSettings.GetAPIServer(), pFolders->getVodPixDir());

   // set settings for vod browser ...
   pChannelDlg->getVodBrowser()->setSettings(&dlgSettings);

   // set log level ...
   VlcLog.SetLogLevel(dlgSettings.GetLogLevel());

   // log folder locations ...
    mInfo (tr("\ndataDir: %1\n").arg(pFolders->getDataDir())
           + tr("logoDir: %1\n").arg(pFolders->getLogoDir())
          + tr("langDir: %1\n").arg(pFolders->getLangDir())
          + tr("modDir:  %1\n").arg(pFolders->getModDir())
          + tr("appDir:  %1").arg(pFolders->getAppDir()));

   // configure trigger and start it ...
   Trigger.SetKartinaClient(&KartinaTv);
   Trigger.start();

   // give timerRec all needed infos ...
   dlgTimeRec.SetXmlParser(&XMLParser);
   dlgTimeRec.SetKartinaTrigger(&Trigger);
   dlgTimeRec.SetSettings(&dlgSettings);
   dlgTimeRec.SetVlcCtrl(&vlcCtrl);
   dlgTimeRec.SetStreamLoader(&streamLoader);

#ifdef INCLUDE_LIBVLC
   // give player the list of shortcuts ...
   ui->player->setShortCuts (&vShortcutPool);

   // give player settings and wait trigger access ...
   ui->player->setSettings(&dlgSettings);
   ui->player->setTrigger(&Trigger);

   // connect vlc control with libvlc player ...
   connect (ui->player, SIGNAL(sigPlayState(int)), &vlcCtrl, SLOT(slotLibVlcStateChange(int)));
   connect (&vlcCtrl, SIGNAL(sigLibVlcPlayMedia(QString)), ui->player, SLOT(playMedia(QString)));
   connect (&vlcCtrl, SIGNAL(sigLibVlcStop()), ui->player, SLOT(stop()));

   // progress bar update ...
   connect (ui->player, SIGNAL(sigSliderPos(int,int,int)), this, SLOT(slotUpdateProgress(int,int,int)));

   // aspect ratio, crop and full screen ...
   connect (this, SIGNAL(sigToggleFullscreen()), ui->player, SLOT(slotToggleFullScreen()));
   connect (this, SIGNAL(sigToggleAspectRatio()), ui->player, SLOT(slotToggleAspectRatio()));
   connect (this, SIGNAL(sigToggleCropGeometry()), ui->player, SLOT(slotToggleCropGeometry()));

   // get state if libVLC player to change player state display ...
   connect (ui->player, SIGNAL(sigPlayState(int)), this, SLOT(slotIncPlayState(int)));
#endif /* INCLUDE_LIBVLC */

   // connect signals and slots ...Поиск в телегиде
   connect (&KartinaTv,    SIGNAL(sigLogout(QString)), this, SLOT(slotLogout(QString)));
   connect (&KartinaTv,    SIGNAL(sigError(QString)), this, SLOT(slotErr(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotTimeShift(QString)), this, SLOT(slotGotTimeShift(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotChannelList(QString)), this, SLOT(slotChanList(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotStreamURL(QString)), this, SLOT(slotStreamURL(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotCookie(QString)), this, SLOT(slotCookie(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotEPG(QString)), this, SLOT(slotEPG(QString)));
   connect (&KartinaTv,    SIGNAL(sigTimeShiftSet(QString)), this, SLOT(slotTimeShift(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotBitRate(QString)), this, SLOT(slotGotBitrate(QString)));
   connect (&streamLoader, SIGNAL(sigStreamDownload(int,QString)), this, SLOT(slotDownloadStarted(int,QString)));
   connect (&Refresh,      SIGNAL(timeout()), &Trigger, SLOT(slotReqChanList()));
   connect (pChannelDlg->getEpgBrowser(),   SIGNAL(anchorClicked(QUrl)), this, SLOT(slotEpgAnchor(QUrl)));
   connect (&dwnLogos,     SIGNAL(sigPixReady()), this, SLOT(slotLogosReady()));
   connect (&dlgSettings,  SIGNAL(sigReloadLogos()), this, SLOT(slotReloadLogos()));
   connect (&KartinaTv,    SIGNAL(sigGotArchivURL(QString)), this, SLOT(slotArchivURL(QString)));
   connect (&dlgSettings,  SIGNAL(sigSetServer(QString)), this, SLOT(slotSetSServer(QString)));
   connect (&dlgSettings,  SIGNAL(sigSetBitRate(int)), this, SLOT(slotSetBitrate(int)));
   connect (&dlgSettings,     SIGNAL(sigSetTimeShift(int)), this, SLOT(slotSetTimeShift(int)));
   connect (&KartinaTv,    SIGNAL(sigGotTimerStreamURL(QString)), &dlgTimeRec, SLOT(slotTimerStreamUrl(QString)));
   connect (&KartinaTv,    SIGNAL(sigSrvForm(QString)), this, SLOT(slotServerForm(QString)));
   connect (&dlgTimeRec,   SIGNAL(sigRecDone()), this, SLOT(slotTimerRecordDone()));
   connect (&dlgTimeRec,   SIGNAL(sigRecActive(int)), this, SLOT(slotTimerRecActive(int)));
   connect (&vlcCtrl,      SIGNAL(sigVlcStarts(int)), this, SLOT(slotVlcStarts(int)));
   connect (&vlcCtrl,      SIGNAL(sigVlcEnds(int)), this, SLOT(slotVlcEnds(int)));
   connect (&dlgTimeRec,   SIGNAL(sigShutdown()), this, SLOT(slotShutdown()));
   connect (this,          SIGNAL(sigLCDStateChange(int)), ui->labState, SLOT(updateState(int)));
   connect (&KartinaTv,    SIGNAL(sigGotVodGenres(QString)), this, SLOT(slotGotVodGenres(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotVideos(QString)), this, SLOT(slotGotVideos(QString)));
   connect (pChannelDlg->getVodBrowser(), SIGNAL(anchorClicked(QUrl)), this, SLOT(slotVodAnchor(QUrl)));
   connect (&KartinaTv,    SIGNAL(sigGotVideoInfo(QString)), this, SLOT(slotGotVideoInfo(QString)));
   connect (&KartinaTv,    SIGNAL(sigGotVodUrl(QString)), this, SLOT(slotVodURL(QString)));
   connect (&trayIcon,     SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotSystrayActivated(QSystemTrayIcon::ActivationReason)));
   connect (this,          SIGNAL(sigHide()), &trayIcon, SLOT(show()));
   connect (this,          SIGNAL(sigShow()), &trayIcon, SLOT(hide()));

   connect (pChannelDlg,   SIGNAL(sigDoubliClickOnListWidget()), this, SLOT(slotDoubleClick()));
   connect (pChannelDlg,   SIGNAL(sigChannelDlgClosed()), this, SLOT(slotChannelDlgClosed()));
   connect (ui->player,    SIGNAL(sigAspectToggle(int)), this, SLOT(slotAspectToggle(int)));
   connect (ui->player,    SIGNAL(sigCropToggle(int)), this, SLOT(slotCropToggle(int)));

   // trigger read of saved timer records ...
   dlgTimeRec.ReadRecordList();

   // enable button ...
   TouchPlayCtrlBtns(false);

   // request authorisation ...
   ConnectionInit();
}

MainWindow::~MainWindow()
{
   delete ui;
}

////////////////////////////////////////////////////////////////////////////////
//                           Events                                           //
////////////////////////////////////////////////////////////////////////////////

void MainWindow::changeEvent(QEvent *e)
{
   QWidget::changeEvent(e);

   switch (e->type())
   {
   // catch minimize event ...
   case QEvent::WindowStateChange:

      // only hide window, if trayicon stuff is available ...
      if (QSystemTrayIcon::isSystemTrayAvailable ())
      {
         if (isMinimized())
         {
            QWindowStateChangeEvent *pEvent = (QWindowStateChangeEvent *)e;

            // store last position only, if it wasn't maximized ...
            if (pEvent->oldState() != Qt::WindowMaximized)
            {
               sizePos = geometry();
            }

            if (dlgSettings.HideToSystray())
            {
               // hide dialog ...
               QTimer::singleShot(300, this, SLOT(hide()));
            }
         }
      }
      break;

      // language switch ...
      case QEvent::LanguageChange:
         ui->retranslateUi(this);
      // translate systray tooltip ...
         CreateSystray();

      // init short cuts ...
         if (bShortCuts)
         {
             fillShortCutTab();
             InitShortCuts();
             bShortCuts = false;
         }
         else // retranslate ShortCutTable and favourites
         {
             retranslateShortcutTable();
         }

       // translate type cbx ...
       pChannelDlg->touchLastOrBestCbx();

       // translate error strings ...
         XMLParser.fillErrorMap();
      break;
      default:
      break;
   }
}

void MainWindow::showEvent(QShowEvent *event)
{
   emit sigShow();

/*
   if (bFirstConnect)
   {
       bFirstConnect = false;

       // start connection stuff in 0.5 seconds ...
       QTimer::singleShot(500, this, SLOT(slotStartConnectionChain()));
   }
*/

   QWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
   emit sigHide();
   QWidget::hideEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
   switch (event->key())
   {
   case Qt::Key_Escape:
      // ignore escape key ...
      event->ignore();
      break;

   default:
      event->accept();
      break;
   }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
   bool bAccept = true;

   // if vlc is running, ask if we want
   // to close it ...
   switch (ePlayState)
   {
   case IncPlay::PS_PLAY:
       bAccept = true;
       break;
   case IncPlay::PS_RECORD:
   case IncPlay::PS_TIMER_RECORD:
   case IncPlay::PS_TIMER_STBY:
      if (!WantToStopRec())
      {
         bAccept = false;
      }
      break;
   default:
      break;
   }

   if (bAccept)
   {
      if (pChannelDlg->isVisible())
      {
          pChannelDlg->close();
      }

      on_pushStop_clicked();

      // disconnect trayicon stuff ...
      disconnect (&trayIcon);

      // clear shortcuts ...
      ClearShortCuts ();

      // clean favourites ...
      pChannelDlg->getListFav()->clear();
      pChannelDlg->HandleFavourites();
      pChannelDlg->CleanContextMenu();

      // cancel any running kartina request ...
      KartinaTv.abort();

      // no logout needed ...
      // close programm right now ...
      QApplication::quit();
//      event->accept();
   }
   else
   {
      event->ignore();
   }
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(ui->actionShow_Live);
    menu.addAction(ui->actionPlay);
    menu.addAction(ui->actionRecord);
    menu.addAction(ui->actionStop);
    menu.addAction(ui->actionJumpBackward);
    menu.addAction(ui->actionJumpForward);
    menu.addSeparator();
    menu.addMenu(ui->menuAspect);
    menu.addMenu(ui->menuCrop);
    menu.exec(event->globalPos());
}

////////////////////////////////////////////////////////////////////////////////
//                           "on_" - Slots                                    //
////////////////////////////////////////////////////////////////////////////////
void MainWindow::on_actionOne_Click_Play_triggered()
{
    if (ui->actionOne_Click_Play->isChecked())
    {
        showInfo.setShowType(ShowInfo::Live);
    }
    else
    {
        showInfo.setShowType(ShowInfo::Unknown);
    }
}

void MainWindow::on_actionChannelsEPG_triggered()
{
    if (ui->actionChannelsEPG->isEnabled())
    {
       pChannelDlg->show();
       ui->actionChannelsEPG->setEnabled(false);
    }
}

void MainWindow::on_actionExit_triggered()
{
    bool bAccept = true;

    // if vlc is running, ask if we want
    // to close it ...
    switch (ePlayState)
    {
    case IncPlay::PS_PLAY:
        bAccept = true;
        break;
    case IncPlay::PS_RECORD:
    case IncPlay::PS_TIMER_RECORD:
    case IncPlay::PS_TIMER_STBY:
       if (!WantToStopRec())
       {
          bAccept = false;
       }
       break;
    default:
       break;
    }

    if (bAccept)
    {
        if (pChannelDlg->isVisible())
        {
            pChannelDlg->close();
        }

        on_pushStop_clicked();

        // disconnect trayicon stuff ...
        disconnect (&trayIcon);

        // clear shortcuts ...
        ClearShortCuts ();

        // clean favourites ...
        pChannelDlg->getListFav()->clear();
        pChannelDlg->HandleFavourites();
        pChannelDlg->CleanContextMenu();

       // cancel any running kartina request ...
       KartinaTv.abort();

       // no logout needed ...
       // close programm right now ...
       QApplication::quit();
    }
}

void MainWindow::on_actionShow_Live_triggered()
{
    on_pushLive_clicked();
}

void MainWindow::on_actionPlay_triggered()
{
    on_pushPlay_clicked();
}

void MainWindow::on_actionRecord_triggered()
{
    on_pushRecord_clicked();
}

void MainWindow::on_actionStop_triggered()
{
    on_pushStop_clicked();
}

void MainWindow::on_actionJumpBackward_triggered()
{
    on_pushBwd_clicked();
}

void MainWindow::on_actionJumpForward_triggered()
{
    on_pushFwd_clicked();
}

void MainWindow::on_actionShow_Upper_Tools_Panel_triggered()
{
    if (ui->player->getFrameTimerInfo()->isHidden())
    {
        ui->player->getFrameTimerInfo()->show();
    }
    else
    {
        ui->player->getFrameTimerInfo()->hide();

    }
}

void MainWindow::on_actionShow_Lower_Tools_Panel_triggered()
{
    if (ui->frameTimerInfo->isHidden())
    {
        ui->frameTimerInfo->show();
    }
    else
    {
        ui->frameTimerInfo->hide();

    }
}

void MainWindow::on_actionShow_Channel_Information_triggered()
{
    if (ui->textEpgShort->isHidden())
    {
        ui->textEpgShort->show();
    }
    else
    {
        ui->textEpgShort->hide();

    }
}

void MainWindow::on_actionAlways_On_Top_triggered()
{
    sizePos = geometry();

    if (bOnTop)
    {
        this->setWindowFlags(flags);
        bOnTop = false;
    }
    else
    {
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
        bOnTop = true;
    }

    setGeometry(sizePos);
    this->show();

}

void MainWindow::on_actionShow_Status_Bar_triggered()
{
    if (ui->statusBar->isHidden())
    {
        ui->statusBar->show();
    }
    else
    {
        ui->statusBar->hide();
    }
}

void MainWindow::on_actionTime_Record_triggered()
{
    uint now   = QDateTime::currentDateTime().toTime_t();
    int  cid = pChannelDlg->getCurrentCid();

    // timeRec.SetRecInfo(now, now, -1);
    dlgTimeRec.SetRecInfo(now, now, cid);
    dlgTimeRec.exec();
}

void MainWindow::on_actionSettings_triggered()
{
   if (dlgSettings.exec() == QDialog::Accepted)
   {
       // if changes where saved, accept it here ...
       ConnectionInit();
   }
}

void MainWindow::on_actionClear_Recent_Channel_List_triggered()
{
    QSettings settings;
    QStringList chans = settings.value("RecentChanList").toStringList();

    chans.clear();

    for (int i = 0; i < MAX_RECENT_CHANNELS; ++i)
    {
        RecentChansActs[i]->setVisible(false);
    }

    settings.setValue("RecentChanList", chans);

}

void MainWindow::on_actionAbout_triggered()
{
    CAboutDialog dlg(this, accountInfo.sExpires);
    dlg.ConnectSettings(&dlgSettings);
    dlg.exec();
}

void MainWindow::on_actionGuid_triggered()
{
    CInstruction inst;
    inst.exec();
}

void MainWindow::on_pushRecord_clicked()
{
#ifdef INCLUDE_LIBVLC

   // is archive play active ...
   if ((showInfo.showType() == ShowInfo::Archive)
      && (showInfo.playState () == IncPlay::PS_PLAY))
   {
      if (AllowAction(IncPlay::PS_RECORD))
      {
         // archive play active ...
         uint    gmt = ui->player->getSilderPos ();
         QString req = QString("cid=%1&gmt=%2").arg(showInfo.channelId()).arg(gmt);

         showInfo.setPlayState(IncPlay::PS_RECORD);

         TouchPlayCtrlBtns(false);
         Trigger.TriggerRequest(Kartina::REQ_ARCHIV, req);
      }
   }
   else
   {

#endif // INCLUDE_LIBVLC
      int cid = pChannelDlg->getCurrentCid();

      if (pChannelDlg->getChanMap()->contains(cid))
      {
            if (AllowAction(IncPlay::PS_RECORD))
            {
               cparser::SChan chan = pChannelDlg->getChanMap()->value(cid);

               // new own downloader ...
               if (vlcCtrl.ownDwnld() && (iDwnReqId != -1))
               {
                  streamLoader.stopDownload (iDwnReqId);
                  iDwnReqId = -1;
               }

               showInfo.setChanId(cid);
               showInfo.setChanName(chan.sName);
               showInfo.setShowType(ShowInfo::Live);
               showInfo.setShowName(chan.sProgramm);
               showInfo.setStartTime(chan.uiStart);
               showInfo.setEndTime(chan.uiEnd);
               showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
               showInfo.setPlayState(IncPlay::PS_RECORD);
               showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                     .arg("rgb(255, 254, 212)")
                                     .arg(createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

               TouchPlayCtrlBtns(false);
               Trigger.TriggerRequest(Kartina::REQ_STREAM, cid);
            }
      }
#ifdef INCLUDE_LIBVLC
   }
#endif // INCLUDE_LIBVLC
}


void MainWindow::on_pushLive_clicked()
{
    int cid = pChannelDlg->getCurrentCid();
    if  (pChannelDlg->getChanMap()->contains(cid))
    {
       // set EPG offset to 0 ...
       int iEpgOffset = 0;
       pChannelDlg->setEpgOffset(iEpgOffset);
       Trigger.TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);

       // fake play button press ...
       if (AllowAction(IncPlay::PS_PLAY))
       {
          int cid  = pChannelDlg->getCurrentCid();

          if (pChannelDlg->getChanMap()->contains(cid))
          {
             if (AllowAction(IncPlay::PS_PLAY))
             {
                cparser::SChan chan = pChannelDlg->getChanMap()->value(cid);

                showInfo.setChanId(cid);
                showInfo.setChanName(chan.sName);
                showInfo.setShowType(ShowInfo::Live);
                showInfo.setShowName(chan.sProgramm);
                showInfo.setStartTime(chan.uiStart);
                showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
                showInfo.setEndTime(chan.uiEnd);
                showInfo.setPlayState(IncPlay::PS_PLAY);
                showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                       .arg("rgb(255, 254, 212)")
                                       .arg(createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

                TouchPlayCtrlBtns(false);
                Trigger.TriggerRequest(Kartina::REQ_STREAM, cid);
             }
          }
       }
    }
}

void MainWindow::on_pushPlay_clicked()
{
#ifdef INCLUDE_LIBVLC
   // play or pause functionality ...
   if ((showInfo.playState() == IncPlay::PS_PLAY)
      && showInfo.canCtrlStream())
   {
      // we're playing ... we want pause ...
      ui->player->pause();

      // update showInfo ...
      showInfo.setPlayState(IncPlay::PS_PAUSE);

      // update buttons ...
      TouchPlayCtrlBtns(true);
   }
   else if ((showInfo.playState() == IncPlay::PS_PAUSE)
      && showInfo.canCtrlStream())
   {
      // we're pausing ... want to play ...
      ui->player->play();

      // update showInfo ...
      showInfo.setPlayState(IncPlay::PS_PLAY);

      // update buttons ...
      TouchPlayCtrlBtns(true);
   }
   else
   {
#endif // INCLUDE_LIBVLC
       int cid = pChannelDlg->getCurrentCid();

       if (pChannelDlg->getChanMap()->contains(cid))
       {
             if (AllowAction(IncPlay::PS_PLAY))
             {
                cparser::SChan chan = pChannelDlg->getChanMap()->value(cid);

                showInfo.setChanId(cid);
                showInfo.setChanName(chan.sName);
                showInfo.setShowType(ShowInfo::Live);
                showInfo.setShowName(chan.sProgramm);
                showInfo.setStartTime(chan.uiStart);
                showInfo.setEndTime(chan.uiEnd);
                showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
                showInfo.setPlayState(IncPlay::PS_PLAY);
                showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                     .arg("rgb(255, 254, 212)")
                                     .arg(createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

               TouchPlayCtrlBtns(false);
               Trigger.TriggerRequest(Kartina::REQ_STREAM, cid);
            }
      }
#ifdef INCLUDE_LIBVLC
   }
#endif // INCLUDE_LIBVLC
}

void MainWindow::on_pushStop_clicked()
{
   if (AllowAction(IncPlay::PS_STOP))
   {
      ui->labState->setHeader("");
      ui->labState->setFooter("");

      // new own downloader ...
      if (vlcCtrl.ownDwnld() && (iDwnReqId != -1))
      {
         streamLoader.stopDownload (iDwnReqId);
         iDwnReqId = -1;
      }

      vlcCtrl.stop();

      showInfo.setPlayState(IncPlay::PS_STOP);
      TouchPlayCtrlBtns(true);
   }
}

#ifdef INCLUDE_LIBVLC
void MainWindow::on_pushBwd_clicked()
{
   // we have minutes but need seconds --> x 60!!!
   int iJmpVal = ui->cbxTimeJumpVal->currentText().toInt() * 60;

   // jump ...
   ui->player->slotTimeJumpRelative(-iJmpVal);
}

void MainWindow::on_pushFwd_clicked()
{
   // we have minutes but need seconds --> x 60!!!
   int iJmpVal = ui->cbxTimeJumpVal->currentText().toInt() * 60;

   // jump ...
   ui->player->slotTimeJumpRelative(iJmpVal);
}
#endif /* INCLUDE_LIBVLC */

////////////////////////////////////////////////////////////////////////////////
//                                Slots                                       //
////////////////////////////////////////////////////////////////////////////////

void MainWindow::exec()
{
   if (bDoInitDlg)
   {
      bDoInitDlg = false;
      initDialog();
   }

   QMainWindow::show();
}

void MainWindow::slotSystrayActivated(QSystemTrayIcon::ActivationReason reason)
{
   switch (reason)
   {
   case QSystemTrayIcon::MiddleClick:
   case QSystemTrayIcon::DoubleClick:
   case QSystemTrayIcon::Trigger:
   case QSystemTrayIcon::Context:
      if (isHidden())
      {
         setGeometry(sizePos);
         QTimer::singleShot(300, this, SLOT(showNormal()));
      }
      break;
   default:
      break;
   }
}

void MainWindow::slotErr(QString str)
{
//   QMessageBox::critical(this, tr("Error"),
//                         tr("%1 Client API reports some errors: %2")
//                         .arg(COMPANY_NAME).arg(str));
   ui->statusBar->showMessage(tr("Error: %1 Client API reports some errors: %2")
                              .arg(COMPANY_NAME).arg(str));
   TouchPlayCtrlBtns();
}

void MainWindow::slotLogout(QString str)
{
    XMLParser.checkResponse(str, __FUNCTION__, __LINE__);

   mInfo(tr("logout done ..."));

 //  QDialog::accept ();
}

void MainWindow::slotStreamURL(QString str)
{
   QString sChan, sShow, sUrl, sTime;

   if (!XMLParser.parseUrl(str, sUrl))
   {
      sShow = CleanShowName (showInfo.showName());
      sChan = showInfo.chanName();

      sTime = sTime = QString("%1 - %2")
                      .arg(QDateTime::fromTime_t(showInfo.starts()).toString("hh:mm"))
                      .arg(QDateTime::fromTime_t(showInfo.ends()).toString("hh:mm"));

      if (sShow == "")
      {
         sShow = sChan;
      }

      // add additional info to LCD ...
      ui->labState->setHeader(sChan);
      ui->labState->setFooter(sTime);

      if (ePlayState == IncPlay::PS_RECORD)
      {
         if (!vlcCtrl.ownDwnld())
         {
            StartVlcRec(sUrl, sShow);
         }
         else
         {
            StartStreamDownload(sUrl, sShow);
         }
      }
      else if (ePlayState == IncPlay::PS_PLAY)
      {
          StartVlcPlay(sUrl);

          if (ui->actionOne_Click_Play->isChecked())
          {
              showInfo.setShowType(ShowInfo::Live);
          }
          else
          {
              showInfo.setShowType(ShowInfo::Unknown);
          }
      }
   }

   TouchPlayCtrlBtns();
}

void MainWindow::slotCookie (QString str)
{
    QString sCookie;

    // parse cookie ...
    if (!XMLParser.parseCookie(str, sCookie, accountInfo))
    {
       KartinaTv.SetCookie(sCookie);

       // request streamserver ...
       Trigger.TriggerRequest(Kartina::REQ_GET_SERVER);
    }
}

void MainWindow::slotServerForm(QString str)
{
   QVector<cparser::SSrv> vSrv;
   QString                sActSrv;

   if (!XMLParser.parseSServers(str, vSrv, sActSrv))
   {
      dlgSettings.SetStreamServerCbx(vSrv, sActSrv);
      mInfo(tr("Active stream server is %1").arg(sActSrv));
   }

   Trigger.TriggerRequest(Kartina::REQ_GETBITRATE);
}

void MainWindow::slotGotTimeShift(QString str)
{
   // parse timeshift ...
   QVector<int> vValues;
   int          iShift;
   QString      sName;

   if (!XMLParser.parseSettings(str, vValues, iShift, sName))
   {
      dlgSettings.fillTimeShiftCbx(vValues, iShift);

      // set timeshift ...
      pChannelDlg->getEpgBrowser()->SetTimeShift(iShift);
      dlgTimeRec.SetTimeShift(iShift);

      // request channel list ...
      Trigger.TriggerRequest(Kartina::REQ_CHANNELLIST);
   }
}

void MainWindow::slotGotBitrate(QString str)
{
   QVector<int> vValues;
   int          iActVal = 0;
   QString      sName;

   if (!XMLParser.parseSettings(str, vValues, iActVal, sName))
   {
      dlgSettings.SetBitrateCbx(vValues, iActVal);
      mInfo (tr("Using Bitrate %1 kbit/s ...").arg(iActVal));
   }

   Trigger.TriggerRequest(Kartina::REQ_GETTIMESHIFT);
}

void MainWindow::slotTimeShift (QString str)
{
    if(!XMLParser.checkResponse(str, __FUNCTION__, __LINE__))
    {
       Trigger.TriggerRequest(Kartina::REQ_CHANNELLIST);
    }
}

void MainWindow::slotChanList (QString str)
{
   QVector<cparser::SChan> chanList;
   QVector<cparser::SChan>::const_iterator cit;

   if (!XMLParser.parseChannelList(str, chanList, dlgSettings.FixTime()))
   {
      FillChanMap(chanList);
      FillChannelList(chanList);

      // set channel list in timeRec class ...
      dlgTimeRec.SetChanList(chanList);
      dlgTimeRec.StartTimer();
   }

   // only download channel logos, if they aren't there ...
   if (!dwnLogos.IsRunning() && !bLogosReady)
   {
      QStringList lLogos;

      for (cit = chanList.constBegin(); cit != chanList.constEnd(); cit ++)
      {
         if(!(*cit).bIsGroup)
         {
            lLogos.push_back((*cit).sIcon);
         }
      }

      dwnLogos.setPictureList(lLogos);
   }

   // create favourite buttons if needed ...
      pChannelDlg->CreateFav();

   TouchPlayCtrlBtns();
}

void MainWindow::slotEPG(QString str)
{
   QVector<cparser::SEpg> epg;
   QDateTime epgTime = QDateTime::currentDateTime().addDays(pChannelDlg->getEpgOffset());
   QModelIndex idx   = pChannelDlg->getChannelList()->currentIndex();
   int cid           = qvariant_cast<int>(idx.data(channellist::cidRole));
   QIcon icon;

   if (!XMLParser.parseEpg(str, epg))
   {
      pChannelDlg->getEpgBrowser()->DisplayEpg(epg, pChannelDlg->getChanMap()->value(cid).sName,
                              cid, epgTime.toTime_t(),
                              accountInfo.bHasArchive ?
                              pChannelDlg->getChanMap()->value(cid).bHasArchive : false);

      // fill epg control ...
      icon = qvariant_cast<QIcon>(idx.data(channellist::iconRole));
      pChannelDlg->getLabChanIcon()->setPixmap(icon.pixmap(24, 24));
      pChannelDlg->getLabChanName()->setText(pChannelDlg->getChanMap()->value(cid).sName);
      pChannelDlg->getLabCurrDay()->setText(epgTime.toString("dd. MMM. yyyy"));

      pChannelDlg->getNavBar()->setCurrentIndex(epgTime.date().dayOfWeek() - 1);

      TouchPlayCtrlBtns();
      pChannelDlg->getChannelList()->setFocus(Qt::OtherFocusReason);

      //initially set the current channel in FillChannelList() to the first channel
      //from the saved recent channel list.
      //if not the first time, set the channel to the recent channels only
      //if a channel was in channel-list clicked
      if (bFirstInit)
      {
          bFirstInit = false;
      }
      else
      {
          if (!bSetRecentChan) //a channel was in channel-list dialog clicked
          {
              // if One_Click_Play is not checked a channel must not be added to the recent-cannel
              // list. In this case it is possible in cannel-list dialog throught channels to jump
              // and these channel will be not added to the recent channel-list
              if (ui->actionOne_Click_Play->isChecked())
              {
                  setRecentChannel(qvariant_cast<QString>(idx.data(channellist::nameRole)));
              }
          }
          else
          {
              bSetRecentChan = false; //a channel was in channel-menu or in recent channel-list clicked
          }
       }

      //it works with accounts 140, 141, ... For these accounts there is no VOD
      if ((dlgSettings.GetPasswd()).toInt() < 1000 )
      {
          if (ui->actionOne_Click_Play->isChecked())
          {
              on_pushPlay_clicked();
          }
      }
      else
      {
         //it works only with the real accounts (not 140,...)!
         // update vod stuff only at startup ...
         if (accountInfo.bHasVOD)
         {
            if (pChannelDlg->getCbxGenre()->count() == 0)
            {
               Trigger.TriggerRequest(Kartina::REQ_GETVODGENRES);
            }
         }
         else
         {
            if (ui->actionOne_Click_Play->isChecked())
            {
                on_pushPlay_clicked();
            }
         }
      }
   }
}

void MainWindow::slotEpgAnchor (const QUrl &link)
{
   // create request string ...
   QString action = link.encodedQueryItemValue(QByteArray("action"));
   bool    ok     = false;

   if (action == "archivrec")
   {
      if (AllowAction(IncPlay::PS_RECORD))
      {
         ok = true;
      }
   }
   else if (action == "archivplay")
   {
      if (AllowAction(IncPlay::PS_PLAY))
      {
         ok = true;
      }
   }
   else if(action == "timerrec")
   {
      uint uiStart = link.encodedQueryItemValue(QByteArray("start")).toUInt();
      uint uiEnd   = link.encodedQueryItemValue(QByteArray("end")).toUInt();
      int  iChan   = link.encodedQueryItemValue(QByteArray("cid")).toInt();

      dlgTimeRec.SetRecInfo(uiStart, uiEnd, iChan, CleanShowName(pChannelDlg->getEpgBrowser()->epgShow(uiStart).sShowName));
      dlgTimeRec.exec();
   }

   if (ok)
   {
      TouchPlayCtrlBtns(false);

      // new own downloader ...
      if (vlcCtrl.ownDwnld() && (iDwnReqId != -1))
      {
         streamLoader.stopDownload (iDwnReqId);
         iDwnReqId = -1;
      }

      QString    cid  = link.encodedQueryItemValue(QByteArray("cid"));
      QString    gmt  = link.encodedQueryItemValue(QByteArray("gmt"));
      QString    req  = QString("cid=%1&gmt=%2").arg(cid.toInt()).arg(gmt.toUInt());
      epg::SShow sepg = pChannelDlg->getEpgBrowser()->epgShow(gmt.toUInt());

      // store all info about show ...
      showInfo.setChanId(cid.toInt());
      showInfo.setChanName(pChannelDlg->getChanMap()->value(cid.toInt()).sName);
      showInfo.setShowName(sepg.sShowName);
      showInfo.setStartTime(gmt.toUInt());
      showInfo.setEndTime(sepg.uiEnd);
      showInfo.setShowType(ShowInfo::Archive);
      showInfo.setPlayState(ePlayState);
      showInfo.setLastJumpTime(0);
      showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                            .arg("rgb(255, 254, 212)")
                            .arg(createTooltip(tr("%1 (Archive)").arg(showInfo.chanName()),
                            QString("%1 %2").arg(sepg.sShowName).arg(sepg.sShowDescr),
                            sepg.uiStart, sepg.uiEnd))));

      // add additional info to LCD ...
      int     iTime = (sepg.uiEnd) ? (int)((sepg.uiEnd - sepg.uiStart) / 60) : 60;
      QString sTime = tr("Length: %1 min.").arg(iTime);
      ui->labState->setHeader(showInfo.chanName() + tr(" (Ar.)"));
      ui->labState->setFooter(sTime);

      Trigger.TriggerRequest(Kartina::REQ_ARCHIV, req);
   }
}

void MainWindow::slotLogosReady()
{
   // downloader sayd ... logos are there ...
   bLogosReady = true;
}

void MainWindow::slotReloadLogos()
{
   bLogosReady = false;

   if (!dwnLogos.IsRunning())
   {
      QStringList lLogos;
      QMap<int, cparser::SChan>::const_iterator cit;

      // create tmp channel list with channels from channelList ...
      for (cit = pChannelDlg->getChanMap()->constBegin(); cit != pChannelDlg->getChanMap()->constEnd(); cit++)
      {
         if (!(*cit).bIsGroup)
         {
            lLogos.push_back((*cit).sIcon);
         }
      }

      dwnLogos.setPictureList(lLogos);
   }
}

void MainWindow::slotArchivURL(QString str)
{
   QString sUrl;

   if (!XMLParser.parseUrl(str, sUrl))
   {
      if (ePlayState == IncPlay::PS_RECORD)
      {
         if (!vlcCtrl.ownDwnld())
         {
            StartVlcRec(sUrl, CleanShowName(showInfo.showName()));
         }
         else
         {
            StartStreamDownload(sUrl, CleanShowName(showInfo.showName()));
         }

         showInfo.setPlayState(IncPlay::PS_RECORD);
      }
      else if (ePlayState == IncPlay::PS_PLAY)
      {
         StartVlcPlay(sUrl);

         showInfo.setPlayState(IncPlay::PS_PLAY);
      }
   }

   TouchPlayCtrlBtns(true);
}

void MainWindow::slotSetSServer(QString sIp)
{
   Trigger.TriggerRequest(Kartina::REQ_SERVER, sIp);
}

void MainWindow::slotSetBitrate(int iRate)
{
   Trigger.TriggerRequest(Kartina::REQ_SETBITRATE, iRate);
}

/* -----------------------------------------------------------------\
|  Method: slotSetTimeShift
|  Begin: 14.09.2011 / 10:00
|  Author: Jo2003
|  Description: set timeshift
|
|  Parameters: timeshift in hours
|
|  Returns: --
\----------------------------------------------------------------- */
void MainWindow::slotSetTimeShift(int iShift)
{
   TouchPlayCtrlBtns(false);

   // set timeshift ...
   pChannelDlg->getEpgBrowser()->SetTimeShift(iShift);
   dlgTimeRec.SetTimeShift(iShift);

   Trigger.TriggerRequest(Kartina::REQ_TIMESHIFT, iShift);
}
void MainWindow::slotTimerRecordDone()
{
   if (ePlayState == IncPlay::PS_TIMER_RECORD)
   {
      mInfo(tr("timeRec reports: record done!"));
      ePlayState = IncPlay::PS_STOP;
      TouchPlayCtrlBtns();
   }
}

void MainWindow::slotTimerRecActive (int iState)
{
   mInfo(tr("timeRec reports: record active!"));
   ePlayState = (IncPlay::ePlayStates)iState;
   TouchPlayCtrlBtns();

   // reset lcd header and footer ...
   ui->labState->setHeader("");
   ui->labState->setFooter("");
}

void MainWindow::slotVlcEnds(int iState)
{
   iState = 0; // suppress warnings ...
   if (ePlayState != IncPlay::PS_STOP)
   {
      mInfo(tr("vlcCtrl reports: vlc player ended!"));
      ePlayState = IncPlay::PS_STOP;
   }
   TouchPlayCtrlBtns();
}

void MainWindow::slotVlcStarts(int iState)
{
   mInfo(tr("vlcCtrl reports: vlc player active!"));

   if (ePlayState != (IncPlay::ePlayStates)iState)
   {
      ePlayState = (IncPlay::ePlayStates)iState;
   }

   TouchPlayCtrlBtns();
}

void MainWindow::slotShutdown()
{
 //  QDialog::accept();
}

void MainWindow::slotSplashScreen()
{
    CAboutDialog dlg(this, accountInfo.sExpires);
    dlg.ConnectSettings(&dlgSettings);
    dlg.exec();
}

void MainWindow::slotIncPlayState(int iState)
{
   switch((IncPlay::ePlayStates)iState)
   {
   case IncPlay::PS_PLAY:
      // might be play, record, timer record -->
      // therefore use internal state ...
      emit sigLCDStateChange ((int)ePlayState);
      break;

   case IncPlay::PS_END:
      // display "stop" in case of "end" ...
      emit sigLCDStateChange ((int)IncPlay::PS_STOP);
      break;

   case IncPlay::PS_ERROR:
      // note about the error also in showInfo class ...
      showInfo.setPlayState((IncPlay::ePlayStates)iState);

      // update play buttons ...
      TouchPlayCtrlBtns(true);

      // fall thru here | |
      //                V V
   default:
      emit sigLCDStateChange (iState);
      break;
   }
}

void MainWindow::slotDownloadStarted(int id, QString sFileName)
{
   Q_PID     vlcpid   = 0;
   QString   sCmdLine, fileName, sExt;
   QFileInfo info(sFileName);

   iDwnReqId = id;
   fileName  = QString ("%1/%2").arg(info.path()).arg(info.completeBaseName());
   sExt      = info.suffix();

   sCmdLine  = vlcCtrl.CreateClArgs(vlcctrl::VLC_REC_LIVE, dlgSettings.GetVLCPath(),
                                    "", dlgSettings.GetBufferTime(), fileName, sExt);

   // start player if we have a command line ...
   if (sCmdLine != "")
   {
      vlcpid = vlcCtrl.start(sCmdLine, -1, dlgSettings.DetachPlayer(), ePlayState);
   }

   // successfully started ?
   if (!vlcpid)
   {
//      QMessageBox::critical(this, tr("Error!"), tr("Can't start VLC-Media Player!"));
      ui->statusBar->showMessage(tr("Error! Can't start VLC-Media Player!"));
      ePlayState = IncPlay::PS_ERROR;
      TouchPlayCtrlBtns();
   }
   else
   {
      mInfo(tr("Started VLC with pid #%1!").arg((uint)vlcpid));
   }
}

void MainWindow::slotGotVodGenres(QString str)
{
   QVector<cparser::SGenre> vGenres;
   QVector<cparser::SGenre>::const_iterator cit;

   // delete content ...
   pChannelDlg->getCbxGenre()->clear();

   if (!XMLParser.parseGenres(str, vGenres))
   {
      // fill genres combo box ...
      pChannelDlg->getCbxGenre()->addItem(tr("All"), QVariant((int)-1));

      for (cit = vGenres.constBegin(); cit != vGenres.constEnd(); cit ++)
      {
         pChannelDlg->getCbxGenre()->addItem((*cit).sGName, QVariant((int)(*cit).uiGid));
      }
   }

   pChannelDlg->getCbxGenre()->setCurrentIndex(0);

   // trigger video load ...
   QUrl url;
   url.addQueryItem("type", "last");
   url.addQueryItem("nums", "10000");
   Trigger.TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));
}

void MainWindow::slotGotVideos(QString str)
{
   QVector<cparser::SVodVideo> vVodList;
   QVector<cparser::SVodVideo>::const_iterator cit;
   cparser::SGenreInfo gInfo;

   if (!XMLParser.parseVodList(str, vVodList, gInfo))
   {
       if (!bVODLogosReady)
       {
         bVODLogosReady = true;

         // download pictures ...
         QStringList lPix;

         for (cit = vVodList.constBegin(); cit != vVodList.constEnd(); cit ++)
         {
            lPix.push_back((*cit).sImg);
         }

         dwnVodPics.setPictureList(lPix);

         // get normal video view ...
         pChannelDlg->on_cbxGenre_activated(0);
      }
      else
      {
         genreInfo = gInfo;
         pChannelDlg->touchVodNavBar(gInfo);
         pChannelDlg->getVodBrowser()->displayVodList (vVodList, pChannelDlg->getCbxGenre()->currentText());
      }

      //it works only with real accounts, not with 140, 141, ...
      if (ui->actionOne_Click_Play->isChecked())
      {
          on_pushPlay_clicked();
      }
   }
}

void MainWindow::slotVodAnchor(const QUrl &link)
{
   QString action = link.encodedQueryItemValue(QByteArray("action"));
   bool ok        = false;
   int  id        = 0;

   if (action == "vod_info")
   {
      id = link.encodedQueryItemValue(QByteArray("vodid")).toInt();
      Trigger.TriggerRequest(Kartina::REQ_GETVIDEOINFO, id);
   }
   else if (action == "backtolist")
   {
      QUrl    url;
      QString sType = pChannelDlg->getCbxLastOrBest()->itemData(pChannelDlg->getCbxLastOrBest()->currentIndex()).toString();
      id            = pChannelDlg->getCbxGenre()->itemData(pChannelDlg->getCbxGenre()->currentIndex()).toInt();
      url.addQueryItem("type", sType);

      if (id != -1)
      {
          url.addQueryItem("genre", QString::number(id));
      }

      Trigger.TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));

      id = pChannelDlg->getCbxGenre()->currentIndex();

      id = pChannelDlg->getCbxGenre()->itemData(id).toInt();

      Trigger.TriggerRequest(Kartina::REQ_GETVIDEOS, id);
   }
   else if (action == "play")
   {
      if (AllowAction(IncPlay::PS_PLAY))
      {
         ok = true;
      }
   }
   else if (action == "record")
   {
      if (AllowAction(IncPlay::PS_RECORD))
      {
         ok = true;
      }
   }

   if (ok)
   {
      TouchPlayCtrlBtns(false);

      // new own downloader ...
      if (vlcCtrl.ownDwnld() && (iDwnReqId != -1))
      {
         streamLoader.stopDownload (iDwnReqId);
         iDwnReqId = -1;
      }

      id = link.encodedQueryItemValue(QByteArray("vid")).toInt();

      showInfo.setChanId(-1);
      showInfo.setChanName("");
      showInfo.setShowName(pChannelDlg->getVodBrowser()->getName());
      showInfo.setStartTime(0);
      showInfo.setEndTime(0);
      showInfo.setShowType(ShowInfo::VOD);
      showInfo.setPlayState(ePlayState);
      showInfo.setLastJumpTime(0);
      showInfo.setHtmlDescr(pChannelDlg->getVodBrowser()->getShortContent());


      ui->labState->setHeader(tr("Video On Demand"));
      ui->labState->setFooter(showInfo.showName());

      Trigger.TriggerRequest(Kartina::REQ_GETVODURL, id);
   }
}

void MainWindow::slotGotVideoInfo(QString str)
{
   cparser::SVodVideo vodInfo;
   if (!XMLParser.parseVideoInfo(str, vodInfo))
   {
      pChannelDlg->getVodBrowser()->displayVideoDetails(vodInfo);
   }
}

void MainWindow::slotVodURL(QString str)
{
   QString sUrl;

   if (!XMLParser.parseUrl(str, sUrl))
   {
      if (ePlayState == IncPlay::PS_RECORD)
      {
         // use own downloader ... ?
         if (!vlcCtrl.ownDwnld())
         {
            StartVlcRec(sUrl, CleanShowName(showInfo.showName()));
         }
         else
         {
            StartStreamDownload(sUrl, CleanShowName(showInfo.showName()), "m4v");
         }

         showInfo.setPlayState(IncPlay::PS_RECORD);
      }
      else if (ePlayState == IncPlay::PS_PLAY)
      {
         StartVlcPlay(sUrl);

         showInfo.setPlayState(IncPlay::PS_PLAY);
      }
   }

   TouchPlayCtrlBtns(true);
}

void MainWindow::slotDoubleClick()
{
    int cid = pChannelDlg->getCurrentCid();

    if (pChannelDlg->getChanMap()->contains(cid))
    {
       if (AllowAction(IncPlay::PS_PLAY))
       {
          cparser::SChan chan = pChannelDlg->getChanMap()->value(cid);

          showInfo.setChanId(cid);
          showInfo.setChanName(chan.sName);
          showInfo.setShowType(ShowInfo::Live);
          showInfo.setShowName(chan.sProgramm);
          showInfo.setStartTime(chan.uiStart);
          showInfo.setEndTime(chan.uiEnd);
          showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
          showInfo.setPlayState(IncPlay::PS_PLAY);
          showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                            .arg("rgb(255, 254, 212)")
                            .arg(createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

          TouchPlayCtrlBtns(false);
          Trigger.TriggerRequest(Kartina::REQ_STREAM, cid);
       }
    }
}

void MainWindow::slotChannelDlgClosed()
{
    ui->actionChannelsEPG->setEnabled(true);
}

void MainWindow::slotSelectChannel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QModelIndex idx;
    QString sChanName = "";
    QString str = "";
    QString str1 = "";
    int i = -1;

    do
    {
        i++;
        idx = pChannelDlg->getModel()->index(i, 0);
        str = qvariant_cast<QString>(idx.data(channellist::nameRole));
        str = str.trimmed();
        str1 = action->text();
        str1 = str1.trimmed();
        sChanName = str;
    } while( !str.contains(str1, Qt::CaseInsensitive) && (i < pChannelDlg->getModel()->rowCount()) );

    if (str.contains(str1, Qt::CaseInsensitive))
    {
        setRecentChannel(sChanName);
        bSetRecentChan = true;

        //go to slotEPG and then until slotGotVideos.
        //In this case in slotEPG the recent channel must not be set
        pChannelDlg->getChannelList()->setCurrentIndex(idx);
        pChannelDlg->getChannelList()->scrollTo(idx, QAbstractItemView::PositionAtTop);
    }
}

void MainWindow::slotAspect()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString sAspectName = "";
    QString str = "";
    int ind = 0;

    sAspectName = action->text();
    sAspectName = sAspectName.trimmed();

    for ( int i = 0; i < MAX_ASPECTS; i++)
    {
        str = Aspect[i]->text();
        str = str.trimmed();

        if (str == sAspectName)
        {
            ind = i;
            break;
        }
    }

    ui->player->getCbxAspect()->setCurrentIndex(ind);
}

void MainWindow::slotCrop()
{
    QAction *action = qobject_cast<QAction *>(sender());
    QString sCropName = "";
    QString str = "";
    int ind = 0;

    sCropName = action->text();
    sCropName = sCropName.trimmed();

    for ( int i = 0; i < MAX_CROPS; i++)
    {
        str = Crop[i]->text();
        str = str.trimmed();

        if (str == sCropName)
        {
            ind = i;
            break;
        }
    }

    ui->player->getCbxCrop()->setCurrentIndex(ind);
}

void MainWindow::slotAspectToggle(int idx)
{
    // aspect ratio was chanded in player, set new aspect ratio in menu
    Aspect[idx]->setChecked(true);
}

void MainWindow::slotCropToggle(int idx)
{
    // crop ratio was chanded in player, set new crop ratio in menu
    Crop[idx]->setChecked(true);
}

void MainWindow::slotChannelUp()
{
    QModelIndex idx;
    int         iRow;
    bool        bSuccess = false;
    idx  = pChannelDlg->getChannelList()->currentIndex();

    do
    {
       iRow = idx.row();

       if (!iRow)
       {
          iRow = pChannelDlg->getModel()->rowCount() - 1;
       }
       else
       {
          iRow --;
       }

       idx = pChannelDlg->getModel()->index(iRow, 0);

       // make sure to not mark a channel group ...
       if (qvariant_cast<int>(idx.data(channellist::cidRole)) != -1)
       {
          bSuccess = true;
       }

    } while (!bSuccess);


    pChannelDlg->getChannelList()->setCurrentIndex(idx);
    pChannelDlg->getChannelList()->scrollTo(idx, QAbstractItemView::PositionAtTop);
}

void MainWindow::slotChannelDown()
{
    QModelIndex idx;
    int         iRow;
    bool        bSuccess = false;
    idx  = pChannelDlg->getChannelList()->currentIndex();

    do
    {
       iRow = idx.row();

       if (iRow == (pChannelDlg->getModel()->rowCount() - 1))
       {
          iRow = 0;
       }
       else
       {
          iRow ++;
       }

       idx = pChannelDlg->getModel()->index(iRow, 0);

       // make sure to not mark a channel group ...
       if (qvariant_cast<int>(idx.data(channellist::cidRole)) != -1)
       {
          bSuccess = true;
       }

    } while (!bSuccess);

    pChannelDlg->getChannelList()->setCurrentIndex(idx);
    pChannelDlg->getChannelList()->scrollTo(idx, QAbstractItemView::PositionAtTop);
}

void MainWindow::slotToggleEpgVod()
{
   int iIdx = pChannelDlg->getTabEpgVOD()->currentIndex();

   iIdx ++;

   if (iIdx > (pChannelDlg->getTabEpgVOD()->count() - 1))
   {
      iIdx = 0;
   }

   pChannelDlg->getTabEpgVOD()->setCurrentIndex(iIdx);
}

// void MainWindow::slotStartConnectionChain()
// {
//    Trigger.TriggerRequest(Kartina::REQ_COOKIE);
// }

void MainWindow::slotUpdateProgress(int iMin, int iMax, int iAct)
{
    ui->progressBar->setMinimum(iMin);
    ui->progressBar->setMaximum(iMax);
    ui->progressBar->setValue(iAct);
}

////////////////////////////////////////////////////////////////////////////////
//                             normal functions                               //
////////////////////////////////////////////////////////////////////////////////

void MainWindow::initDialog()
{
    // create systray  ...
    CreateSystray();

    // get player module ...
    vlcCtrl.LoadPlayerModule(dlgSettings.GetPlayerModule());

    // set language as read ...
    pTranslator->load(QString("lang_%1").arg(dlgSettings.GetLanguage ()),
                      pFolders->getLangDir());

    // -------------------------------------------
    // set last windows size / position ...
    // -------------------------------------------
    bool ok = false;
    sizePos = dlgSettings.GetWindowRect(&ok);

    if (ok)
    {
       setGeometry(sizePos);
    }
    else
    {
       // store default size ...
       sizePos = geometry();
    }

    // -------------------------------------------
    // maximize if it was maximized
    // -------------------------------------------
    if (dlgSettings.IsMaximized())
    {
       setWindowState(Qt::WindowMaximized);
    }
}

void MainWindow::ConnectionInit()
{
    VlcLog.SetLogLevel(dlgSettings.GetLogLevel());

    pChannelDlg->getModel()->clear();
    KartinaTv.abort();

    // update connection data ...
    KartinaTv.SetData(dlgSettings.GetAPIServer(), dlgSettings.GetUser(), dlgSettings.GetPasswd(),
                         dlgSettings.GetErosPasswd(), dlgSettings.AllowEros());

    // set proxy ...
    if (dlgSettings.UseProxy())
    {
       KartinaTv.setProxy(dlgSettings.GetProxyHost(), dlgSettings.GetProxyPort(),
                          dlgSettings.GetProxyUser(), dlgSettings.GetProxyPasswd());

       dwnLogos.setProxy(dlgSettings.GetProxyHost(), dlgSettings.GetProxyPort(),
                         dlgSettings.GetProxyUser(), dlgSettings.GetProxyPasswd());

       dwnVodPics.setProxy(dlgSettings.GetProxyHost(), dlgSettings.GetProxyPort(),
                           dlgSettings.GetProxyUser(), dlgSettings.GetProxyPasswd());

       streamLoader.setProxy(dlgSettings.GetProxyHost(), dlgSettings.GetProxyPort(),
                             dlgSettings.GetProxyUser(), dlgSettings.GetProxyPasswd());
    }

    // set language as read ...
    pTranslator->load(QString("lang_%1").arg(dlgSettings.GetLanguage ()),
                      pFolders->getLangDir());

#ifdef INCLUDE_LIBVLC
    // do we use libVLC ?
    if (dlgSettings.GetPlayerModule().contains("libvlc", Qt::CaseInsensitive))
    {
       vlcCtrl.UseLibVlc(true);
    }
    else
    {
       vlcCtrl.UseLibVlc(false);
    }
#endif /* INCLUDE_LIBVLC */

    // give vlcCtrl needed infos ...
    vlcCtrl.LoadPlayerModule(dlgSettings.GetPlayerModule());
    vlcCtrl.SetTranslitSettings(dlgSettings.TranslitRecFile());

    TouchPlayCtrlBtns(false);

    // authenticate ...
    Trigger.TriggerRequest(Kartina::REQ_COOKIE);

    // set refresh timer ...
    if (dlgSettings.DoRefresh())
    {
       if (!Refresh.isActive())
       {
          Refresh.start(dlgSettings.GetRefrInt() * 60000); // 1 minutes: (60 * 1000 msec) ...
       }
    }
    else
    {
       if (Refresh.isActive())
       {
          Refresh.stop();
       }
    }
}

bool MainWindow::WantToClose()
{
   QString sText = HTML_SITE;
   sText.replace(TMPL_CONT, tr("VLC is still running.<br />"
                               "<b>Closing VLC record will also close the started VLC-Player.</b>"
                               "<br /> <br />"
                               "Do you really want to close VLC Record now?"));

   if (QMessageBox::question(this, tr("Question"), sText,
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
   {
      return true;
   }
   else
   {
      return false;
   }
}

void MainWindow::FillChanMap(const QVector<cparser::SChan> &chanlist)
{
   QVector<cparser::SChan>::const_iterator cit;

   pChannelDlg->getChanMap()->clear();

   // create channel map ...
   for (cit = chanlist.constBegin(); cit != chanlist.constEnd(); cit++)
   {
      if (!(*cit).bIsGroup)
      {
         pChannelDlg->getChanMap()->insert((*cit).iId, *cit);
      }
   }
}

int MainWindow::FillChannelList (const QVector<cparser::SChan> &chanlist)
{
   QString  sLine;
   QString  sLogoFile;
   QStandardItem *pItem;
   int      iRow, iRowGroup;
   QPixmap  Pix(16, 16);
   QPixmap  icon;
   int      iChanCount = 0;
   int      iChanGroupCount = 0;

   int iRecentRow = 0;
   QString sRecentChan = "";
   QString sChan = "";

   // first time and after saving of settings set the current channel-row
   // to the first channel-row from the recent channels list if it exists.
   QSettings settings;
   QStringList chans = settings.value("RecentChanList").toStringList();

   if (chans.size() > 0)
   {
       sRecentChan = chans[0];
       sRecentChan = sRecentChan.trimmed();
   }

   // get current item ...
   iRow      = pChannelDlg->getChannelList()->currentIndex().row();
   iRow      = (iRow <= 0) ? 1 : iRow;
   iRowGroup = pChannelDlg->getCbxChannelGroup()->currentIndex();
   iRowGroup = (iRowGroup < 0) ? 0 : iRowGroup;

   // clear channel list ...
   pChannelDlg->getModel()->clear();

   // clear channel group list ...
   pChannelDlg->getCbxChannelGroup()->clear();

   for (int i = 0; i < chanlist.size(); i++)
   {
       // create new item ...
       pItem = new QStandardItem;

      // is this a channel group ... ?
      if (chanlist[i].bIsGroup)
      {
          pItem->setData(-1, channellist::cidRole);
          pItem->setData(chanlist[i].sName, channellist::nameRole);
          pItem->setData(chanlist[i].sProgramm, channellist::bgcolorRole);
          pItem->setData(QIcon(":png/group"), channellist::iconRole);

         // add channel group entry ...
         Pix.fill(QColor(chanlist[i].sProgramm));
         pChannelDlg->getCbxChannelGroup()->addItem(QIcon(Pix), chanlist[i].sName, QVariant(i));

         //only one time create channel group menu
         if (bFirstInit)
         {
            ChanGroup[iChanGroupCount] = new QMenu(chanlist[i].sName, this);
            ui->menuChannels->addMenu(ChanGroup[iChanGroupCount]);
            iChanGroupCount++;
         }
      }
      else
      {
         sLogoFile = QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(chanlist[i].iId);
         sLine     = QString("%1. %2").arg(++ iChanCount).arg(chanlist[i].sName);

         // check if file exists ...
         if (!QFile::exists(sLogoFile))
         {
             // no --> load default image ...
             icon.load(":png/no_logo");
         }
         else
         {
             //////////////////////////////////////////////////////////
             // Note: Some Icons can't be loaded.
             // First we try to load the image.
             // If this isn't possible we force
             // the file format to gif.
             // If it's still impossible to load
             // the image we take the default image instead.
             //////////////////////////////////////////////////////////

             // check if file can be loaded ...
             if (!icon.load(sLogoFile, "image/gif"))
             {
                // still can't load --> load default image ...
                icon.load(":png/no_logo");
//                mInfo(tr("Can't load channel image \"%1.gif\" ...").arg(chanlist[i].iId));
                ui->statusBar->showMessage(tr("Can't load channel image \"%1.gif\" ...").
                                           arg(chanlist[i].iId));
             }
         }

         pItem->setData(chanlist[i].iId, channellist::cidRole);
         pItem->setData(sLine, channellist::nameRole);
         pItem->setData(QIcon(icon), channellist::iconRole);

         if(dlgSettings.extChanList())
         {
            pItem->setData(chanlist[i].sProgramm, channellist::progRole);
            pItem->setData(chanlist[i].uiStart, channellist::startRole);
            pItem->setData(chanlist[i].uiEnd, channellist::endRole);
         }
         else
         {
            pItem->setToolTip(createTooltip(chanlist[i].sName, chanlist[i].sProgramm,
                                            chanlist[i].uiStart, chanlist[i].uiEnd));
         }

         //only one time create channel actions
         if (bFirstInit)
         {
            ChannelActs[i] = new QAction(this);
            ChannelActs[i]->setText(sLine);
            connect(ChannelActs[i], SIGNAL(triggered()), this, SLOT(slotSelectChannel()));
            ChanGroup[iChanGroupCount - 1]->addAction(ChannelActs[i]);
         }

         // set icons to the channel actions
         if (QFile::exists(sLogoFile))
         {
             ChannelActs[i]->setIcon(QIcon(sLogoFile));
         }
         else
         {
             // no --> load default image ...
             icon.load(":png/no_logo");
             ChannelActs[i]->setIcon(QIcon(icon));
         }

         sChan = chanlist[i].sName;
         sChan = sChan.trimmed();

         //find recent channel-row
         if (sRecentChan.contains(sChan, Qt::CaseInsensitive))
         {
             iRecentRow = i;
         }
      }

      pChannelDlg->getModel()->appendRow(pItem);
   }

   // first time and after saving of settings set the current channel-row
   // to the first channel-row from the recent channels list if it exists
   // and update the recent channel list actions
   if (chans.size() > 0 && iRecentRow > 0)
   {
      updateRecentChanActions(); //make the recent channel-list with icons visible
      iRow = iRecentRow; //set the current channel-row to the recent channel-row
   }

   pChannelDlg->getCbxChannelGroup()->setCurrentIndex(iRowGroup);
   pChannelDlg->getChannelList()->setCurrentIndex(pChannelDlg->getModel()->index(iRow, 0)); //go to slotEPG
   pChannelDlg->getChannelList()->scrollTo(pChannelDlg->getModel()->index(iRow, 0));

   return 0;
}

int MainWindow::StartVlcRec (const QString &sURL, const QString &sChannel)
{
   int         iRV      = -1;
   Q_PID       vlcpid   = 0;
   QDateTime   now      = QDateTime::currentDateTime();
   QString     sExt     = "ts", fileName;
   QString     sCmdLine;

   // should we ask for file name ... ?
   if (dlgSettings.AskForRecFile())
   {
      // yes! Create file save dialog ...
      QString   sFilter;
      QString   sTarget  = QString("%1/%2(%3)").arg(dlgSettings.GetTargetDir())
                          .arg(sChannel).arg(now.toString("yyyy-MM-dd__hh-mm"));

      fileName = QFileDialog::getSaveFileName(this, tr("Save Stream as"),
                 sTarget, QString("MPEG 4 Container (*.mp4);;Transport Stream (*.ts);;AVI File (*.avi)"),
                 &sFilter);

      if (fileName != "")
      {
         // which filter was used ... ?
         if (sFilter == "Transport Stream (*.ts)")
         {
            sExt = "ts";
         }
         else if (sFilter ==  "AVI File (*.avi)")
         {
            sExt = "avi";
         }
         else if (sFilter ==  "MPEG 4 Container (*.mp4)")
         {
            sExt = "mp4";
         }

         QFileInfo info(fileName);

         // re-create complete file name ...
         fileName = QString ("%1/%2").arg(info.path())
                    .arg(info.completeBaseName());
      }
   }
   else
   {
      // create filename as we think it's good ...
      fileName = QString("%1/%2(%3)").arg(dlgSettings.GetTargetDir())
                 .arg(sChannel).arg(now.toString("yyyy-MM-dd__hh-mm"));
   }

   if (fileName != "")
   {
      if (showInfo.showType() == ShowInfo::Live)
      {
         // normal stream using HTTP ...
          sCmdLine = vlcCtrl.CreateClArgs(vlcctrl::VLC_REC_LIVE,
                                         dlgSettings.GetVLCPath(),
                                         sURL, dlgSettings.GetBufferTime(),
                                         fileName, sExt);
      }
      else
      {
         // archiv using HTTP ...
         sCmdLine = vlcCtrl.CreateClArgs(vlcctrl::VLC_REC_ARCH,
                                         dlgSettings.GetVLCPath(),
                                         sURL, dlgSettings.GetBufferTime(),
                                         fileName, sExt);
      }

      // start player if we have a command line ...
      if (sCmdLine != "")
      {
         ui->textEpgShort->setHtml(showInfo.htmlDescr());
         vlcpid = vlcCtrl.start(sCmdLine, -1, dlgSettings.DetachPlayer(), ePlayState);
      }

      // successfully started ?
      if (!vlcpid)
      {
         iRV = -1;
//         QMessageBox::critical(this, tr("Error!"), tr("Can't start VLC-Media Player!"));
         ui->statusBar->showMessage(tr("Error! Can't start VLC-Media Player!"));
         ePlayState = IncPlay::PS_ERROR;
         TouchPlayCtrlBtns();
      }
      else
      {
         iRV = 0;
         mInfo(tr("Started VLC with pid #%1!").arg((uint)vlcpid));
      }
   }

   return iRV;
}

int MainWindow::StartVlcPlay (const QString &sURL)
{
   int         iRV      = 0;
   Q_PID       vlcpid   = 0;
   QString     sCmdLine;

   if (showInfo.showType() == ShowInfo::Live)
   {
      // normal stream using HTTP ...
      sCmdLine = vlcCtrl.CreateClArgs(vlcctrl::VLC_PLAY_LIVE,
                                      dlgSettings.GetVLCPath(), sURL,
                                      dlgSettings.GetBufferTime());
   }
   else
   {
      // archiv using HTTP ...
      sCmdLine = vlcCtrl.CreateClArgs(vlcctrl::VLC_PLAY_ARCH,
                                      dlgSettings.GetVLCPath(), sURL,
                                      dlgSettings.GetBufferTime());
   }

   // start player if we have a command line ...
   if (sCmdLine != "")
   {
      ui->textEpgShort->setHtml(showInfo.htmlDescr());
      vlcpid = vlcCtrl.start(sCmdLine, -1, dlgSettings.DetachPlayer(), ePlayState);
   }

   // successfully started ?
   if (!vlcpid)
   {
      iRV = -1;
//      QMessageBox::critical(this, tr("Error!"), tr("Can't start VLC-Media Player!"));
      ui->statusBar->showMessage(tr("Error! Can't start VLC-Media Player!"));
      ePlayState = IncPlay::PS_ERROR;
      TouchPlayCtrlBtns();
   }
   else
   {
      mInfo(tr("Started VLC with pid #%1!").arg((uint)vlcpid));
   }
   return iRV;
}

void MainWindow::StartStreamDownload (const QString &sURL, const QString &sName, const QString &sFileExt)
{
   QString   sExt = sFileExt, fileName;
   QDateTime now  = QDateTime::currentDateTime();

   // should we ask for file name ... ?
   if (dlgSettings.AskForRecFile())
   {
      // yes! Create file save dialog ...
      QString   sFilter;
      QString   sTarget  = QString("%1/%2(%3)").arg(dlgSettings.GetTargetDir())
                          .arg(sName).arg(now.toString("yyyy-MM-dd__hh-mm"));

      fileName = QFileDialog::getSaveFileName(this, tr("Save Stream as"),
                 sTarget, QString("Transport Stream (*.ts);;MPEG 4 Video (*.m4v)"),
                 &sFilter);

      if (fileName != "")
      {
         QFileInfo info(fileName);

         // re-create complete file name ...
         fileName = QString ("%1/%2").arg(info.path())
                    .arg(info.completeBaseName());
      }
   }
   else
   {
      // create filename as we think it's good ...
      fileName = QString("%1/%2(%3)").arg(dlgSettings.GetTargetDir())
                 .arg(sName).arg(now.toString("yyyy-MM-dd__hh-mm"));
   }

   if (fileName != "")
   {
      ui->textEpgShort->setHtml(showInfo.htmlDescr());
      streamLoader.downloadStream (sURL, QString("%1.%2").arg(fileName).arg(sExt),
                                   dlgSettings.GetBufferTime ());
   }
}

void MainWindow::TouchPlayCtrlBtns (bool bEnable)
{
#ifdef INCLUDE_LIBVLC
   if (vlcCtrl.withLibVLC())
   {
      if ((showInfo.playState() == IncPlay::PS_PLAY)
         && showInfo.canCtrlStream() && bEnable)
      {
         ui->pushBwd->setEnabled(true);
         ui->pushFwd->setEnabled(true);
         ui->actionJumpBackward->setEnabled(true);
         ui->actionJumpForward->setEnabled(true);
         ui->cbxTimeJumpVal->setEnabled(true);
         ui->pushPlay->setIcon(QIcon(":/app/pause"));
         ui->actionPlay->setIcon(QIcon(":/app/pause"));
         ui->actionPlay->setText(tr("Pause"));
      }
      else
      {
         ui->pushBwd->setEnabled(false);
         ui->pushFwd->setEnabled(false);
         ui->actionJumpBackward->setEnabled(false);
         ui->actionJumpForward->setEnabled(false);
         ui->cbxTimeJumpVal->setEnabled(false);

         ui->pushPlay->setIcon(QIcon(":/app/play"));
         ui->actionPlay->setIcon(QIcon(":/app/play"));
         ui->actionPlay->setText(tr("Play selected Channel"));
      }
   }
   else
   {
      ui->pushBwd->setEnabled(false);
      ui->pushFwd->setEnabled(false);
      ui->actionJumpBackward->setEnabled(false);
      ui->actionJumpForward->setEnabled(false);
      ui->cbxTimeJumpVal->setEnabled(false);
   }
#endif /* INCLUDE_LIBVLC */

   switch (ePlayState)
   {
   case IncPlay::PS_PLAY:

      ui->pushPlay->setEnabled(bEnable);
      ui->actionPlay->setEnabled(bEnable);
      ui->pushRecord->setEnabled(bEnable);
      ui->actionRecord->setEnabled(bEnable);
      ui->pushStop->setEnabled(bEnable);
      ui->actionStop->setEnabled(bEnable);
      ui->pushLive->setEnabled(bEnable);
      ui->actionShow_Live->setEnabled(bEnable);
      break;

   case IncPlay::PS_RECORD:
      ui->pushPlay->setEnabled(false);
      ui->actionPlay->setEnabled(false);
      ui->pushRecord->setEnabled(false);
      ui->actionRecord->setEnabled(false);
      ui->pushStop->setEnabled(bEnable);
      ui->actionStop->setEnabled(bEnable);
      ui->pushLive->setEnabled(false);
      ui->actionShow_Live->setEnabled(false);
      break;

   case IncPlay::PS_TIMER_RECORD:
   case IncPlay::PS_TIMER_STBY:
      ui->pushPlay->setEnabled(false);
      ui->actionPlay->setEnabled(false);
      ui->pushRecord->setEnabled(false);
      ui->actionRecord->setEnabled(false);
      ui->pushStop->setEnabled(bEnable);
      ui->actionStop->setEnabled(bEnable);
      ui->pushLive->setEnabled(false);
      ui->actionShow_Live->setEnabled(false);
      break;

   default:
      ui->pushPlay->setEnabled(bEnable);
      ui->actionPlay->setEnabled(bEnable);
      ui->pushRecord->setEnabled(bEnable);
      ui->actionRecord->setEnabled(bEnable);
      ui->pushStop->setEnabled(false);
      ui->actionStop->setEnabled(false);
      ui->pushLive->setEnabled(bEnable);
      ui->actionShow_Live->setEnabled(bEnable);
      break;
   }

   emit sigLCDStateChange((int)ePlayState);
}

QString MainWindow::CleanShowName(const QString &str)
{
   QString sName = str;

   // remove html code for ' " ' ...
   sName.replace(QString("&quot;"), QString(" "));

   // remove Windows forbidden characters
   // <>:?*|"\/ and in addition '-.
   sName.replace(QRegExp("[<>:?*/|\\\\\"'.,-]"), " ");

   // remove mutliple spaces ...
   sName = sName.simplified();

   // find space at good position ...
   int iIdx = sName.indexOf(QChar(' '), MAX_NAME_LEN - 1);

   if (iIdx <= (MAX_NAME_LEN + 5))
   {
      // found space at needed position ...
      sName = sName.left(iIdx);
   }
   else
   {
      iIdx = sName.indexOf(QChar(' '));

      if ((iIdx > 1) && (iIdx <= (MAX_NAME_LEN + 5)))
      {
         // find cut position shorter than name length ...
         sName = sName.left(iIdx);
      }
      else
      {
         // can't find good cut position ...
         sName = sName.left(MAX_NAME_LEN);
      }
   }

   return sName;
}

bool MainWindow::WantToStopRec()
{
   QString sText = HTML_SITE;
   sText.replace(TMPL_CONT, tr("Pending Record!"
                               "<br /> <br />"
                               "Do you really want to stop recording now?"));

   if (QMessageBox::question(this, tr("Question"), sText,
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
   {
      return true;
   }
   else
   {
      return false;
   }
}

int MainWindow::AllowAction (IncPlay::ePlayStates newState)
{
   int iRV = 0;

   switch (ePlayState)
   {
   case IncPlay::PS_RECORD:
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      {
         // pending record ...
         switch (newState)
         {
            // requested action stop or new record ...
         case IncPlay::PS_STOP:
         case IncPlay::PS_RECORD:
            // ask for permission ...
            if (WantToStopRec ())
            {
               // permission granted ...
               iRV        = 1;

               // set new state ...
               ePlayState = newState;
            }
            break;
         default:
            // all other actions permitted ...
            break;
         }
      }
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      break;

   case IncPlay::PS_TIMER_RECORD:
   case IncPlay::PS_TIMER_STBY:
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      if (newState == IncPlay::PS_STOP)
      {
         // ask for permission ...
         if (WantToStopRec ())
         {
            // permission granted ...
            iRV        = 1;

            // set new state ...
            ePlayState = newState;
         }
      }
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      break;

   default:
      // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // don't ask for every fart.
      // Let the user decide what he wants to do!
      iRV        = 1;

      // set new state ...
      ePlayState = newState;
      // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      break;
   }

   return iRV;
}

bool MainWindow::TimeJumpAllowed()
{
   bool bRV = true;

   switch (ePlayState)
   {
   // make sure that any kind of record
   // works without time jump ...
   case IncPlay::PS_RECORD:
   case IncPlay::PS_TIMER_RECORD:
   case IncPlay::PS_TIMER_STBY:
      bRV = false;
   default:
      break;
   }

   return bRV;
}

void MainWindow::CreateSystray()
{
   trayIcon.setIcon(QIcon(":/app/tv"));
   trayIcon.setToolTip(tr("KTV-Recorder - Click to activate!"));
}

void MainWindow::setRecentChannel(const QString &ChanName)
 {
     QSettings settings;
     QStringList chans = settings.value("RecentChanList").toStringList();

     chans.removeAll(ChanName);
     chans.prepend(ChanName);

     while (chans.size() > MAX_RECENT_CHANNELS)
     {
         chans.removeLast();
     }

     settings.setValue("RecentChanList", chans);
     updateRecentChanActions();
 }

 void MainWindow::updateRecentChanActions()
 {
     QString  sLogoFile;
     QSettings settings;
     QStringList chans = settings.value("RecentChanList").toStringList();

     int numRecentChans = qMin(chans.size(), (int)MAX_RECENT_CHANNELS);

     for (int i = 0; i < numRecentChans; ++i)
     {
         sLogoFile = QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(getChanId(chans[i]));

         if (QFile::exists(sLogoFile))
         {
             RecentChansActs[i]->setIcon(QIcon(sLogoFile));
         }

         RecentChansActs[i]->setText(chans[i]);
         RecentChansActs[i]->setVisible(true);
     }

     for (int j = numRecentChans; j < MAX_RECENT_CHANNELS; ++j)
     {
         RecentChansActs[j]->setVisible(false);
     }
 }

 int MainWindow::getChanId(const QString &chanName)
 {
     int iId = -1;
     QModelIndex idx;
     QString str = "";
     QString str1 = chanName;

     str1 = str1.trimmed();

     for (int i = 0; i < pChannelDlg->getModel()->rowCount(); i++)
     {
         idx = pChannelDlg->getModel()->index(i, 0);
         str = qvariant_cast<QString>(idx.data(channellist::nameRole));
         str = str.trimmed();

         if (str.contains(str1, Qt::CaseInsensitive))
         {
             return qvariant_cast<int>(idx.data(channellist::cidRole));
         }
     }
     return iId;
 }

 QString MainWindow::createTooltip (const QString & name, const QString & prog, uint start, uint end)
 {
     // create tool tip with programm info ...
     QString sToolTip = PROG_INFO_TOOL_TIP;
     sToolTip.replace(TMPL_PROG, tr("Program:"));
     sToolTip.replace(TMPL_START, tr("Start:"));
     sToolTip.replace(TMPL_END, tr("End:"));
     sToolTip.replace(TMPL_TIME, tr("Length:"));

     sToolTip = sToolTip.arg(name).arg(prog)
                 .arg(QDateTime::fromTime_t(start).toString(DEF_TIME_FORMAT))
                 .arg(end ? QDateTime::fromTime_t(end).toString(DEF_TIME_FORMAT) : "")
                 .arg(end ? tr("%1 min.").arg((end - start) / 60)                : "");

     return sToolTip;
 }

 void MainWindow::InitShortCuts()
 {
     CShortcutEx *pShortCut;
     QVector<Ui::SShortCuts>::const_iterator  cit;

     // go through table and create a shortcut for every entry ...
     for (cit = vShortCutTab.constBegin(); cit != vShortCutTab.constEnd(); cit ++)
     {
        dlgSettings.addShortCut((*cit).sDescr, (*cit).pObj->objectName(), (*cit).pSlot, (*cit).sShortCut);

        pShortCut = new CShortcutEx (QKeySequence(dlgSettings.shortCut((*cit).pObj->objectName(), (*cit).pSlot)), this);

        if (pShortCut)
        {
           connect (pShortCut, SIGNAL(activated()), (*cit).pObj, (*cit).pSlot);

           // save shortcut ...
           vShortcutPool.push_back(pShortCut);
        }
     }
 }

 void MainWindow::fillShortCutTab()
 {
     vShortCutTab.clear();
     Ui::SShortCuts *pShort;
     Ui::SShortCuts tmpTab[] = {
        // ShortCut Source Table:
        // All shortcuts used in this program will be generated from this
        // table. If you need further shortcuts, add them here.
        // Note: Make sure every object has an object name!
        // ---------------------------------------------------------------
        //  Description
        //  |                        Object Pointer (which owns slot function)
        //  |                        |             Slot function
        //  |                        |             |                                       default Shortcut
        //  |                        |             |                                       |
        //  V                        V             V                                       V
         {tr("Play / Pause"),         this,         SLOT(on_actionPlay_triggered()),        "ALT+P"},
         {tr("Stop"),                 this,         SLOT(on_actionStop_triggered()),        "ALT+S"},
         {tr("Record"),               this,         SLOT(on_actionRecord_triggered()),      "ALT+R"},
         {tr("Timer Record"),         this,         SLOT(on_actionTime_Record_triggered()), "ALT+T"},
         {tr("Settings"),             this,         SLOT(on_actionSettings_triggered()),    "ALT+O"},
         {tr("About"),                this,         SLOT(on_actionAbout_triggered()),       "ALT+I"},
         {tr("Channels, EPG/VOD"),    this,         SLOT(on_actionChannelsEPG_triggered()), "ALT+K"},
         {tr("Search EPG"),           pChannelDlg,  SLOT(on_btnSearch_clicked()),           "CTRL+F"},
         {tr("Text Size +"),          pChannelDlg,  SLOT(on_btnFontLarger_clicked()),       "ALT++"},
         {tr("Text Size -"),          pChannelDlg,  SLOT(on_btnFontSmaller_clicked()),      "ALT+-"},
         {tr("Quit"),                 this,         SLOT(on_actionExit_triggered()),        "ALT+Q"},
         {tr("Toggle Aspect Ratio"),  ui->player,   SLOT(slotToggleAspectRatio()),          "ALT+A"},
         {tr("Toggle Crop Geometry"), ui->player,   SLOT(slotToggleCropGeometry()),         "ALT+C"},
         {tr("Toggle Fullscreen"),    ui->player,   SLOT(slotToggleFullScreen()),           "ALT+F"},
         {tr("Volume +"),             ui->player,   SLOT(slotMoreLoudly()),                 "+"},
         {tr("Volume -"),             ui->player,   SLOT(slotMoreQuietly()),                "-"},
         {tr("Toggle Mute"),          ui->player,   SLOT(slotMute()),                       "M"},
         {tr("Jump Forward"),         this,         SLOT(on_pushFwd_clicked()),             "CTRL+ALT+F"},
         {tr("Jump Backward"),        this,         SLOT(on_pushBwd_clicked()),             "CTRL+ALT+B"},
         {tr("Next Channel"),         this,         SLOT(slotChannelDown()),                "CTRL+N"},
         {tr("Previous Channel"),     this,         SLOT(slotChannelUp()),                  "CTRL+P"},
         {tr("Show EPG / VOD"),       this,         SLOT(slotToggleEpgVod()),               "CTRL+E"},
        // add further entries below ...

        // last entry, don't touch ...
        {"",                         NULL,       NULL,                              ""}
     };

     pShort = tmpTab;

     while (pShort->pObj != NULL)
     {
        vShortCutTab.push_back(*pShort);
        pShort ++;
     }
 }

 void MainWindow::retranslateShortcutTable()
 {
     // re-translate shortcut table if there's something to
     // translate ...
     if (dlgSettings.shortCutCount() > 0)
     {
         QVector<Ui::SShortCuts>::const_iterator cit;

         // update shortcut vector to include
         // new translation ...
         fillShortCutTab();

         // go through table and update shortcut description ...
         for (cit = vShortCutTab.constBegin(); cit != vShortCutTab.constEnd(); cit ++)
         {
            dlgSettings.updateShortcutDescr((*cit).sDescr, (*cit).pObj->objectName(), (*cit).pSlot);
         }

         // add favourites ...
         pChannelDlg->updateFavourites();
     }
 }

 void MainWindow::ClearShortCuts()
 {
    QVector<CShortcutEx *>::iterator it;

    // free all shortcuts ...
    for (it = vShortcutPool.begin(); it != vShortcutPool.end(); it++)
    {
       delete *it;
    }

    vShortcutPool.clear();
 }

/************************* History ***************************\
| $Log$
\*************************************************************/


