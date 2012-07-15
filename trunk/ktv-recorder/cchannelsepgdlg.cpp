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
    connect (ui->channelList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentChannelChanged(QModelIndex)));
    connect (ui->hFrameFav, SIGNAL(sigAddFav(int)), this, SLOT(slotAddFav(int)));

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
        pModel = NULL;
    }

    if (pDelegate)
    {
        delete pDelegate;
        pDelegate = NULL;
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
    ui->channelList->scrollTo(idx, QAbstractItemView::PositionAtTop);
}

void CChannelsEPGdlg::on_cbxGenre_activated(int index)
{
    // check for vod favourites ...
    QString sType = ui->cbxLastOrBest->itemData(ui->cbxLastOrBest->currentIndex()).toString();
    int     iGid  = ui->cbxGenre->itemData(index).toInt();
    QUrl    url;

    if (sType == "vodfav")
    {
       // set filter cbx to "last"  ...
       ui->cbxLastOrBest->setCurrentIndex(0);
       sType = "last";
    }

    url.addQueryItem("type", sType);

    if (iGid != -1)
    {
       url.addQueryItem("genre", QString::number(iGid));
    }

    pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));
}

void CChannelsEPGdlg::on_cbxLastOrBest_activated(int index)
{
    QString sType = ui->cbxLastOrBest->itemData(index).toString();

    if (sType == "vodfav")
    {
        pTrigger->TriggerRequest(Kartina::REQ_GET_VOD_FAV);
    }
    else
    {
       int  iGid  = ui->cbxGenre->itemData(ui->cbxGenre->currentIndex()).toInt();
       QUrl url;

       url.addQueryItem("type", sType);

       if (iGid != -1)
       {
          url.addQueryItem("genre", QString::number(iGid));
       }

       pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));
    }
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
    int     iGid;
    QString sType;
    QUrl    url;

    if (ui->lineVodSearch->text() != "")
    {
       url.addQueryItem("type", "text");

       // when searching show up to 100 results ...
       url.addQueryItem("nums", QString::number(100));
       url.addQueryItem("query", ui->lineVodSearch->text());

       iGid = ui->cbxGenre->itemData(ui->cbxGenre->currentIndex()).toInt();

       if (iGid != -1)
       {
          url.addQueryItem("genre", QString::number(iGid));
       }
    }
    else
    {
       // no text means normal list ...
       iGid  = ui->cbxGenre->itemData(ui->cbxGenre->currentIndex()).toInt();
       sType = ui->cbxLastOrBest->itemData(ui->cbxLastOrBest->currentIndex()).toString();

       // make sure type is supported ...
       if (sType == "vodfav")
       {
           sType = "last";
           ui->cbxLastOrBest->setCurrentIndex(0);
       }

       url.addQueryItem("type", sType);

       if (iGid != -1)
       {
          url.addQueryItem("genre", QString::number(iGid));
       }
    }

    pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));
}

void CChannelsEPGdlg::on_cbxSites_activated(int index)
{
    // something changed ... ?
    if ((index + 1) != pGenreInfo->iPage)
    {
       QUrl    url;
       QString sType  = ui->cbxLastOrBest->itemData(ui->cbxLastOrBest->currentIndex()).toString();
       int     iGenre = ui->cbxGenre->itemData(ui->cbxGenre->currentIndex()).toInt();

       url.addQueryItem("type", sType);
       url.addQueryItem("page", QString::number(index + 1));

       if (iGenre != -1)
       {
          url.addQueryItem("genre", QString::number(iGenre));
       }

       pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));
    }

}

void CChannelsEPGdlg::on_btnPrevSite_clicked()
{
    QUrl    url;
    QString sType  = ui->cbxLastOrBest->itemData(ui->cbxLastOrBest->currentIndex()).toString();
    int     iGenre = ui->cbxGenre->itemData(ui->cbxGenre->currentIndex()).toInt();
    int     iPage  = ui->cbxSites->currentIndex() + 1;

    url.addQueryItem("type", sType);
    url.addQueryItem("page", QString::number(iPage - 1));

    if (iGenre != -1)
    {
       url.addQueryItem("genre", QString::number(iGenre));
    }

    pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));

}

