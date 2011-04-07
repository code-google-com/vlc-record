#include <QMessageBox>
#include "cchannelsepgdlg.h"
#include "ui_cchannelsepgdlg.h"

// for folders ...
extern CDirStuff *pFolders;

// global showinfo class ...
extern CShowInfo showInfo;

CChannelsEPGdlg::CChannelsEPGdlg(QWidget *parent) :
        QDialog(parent, Qt::Window),
    ui(new Ui::CChannelsEPGdlg)
{
    ui->setupUi(this);

    // set channel list model and delegate ...
    pModel    = new QStandardItemModel;
    pDelegate = new QChanListDelegate;

    ui->channelList->setItemDelegate(pDelegate);
    ui->channelList->setModel(pModel);

    pTrigger = NULL;
    pSettings = NULL;
    iFontSzChg     = 0;
    iEpgOffset     = 0;
    ePlayState     = IncPlay::PS_WTF;
    bSaveCng       = false;
    pStatusBar = NULL;
    pCbxTimeShift = NULL;

    // init favourite buttons ...
    for (int i = 0; i < MAX_NO_FAVOURITES; i++)
    {
       pFavBtn[i]     = NULL;
       pFavAct[i]     = NULL;
       pContextAct[i] = NULL;
    }

    favContext.setParent(this, Qt::Popup);

    connect (ui->channelList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChanListContext(QPoint)));
    connect (&favContext,   SIGNAL(triggered(QAction*)), this, SLOT(slotChgFavourites(QAction*)));
    connect(ui->channelList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentChannelChanged(QModelIndex)));

    // -------------------------------------------
    // get favourites ...
    // -------------------------------------------
    lFavourites = pSettings->GetFavourites();
}

