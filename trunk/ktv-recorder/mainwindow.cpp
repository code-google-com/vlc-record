#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qchanlistdelegate.h"
#include "small_helpers.h"

// for logging ...
extern CLogFile VlcLog;

// for folders ...
extern CDirStuff *pFolders;

// global showinfo class ...
extern CShowInfo showInfo;

// global rec db ...
extern CVlcRecDB *pDb;

MainWindow::MainWindow(QTranslator *trans, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   ui->setupUi(this);
   QString sHlp;

#ifdef INCLUDE_LIBVLC
   // build layout stack ...
   pVideoWidget  =  NULL;
   stackedLayout = new QStackedLayout();
   stackedLayout->setMargin(0);
   ui->vMainLayout->removeWidget(ui->masterFrame);
   stackedLayout->addWidget(ui->masterFrame);
   ui->vMainLayout->addLayout(stackedLayout);
#endif

   ePlayState     = IncPlay::PS_WTF;
   pTranslator    = trans;
   iDwnReqId      = -1;
   ulStartFlags   =  0;
   bDoInitDlg     = true;
   bOnTop         = false;
   flags          = this->windowFlags();
   bFirstInit     = true;
   bFirstConnect  = true;
   bVODLogosReady = false;
   bGotVOD        = false; // update vod stuff only at startup ...
   bEpgRefresh    = false;

   // init VOD site backup ...
   lastVodSite.iScrollBarVal = 0;

   // init account info ...
   accountInfo.bHasArchive = false;
   accountInfo.bHasVOD     = false; // update vod stuff only at startup ...
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
   dlgParentalControl.setParent(this, Qt::Dialog);
   dlgParentalControl.setAccountInfo(&accountInfo);
   secCodeDlg.setParent(this, Qt::Dialog);
   dlgParentalControl.setXmlParser(&XMLParser);
   dlgParentalControl.setWaitTrigger(&Trigger);
   vlcCtrl.setParent(this);
   trayIcon.setParent(this);

   // non-modal ...
   Help.setParent(NULL);

   pChannelDlg = new CChannelsEPGdlg(this);
   pChannelDlg->initDialog(true);
   pChannelDlg->setTrigger(&Trigger); // set the pointer to Trigger by CChannelsEPGdlg
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

   QString sAspect[] = {"std.","1:1","4:3","16:9","16:10","2.21:1","5:4"};
   QString sCrop[] = {"std.","1:1","4:3","16:9","16:10","1.85:1","2.21:1","2.35:1","2.39:1","5:4"};

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

   VlcLog.SetLogFile(pFolders->getDataDir(), APP_LOG_FILE);

   // update checker ...
   pUpdateChecker = new QNetworkAccessManager(this);

   // set host for pix cache ...
   pixCache.setHost(dlgSettings.GetAPIServer());

   // set settings for vod browser ...
   pChannelDlg->getVodBrowser()->setSettings(&dlgSettings);

   // set pix cache ...
   pChannelDlg->getVodBrowser()->setPixCache(&pixCache);

   // set log level ...
   VlcLog.SetLogLevel(dlgSettings.GetLogLevel());

   // log folder locations ...
    mInfo (tr("\ndataDir: %1\n").arg(pFolders->getDataDir())
           + tr("logoDir: %1\n").arg(pFolders->getLogoDir())
          + tr("langDir: %1\n").arg(pFolders->getLangDir())
          + tr("modDir:  %1\n").arg(pFolders->getModDir())
          + tr("appDir:  %1").arg(pFolders->getAppDir()));

    // set help file ...
    // be sure the file we want to load exists ... fallback to english help ...
    sHlp = QString("%1/help_%2.qhc").arg(pFolders->getDocDir()).arg(dlgSettings.GetLanguage());

    if (!QFile::exists(sHlp))
    {
        sHlp = QString("%1/help_en.qhc").arg(pFolders->getDocDir());
    }

    Help.setHelpFile(sHlp);

   // configure trigger and start it ...
   Trigger.SetKartinaClient(&KartinaTv);
   Trigger.start();

   // give timerRec all needed infos ...
   dlgTimeRec.SetXmlParser(&XMLParser);
   dlgTimeRec.SetKartinaTrigger(&Trigger);
   dlgTimeRec.SetSettings(&dlgSettings);
   dlgTimeRec.SetVlcCtrl(&vlcCtrl);
   dlgTimeRec.SetStreamLoader(&streamLoader);

   // hide / remove VOD tab widget ...
   vodTabWidget.iPos    = 1; // index of VOD tab
   vodTabWidget.icon = pChannelDlg->getTabEpgVOD()->tabIcon(vodTabWidget.iPos);
   vodTabWidget.sText   = pChannelDlg->getTabEpgVOD()->tabText(vodTabWidget.iPos);
   vodTabWidget.pWidget = pChannelDlg->getTabEpgVOD()->widget(vodTabWidget.iPos);
   pChannelDlg->getTabEpgVOD()->removeTab(vodTabWidget.iPos);

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

   // give player the list of shortcuts ...
   ui->player->setShortCuts (&vShortcutPool);

   // give player settings and wait trigger access ...
   ui->player->setSettings(&dlgSettings);
   ui->player->setTrigger(&Trigger);

   // connect vlc control with libvlc player ...
   connect (ui->player, SIGNAL(sigPlayState(int)), &vlcCtrl, SLOT(slotLibVlcStateChange(int)));
   connect (&vlcCtrl, SIGNAL(sigLibVlcPlayMedia(QString)), ui->player, SLOT(playMedia(QString)));
   connect (&vlcCtrl, SIGNAL(sigLibVlcStop()), ui->player, SLOT(stop()));

   // short info update on archive play ...
   connect (ui->player, SIGNAL(sigCheckArchProg(ulong)), this, SLOT(slotCheckArchProg(ulong)));
   connect (this, SIGNAL(sigShowInfoUpdated()), ui->player, SLOT(slotShowInfoUpdated()));
   connect (ui->player, SIGNAL(sigToggleFullscreen()), this, SLOT(slotToggleFullscreen()));
   connect (this, SIGNAL(sigFullScreenToggled(int)), ui->player, SLOT(slotFsToggled(int)));

   // aspect ratio, crop and full screen ...
   connect (this, SIGNAL(sigToggleFullscreen()), ui->player, SLOT(on_btnFullScreen_clicked()));
   connect (this, SIGNAL(sigToggleAspectRatio()), ui->player, SLOT(slotToggleAspectRatio()));
   connect (this, SIGNAL(sigToggleCropGeometry()), ui->player, SLOT(slotToggleCropGeometry()));

   // get state if libVLC player to change player state display ...
   connect (ui->player, SIGNAL(sigPlayState(int)), this, SLOT(slotIncPlayState(int)));
#endif /* INCLUDE_LIBVLC */

   // connect signals and slots ...
   connect (&pixCache,      SIGNAL(allDone()), this, SLOT(slotRefreshChanLogos()));
   connect (&KartinaTv,     SIGNAL(sigHttpResponse(QString,int)), this, SLOT(slotKartinaResponse(QString,int)));
   connect (&KartinaTv,     SIGNAL(sigError(QString,int,int)), this, SLOT(slotKartinaErr(QString,int,int)));
   connect (&streamLoader,  SIGNAL(sigStreamDownload(int,QString)), this, SLOT(slotDownloadStarted(int,QString)));
   connect (&Refresh,       SIGNAL(timeout()), this, SLOT(slotEpgRefresh()));
   connect (this,           SIGNAL(sigEpgRefresh()), &Trigger, SLOT(slotReqChanList()));
   connect (&tEpgRefresh,   SIGNAL(timeout()), &Trigger, SLOT(slotReqChanList()));
   connect (pChannelDlg->getEpgBrowser(),   SIGNAL(anchorClicked(QUrl)), this, SLOT(slotEpgAnchor(QUrl)));
   connect (&dlgSettings,   SIGNAL(sigReloadLogos()), this, SLOT(slotReloadLogos()));
   connect (&dlgSettings,   SIGNAL(sigSetServer(QString)), this, SLOT(slotSetSServer(QString)));
   connect (&dlgSettings,   SIGNAL(sigSetBitRate(int)), this, SLOT(slotSetBitrate(int)));
   connect (&dlgSettings,   SIGNAL(sigSetTimeShift(int)), this, SLOT(slotSetTimeShift(int)));
   connect (&dlgTimeRec,    SIGNAL(sigRecDone()), this, SLOT(slotTimerRecordDone()));
   connect (&dlgTimeRec,    SIGNAL(sigRecActive(int)), this, SLOT(slotTimerRecActive(int)));
   connect (&vlcCtrl,       SIGNAL(sigVlcStarts(int)), this, SLOT(slotVlcStarts(int)));
   connect (&vlcCtrl,       SIGNAL(sigVlcEnds(int)), this, SLOT(slotVlcEnds(int)));
   connect (&dlgTimeRec,    SIGNAL(sigShutdown()), this, SLOT(slotShutdown()));
   connect (this,           SIGNAL(sigLCDStateChange(int)), ui->labState, SLOT(updateState(int)));
   connect (pChannelDlg->getVodBrowser(), SIGNAL(anchorClicked(QUrl)), this, SLOT(slotVodAnchor(QUrl)));
   connect (pUpdateChecker, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotUpdateAnswer (QNetworkReply*)));

   if (dlgSettings.HideToSystray() && QSystemTrayIcon::isSystemTrayAvailable())
   {
       connect (this,          SIGNAL(sigHide()), &trayIcon, SLOT(show()));
       connect (this,          SIGNAL(sigShow()), &trayIcon, SLOT(hide()));
   }

   connect (pChannelDlg,         SIGNAL(sigDoubliClickOnListWidget()), this, SLOT(slotDoubleClick()));
   connect (pChannelDlg,         SIGNAL(sigChannelDlgClosed()), this, SLOT(slotChannelDlgClosed()));
   connect (ui->player,          SIGNAL(sigAspectToggle(int)), this, SLOT(slotAspectToggle(int)));
   connect (ui->player,          SIGNAL(sigCropToggle(int)), this, SLOT(slotCropToggle(int)));
   connect (this,                SIGNAL(sigLockParentalManager()), &dlgParentalControl, SLOT(slotLockParentalManager()));

   // trigger read of saved timer records ...
   dlgTimeRec.ReadRecordList();

   // enable button ...
   TouchPlayCtrlBtns(false);

   // hide upper toolbar  and shortEPG window
   ui->player->getFrameTimerInfo()->hide();
   ui->textEpgShort->hide();

   // request authorisation ...
   ConnectionInit();
}

