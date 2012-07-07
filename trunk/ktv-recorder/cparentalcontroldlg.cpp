#include "cparentalcontroldlg.h"
#include "ui_cparentalcontroldlg.h"
#include <QRadioButton>

// log file functions ...
extern CLogFile VlcLog;

// for folders ...
extern CDirStuff *pFolders;

// storage db ...
extern CVlcRecDB *pDb;

CParentalControlDlg::CParentalControlDlg(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CParentalControlDlg)
{
    m_ui->setupUi(this);

    pParser         = NULL;
    pCmdQueue       = NULL;
    pAccountInfo    = NULL;
}

CParentalControlDlg::~CParentalControlDlg()
{
    delete m_ui;
}

void CParentalControlDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
       {
          m_ui->retranslateUi(this);
       }
       break;
    default:
        break;
    }
}

void CParentalControlDlg::on_btnEnterManager_clicked()
{
    sTempPasswd = m_ui->linePasswd->text();
    m_ui->linePasswd->clear();
    pCmdQueue->TriggerRequest(Kartina::REQ_CHANLIST_ALL, sTempPasswd);
}

void CParentalControlDlg::on_linePasswd_returnPressed()
{
   on_btnEnterManager_clicked();
}

void CParentalControlDlg::on_btnSaveExitManager_clicked()
{
    //////////////////////////////////////////////////
    // Channel Manager ...
    //////////////////////////////////////////////////

    // find differences ...
    QVector<int>     toShow, toHide, nowShow;
    QListWidgetItem *pItem;
    QStringList      ids;
    int              i, cid;

    // build show vector with actual values ...
    for (i = 0; i < channelVector.count(); i++)
    {
       if (!channelVector[i].bIsGroup)
       {
          if (!channelVector[i].bIsHidden)
          {
             nowShow.append(channelVector[i].iId);
          }
       }
    }

    // create hide and show vector with changed values only ...
    for (i = 0; i < m_ui->listHide->count(); i++)
    {
       pItem = m_ui->listHide->item(i);
       cid   = qvariant_cast<int>(pItem->data(Qt::UserRole));

       if (pItem->isSelected())
       {
          // means hide ...
          if (nowShow.contains(cid))
          {
             toHide.append(cid);
          }
       }
       else
       {
          // means show ...
          if (!nowShow.contains(cid))
          {
             toShow.append(cid);
          }
       }
    }

    // enqueue channel list changes ...
    if (toHide.count())
    {
       for (i = 0; i < toHide.count(); i++)
       {
          ids << QString::number(toHide[i]);
       }
       pCmdQueue->TriggerRequest(Kartina::REQ_SETCHAN_HIDE, ids.join(","), sTempPasswd);
    }

    ids.clear();
    if (toShow.count())
    {
       for (i = 0; i < toShow.count(); i++)
       {
          ids << QString::number(toShow[i]);
       }
       pCmdQueue->TriggerRequest(Kartina::REQ_SETCHAN_SHOW, ids.join(","), sTempPasswd);
    }

    if (toHide.count() || toShow.count())
    {
       // request new channel list ...
  //     pCmdQueue->TriggerRequest(Kartina::REQ_CHANNELLIST);

       QMessageBox::warning(this, tr("Warning!"),
                             tr("For activating your changes in Channel Manager\n"
                                "please restart the program."));

    }

    //////////////////////////////////////////////////
    // VOD Manager ...
    //////////////////////////////////////////////////
    if (pAccountInfo->bHasVOD)
    {
        QVBoxLayout  *pMainLayout, *pChildLayout;
        QLayoutItem  *child;
        QRadioButton *pRadShow, *pRadHide, *pRadPass;
        QString       sRules, sAccess;
        pMainLayout = (QVBoxLayout *)m_ui->scrollAreaContents->layout();

        for (i = 0; i < pMainLayout->count(); i++)
        {
           // layout was buildt from rates vector so we can
           // assume that the index of the layout is equal
           // to the index in the rates vector ...
           if ((child = pMainLayout->itemAt(i)) != 0)
           {
              // simple check ...
              if (vodRatesVector[i].sGenre == child->widget()->objectName())
              {
                 sAccess      = "";
                 pChildLayout = (QVBoxLayout *)child->widget()->layout();

                 // assume the order as created ...
                 pRadShow = (QRadioButton *)pChildLayout->itemAt(1)->widget();
                 pRadHide = (QRadioButton *)pChildLayout->itemAt(2)->widget();
                 pRadPass = (QRadioButton *)pChildLayout->itemAt(3)->widget();

                 if (pRadShow->isChecked())
                 {
                    sAccess = "show";
                 }
                 else if (pRadHide->isChecked())
                 {
                    sAccess = "hide";
                 }
                 else if (pRadPass->isChecked())
                 {
                    sAccess = "pass";
                 }

                 if ((sAccess != "") && (sAccess != vodRatesVector[i].sAccess))
                 {
                    sRules += QString("&%1=%2")
                          .arg(vodRatesVector[i].sGenre)
                          .arg(sAccess);
                 }
             }
          }
       }

       if (sRules != "")
       {
          mInfo(tr("Changed VOD Rate: %1").arg(sRules));
          pCmdQueue->TriggerRequest(Kartina::REQ_SET_VOD_MANAGER, sRules, sTempPasswd);
       }
    }

    QTimer::singleShot(1000, this, SLOT(slotLockParentalManager()));
}