void CChannelsEPGdlg::on_btnNextSite_clicked()
{
    QUrl    url;
    QString sType  = ui->cbxLastOrBest->itemData(ui->cbxLastOrBest->currentIndex()).toString();
    int     iGenre = ui->cbxGenre->itemData(ui->cbxGenre->currentIndex()).toInt();
    int     iPage  = ui->cbxSites->currentIndex() + 1;

    url.addQueryItem("type", sType);
    url.addQueryItem("page", QString::number(iPage + 1));

    if (iGenre != -1)
    {
       url.addQueryItem("genre", QString::number(iGenre));
    }

    pTrigger->TriggerRequest(Kartina::REQ_GETVIDEOS, QString(url.encodedQuery()));

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
         QDateTime epgTime  = QDateTime::currentDateTime().addDays(iEpgOffset);
         int       iDay     = epgTime.date().dayOfWeek() - 1;
         int       iOffBack = iEpgOffset;

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
            correctEpgOffset ();

            if(iOffBack != iEpgOffset)
            {
                pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
            }
            else
            {
                // no change -> revert nav button ...
                pEpgNavbar->setCurrentIndex(iDay);
            }
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
         int iOffBack = iEpgOffset;
         iEpgOffset  -= 7 + iActDay;

         correctEpgOffset();

         if (iOffBack != iEpgOffset)
         {
             pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
         }

      }
}

