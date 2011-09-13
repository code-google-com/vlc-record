/*------------------------------ Information ---------------------------*//**
 *
 *  $HeadURL$
 *
 *  @file     qftsettings.cpp
 *
 *  @author   Jo2003
 *
 *  @date     13.09.2011
 *
 *  $Id$
 *
 *///------------------------- (c) 2011 by Jo2003  --------------------------
#include "qftsettings.h"
#include "ui_qftsettings.h"

// for folders ...
extern CDirStuff *pFolders;

// storage db ...
extern CVlcRecDB *pDb;

//---------------------------------------------------------------------------
//
//! \brief   constructs QFTSettings object
//
//! \author  Jo2003
//! \date    13.09.2011 / 10:00
//
//! \param   parent pointer to parent widget
//
//! \return  --
//---------------------------------------------------------------------------
QFTSettings::QFTSettings(QWidget *parent, QTranslator *pTrans) :
   QDialog(parent),
   ui(new Ui::QFTSettings),
   pTranslator(pTrans)
{
    ui->setupUi(this);

    // set company name ...
    QString s = ui->groupAccount->title();
    ui->groupAccount->setTitle(s.arg(COMPANY_NAME));

    // set default user and password ...
    ui->lineUsr->setText("144");
    ui->linePass->setText("441");

    resize(sizeHint());
}

//---------------------------------------------------------------------------
//
//! \brief   destroys QFTSettings object
//
//! \author  Jo2003
//! \date    13.09.2011 / 10:00
//
//! \param   --
//
//! \return  --
//---------------------------------------------------------------------------
QFTSettings::~QFTSettings()
{
    delete ui;
}

//---------------------------------------------------------------------------
//
//! \brief   catch incoming event
//
//! \author  Jo2003
//! \date    13.09.2011 / 11:00
//
//! \param   e pointer to incoming event
//
//! \return  --
//---------------------------------------------------------------------------
void QFTSettings::changeEvent(QEvent *e)
{
   QDialog::changeEvent(e);

   switch (e->type())
   {
   case QEvent::LanguageChange:
      {
         // save current index from comboboxes ...
         int iLanIdx = ui->cbxLanguage->currentIndex();

         ui->retranslateUi(this);

         // re-set index to comboboxes ...
         ui->cbxLanguage->setCurrentIndex(iLanIdx);

         // set company name ...
         QString s = ui->groupAccount->title();
         ui->groupAccount->setTitle(s.arg(COMPANY_NAME));
      }
      break;

   default:
       break;
   }
}

//---------------------------------------------------------------------------
//
//! \brief   language changed
//
//! \author  Jo2003
//! \date    13.09.2011 / 10:05
//
//! \param   str text label for new language
//
//! \return  --
//---------------------------------------------------------------------------
void QFTSettings::on_cbxLanguage_currentIndexChanged(QString str)
{
   // set language as read ...
   pTranslator->load(QString("lang_%1").arg(str), pFolders->getLangDir());
}

//---------------------------------------------------------------------------
//
//! \brief   save first time settings
//
//! \author  Jo2003
//! \date    13.09.2011 / 11:40
//
//! \param   --
//
//! \return  --
//---------------------------------------------------------------------------
void QFTSettings::saveFTSettings()
{
   // use settings from form ...
   pDb->setValue("User",         ui->lineUsr->text());
   pDb->setValue("Passwd",       ui->linePass->text());
   pDb->setValue("ErosPasswd",   ui->lineErosPass->text());
   pDb->setValue("AllowAdult",   (int)ui->checkAdult->checkState());
   pDb->setValue("Language",     ui->cbxLanguage->currentText());

   // set commonly used settings ...
   pDb->setValue("TargetDir",    tr("%1/Videos").arg(QDir::homePath()));
   pDb->setValue("FixTime",      (int)Qt::Checked);  // fix time
   pDb->setValue("Refresh",      (int)Qt::Checked);  // refresh channel list
   pDb->setValue("RefIntv",      5);                 // refresh interval
   pDb->setValue("ExtChanList",  (int)Qt::Checked);  // show extended channel list
   pDb->setValue("PlayerModule", "5_libvlc.mod");    // default player module
   pDb->setValue("HttpCache",    8000);              // 8 sec. cache

   // set windows size and position ...
   pDb->setValue("WndRect",        "88;81;1220;752");
   pDb->setValue("spVChanEpg",     "264;388;");
   pDb->setValue("spVChanEpgPlay", "657;536;");
   pDb->setValue("spHPlay",        "444;191;");


#ifdef Q_OS_WIN32
   pDb->setValue("ShutdwnCmd", "shutdown.exe -s -f -t 5");
#endif
}

//---------------------------------------------------------------------------
//
//! \brief   button "reset" was pressed
//
//! \author  Jo2003
//! \date    13.09.2011 / 12:50
//
//! \param   --
//
//! \return  --
//---------------------------------------------------------------------------
void QFTSettings::on_pushReset_clicked()
{
   ui->checkAdult->setCheckState(Qt::Unchecked);
   ui->lineErosPass->setText("");
   ui->lineUsr->setText("144");
   ui->linePass->setText("441");
}

//---------------------------------------------------------------------------
//
//! \brief   button "save" was pressed
//
//! \author  Jo2003
//! \date    13.09.2011 / 12:50
//
//! \param   --
//
//! \return  --
//---------------------------------------------------------------------------
void QFTSettings::on_pushSave_clicked()
{
   // save content ...
   saveFTSettings();
   accept();
}