void CParentalControlDlg::on_btnChgPCode_clicked()
{
   // precheck ...
   QRegExp rx("[^0-9]+");
   QString sOldPCode = m_ui->lineOldPCode->text();
   QString sNewPCode = m_ui->lineNewPCode->text();
   QString sConPCode = m_ui->lineConfirmPCode->text();

   if (   (sNewPCode != sOldPCode) // new and old codes are not equal
       && (sNewPCode == sConPCode) // new and confirm are equal
       && (sNewPCode.count() > 0) // there is a new password
       && (sNewPCode.indexOf(rx) == -1) ) // there are only numbers
   {
      // disable dialog items while we're settings ...
      m_ui->lineOldPCode->setDisabled(true);
      m_ui->lineNewPCode->setDisabled(true);
      m_ui->lineConfirmPCode->setDisabled(true);
      m_ui->btnChgPCode->setDisabled(true);

      pCmdQueue->TriggerRequest(Kartina::REQ_SET_PCODE, sOldPCode, sNewPCode);
   }
   else
   {
       QMessageBox::critical(this, tr("Error!"),
                             tr("<b>Please check the data entered.</b>\n"
                                "<br /> <br />\n"
                                "To change the parent code make sure:\n"
                                "<ul>\n"
                                "<li>The old code is correct.</li>\n"
                                "<li>The new code and the old code are not equal.</li>\n"
                                "<li>The new code and the confirm code are equal.</li>\n"
                                "<li>The new code isn't empty.</li>\n"
                                "<li>The new code contains <b style='color: red;'>numbers only</b>.</li>\n"
                                "</ul>\n"));

       // clear form ...
       m_ui->lineOldPCode->clear();
       m_ui->lineNewPCode->clear();
       m_ui->lineConfirmPCode->clear();
   }
}

void CParentalControlDlg::slotLockParentalManager()
{
    sTempPasswd = "";
    m_ui->stackedWidget->setCurrentIndex(0);
    m_ui->tabWidget->setTabIcon(0, QIcon(":/access/locked"));
}

void CParentalControlDlg::slotBuildChanManager(const QString &str)
{
    QString          sLogo;
    QListWidgetItem *pItem;

    channelVector.clear();

    if (!pParser->parseChannelList(str, channelVector, false))
    {
       m_ui->listHide->clear();

       for (int i = 0; i < channelVector.count(); i++)
       {
          if (!channelVector[i].bIsGroup)
          {
             sLogo = QString("%1/%2.gif").arg(pFolders->getLogoDir()).arg(channelVector[i].iId);
             pItem = new QListWidgetItem (QIcon(sLogo), channelVector[i].sName);

             // save channel id in list item ...
             pItem->setData(Qt::UserRole, channelVector[i].iId);

             m_ui->listHide->addItem(pItem);

             if (channelVector[i].bIsHidden)
             {
                pItem->setSelected(true);
             }
          }
       }

       if (pAccountInfo->bHasVOD)
       {
           // request vod manager data ...
           pCmdQueue->TriggerRequest(Kartina::REQ_GET_VOD_MANAGER, sTempPasswd);
       }
       else
       {
           // show manager widget ...
           m_ui->stackedWidget->setCurrentIndex(1);
           m_ui->tabWidget->setTabIcon(m_ui->tabWidget->currentIndex(), QIcon(":/access/unlocked"));
       }
    }
}