CChannelsEPGdlg::~CChannelsEPGdlg()
{
    delete ui;

    if (pModel)
    {
        delete pModel;
    }

    if (pDelegate)
    {
        delete pDelegate;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                           Events                                           //
////////////////////////////////////////////////////////////////////////////////

void CChannelsEPGdlg::closeEvent(QCloseEvent *event)
{
  // We want to close ChannelDlg, store all needed values ...
  // Note: putting this function in destructor doesn't work!
  if (bSaveCng)
  {
      savePositions();
      bSaveCng = false;
  }

  emit sigChannelDlgClosed();
}

void CChannelsEPGdlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);

    switch(e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        TouchEpgNavi(false);
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//                           "on_" - Slots                                    //
////////////////////////////////////////////////////////////////////////////////

void CChannelsEPGdlg::on_cbxChannelGroup_activated(int index)
{
    int row = ui->cbxChannelGroup->itemData(index).toInt();
    QModelIndex idx = pModel->index(row + 1, 0);

    ui->channelList->setCurrentIndex(idx);
    ui->channelList->scrollTo(idx);
}

void CChannelsEPGdlg::on_cbxGenre_currentIndexChanged(int index)
{
    int iGid = ui->cbxGenre->itemData(index).toInt();

    pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, iGid);
}

void CChannelsEPGdlg::on_btnSearch_clicked()
{
    if(!ui->textEpg->find(ui->lineSearch->text()))
    {
       // not found --> set cursor to document start ...
       ui->textEpg->moveCursor(QTextCursor::Start);
    }
}

void CChannelsEPGdlg::on_btnFontSmaller_clicked()
{
    QFont f;

    ui->textEpg->ReduceFont();
    pTextEpgShort->ReduceFont();
    ui->vodBrowser->ReduceFont();

    f = ui->channelList->font();
    f.setPointSize(f.pointSize() - 1);
    ui->channelList->setFont(f);

    f = ui->cbxChannelGroup->font();
    f.setPointSize(f.pointSize() - 1);
    ui->cbxChannelGroup->setFont(f);

    iFontSzChg --;
}

void CChannelsEPGdlg::on_btnFontLarger_clicked()
{
    QFont f;

    ui->textEpg->EnlargeFont();
    pTextEpgShort->EnlargeFont();
    ui->vodBrowser->EnlargeFont();

    f = ui->channelList->font();
    f.setPointSize(f.pointSize() + 1);
    ui->channelList->setFont(f);

    f = ui->cbxChannelGroup->font();
    f.setPointSize(f.pointSize() + 1);
    ui->cbxChannelGroup->setFont(f);

    iFontSzChg ++;
}

void CChannelsEPGdlg::on_btnVodSearch_clicked()
{
    int iIdx = ui->cbxSearchArea->currentIndex();
    vodbrowser::eSearchArea eArea = (vodbrowser::eSearchArea)ui->cbxSearchArea->itemData(iIdx).toUInt();
    ui->vodBrowser->findVideos(ui->lineVodSearch->text(), eArea);

}

void CChannelsEPGdlg::on_channelList_doubleClicked(const QModelIndex & index)
{
   int cid = qvariant_cast<int>(index.data(channellist::cidRole));

   if (chanMap.contains(cid))
   {
       emit sigDoubliClickOnListWidget();
   }
}

void CChannelsEPGdlg::on_lineSearch_returnPressed()
{
    if(!ui->textEpg->find(ui->lineSearch->text()))
    {
       // not found --> set cursor to document start ...
       ui->textEpg->moveCursor(QTextCursor::Start);
    }

}

////////////////////////////////////////////////////////////////////////////////
//                                Slots                                       //
////////////////////////////////////////////////////////////////////////////////

void CChannelsEPGdlg::slotDayTabChanged(int iIdx)
{
int cid = getCurrentCid();

if (chanMap.contains(cid))
{
         QDateTime epgTime = QDateTime::currentDateTime().addDays(iEpgOffset);
         int       iDay    = epgTime.date().dayOfWeek() - 1;

         // earlier or later ... ?
         if (iIdx < iDay)
         {
            // earlier ...
            iEpgOffset -= iDay - iIdx;
         }
         else if (iIdx > iDay)
         {
            // later ...
            iEpgOffset += iIdx - iDay;
         }

         // get epg for requested day ...
         if (iIdx != iDay)
         {
            pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
         }
   }
}

void CChannelsEPGdlg::slotbtnBack_clicked()
{
   int cid = getCurrentCid();

      if  (chanMap.contains(cid))
      {
         // set actual day in previous week to munday ...
         int iActDay  = pEpgNavbar->currentIndex();
         iEpgOffset  -= 7 + iActDay;
         pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
      }
}

void CChannelsEPGdlg::slotbtnNext_clicked()
{
    int cid = getCurrentCid();

      if (chanMap.contains(cid))
      {
         // set actual day in next week to munday ...
         int iActDay  = pEpgNavbar->currentIndex();
         iEpgOffset  += 7 - iActDay;
         pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
      }
}

void CChannelsEPGdlg::slotChanListContext(const QPoint &pt)
{
    int cid = getCurrentCid();

    if (chanMap.contains(cid)) // real channel ...
    {
         // create context menu ...
         CleanContextMenu();
         pContextAct[0] = new CFavAction (&favContext);
         QString    sLogoFile = QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(cid);

         // is channel already in favourites ... ?
         if (lFavourites.contains(cid))
         {
            // create remove menu ...
            // action.setText(tr("Remove \"%1\" from favourites").arg(pItem->GetName()));
            pContextAct[0]->setText(tr("Remove from favourites"));
            pContextAct[0]->setIcon(QIcon(sLogoFile));
            pContextAct[0]->setFavData(cid, kartinafav::FAV_DEL);
         }
         else
         {
            // create add menu ...
            // action.setText(tr("Add \"%1\" to favourites").arg(pItem->GetName()));
            pContextAct[0]->setText(tr("Add to favourites"));
            pContextAct[0]->setIcon(QIcon(sLogoFile));
            pContextAct[0]->setFavData(cid, kartinafav::FAV_ADD);
         }

         // add action to menu ...
         favContext.addAction(pContextAct[0]);

         // display menu ...
         favContext.exec(ui->channelList->mapToGlobal(pt));
      }
}

void CChannelsEPGdlg::slotChgFavourites (QAction *pAct)
{
   CFavAction      *pAction = (CFavAction *)pAct;
   int              iCid    = 0;
   kartinafav::eAct action  = kartinafav::FAV_WHAT;

   // get action details ...
   pAction->favData(iCid, action);

   // what to do ... ?
   if (action == kartinafav::FAV_ADD)
   {
      if (lFavourites.count() < MAX_NO_FAVOURITES)
      {
         // add new favourite ...
         lFavourites.push_back(iCid);

         HandleFavourites();
         bSaveCng = true;
      }
      else
      {
//         QMessageBox::information(this, tr("Note"),
//                                  tr("Max. number of favourites (%1) reached.")
//                                  .arg(MAX_NO_FAVOURITES));
         pStatusBar->showMessage(tr("Note: Max. number of favourites (%1) reached.").arg(MAX_NO_FAVOURITES));
      }
   }
   else if (action == kartinafav::FAV_DEL)
   {
      // remove favourite ...
      lFavourites.removeOne(iCid);

      HandleFavourites();
      bSaveCng = true;
   }
}

void CChannelsEPGdlg::slotHandleFavAction(QAction *pAct)
{
    CFavAction      *pAction = (CFavAction *)pAct;
    int              iCid    = 0;
    kartinafav::eAct act     = kartinafav::FAV_WHAT;

    if (pAction)
    {
       pAction->favData(iCid, act);

       // search in channel list for cannel id ...
       QModelIndex idx;

       // go through channel list ...
       for (int i = 0; i < pModel->rowCount(); i++)
       {
          idx = pModel->index(i, 0);

          // check if this is favourite channel ...
          if (qvariant_cast<int>(idx.data(channellist::cidRole)) == iCid)
          {
             // found --> mark row ...
             ui->channelList->setCurrentIndex(idx);
             ui->channelList->scrollTo(idx);
             break;
          }
       }
    }
}

void CChannelsEPGdlg::slotFavBtnContext(const QPoint &pt)
{
   QString     sLogoFile;

   CleanContextMenu();

   for (int i = 0; i < lFavourites.count(); i++)
   {
      pContextAct[i] = new CFavAction(&favContext);

      if (pContextAct[i])
      {
         sLogoFile = QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(lFavourites[i]);
         pContextAct[i]->setIcon(QIcon(sLogoFile));
         pContextAct[i]->setText(tr("Remove from favourites"));
         pContextAct[i]->setFavData(lFavourites[i], kartinafav::FAV_DEL);
         favContext.addAction(pContextAct[i]);
      }
   }

   // display menu over first button since we have no way
   // to find out on over which button we clicked ...
   favContext.exec(pFavBtn[0]->mapToGlobal(pt));
}

void CChannelsEPGdlg::slotCurrentChannelChanged(const QModelIndex & current)
{
   int cid = qvariant_cast<int>(current.data(channellist::cidRole));

   if (chanMap.contains(cid))
   {
      // get whole channel entry ...
      cparser::SChan entry = chanMap.value(cid);
      int iTs;

      pTextEpgShort->setHtml(QString(TMPL_BACKCOLOR)
                                .arg("rgb(255, 254, 212)")
                                .arg(createTooltip(entry.sName, entry.sProgramm, entry.uiStart, entry.uiEnd)));

      SetProgress (entry.uiStart, entry.uiEnd);

      // quick'n'dirty timeshift hack ...
      if (entry.vTs.count() <= 2) // no timeshift available ...
      {
         ui->textEpg->SetTimeShift(0);
      }
      else
      {
         iTs = pCbxTimeShift->currentText().toInt();

         if (ui->textEpg->GetTimeShift() != iTs)
         {
            ui->textEpg->SetTimeShift(iTs);
         }
      }

      // was this a refresh or was channel changed ... ?
      if (cid != ui->textEpg->GetCid())
      {
         // load epg ...
         pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
      }
      else // same channel ...
      {
         // refresh epg only, if we view current day in epg ...
         if (iEpgOffset == 0) // 0 means today!
         {
            pTrigger->TriggerRequest(Kartina::REQ_EPG, cid);
         }
      }
   }
}
////////////////////////////////////////////////////////////////////////////////
//                             normal functions                               //
////////////////////////////////////////////////////////////////////////////////

void CChannelsEPGdlg::initDialog (bool bInit)
{
    bool ok = false;

    // -------------------------------------------
    // create epg nav bar ...
    // -------------------------------------------
    TouchEpgNavi(bInit);

    // -------------------------------------------
    // set font size to last used
    // -------------------------------------------
    iFontSzChg = pSettings->GetCustFontSize();

    if (iFontSzChg)
    {
       QFont f;
       ui->textEpg->ChangeFontSize(iFontSzChg);
       pTextEpgShort->ChangeFontSize(iFontSzChg);
       ui->vodBrowser->ChangeFontSize(iFontSzChg);

       f = ui->channelList->font();
       f.setPointSize(f.pointSize() + iFontSzChg);
       ui->channelList->setFont(f);

       f = ui->cbxChannelGroup->font();
       f.setPointSize(f.pointSize() + iFontSzChg);
       ui->cbxChannelGroup->setFont(f);
    }

    // -------------------------------------------
    // set splitter sizes as last used
    // -------------------------------------------
    QList<int> sSplit;
 #ifndef INCLUDE_LIBVLC
    sSplit = pSettings->GetSplitterSizes("spChanEpg", &ok);
    if (ok)
    {
       ui->vSplitterChanEpg->setSizes(sSplit);
    }

 #else /* ifdef INCLUDE_LIBVLC */
    sSplit = pSettings->GetSplitterSizes("spVChanEpg", &ok);
    if (ok)
    {
       ui->vSplitterChanEpg->setSizes(sSplit);
    }
#endif /* INCLUDE_LIBVLC */
}

void CChannelsEPGdlg::TouchEpgNavi (bool bCreate)
{
   QToolButton *pBtn;

   if (bCreate)
   {
    /*
        Note: We can't create the navbar in Qt Creator or
        Designer because QTabBar isn't supported there.
        Therefore we create it manually here.

        Format navbar:

        /-----------------------------------------------------\
        | <-- |\/\/| /Mon/Tue/Wed/Thu/Fri/Sat/Sun/ |\/\/| --> |
        \-----------------------------------------------------/

      */

      // create back button and set style ...
      pBtn = new QToolButton;
      pBtn->setIcon(QIcon(":png/back"));
      pBtn->setAutoRaise(true);
      pBtn->setMaximumHeight(EPG_NAVBAR_HEIGHT);
      pBtn->setToolTip(tr("1 week backward"));

      // connect signal with slot ...
      connect (pBtn, SIGNAL(clicked()), this, SLOT(slotbtnBack_clicked()));

      // add button to layout ...
      ui->hLayoutEpgNavi->addWidget(pBtn);

      // create tabbar (epg navbar) and set height ...
      pEpgNavbar = new QTabBar;
      pEpgNavbar->setMaximumHeight(EPG_NAVBAR_HEIGHT);

      // set style so tabs look like whole design ...
      pEpgNavbar->setStyleSheet(QString(NAVBAR_STYLE));

      // connect signal with slot ...
      connect (pEpgNavbar, SIGNAL(currentChanged(int)), this, SLOT(slotDayTabChanged(int)));

      // add h spacer ...
      ui->hLayoutEpgNavi->addStretch();

      // add navbar ...
      ui->hLayoutEpgNavi->addWidget(pEpgNavbar);

      // add h spacer ...
      ui->hLayoutEpgNavi->addStretch();

      // create next button and set style ...
      pBtn = new QToolButton;
      pBtn->setIcon(QIcon(":png/next"));
      pBtn->setAutoRaise(true);
      pBtn->setMaximumHeight(EPG_NAVBAR_HEIGHT);
      pBtn->setToolTip(tr("1 week forward"));

      // connect signal with slot ...
      connect (pBtn, SIGNAL(clicked()), this, SLOT(slotbtnNext_clicked()));

      // add button to layout ...
      ui->hLayoutEpgNavi->addWidget(pBtn);

      // create day tabs ...
      pEpgNavbar->addTab(tr("Mon"));
      pEpgNavbar->addTab(tr("Tue"));
      pEpgNavbar->addTab(tr("Wed"));
      pEpgNavbar->addTab(tr("Thu"));
      pEpgNavbar->addTab(tr("Fri"));
      pEpgNavbar->addTab(tr("Sat"));

      // set color for normal week days ...
      for (int i = 0; i < 5; i++)
      {
         pEpgNavbar->setTabTextColor(i, QColor("white"));
      }

      pEpgNavbar->setTabTextColor(5, QColor("#00a"));
      pEpgNavbar->addTab(tr("Sun"));
      pEpgNavbar->setTabTextColor(6, QColor("#800"));

   }
   else
   {
      // no creation, only translation ...
      pEpgNavbar->setTabText(0, tr("Mon"));
      pEpgNavbar->setTabText(1, tr("Tue"));
      pEpgNavbar->setTabText(2, tr("Wed"));
      pEpgNavbar->setTabText(3, tr("Thu"));
      pEpgNavbar->setTabText(4, tr("Fri"));
      pEpgNavbar->setTabText(5, tr("Sat"));
      pEpgNavbar->setTabText(6, tr("Sun"));

      // fill in tooltip for navi buttons ...
      int iIdx;
      // back button ...
      iIdx = 0;
      pBtn = (QToolButton *)ui->hLayoutEpgNavi->itemAt(iIdx)->widget();
      pBtn->setToolTip(tr("1 week backward"));

      // next button ...
      iIdx = ui->hLayoutEpgNavi->count() - 1;
      pBtn = (QToolButton *)ui->hLayoutEpgNavi->itemAt(iIdx)->widget();
      pBtn->setToolTip(tr("1 week forward"));
   }
}

void CChannelsEPGdlg::CreateFav()
{
    // create favourite buttons if needed ...
    if ((lFavourites.count() > 0) && (ui->gLayoutFav->count() == 0))
    {
       HandleFavourites();
    }
}

void CChannelsEPGdlg::HandleFavourites()
{
   int i;
   QPixmap pic;
   QString sObj;

   // remove all favourite buttons ...
   for (i = 0; i < MAX_NO_FAVOURITES; i++)
   {
      if (pFavBtn[i] != NULL)
      {
         // delete shortcut entry from shortcut table ...
         sObj = QString("pFavAct[%1]").arg(i);
         pSettings->delShortCut(sObj, SLOT(slotHandleFavAction(QAction*)));

         ui->gLayoutFav->removeWidget(pFavBtn[i]);
         delete pFavAct[i];
         delete pFavBtn[i];
         pFavBtn[i] = NULL;
         pFavAct[i] = NULL;
      }
   }

   // re-create all buttons ...
   for (i = 0; i < lFavourites.count(); i++)
   {
      pFavBtn[i] = new QToolButton (this);
      pFavAct[i] = new CFavAction (this);

      if (pFavBtn[i] && pFavAct[i])
      {
         // -------------------------
         // init action ...
         // -------------------------

         // add logo ...
         pic.load(QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(lFavourites[i]));
         pFavAct[i]->setIcon(QIcon(pic.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

         // store channel id in action ...
         pFavAct[i]->setFavData(lFavourites[i], kartinafav::FAV_WHAT);

         // add shortcut to shortcut table ...
         sObj = QString("pFavAct[%1]").arg(i);
         pSettings->addShortCut(tr("Favourite %1").arg(i + 1), sObj,
                                SLOT(slotHandleFavAction(QAction*)),
                                QString("ALT+%1").arg(i));

         // set shortcut ...
         pFavAct[i]->setShortcut(QKeySequence(pSettings->shortCut(sObj,
                                 SLOT(slotHandleFavAction(QAction*)))));

         // add channel name as tooltip ...
         pFavAct[i]->setToolTip(chanMap.value(lFavourites[i]).sName);

         // style the tool button ...
         pFavBtn[i]->setStyleSheet(FAVBTN_STYLE);

         // set action ...
         pFavBtn[i]->setDefaultAction(pFavAct[i]);

         // set icon size ...
         pFavBtn[i]->setIconSize(QSize(32, 32));

         // we will use own context menu ...
         pFavBtn[i]->setContextMenuPolicy(Qt::CustomContextMenu);

         // connect button trigger with slot function ...
         connect (pFavBtn[i], SIGNAL(triggered(QAction*)), this, SLOT(slotHandleFavAction(QAction*)));

         connect (pFavBtn[i], SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotFavBtnContext(QPoint)));

         // add button to layout ...
         ui->gLayoutFav->addWidget(pFavBtn[i], i / (MAX_NO_FAVOURITES / 2), i % (MAX_NO_FAVOURITES / 2), Qt::AlignCenter);
      }
      else
      {
         // memory allocation problem ... should never happen ...
         if (pFavBtn[i])
         {
            delete pFavBtn[i];
            pFavBtn[i] = NULL;
         }

         if (pFavAct[i])
         {
            delete pFavAct[i];
            pFavAct[i] = NULL;
         }
      }
   }
}

void CChannelsEPGdlg::savePositions()
{
   // -------------------------------------------
   // save gui settings ...
   // -------------------------------------------
   if (windowState() != Qt::WindowMaximized)
   {
      pSettings->SaveWindowRect(geometry());
      pSettings->SetIsMaximized(false);
   }
   else
   {
      pSettings->SetIsMaximized(true);
   }

#ifndef INCLUDE_LIBVLC
   pSettings.SaveSplitterSizes("spChanEpg", ui->vSplitterChanEpg->sizes());
#else  /* ifdef INCLUDE_LIBVLC */
   pSettings->SaveSplitterSizes("spVChanEpg", ui->vSplitterChanEpg->sizes());
#endif /* INCLUDE_LIBVLC */

   pSettings->SetCustFontSize(iFontSzChg);
   pSettings->SaveFavourites(lFavourites);
}

void CChannelsEPGdlg::CleanContextMenu()
{
   for (int i = 0; i < MAX_NO_FAVOURITES; i++)
   {
      if (pContextAct[i] != NULL)
      {
         favContext.removeAction(pContextAct[i]);
         delete pContextAct[i];
         pContextAct[i] = NULL;
      }
   }
}

void CChannelsEPGdlg::SetProgress (const uint &start, const uint &end)
{
   int iPercent = 0;
   int iNow;

   if (start && end)
   {
      int iLength  = (int)(end - start);
      iNow     = (int)(QDateTime::currentDateTime().toTime_t() - start);

      // error check (div / 0 PC doesn't like ;-) ) ...
      if ((iNow > 0) && (iLength > 0))
      {
         // get percent ...
         iPercent  = (int)((iNow * 100) / iLength);
      }
   }

   pProgressBar->setValue(iPercent);
}

////////////////////////////////////////////////////////////////////////////////
//             Functions needed for connection with the MainWindow            //                               //
////////////////////////////////////////////////////////////////////////////////

QMap<int, cparser::SChan>* CChannelsEPGdlg::getChanMap()
{
    return &chanMap;
}

CEpgBrowser* CChannelsEPGdlg::getEpgBrowser()
{
    return ui->textEpg;
}

CVodBrowser* CChannelsEPGdlg::getVodBrowser()
{
    return ui->vodBrowser;
}

QComboBox* CChannelsEPGdlg::getCbxChannelGroup()
{
    return ui->cbxChannelGroup;
}

QComboBox* CChannelsEPGdlg::getCbxSearchArea()
{
    return ui->cbxSearchArea;
}
QLabel* CChannelsEPGdlg::getLabChanIcon()
{
    return ui->labChanIcon;
}

QLabel* CChannelsEPGdlg::getLabCurrDay()
{
    return ui->labCurrDay;
}

QTabBar* CChannelsEPGdlg::getNavBar()
{
    return pEpgNavbar;
}

QLabel* CChannelsEPGdlg::getLabChanName()
{
    return ui->labChanName;
}

QComboBox* CChannelsEPGdlg::getCbxGenre()
{
    return ui->cbxGenre;
}

void CChannelsEPGdlg::setTrigger(CWaitTrigger *pTrig)
{
    pTrigger =pTrig;
}

void CChannelsEPGdlg::setProgressBar(QProgressBar* pProgBar)
{
    pProgressBar = pProgBar;
}

void CChannelsEPGdlg::setTextEpgShort(CEpgBrowser* pEpgShort)
{
    pTextEpgShort = pEpgShort;
}

QList<int>* CChannelsEPGdlg::getListFav()
{
    return &lFavourites;
}

int CChannelsEPGdlg::getEpgOffset()
{
    return iEpgOffset;
}

void CChannelsEPGdlg::setStatusBar(QStatusBar *pStBar)
{
    pStatusBar = pStBar;
}

QTabWidget* CChannelsEPGdlg::getTabEpgVOD()
{
    return ui->tabEpgVod;
}

void CChannelsEPGdlg::setSettings(CSettingsDlg *pSett)
{
    pSettings = pSett;
}

QStandardItemModel* CChannelsEPGdlg::getModel()
{
    return pModel;
}

QListView* CChannelsEPGdlg::getChannelList()
{
    return ui->channelList;
}

QString CChannelsEPGdlg::createTooltip (const QString & name, const QString & prog, uint start, uint end)
{
   // create tool tip with programm info ...
   QString sToolTip = PROG_INFO_TOOL_TIP;
   sToolTip.replace(TMPL_PROG, tr("Program:"));
   sToolTip.replace(TMPL_START, tr("Start:"));
   sToolTip.replace(TMPL_END, tr("End:"));

   sToolTip = sToolTip.arg(name).arg(prog)
               .arg(QDateTime::fromTime_t(start).toString(DEF_TIME_FORMAT))
               .arg(QDateTime::fromTime_t(end).toString(DEF_TIME_FORMAT));

   return sToolTip;
}

int CChannelsEPGdlg::getCurrentCid()
{
   QModelIndex idx = ui->channelList->currentIndex();
   int         cid = qvariant_cast<int>(idx.data(channellist::cidRole));

   return cid;
}

void CChannelsEPGdlg::setCbxTimeShift(QComboBox *pCmxTiSh)
{
    pCbxTimeShift = pCmxTiSh;
}

void CChannelsEPGdlg::updateFavourites()
{
    // add favourites ...
    for (int i = 0; i < MAX_NO_FAVOURITES; i++)
    {
       if (pFavAct[i] != NULL)
       {
          pSettings->updateShortcutDescr(tr("Favourite %1").arg(i + 1),
                               QString("pFavAct[%1]").arg(i),
                               SLOT(slotHandleFavAction(QAction*)));
       }
    }
}