MainWindow::~MainWindow()
{
   delete ui;

   if (pUpdateChecker)
   {
       delete pUpdateChecker;
       pUpdateChecker = NULL;
   }

#ifdef INCLUDE_LIBVLC
   if (stackedLayout)
   {
      delete stackedLayout;
      stackedLayout = NULL;
   }
#endif // INCLUDE_LIBVLC

}

////////////////////////////////////////////////////////////////////////////////
//                           Events                                           //
////////////////////////////////////////////////////////////////////////////////

void MainWindow::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    // catch minimize event ...
    case QEvent::WindowStateChange:

       // printStateChange (((QWindowStateChangeEvent *)e)->oldState());
       if (isMinimized())
       {
          Help.close();

          // only hide window, if trayicon stuff is available ...
          if (QSystemTrayIcon::isSystemTrayAvailable () && dlgSettings.HideToSystray())
          {
             // hide dialog ...
             QTimer::singleShot(300, this, SLOT(hide()));
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

       // translate genre cbx ...
       pChannelDlg->touchGenreCbx();

       // translate error strings ...
       KartinaTv.fillErrorMap();
       break;
    default:
       QWidget::changeEvent(e);
       break;
   }
}

void MainWindow::showEvent(QShowEvent *event)
{
   emit sigShow();

   if (!(ulStartFlags & FLAG_CONN_CHAIN))
   {
       ulStartFlags |= FLAG_CONN_CHAIN;

       // start connection stuff in 0.5 seconds ...
//       QTimer::singleShot(500, this, SLOT(slotStartConnectionChain()));
   }

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
      // close help dialog ..
      Help.close();

      // We want to close program, store all needed values ...
      // Note: putting this function in destructor doesn't work!
      savePositions();

      if (pChannelDlg->isVisible())
      {
          pChannelDlg->close();
      }

      on_pushStop_clicked();

      // clear shortcuts ...
      ClearShortCuts ();

      // clean favourites ...
      pChannelDlg->getListFav()->clear();
      pChannelDlg->HandleFavourites();
      pChannelDlg->CleanContextMenu();

      // cancel any running kartina request ...
      Trigger.TriggerRequest (Kartina::REQ_ABORT);

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
        // close help dialog ..
        Help.close();

        // We want to close program, store all needed values ...
        // Note: putting this function in destructor doesn't work!
        savePositions();

        if (pChannelDlg->isVisible())
        {
            pChannelDlg->close();
        }

        on_pushStop_clicked();

        // clear shortcuts ...
        ClearShortCuts ();

        // clean favourites ...
        pChannelDlg->getListFav()->clear();
        pChannelDlg->HandleFavourites();
        pChannelDlg->CleanContextMenu();

       // cancel any running kartina request ...
       Trigger.TriggerRequest (Kartina::REQ_ABORT);

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

void MainWindow::on_actionParental_Control_triggered()
{
    // pause EPG reload ...
    if (Refresh.isActive())
    {
        Refresh.stop();
    }

    dlgParentalControl.exec();

    // lock parental manager ...
    emit sigLockParentalManager();

    // enable EPG reload again ...
    if (dlgSettings.DoRefresh() && !Refresh.isActive())
    {
       Refresh.start(dlgSettings.GetRefrInt() * 60000); // 1 minutes: (60 * 1000 msec) ...
    }
}

void MainWindow::on_actionAbout_triggered()
{
    CAboutDialog dlg(this, accountInfo.sExpires);
    dlg.ConnectSettings(&dlgSettings);
    dlg.exec();
}

void MainWindow::on_actionGuid_triggered()
{
    Help.show();
    Help.raise();
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
         Trigger.TriggerRequest(Kartina::REQ_ARCHIV, req, showInfo.pCode());
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

               if (grantAdultAccess(chan.bIsProtected))
               {
                  // new own downloader ...
                  if (vlcCtrl.ownDwnld() && (iDwnReqId != -1))
                  {
                     streamLoader.stopDownload (iDwnReqId);
                     iDwnReqId = -1;
                  }

                  showInfo.cleanShowInfo();
                  showInfo.setChanId(cid);
                  showInfo.setChanName(chan.sName);
                  showInfo.setShowType(ShowInfo::Live);
                  showInfo.setShowName(chan.sProgramm);
                  showInfo.setStartTime(chan.uiStart);
                  showInfo.setEndTime(chan.uiEnd);
                  showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
                  showInfo.setPCode(secCodeDlg.passWd());
                  showInfo.setPlayState(IncPlay::PS_RECORD);
                  showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                         .arg("rgb(255, 254, 212)")
                                         .arg(CShowInfo::createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

                  TouchPlayCtrlBtns(false);
                  Trigger.TriggerRequest(Kartina::REQ_STREAM, cid, secCodeDlg.passWd());
               }
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

                if (grantAdultAccess(chan.bIsProtected))
                {
                   showInfo.cleanShowInfo();
                   showInfo.setChanId(cid);
                   showInfo.setChanName(chan.sName);
                   showInfo.setShowType(ShowInfo::Live);
                   showInfo.setShowName(chan.sProgramm);
                   showInfo.setStartTime(chan.uiStart);
                   showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
                   showInfo.setEndTime(chan.uiEnd);
                   showInfo.setPCode(secCodeDlg.passWd());
                   showInfo.setPlayState(IncPlay::PS_PLAY);
                   showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                          .arg("rgb(255, 254, 212)")
                                          .arg(CShowInfo::createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

                   TouchPlayCtrlBtns(false);
                   Trigger.TriggerRequest(Kartina::REQ_STREAM, cid, secCodeDlg.passWd());
                }
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

                if (grantAdultAccess(chan.bIsProtected))
                {
                   showInfo.cleanShowInfo();
                   showInfo.setChanId(cid);
                   showInfo.setChanName(chan.sName);
                   showInfo.setShowType(ShowInfo::Live);
                   showInfo.setShowName(chan.sProgramm);
                   showInfo.setStartTime(chan.uiStart);
                   showInfo.setEndTime(chan.uiEnd);
                   showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
                   showInfo.setPlayState(IncPlay::PS_PLAY);
                   showInfo.setPCode(secCodeDlg.passWd());
                   showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                          .arg("rgb(255, 254, 212)")
                                          .arg(CShowInfo::createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

                   TouchPlayCtrlBtns(false);
                   Trigger.TriggerRequest(Kartina::REQ_STREAM, cid, secCodeDlg.passWd());
                }
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

/* -----------------------------------------------------------------\
|  Method: slotKartinaResponse [slot]
|  Begin: 29.05.2012
|  Author: Jo2003
|  Description: A central point to catch all http responses
|               from kartina client.
|               Please note: There is no real need for this
|               function because signals / slots can be connected
|               directly. The main goal of this function is to have
|               a central point to find out which function is called
|               when a certain response comes in.
|
|  Parameters: resp: response string
|              req: request type as defined in Kartina workspace
|
|  Returns: --
\----------------------------------------------------------------- */
void MainWindow::slotKartinaResponse(const QString& resp, int req)
{
   // helper macro to have a nice info printout ...
#define mkCase(__x__, __y__) \
      case __x__: \
         mInfo(tr("\n  --> HTTP Response '%1', calling '%2'").arg(#__x__).arg(#__y__)); \
         __y__; \
         break

   switch ((Kartina::EReq)req)
   {
   ///////////////////////////////////////////////
   // This function also grabs all settings
   // from response. After that channel list
   // will be requested.
   mkCase(Kartina::REQ_COOKIE, slotCookie(resp));

   ///////////////////////////////////////////////
   // Fills channel list as well as channel map.
   // Due to changing actual channel entry
   // slotCurrentChannelChanged() will be called
   // which requests the EPG ...
   mkCase(Kartina::REQ_CHANNELLIST, slotChanList(resp));

   ///////////////////////////////////////////////
   // Fills EPG browser and triggers the load
   // of VOD genres (if there in account info).
   mkCase(Kartina::REQ_EPG, slotEPG(resp));

   ///////////////////////////////////////////////
   // Indicates that a new timeshift value was set.
   // Triggers reload of channel list.
   mkCase(Kartina::REQ_TIMESHIFT, slotTimeShift(resp));

   ///////////////////////////////////////////////
   // Got Stream URL, start play or record
   mkCase(Kartina::REQ_STREAM, slotStreamURL(resp));

   ///////////////////////////////////////////////
   // Got requested stream url for timer record
   mkCase(Kartina::REQ_TIMERREC, dlgTimeRec.slotTimerStreamUrl(resp));

   ///////////////////////////////////////////////
   // got requested archiv url
   mkCase(Kartina::REQ_ARCHIV, slotArchivURL(resp));

   ///////////////////////////////////////////////
   // logout done
   mkCase(Kartina::REQ_LOGOUT, slotLogout(resp));

   ///////////////////////////////////////////////
   // got requested VOD genres
   mkCase(Kartina::REQ_GETVODGENRES, slotGotVodGenres(resp));

   ///////////////////////////////////////////////
   // got requested videos
   mkCase(Kartina::REQ_GETVIDEOS, slotGotVideos(resp));

   ///////////////////////////////////////////////
   // got requested video details
   mkCase(Kartina::REQ_GETVIDEOINFO, slotGotVideoInfo(resp));

   ///////////////////////////////////////////////
   // got requested vod url
   mkCase(Kartina::REQ_GETVODURL, slotVodURL(resp));

   ///////////////////////////////////////////////
   // got complete channel list
   // (used in settings dialog)
   mkCase(Kartina::REQ_CHANLIST_ALL, dlgParentalControl.slotBuildChanManager(resp));

   ///////////////////////////////////////////////
   // got requested VOD management data
   // (used in settings dialog)
   mkCase(Kartina::REQ_GET_VOD_MANAGER, dlgParentalControl.slotBuildVodManager(resp));

   ///////////////////////////////////////////////
   // handle vod favourites like vod genre to display
   // all videos in favourites
   mkCase(Kartina::REQ_GET_VOD_FAV, slotGotVideos(resp, true));

   ///////////////////////////////////////////////
   // response after trying to change parent code
   mkCase(Kartina::REQ_SET_PCODE, slotPCodeChangeResp(resp));

   ///////////////////////////////////////////////
   // Make sure the unused responses are listed
   // This makes it easier to understand the log.
   mkCase(Kartina::REQ_ADD_VOD_FAV, slotUnused(resp));
   mkCase(Kartina::REQ_REM_VOD_FAV, slotUnused(resp));
   mkCase(Kartina::REQ_SET_VOD_MANAGER, slotUnused(resp));
   mkCase(Kartina::REQ_SETCHAN_SHOW, slotUnused(resp));
   mkCase(Kartina::REQ_SETCHAN_HIDE, slotUnused(resp));
   mkCase(Kartina::REQ_SETBITRATE, slotUnused(resp));
   mkCase(Kartina::REQ_GETBITRATE, slotUnused(resp));
   mkCase(Kartina::REQ_GETTIMESHIFT, slotUnused(resp));
   mkCase(Kartina::REQ_GET_SERVER, slotUnused(resp));
   mkCase(Kartina::REQ_SERVER, slotUnused(resp));
   mkCase(Kartina::REQ_HTTPBUFF, slotUnused(resp));
   default:
      break;
   }
#undef mkCase
}

void MainWindow::slotUnused(const QString &str)
{
   Q_UNUSED(str)
}

void MainWindow::slotKartinaErr (const QString &str, int req, int err)
{
    // special error handling for special requests ...
    switch ((Kartina::EReq)req)
    {
    case Kartina::REQ_SET_PCODE:
       dlgParentalControl.slotEnablePCodeForm();
       break;
    default:
       break;
    }

    // special error handling for special errors ...
    switch ((Kartina::EErr)err)
    {
    case Kartina::ERR_WRONG_PCODE:
       showInfo.setPCode("");
       secCodeDlg.slotClearPasswd();
       break;
    default:
       break;
    }

    mErr(tr("Error %1 (%2) in request '%3'")
         .arg(err)
         .arg(metaKartina.errValToKey((Kartina::EErr)err))
         .arg(metaKartina.reqValToKey((Kartina::EReq)req)));

//    QMessageBox::critical(this, tr("Error"), tr("%1 Client API Error:\n%2 (#%3)")
//           .arg(COMPANY_NAME).arg(str).arg(err));

    ui->statusBar->showMessage(tr("Error: %1 Client API Error: %2 (#%3)")
                               .arg(COMPANY_NAME).arg(str).arg(err));
    TouchPlayCtrlBtns();
}

void MainWindow::slotLogout(const QString &str)
{
   // no need to look for errors in response ...
   Q_UNUSED(str);

   mInfo(tr("logout done ..."));

 //  QDialog::accept ();
}

void MainWindow::slotStreamURL(const QString &str)
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

void MainWindow::slotCookie (const QString &str)
{
    QString sCookie;

    // parse cookie ...
    if (!XMLParser.parseCookie(str, sCookie, accountInfo))
    {
       KartinaTv.SetCookie(sCookie);

       // decide if we should enable / disable VOD stuff ...
       if (accountInfo.bHasVOD)
       {
          if (!pChannelDlg->getTabEpgVOD()->widget(vodTabWidget.iPos))
          {
             // make sure tab text is translated as needed
             QString title = (pTranslator + Translators::TRANS_OWN)->translate(objectName().toUtf8().constData(),
                                                    vodTabWidget.sText.toUtf8().constData());

             // add tab ...
             pChannelDlg->getTabEpgVOD()->addTab(vodTabWidget.pWidget, (title != "") ? title : vodTabWidget.sText);
             pChannelDlg->getTabEpgVOD()->adjustSize();
          }
       }
       else
       {
          if (pChannelDlg->getTabEpgVOD()->widget(vodTabWidget.iPos))
          {
             // make sure the widget we want to remove
             // is not the active one ...
             pChannelDlg->getTabEpgVOD()->setCurrentIndex(0);
             pChannelDlg->getTabEpgVOD()->removeTab(vodTabWidget.iPos);
          }
       }
       // ------------------------------------------------
       // parse settings (new 11.05.2012)
       // ------------------------------------------------

       // timeshift
       QVector<int> values;
       int          actVal = -1;
       if (!XMLParser.parseSetting(str, "timeshift", values, actVal))
       {
          dlgSettings.fillTimeShiftCbx(values, actVal);

          // set timeshift ...
          pChannelDlg->getEpgBrowser()->SetTimeShift(actVal);
          dlgTimeRec.SetTimeShift(actVal);
          mInfo(tr("Using following timeshift: %1").arg(actVal));
       }

       // bitrate
       values.clear();
       actVal = -1;
       if (!XMLParser.parseSetting(str, "bitrate", values, actVal))
       {
          dlgSettings.SetBitrateCbx(values, actVal);
          mInfo (tr("Using Bitrate %1 kbit/s ...").arg(actVal));
       }

       // stream server
       QVector<cparser::SSrv> vSrv;
       QString sActIp;
       if (!XMLParser.parseSServersLogin(str, vSrv, sActIp))
       {
          dlgSettings.SetStreamServerCbx(vSrv, sActIp);
          mInfo(tr("Active stream server is %1").arg(sActIp));
       }

       // request channel list ...
       Trigger.TriggerRequest(Kartina::REQ_CHANNELLIST);
    }
 }

void MainWindow::slotTimeShift (const QString &str)
{
    Q_UNUSED(str)
    Trigger.TriggerRequest(Kartina::REQ_CHANNELLIST);
}

void MainWindow::slotChanList (const QString &str)
{
   QVector<cparser::SChan> chanList;

   if (!XMLParser.parseChannelList(str, chanList, dlgSettings.FixTime()))
   {
      FillChanMap(chanList);
      FillChannelList(chanList);

      // set channel list in timeRec class ...
      dlgTimeRec.SetChanList(chanList);
      dlgTimeRec.StartTimer();
   }

   // create favourite buttons if needed ...
   pChannelDlg->CreateFav();

   TouchPlayCtrlBtns();
}

void MainWindow::slotEPG(const QString &str)
{
   QDateTime epgTime = QDateTime::currentDateTime().addDays(pChannelDlg->getEpgOffset());
   QModelIndex idx   = pChannelDlg->getChannelList()->currentIndex();
   int cid           = qvariant_cast<int>(idx.data(channellist::cidRole));
   QIcon icon;

   if (!XMLParser.parseEpg(str, vEpgList))
   {
      pChannelDlg->getEpgBrowser()->DisplayEpg(vEpgList, pChannelDlg->getChanMap()->value(cid).sName,
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
              if (!bEpgRefresh)
              {
                  on_pushPlay_clicked();
              }
              else
              {
                  bEpgRefresh = false;
              }
          }
      }
      else
      {

         //it works only with the real accounts (not 140,...)!
         // update vod stuff only at startup ...
         if (!bGotVOD)
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
                if (!bEpgRefresh)
                {
                    on_pushPlay_clicked();
                }
                else
                {
                    bEpgRefresh = false;
                }
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
      QString cid  = link.encodedQueryItemValue(QByteArray("cid"));
      cparser::SChan chan = pChannelDlg->getChanMap()->value(cid.toInt());

      if (grantAdultAccess(chan.bIsProtected))
      {
          TouchPlayCtrlBtns(false);

          // new own downloader ...
          if (vlcCtrl.ownDwnld() && (iDwnReqId != -1))
          {
             streamLoader.stopDownload (iDwnReqId);
             iDwnReqId = -1;
          }

          QString    gmt  = link.encodedQueryItemValue(QByteArray("gmt"));
          QString    req  = QString("cid=%1&gmt=%2").arg(cid.toInt()).arg(gmt.toUInt());
          epg::SShow sepg = pChannelDlg->getEpgBrowser()->epgShow(gmt.toUInt());

          // store all info about show ...
          showInfo.cleanShowInfo();
          showInfo.setEpgMap(pChannelDlg->getTextEpg()->exportProgMap());
          showInfo.setChanId(cid.toInt());
          showInfo.setChanName(chan.sName);
          showInfo.setShowName(sepg.sShowName);
          showInfo.setStartTime(gmt.toUInt());
          showInfo.setEndTime(sepg.uiEnd);
          showInfo.setShowType(ShowInfo::Archive);
          showInfo.setPlayState(ePlayState);
          showInfo.setLastJumpTime(0);
          showInfo.setPCode(secCodeDlg.passWd());

          showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                 .arg("rgb(255, 254, 212)")
                                 .arg(CShowInfo::createTooltip(tr("%1 (Archive)").arg(showInfo.chanName()),
                                                    QString("%1 %2").arg(sepg.sShowName).arg(sepg.sShowDescr),
                                                    sepg.uiStart, sepg.uiEnd))));

          // add additional info to LCD ...
          int     iTime = (sepg.uiEnd) ? (int)((sepg.uiEnd - sepg.uiStart) / 60) : 60;
          QString sTime = tr("Length: %1 min.").arg(iTime);

          ui->labState->setHeader(showInfo.chanName() + tr(" (Ar.)"));
          ui->labState->setFooter(sTime);

          Trigger.TriggerRequest(Kartina::REQ_ARCHIV, req, secCodeDlg.passWd());
      }
   }
}

void MainWindow::slotReloadLogos()
{
    QChanMap::const_iterator cit;
    // create tmp channel list with channels from channelList ...

    for (cit = pChannelDlg->getChanMap()->constBegin(); cit != pChannelDlg->getChanMap()->constEnd(); cit++)
    {
        if (!(*cit).bIsGroup)
        {
            pixCache.enqueuePic((*cit).sIcon, pFolders->getLogoDir());
        }
    }
}

void MainWindow::slotArchivURL(const QString &str)
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

void MainWindow::slotVlcEnds(int iState __UNUSED)
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

void MainWindow::slotGotVodGenres(const QString &str)
{
   QVector<cparser::SGenre> vGenres;
   QVector<cparser::SGenre>::const_iterator cit;
   QString sName;

   // delete content ...
   pChannelDlg->getCbxGenre()->clear();

   if (!XMLParser.parseGenres(str, vGenres))
   {
      // fill genres combo box ...
      pChannelDlg->getCbxGenre()->addItem(tr("All"), QVariant((int)-1));

      for (cit = vGenres.constBegin(); cit != vGenres.constEnd(); cit ++)
      {
         // make first genre character upper case ...
         sName    = (*cit).sGName;
         sName[0] = sName[0].toUpper();
         pChannelDlg->getCbxGenre()->addItem(sName, QVariant((int)(*cit).uiGid));
      }
   }

   pChannelDlg->getCbxGenre()->setCurrentIndex(0);

   bGotVOD = true; // update vod stuff only at startup ...

   // trigger video load ...
   QUrl url;
   url.addQueryItem("type", pChannelDlg->getCbxLastOrBest()->itemData(pChannelDlg->getCbxLastOrBest()->currentIndex()).toString());
   url.addQueryItem("nums", "20");
   Trigger.TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));
}

void MainWindow::slotGotVideos(const QString &str, bool bVodFavs)
{
   QVector<cparser::SVodVideo> vVodList;
   cparser::SGenreInfo gInfo;

   if (!XMLParser.parseVodList(str, vVodList, gInfo))
   {
       QString sGenre = bVodFavs ? pChannelDlg->getCbxLastOrBest()->currentText() : pChannelDlg->getCbxGenre()->currentText();
       genreInfo      = gInfo;
       pChannelDlg->touchVodNavBar(gInfo);
       pChannelDlg->getVodBrowser()->displayVodList (vVodList, sGenre);

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

   // check password ...
   if (link.encodedQueryItemValue(QByteArray("pass_protect")).toInt())
   {
      // need password ... ?
      if (secCodeDlg.passWd().isEmpty())
      {
         // request password ...
         secCodeDlg.exec();
      }

      // no further error check here, API will tell
      // about a missing password ...
   }

   if (action == "vod_info")
   {
      // buffer last used page (whole code) ...
      lastVodSite.sContent      = pChannelDlg->getVodBrowser()->toHtml();
      lastVodSite.iScrollBarVal = pChannelDlg->getVodBrowser()->verticalScrollBar()->value();

      id = link.encodedQueryItemValue(QByteArray("vodid")).toInt();
      Trigger.TriggerRequest(Kartina::REQ_GETVIDEOINFO, id, secCodeDlg.passWd());
   }
   else if (action == "backtolist")
   {
      // restore last used page ...
      pChannelDlg->getVodBrowser()->setHtml(lastVodSite.sContent);
      pChannelDlg->getVodBrowser()->verticalScrollBar()->setValue(lastVodSite.iScrollBarVal);
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
   else if (action == "add_fav")
   {
      id = link.encodedQueryItemValue(QByteArray("vodid")).toInt();
      Trigger.TriggerRequest(Kartina::REQ_ADD_VOD_FAV, id, secCodeDlg.passWd());
      Trigger.TriggerRequest(Kartina::REQ_GETVIDEOINFO, id, secCodeDlg.passWd());
   }
   else if (action == "del_fav")
   {
      id = link.encodedQueryItemValue(QByteArray("vodid")).toInt();
      Trigger.TriggerRequest(Kartina::REQ_REM_VOD_FAV, id, secCodeDlg.passWd());
      Trigger.TriggerRequest(Kartina::REQ_GETVIDEOINFO, id, secCodeDlg.passWd());
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

      showInfo.cleanShowInfo();
      showInfo.setShowName(pChannelDlg->getVodBrowser()->getName());
      showInfo.setShowType(ShowInfo::VOD);
      showInfo.setPlayState(ePlayState);
      showInfo.setHtmlDescr(pChannelDlg->getVodBrowser()->getShortContent());
      showInfo.setVodId(id);

      ui->labState->setHeader(tr("Video On Demand"));
      ui->labState->setFooter(showInfo.showName());

      Trigger.TriggerRequest(Kartina::REQ_GETVODURL, id, secCodeDlg.passWd());
   }
}

void MainWindow::slotGotVideoInfo(const QString &str)
{
   cparser::SVodVideo vodInfo;
   if (!XMLParser.parseVideoInfo(str, vodInfo))
   {
      pChannelDlg->getVodBrowser()->displayVideoDetails(vodInfo);
   }
}

void MainWindow::slotVodURL(const QString &str)
{
   QStringList sUrls;

   if (!XMLParser.parseVodUrls(str, sUrls))
   {
      if (sUrls.count() > 1)
      {
          showInfo.setAdUrl(sUrls[1]);
      }

      if (ePlayState == IncPlay::PS_RECORD)
      {
         // use own downloader ... ?
         if (!vlcCtrl.ownDwnld())
         {
            StartVlcRec(sUrls[0], CleanShowName(showInfo.showName()));
         }
         else
         {
            StartStreamDownload(sUrls[0], CleanShowName(showInfo.showName()), "m4v");
         }

         showInfo.setPlayState(IncPlay::PS_RECORD);
      }
      else if (ePlayState == IncPlay::PS_PLAY)
      {
         StartVlcPlay(sUrls[0]);

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

          if (grantAdultAccess(chan.bIsProtected))
          {
             showInfo.cleanShowInfo();
             showInfo.setChanId(cid);
             showInfo.setChanName(chan.sName);
             showInfo.setShowType(ShowInfo::Live);
             showInfo.setShowName(chan.sProgramm);
             showInfo.setStartTime(chan.uiStart);
             showInfo.setEndTime(chan.uiEnd);
             showInfo.setLastJumpTime(QDateTime::currentDateTime().toTime_t());
             showInfo.setPlayState(IncPlay::PS_PLAY);
             showInfo.setPCode(secCodeDlg.passWd());
             showInfo.setHtmlDescr((QString(TMPL_BACKCOLOR)
                                    .arg("rgb(255, 254, 212)")
                                    .arg(CShowInfo::createTooltip(chan.sName, chan.sProgramm, chan.uiStart, chan.uiEnd))));

             TouchPlayCtrlBtns(false);
             Trigger.TriggerRequest(Kartina::REQ_STREAM, cid, secCodeDlg.passWd());
          }
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

void MainWindow::slotUpdateAnswer (QNetworkReply* pRes)
{
   if (pRes->error() == QNetworkReply::NoError)
   {
      // got update info ...
      QByteArray        ba = pRes->readAll();
      cparser::SUpdInfo updInfo;

      if (!XMLParser.parseUpdInfo(QString(ba), updInfo))
      {
         // compare version ...
         if ((updInfo.iMinor > atoi(VERSION_MINOR))
            && (updInfo.iMajor == atoi(VERSION_MAJOR))
            && (updInfo.sUrl != ""))
         {
            QString s       = HTML_SITE;
            QString content = tr("There is the new version %1 of %2 available.<br />Click %3 to download!")
                  .arg(updInfo.sVersion)
                  .arg(APP_NAME)
                  .arg(QString("<a href='%1'>%2</a>").arg(updInfo.sUrl).arg(tr("here")));

            s.replace(TMPL_CONT, content);

            QMessageBox::information(this, tr("Update available"), s);
         }
      }
   }
   else
   {
      // only tell in log about the error ...
      mInfo(pRes->errorString());
   }

   // schedule deletion ...
   pRes->deleteLater();
}

void MainWindow::slotCheckArchProg(ulong ulArcGmt)
{
   // is actual showinfo still actual ?
    if (!CSmallHelpers::inBetween(showInfo.starts(), showInfo.ends(), (uint)ulArcGmt))
    {
       // search in archiv program map for matching entry ...
       if (!showInfo.autoUpdate(ulArcGmt, vEpgList))
       {
          // add additional info to LCD ...
          int     iTime = showInfo.ends() ? (int)((showInfo.ends() - showInfo.starts()) / 60) : 60;
          QString sTime = tr("Length: %1 min.").arg(iTime);
          ui->labState->setFooter(sTime);
          ui->labState->updateState(showInfo.playState());

          // set short epg info ...
          ui->textEpgShort->setHtml(showInfo.htmlDescr());

          // done ...
          emit sigShowInfoUpdated();

          bEpgRefresh = true;
          tEpgRefresh.setSingleShot(true);
          tEpgRefresh.start(61000); // 1000 ms = 1 sec
       }
    }
}

void MainWindow::slotEpgRefresh()
{
    bEpgRefresh = true;
    emit sigEpgRefresh();
}

void MainWindow::slotRefreshChanLogos()
{
   if (!(ulStartFlags & FLAG_CLOGO_COMPL))
   {
      QStandardItem      *pItem;
      QPixmap             icon;
      int                 cid, curCid, i;
      QString             fLogo;

      // get current selection ...
      curCid = pChannelDlg->getModel()->itemFromIndex(pChannelDlg->getChannelList()->currentIndex())->data(channellist::cidRole).toInt();

      for (i = 0; i < pChannelDlg->getModel()->rowCount(); i++)
      {
         pItem = pChannelDlg->getModel()->item(i);
         cid   = pItem->data(channellist::cidRole).toInt();
         fLogo = QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(cid);

         if (icon.load(fLogo, "image/gif"))
         {
            pItem->setData(QIcon(icon), channellist::iconRole);

            // update channel icon on EPG browser ...
            if (cid == curCid)
            {
               pChannelDlg->getLabChanIcon()->setPixmap(QIcon(icon).pixmap(24, 24));
            }
         }
      }

      // mark logo stuff as completed ...
      ulStartFlags |= FLAG_CLOGO_COMPL;
   }
}

void MainWindow::slotPCodeChangeResp(const QString &str)
{
    Q_UNUSED(str)

    // clear buffered password ...
    secCodeDlg.setPasswd("");
    showInfo.setPCode("");

    dlgParentalControl.slotNewPCodeSet();
}

void MainWindow::slotRestoreMinimized()
{
   setWindowState(windowState() & ~(Qt::WindowMinimized | Qt::WindowActive));
   show();
}

#ifdef INCLUDE_LIBVLC
void MainWindow::slotToggleFullscreen()
{
   if (!pVideoWidget)
   {
      // get videoWidget ...
      pVideoWidget = ui->player->getAndRemoveVideoWidget();

      // add videoWidget to stacked layout ...
      stackedLayout->addWidget(pVideoWidget);

      // make sure videoWidget is the one we see ...
      stackedLayout->setCurrentWidget(pVideoWidget);

      // make dialog fullscreen ...
      toggleFullscreen();

      emit sigFullScreenToggled(1);
   }
   else
   {
      // remove videoWidget from stacked layout ...
      stackedLayout->removeWidget(pVideoWidget);

      // make sure main dialog is visible ...
      stackedLayout->setCurrentWidget(ui->masterFrame);

      // put videoWidget back into player widget ...
      ui->player->addAndEmbedVideoWidget();

      // reset videoWidgets local pointer ...
      pVideoWidget = NULL;

      // show normal ...
      toggleFullscreen();

      emit sigFullScreenToggled(0);
   }
}

#endif // INCLUDE_LIBVLC

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
    (pTranslator + Translators::TRANS_OWN)->load(QString("lang_%1").arg(dlgSettings.GetLanguage ()),
                      pFolders->getLangDir());
    (pTranslator + Translators::TRANS_QT)->load(QString("qt_%1").arg(dlgSettings.GetLanguage ()),
                      pFolders->getQtLangDir());

    // -------------------------------------------
    // set last windows size / position ...
    // -------------------------------------------
    if (dlgSettings.getGeometry().size() > 0)
    {
        restoreGeometry(dlgSettings.getGeometry());
    }
    else
    {
        // delete old values ...
        pDb->removeSetting("WndRect");
        pDb->removeSetting("WndState");
        pDb->removeSetting("IsMaximized");
    }

    // check for program updates ...
    if (dlgSettings.checkForUpdate())
    {
        pUpdateChecker->get(QNetworkRequest(QUrl(UPD_CHECK_URL)));
    }
}

void MainWindow::ConnectionInit()
{
    QString sHlp;
    VlcLog.SetLogLevel(dlgSettings.GetLogLevel());

    pChannelDlg->getModel()->clear();
    KartinaTv.abort();

    // set connection data ...
    KartinaTv.SetData(dlgSettings.GetAPIServer(), dlgSettings.GetUser(), dlgSettings.GetPasswd());

    // set proxy stuff ...
    if (dlgSettings.UseProxy())
    {
       QNetworkProxy proxy(QNetworkProxy::HttpCachingProxy,
                           dlgSettings.GetProxyHost(), dlgSettings.GetProxyPort(),
                           dlgSettings.GetProxyUser(), dlgSettings.GetProxyPasswd());

       KartinaTv.setProxy(proxy);
       pixCache.setProxy(proxy);
       streamLoader.setProxy(proxy);
       pUpdateChecker->setProxy(proxy);
   }

    // set language as read ...
    (pTranslator + Translators::TRANS_OWN)->load(QString("lang_%1").arg(dlgSettings.GetLanguage ()),
                                                 pFolders->getLangDir());
    (pTranslator + Translators::TRANS_QT)->load(QString("qt_%1").arg(dlgSettings.GetLanguage ()),
                                                pFolders->getQtLangDir());

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

    // lock parental manager ...
    emit sigLockParentalManager();

    if (dlgSettings.HideToSystray() && QSystemTrayIcon::isSystemTrayAvailable())
    {
        connect (this, SIGNAL(sigHide()), &trayIcon, SLOT(show()));
        connect (this, SIGNAL(sigShow()), &trayIcon, SLOT(hide()));
    }
    else
    {
        disconnect(this, SIGNAL(sigHide()));
        disconnect(this, SIGNAL(sigShow()));
    }

    // set new(?) helpfile ...
    // be sure the file we want to load exists ... fallback to english help ...
    sHlp = QString("%1/help_%2.qhc").arg(pFolders->getDocDir()).arg(dlgSettings.GetLanguage());

    if (!QFile::exists(sHlp))
    {
        sHlp = QString("%1/help_en.qhc").arg(pFolders->getDocDir());
    }

    Help.setHelpFile(sHlp);
}

void MainWindow::savePositions()
{
   // -------------------------------------------
   // save gui settings ...
   // -------------------------------------------
   dlgSettings.setGeometry(saveGeometry());
}

void MainWindow::CreateSystray()
{
   trayIcon.setIcon(QIcon(":/app/tv"));
   trayIcon.setToolTip(APP_NAME);

   // create context menu for tray icon ...
   QMenu *pShowMenu = new QMenu (this);
   pShowMenu->addAction(QIcon(":/app/restore"),
                        tr("&restore %1").arg(APP_NAME),
                        this, SLOT(slotRestoreMinimized()));
   trayIcon.setContextMenu(pShowMenu);
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
   QString   sLine;
   QString   sLogoFile;
   QStandardItem *pItem;
   bool      bMissingIcon = false;
   int       iRow, iRowGroup;
   QFileInfo fInfo;
   QPixmap   Pix(16, 16);
   QPixmap   icon;
   int       iChanCount = 0;
   int       iChanGroupCount = 0;

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
       // check if we should display channel in channel list ...
       if (chanlist[i].bIsHidden)
       {
          mInfo(tr("Exclude '%1' from channel list (hidden: %2, protected: %3).")
                .arg(chanlist[i].sName)
                .arg(chanlist[i].bIsHidden)
                .arg(chanlist[i].bIsProtected));
       }
       else
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
             fInfo.setFile(chanlist[i].sIcon);
             sLogoFile = QString("%1/%2").arg(pFolders->getLogoDir()).arg(fInfo.fileName());
             sLine     = QString("%1. %2").arg(++ iChanCount).arg(chanlist[i].sName);

             // check if file exists ...
             if (!QFile::exists(sLogoFile))
             {
                 // no --> load default image ...
                 icon.load(":png/no_logo");

                 // enqueue pic to cache ...
                 pixCache.enqueuePic(chanlist[i].sIcon, pFolders->getLogoDir());

                 // mark for reload ...
                 bMissingIcon = true;
             }
             else
             {
                 // check if image file can be loaded ...
                 if (!icon.load(sLogoFile, "image/gif"))
                 {
                    // can't load --> load default image ...
                    icon.load(":png/no_logo");

    //                mInfo(tr("Can't load channel image \"%1.gif\" ...").arg(chanlist[i].iId));
                    ui->statusBar->showMessage(tr("Can't load channel image \"%1.gif\" ...").
                                               arg(chanlist[i].iId));

                    // delete logo file ...
                    QFile::remove(sLogoFile);

                    // enqueue pic to cache ...
                    pixCache.enqueuePic(chanlist[i].sIcon, pFolders->getLogoDir());

                    // mark for reload ...
                    bMissingIcon = true;
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
                 pItem->setToolTip(CShowInfo::createTooltip(chanlist[i].sName, chanlist[i].sProgramm,
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

       } // not hidden ...

   }

   if (!bMissingIcon)
   {
       ulStartFlags |= FLAG_CLOGO_COMPL;
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
         {tr("Toggle Fullscreen"),    ui->player,   SLOT(on_btnFullScreen_clicked()),       "ALT+F"},
         {tr("Volume +"),             ui->player,   SLOT(slotMoreLoudly()),                 "+"},
         {tr("Volume -"),             ui->player,   SLOT(slotMoreQuietly()),                "-"},
         {tr("Toggle Mute"),          ui->player,   SLOT(slotMute()),                       "M"},
         {tr("Jump Forward"),         this,         SLOT(on_pushFwd_clicked()),             "CTRL+ALT+F"},
         {tr("Jump Backward"),        this,         SLOT(on_pushBwd_clicked()),             "CTRL+ALT+B"},
         {tr("Next Channel"),         this,         SLOT(slotChannelDown()),                "CTRL+N"},
         {tr("Previous Channel"),     this,         SLOT(slotChannelUp()),                  "CTRL+P"},
         {tr("Show EPG / VOD"),       this,         SLOT(slotToggleEpgVod()),               "CTRL+E"},
         {tr("Help"),                 this,         SLOT(on_actionGuid_triggered()),        "F1"},
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

 int MainWindow::grantAdultAccess(bool bProtected)
 {
    int iRV = 0;

    if (!bProtected)
    {
       // unprotected channel --> always grant access ...
       iRV = 1;
    }
    else
    {
       // protected channel allowed ... ?
       if (dlgSettings.AllowEros())
       {
           secCodeDlg.setPasswd(pDb->password("ErosPasswdEnc"));
           iRV = 1;
       }
       else
       {
           // request password ...
           secCodeDlg.exec();

           // we only can grant access if we have the correct password ...
           if ( secCodeDlg.passWd() == pDb->password("ErosPasswdEnc") )
           {
               iRV = 1;
           }
           else
           {
               QMessageBox::critical(this, tr("Error!"),
                   tr("<b>The parent code is empty or not correct.</b>\n</ul>\n"));
           }
       }
    }

    return iRV;
 }

 void MainWindow::toggleFullscreen()
 {
     setWindowState(windowState() ^ Qt::WindowFullScreen);
     show();
 }

 void MainWindow::printStateChange(const Qt::WindowStates &old)
 {
    Qt::WindowStates cur = windowState();
    QStringList sOld, sNew;

    if (old.testFlag(Qt::WindowNoState))
    {
       sOld << "Qt::WindowNoState";
    }

    if (old.testFlag(Qt::WindowMinimized))
    {
       sOld << "Qt::WindowMinimized";
    }

    if (old.testFlag(Qt::WindowMaximized))
    {
       sOld << "Qt::WindowMaximized";
    }

    if (old.testFlag(Qt::WindowFullScreen))
    {
       sOld << "Qt::WindowFullScreen";
    }

    if (old.testFlag(Qt::WindowActive))
    {
       sOld << "Qt::WindowActive";
    }

    //////////

    if (cur.testFlag(Qt::WindowNoState))
    {
       sNew << "Qt::WindowNoState";
    }

    if (cur.testFlag(Qt::WindowMinimized))
    {
       sNew << "Qt::WindowMinimized";
    }

    if (cur.testFlag(Qt::WindowMaximized))
    {
       sNew << "Qt::WindowMaximized";
    }

    if (cur.testFlag(Qt::WindowFullScreen))
    {
       sNew << "Qt::WindowFullScreen";
    }

    if (cur.testFlag(Qt::WindowActive))
    {
       sNew << "Qt::WindowActive";
    }

    mInfo(tr("WindowState change: \n --> %1 <--> %2").arg(sOld.join("|")).arg(sNew.join("|")));
 }

/************************* History ***************************\
| $Log$
\*************************************************************/