void CParentalControlDlg::slotBuildVodManager(const QString &str)
{
    vodRatesVector.clear();

    QLayout      *pLayout = m_ui->scrollAreaContents->layout();
    QVBoxLayout  *pVMainLayout, *pVChildLayout;
    QLabel       *pTitle;
    QFrame       *pChildWidget;
    QRadioButton *pRadHide, *pRadShow, *pRadPass;
    QString       sLabel;
    QFont         font;
    QMap<QString, QString> transMap;

    // help translate the vod manager strings ...
    transMap.insert("blood",    tr("blood"));
    transMap.insert("violence", tr("violence"));
    transMap.insert("obscene",  tr("obscene"));
    transMap.insert("obsence",  tr("obscene")); // fix a small typo in API ...
    transMap.insert("porn",     tr("porn"));
    transMap.insert("horror",   tr("horror"));

    // clear layout ...
    if (pLayout)
    {
       QLayoutItem *child;
       while ((child = pLayout->takeAt(0)) != 0)
       {
          child->widget()->deleteLater();
          delete child;
       }
       delete pLayout;
    }

    if (!pParser->parseVodManager(str, vodRatesVector))
    {
       pVMainLayout = new QVBoxLayout();

       // make forms for every rate ...
       for (int i = 0; i < vodRatesVector.count(); i++)
       {
           // translate label ...
           sLabel    = transMap.value(vodRatesVector[i].sGenre, vodRatesVector[i].sGenre);

          // make label better looking ...
          sLabel[0] = sLabel[0].toUpper();
          sLabel   += ":";

          // create the whole bunch new widgets needed in this form ...
          pChildWidget  = new QFrame();
          pVChildLayout = new QVBoxLayout();
          pTitle        = new QLabel(sLabel);
          pRadHide      = new QRadioButton (tr("hide"));
          pRadShow      = new QRadioButton (tr("show"));
          pRadPass      = new QRadioButton (tr("password protected"));

          // store the name of the form so we can check it later ...
          pChildWidget->setObjectName(vodRatesVector[i].sGenre);

          // make label bold ...
          font = pTitle->font();
          font.setBold(true);
          pTitle->setFont(font);

          // set spacing ...
          pVChildLayout->setSpacing(2);

          // add all widgets to layout ...
          pVChildLayout->addWidget(pTitle);
          pVChildLayout->addWidget(pRadShow);
          pVChildLayout->addWidget(pRadHide);
          pVChildLayout->addWidget(pRadPass);

          // set layout to form ...
          pChildWidget->setLayout(pVChildLayout);

          // mark radio button as needed ...
          if (vodRatesVector[i].sAccess == "hide")
          {
             pRadHide->setChecked(true);
          }
          else if (vodRatesVector[i].sAccess == "show")
          {
             pRadShow->setChecked(true);
          }
          else if (vodRatesVector[i].sAccess == "pass")
          {
             pRadPass->setChecked(true);
          }

          // add form to main layout ...
          pVMainLayout->addWidget(pChildWidget);
       }

       // show what we've done ...
       m_ui->scrollAreaContents->setLayout(pVMainLayout);
       m_ui->stackedWidget->setCurrentIndex(1);
       m_ui->tabWidget->setTabIcon(m_ui->tabWidget->currentIndex(), QIcon(":/access/unlocked"));
    }
}

void CParentalControlDlg::slotNewPCodeSet()
{
   // internally store changed pcode ...
   sTempPasswd = m_ui->lineNewPCode->text();

   // save to database the new ErosPasswd . . .
   pDb->setPassword("ErosPasswdEnc", sTempPasswd);

   slotEnablePCodeForm();

   QMessageBox::information(this, tr("Information"), tr("Parent Code successfully changed."));
   mInfo(tr("Parent Code successfully changed."));
   // on error we'll get a message box from the XML parser ...
}

void CParentalControlDlg::slotEnablePCodeForm()
{
   // clear form ...
   m_ui->lineOldPCode->clear();
   m_ui->lineNewPCode->clear();
   m_ui->lineConfirmPCode->clear();

   // enable items ...
   m_ui->lineOldPCode->setEnabled(true);
   m_ui->lineNewPCode->setEnabled(true);
   m_ui->lineConfirmPCode->setEnabled(true);
   m_ui->btnChgPCode->setEnabled(true);
}

void CParentalControlDlg::setXmlParser(CKartinaXMLParser *parser)
{
   pParser = parser;
}

void CParentalControlDlg::setWaitTrigger(CWaitTrigger *pTrigger)
{
   pCmdQueue = pTrigger;
}

void CParentalControlDlg::setAccountInfo(const cparser::SAccountInfo *pInfo)
{
    pAccountInfo = pInfo;
}