void CChannelsEPGdlg::slotbtnNext_clicked()
{
    int cid = getCurrentCid();

      if (chanMap.contains(cid))
      {
         // set actual day in next week to munday ...
         int iActDay  = pEpgNavbar->currentIndex();
         int iOffBack = iEpgOffset;
         iEpgOffset  += 7 - iActDay;

         correctEpgOffset();

         if (iOffBack != iEpgOffset)
         {
             pTrigger->TriggerRequest(Kartina::REQ_EPG, cid, iEpgOffset);
         }
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
       if (!lFavourites.contains(iCid))
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
   }
   else if (action == kartinafav::FAV_DEL)
   {
       if (lFavourites.contains(iCid))
       {
          // remove favourite ...
          lFavourites.removeOne(iCid);

          HandleFavourites();
          bSaveCng = true;
       }
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
             ui->channelList->scrollTo(idx, QAbstractItemView::PositionAtTop);
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
         pContextAct[i]->setText(tr("Remove \"%1\" from favourites").arg(chanMap[lFavourites[i]].sName));
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

      // update short info if we're in live mode
      if (showInfo.showType() == ShowInfo::Live)
      {
        pTextEpgShort->setHtml(QString(TMPL_BACKCOLOR)
                                  .arg("rgb(255, 254, 212)")
                                  .arg(CShowInfo::createTooltip(entry.sName, entry.sProgramm, entry.uiStart, entry.uiEnd)));
      }


      // quick'n'dirty timeshift hack ...
      if (entry.vTs.count() <= 2) // no timeshift available ...
      {
         ui->textEpg->SetTimeShift(0);
      }
      else
      {
         iTs = pSettings->getTimeShift();

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

void CChannelsEPGdlg::slotAddFav(int cid)
{
   if (!lFavourites.contains(cid))
   {
      if (lFavourites.count() < MAX_NO_FAVOURITES)
      {
         lFavourites.append(cid);
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
}

////////////////////////////////////////////////////////////////////////////////
//                             normal functions                               //
////////////////////////////////////////////////////////////////////////////////
void CChannelsEPGdlg::touchLastOrBestCbx()
{
    // fill / update search area combo box ...
    if (!ui->cbxLastOrBest->count())
    {
        ui->cbxLastOrBest->addItem(tr("Newest"), "last");
        ui->cbxLastOrBest->addItem(tr("Best"), "best");
        ui->cbxLastOrBest->addItem(tr("My Favourites"), "vodfav");
        ui->cbxLastOrBest->setCurrentIndex(0);
    }
    else
    {
        int idx;

        if ((idx = ui->cbxLastOrBest->findData("last")) > -1)
        {
            ui->cbxLastOrBest->setItemText(idx, tr("Newest"));
        }

        if ((idx = ui->cbxLastOrBest->findData("best")) > -1)
        {
            ui->cbxLastOrBest->setItemText(idx, tr("Best"));
        }

        if ((idx = ui->cbxLastOrBest->findData("vodfav")) > -1)
        {
            ui->cbxLastOrBest->setItemText(idx, tr("My Favourites"));
        }
    }
}

void CChannelsEPGdlg::touchVodNavBar(const cparser::SGenreInfo &gInfo)
{
    // delete sites ...
    ui->cbxSites->clear();

    // (de-)activate prev button ...
    if (gInfo.iPage == 1)
    {
       ui->btnPrevSite->setDisabled(true);
    }
    else
    {
       ui->btnPrevSite->setEnabled(true);
    }

    int iSites = gInfo.iTotal / VIDEOS_PER_SITE;

    if (gInfo.iTotal % VIDEOS_PER_SITE)
    {
       iSites ++;
    }

    for (int i = 1; i <= iSites; i++)
    {
       ui->cbxSites->addItem(QString::number(i));
    }

    ui->cbxSites->setCurrentIndex(gInfo.iPage - 1);

    if (iSites == gInfo.iPage)
    {
       ui->btnNextSite->setDisabled(true);
    }
    else
    {
       ui->btnNextSite->setEnabled(true);
    }
}

void CChannelsEPGdlg::touchGenreCbx()
{
   if (ui->cbxGenre->count())
   {
      int idx;

      if ((idx = ui->cbxGenre->findData((int)-1)) > -1)
      {
         ui->cbxGenre->setItemText(idx, tr("All"));
      }
   }
}

void CChannelsEPGdlg::initDialog (bool bInit)
{
    bool ok = false;

    // fill type combo box ...
    touchLastOrBestCbx();

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
   QPushButton *pBtn;

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
      pBtn = new QPushButton;
      pBtn->setIcon(QIcon(":png/back"));
      pBtn->setFlat(true);
      pBtn->setAutoDefault(false);
      pBtn->setMaximumHeight(EPG_NAVBAR_HEIGHT);
      pBtn->setMaximumWidth(EPG_NAVBAR_HEIGHT);
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
      pBtn = new QPushButton;
      pBtn->setIcon(QIcon(":png/next"));
      pBtn->setFlat(true);
      pBtn->setAutoDefault(false);
      pBtn->setMaximumHeight(EPG_NAVBAR_HEIGHT);
      pBtn->setMaximumWidth(EPG_NAVBAR_HEIGHT);
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
      pBtn = (QPushButton *)ui->hLayoutEpgNavi->itemAt(iIdx)->widget();
      pBtn->setToolTip(tr("1 week backward"));

      // next button ...
      iIdx = ui->hLayoutEpgNavi->count() - 1;
      pBtn = (QPushButton *)ui->hLayoutEpgNavi->itemAt(iIdx)->widget();
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

void CChannelsEPGdlg::correctEpgOffset()
{
    if (iEpgOffset > 7)
    {
        iEpgOffset = 7;
    }
    else if (iEpgOffset < -14)
    {
        iEpgOffset = -14;
    }
}

////////////////////////////////////////////////////////////////////////////////
//             Functions needed for connection with the MainWindow            //                               //
////////////////////////////////////////////////////////////////////////////////

QChanMap* CChannelsEPGdlg::getChanMap()
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

QComboBox* CChannelsEPGdlg::getCbxLastOrBest()
{
    return ui->cbxLastOrBest;
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

void CChannelsEPGdlg::setGenreInfo(cparser::SGenreInfo *pGenrInf)
{
    pGenreInfo = pGenrInf;
}

void CChannelsEPGdlg::setEpgOffset(int iEpgOffs)
{
    iEpgOffset = iEpgOffs;
}

void CChannelsEPGdlg::activateVOD()
{
    on_cbxGenre_activated(0);
}

CEpgBrowser* CChannelsEPGdlg::getTextEpg()
{
    return ui->textEpg;
}

int CChannelsEPGdlg::getCurrentCid()
{
   QModelIndex idx = ui->channelList->currentIndex();
   int         cid = qvariant_cast<int>(idx.data(channellist::cidRole));

   return cid;
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
