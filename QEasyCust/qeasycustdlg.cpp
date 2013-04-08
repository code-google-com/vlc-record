/*------------------------------ Information ---------------------------*//**
*
*  $HeadURL$
*
*  @file     qeasycustdlg.cpp
*
*  @author   Jo2003
*
*  @date     04.04.2013
*
*  $Id$
*
*/ //----------------- (c) 2013 Jo2003 --------------------------------------
#include "qeasycustdlg.h"
#include "ui_qeasycustdlg.h"
#include <QString>
#include <QApplication>
#include <QFileDialog>
#include <QPixmap>
#include <QPicture>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>

#include "file_templates.h"

//----------------------------------------------------------------------
//! \fn       QEasyCustDlg
//! \brief    contructor
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    parent (QWidget*) pointer to parent widget
//
//! \return   --
//----------------------------------------------------------------------
QEasyCustDlg::QEasyCustDlg(QWidget *parent) :
   QDialog(parent),
   ui(new Ui::QEasyCustDlg)
{
   ui->setupUi(this);

   setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
   sAppPath = QApplication::applicationDirPath();

   if ((procBuilder = new QProcess(this)) != NULL)
   {
      connect (procBuilder, SIGNAL(readyReadStandardError()), this, SLOT(slotReadProcData()));
      connect (procBuilder, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReadProcData()));
      connect (procBuilder, SIGNAL(finished(int)), this, SLOT(slotStepDone(int)));
      connect (procBuilder, SIGNAL(error(QProcess::ProcessError)), this, SLOT(slotProcError(QProcess::ProcessError)));
   }
}

//----------------------------------------------------------------------
//! \fn       ~QEasyCustDlg
//! \brief    destructor
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
QEasyCustDlg::~QEasyCustDlg()
{
   delete ui;
}

//----------------------------------------------------------------------
//! \fn       showEvent
//! \brief    event when program is shown
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    e (QShowEvent*) pointer to show event
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::showEvent(QShowEvent *e)
{
   e->accept();

   // load last saved customization
   if (settings.contains("LastSave"))
   {
      readValues(settings.value("LastSave").toString());
      showPngImage(ui->labLogo, ui->lineLogoFile->text());
      showPngImage(ui->labBg, ui->lineBgFile->text());
   }
}

//----------------------------------------------------------------------
//! \fn       keyPressEvent
//! \brief    filter some key press events
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    e (QKeyEvent*) pointer to event
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::keyPressEvent(QKeyEvent *e)
{
   if (e->key() == Qt::Key_Escape)
   {
      e->ignore();
   }
   else
   {
      e->accept();
   }
}

//----------------------------------------------------------------------
//! \fn       on_pushGetLogo_clicked
//! \brief    get logo button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushGetLogo_clicked()
{
   QString name = QFileDialog::getOpenFileName(this,
                                               tr("Get Logo File"),
                                               ui->lineLogoFile->text(),
                                               "PNG Image (*.png);;"
                                               "All Files (*.*)");

   if (!name.isEmpty())
   {
      ui->lineLogoFile->setText(name);
      showPngImage(ui->labLogo, name);
   }
}

//----------------------------------------------------------------------
//! \fn       on_pushGetBg_clicked
//! \brief    get background button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushGetBg_clicked()
{
   QString name = QFileDialog::getOpenFileName(this,
                                               tr("Get Background File"),
                                               ui->lineBgFile->text(),
                                               "PNG Image (*.png);;"
                                               "All Files (*.*)");

   if (!name.isEmpty())
   {
      ui->lineBgFile->setText(name);
      showPngImage(ui->labBg, name);
   }
}

//----------------------------------------------------------------------
//! \fn       on_pushGo_clicked
//! \brief    customize button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushGo_clicked()
{
   QString  cmdLine;
   SPatch patchFile;
   QVector<SPatch> vPatchFiles;
   int i;

   patchMap.clear();
   ui->plainTextEdit->clear();

   // which values we really need ...
   if (ui->lineIntName->text().isEmpty()
       || ui->lineOffName->text().isEmpty()
       || ui->lineVersion->text().isEmpty()
       || ui->lineApiSrv->text().isEmpty()
       || ui->lineApiXml->text().isEmpty()
       || ui->lineApiJson->text().isEmpty()
       || ui->lineLogoFile->text().isEmpty()
       || ui->listLang->selectedItems().isEmpty())
   {
      QMessageBox::critical(this, tr("Error: Missing values!"), tr("Missing values at Strings or Images page!"));
      return;
   }

   if (ui->lineCompName->text() == "")
   {
      ui->lineCompName->setText(ui->lineOffName->text());
   }

   // some lines are used more than once - this is historical ...
   patchMap.insert(TMPL_INT_NAME, ui->lineIntName->text());
   patchMap.insert(TMPL_OFF_NAME, ui->lineOffName->text());
   patchMap.insert(TMPL_PROGRAM , ui->lineOffName->text());
   patchMap.insert(TMPL_VERSION, ui->lineVersion->text());
   patchMap.insert(TMPL_UPD_URL, ui->lineUpdUrl->text());
   patchMap.insert(TMPL_API_SRV, ui->lineApiSrv->text());
   patchMap.insert(TMPL_APISERVER, ui->lineApiSrv->text());
   patchMap.insert(TMPL_COMP_NAME, ui->lineCompName->text());
   patchMap.insert(TMPL_SERVICE, ui->lineCompName->text());
   patchMap.insert(TMPL_COMP_LINK, ui->lineCompLink->text());
   patchMap.insert(TMPL_API_XML, ui->lineApiXml->text());
   patchMap.insert(TMPL_API_JSON, ui->lineApiJson->text());

   // prepare process ...
   if (procBuilder)
   {
      // create needed folders ...
      if (!createCleanFolders())
      {
         cmdQueue.clear();

         // merged channels for one session ...
         procBuilder->setProcessChannelMode(QProcess::MergedChannels);

         //////////////////////////////////////////////////////////////////////
         // first create Windows icon ...
         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 64x64 \"%3\" \"%1/%4/%5.ico\"")
               .arg(sAppPath).arg(CONV_EXE).arg(ui->lineLogoFile->text())
               .arg(PATH_ICONS).arg(ui->lineIntName->text());

         cmdQueue << cmdLine;

         //////////////////////////////////////////////////////////////////////
         // create mac icns file ...
         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 128x128 \"%3\" \"%1/%4/128.png\"")
               .arg(sAppPath).arg(CONV_EXE).arg(ui->lineLogoFile->text())
               .arg(PATH_TEMP);

         cmdQueue << cmdLine;

         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 48x48 \"%3\" \"%1/%4/48.png\"")
               .arg(sAppPath).arg(CONV_EXE).arg(ui->lineLogoFile->text())
               .arg(PATH_TEMP);

         cmdQueue << cmdLine;

         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 32x32 \"%3\" \"%1/%4/32.png\"")
               .arg(sAppPath).arg(CONV_EXE).arg(ui->lineLogoFile->text())
               .arg(PATH_TEMP);

         cmdQueue << cmdLine;

         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 16x16 \"%3\" \"%1/%4/16.png\"")
               .arg(sAppPath).arg(CONV_EXE).arg(ui->lineLogoFile->text())
               .arg(PATH_TEMP);

         cmdQueue << cmdLine;

         cmdLine = QString("\"%1/%2\" \"%1/%3/%4.icns\" \"%1/%5/16.png\" \"%1/%5/32.png\" \"%1/%5/48.png\" \"%1/%5/128.png\"")
               .arg(sAppPath).arg(ICNS_EXE).arg(PATH_ICONS).arg(ui->lineIntName->text())
               .arg(PATH_TEMP);

         cmdQueue << cmdLine;

         //////////////////////////////////////////////////////////////////////
         // create live png stuff ...
         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 128x128 -brightness-contrast 60x-50 \"%3\" \"%1/%5/logo_tmp.png\"")
               .arg(sAppPath).arg(CONV_EXE)
               .arg(ui->lineLogoFile->text()).arg(PATH_TEMP);

         cmdQueue << cmdLine;

         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 128x128 \"%3/%4\" \"%5/%6/live_tmp.png\"")
               .arg(sAppPath).arg(CONV_EXE).arg(sAppPath)
               .arg(LIVE_PNG).arg(sAppPath).arg(PATH_TEMP);

         cmdQueue << cmdLine;

         cmdLine = QString("\"%1/%2\" -gravity center \"%1/%3/live_tmp.png\" \"%1/%3/logo_tmp.png\" \"%1/%4/%5/live.png\"")
               .arg(sAppPath).arg(COMP_EXE).arg(PATH_TEMP)
               .arg(PATH_CUST).arg(ui->lineIntName->text());

         cmdQueue << cmdLine;

         //////////////////////////////////////////////////////////////////////
         // program logo ...
         cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 128x128 \"%3\" \"%1/%4/%5/logo.png\"")
               .arg(sAppPath).arg(CONV_EXE).arg(ui->lineLogoFile->text())
               .arg(PATH_CUST).arg(ui->lineIntName->text());

         cmdQueue << cmdLine;

         //////////////////////////////////////////////////////////////////////
         // player background ...
         if (ui->lineBgFile->text() != "")
         {
            QPixmap pix(ui->lineBgFile->text(), "png");
            QSize   sz = pix.size();

            // maximum display size is 350 x 200 ...
            if ((sz.height() > 200) || (sz.width() > 350))
            {
               // we have to resize
               cmdLine = QString("\"%1/%2\" -filter Lanczos -resize 350x200 \"%3\" \"%1/%4/%5/bg.png\"")
                     .arg(sAppPath).arg(CONV_EXE).arg(ui->lineBgFile->text())
                     .arg(PATH_CUST).arg(ui->lineIntName->text());

               cmdQueue << cmdLine;
            }
            else
            {
               // simply copy ...
               if(!QFile::copy(ui->lineBgFile->text(), QString("%1/%2/%3/bg.png")
                           .arg(sAppPath).arg(PATH_CUST).arg(ui->lineIntName->text())))
               {
                  QMessageBox::critical(this, tr("Error copying file!"), tr("Can't copy background image!"));
               }
            }

            patchMap.insert(TMPL_BG_IMAGE, "bg.png");
         }
         else
         {
            patchMap.insert(TMPL_BG_IMAGE, "logo.png");
         }

         enableDisableDlg(false);

         // collect a vector of all files to patch ...

         // cust.xml
         patchFile.src = QString("%1/%2/cust.xml").arg(sAppPath).arg(PATH_TMPL);
         patchFile.trg = QString("%1/%2/%3/cust.xml").arg(sAppPath).arg(PATH_CUST).arg(ui->lineIntName->text());
         vPatchFiles.append(patchFile);

         // cust.qrc
         patchFile.src = QString("%1/%2/cust.qrc").arg(sAppPath).arg(PATH_TMPL);
         patchFile.trg = QString("%1/%2/%3/cust.qrc").arg(sAppPath).arg(PATH_CUST).arg(ui->lineIntName->text());
         vPatchFiles.append(patchFile);

         // install script ...
         patchFile.src = QString("%1/%2/installer.tmpl").arg(sAppPath).arg(PATH_TMPL);
         patchFile.trg = QString("%1/%2/%3.nsi").arg(sAppPath).arg(PATH_INST).arg(ui->lineIntName->text());
         vPatchFiles.append(patchFile);

         // help files ...
         for (i = 0; i < ui->listLang->count(); i++)
         {
            if (ui->listLang->item(i)->isSelected())
            {
               patchFile.src = QString("%1/%2/help_%3.tmpl").arg(sAppPath).arg(PATH_HLP_SRC).arg(ui->listLang->item(i)->text());
               patchFile.trg = QString("%1/%2/help_%3.html").arg(sAppPath).arg(PATH_HLP_SRC).arg(ui->listLang->item(i)->text());
               vPatchFiles.append(patchFile);

               patchFile.src = QString("%1/%2/help_%3.qhp_tmpl").arg(sAppPath).arg(PATH_HLP_SRC).arg(ui->listLang->item(i)->text());
               patchFile.trg = QString("%1/%2/help_%3.qhp").arg(sAppPath).arg(PATH_HLP_SRC).arg(ui->listLang->item(i)->text());
               vPatchFiles.append(patchFile);

               // add commands for help creation ...
               cmdLine = QString("\"%1/%2\" \"%1/%3/help_%4.qhp\" -o \"%1/%3/help_%4.qch\"")
                        .arg(sAppPath).arg(QHG_EXE).arg(PATH_HLP_SRC)
                        .arg(ui->listLang->item(i)->text());

               cmdQueue << cmdLine;

               cmdLine = QString("\"%1/%2\" \"%1/%3/help_%4.qhcp\" -o \"%1/%3/help_%4.qhc\"")
                        .arg(sAppPath).arg(QCG_EXE).arg(PATH_HLP_SRC)
                        .arg(ui->listLang->item(i)->text());

               cmdQueue << cmdLine;

               cmdLine = QString("\"%1/%2\" \"%1/%3/help_%4.qhc\" \"%1/%3/help_%4.qch\" \"%1/%5/\"")
                        .arg(sAppPath).arg(COPY_EXE).arg(PATH_HLP_SRC)
                        .arg(ui->listLang->item(i)->text()).arg(PATH_HLP);

               cmdQueue << cmdLine;

               // add commands for language copy ...
               if (ui->listLang->item(i)->text() != "en")
               {
                  cmdLine = QString("\"%1/%2\" \"%1/%3/lang_%4.qm\" \"%1/%5/\"")
                           .arg(sAppPath).arg(COPY_EXE).arg(PATH_LNG_SRC)
                           .arg(ui->listLang->item(i)->text()).arg(PATH_LNG);

                  cmdQueue << cmdLine;
               }
            }
         }

         patchAllFiles(vPatchFiles);

         // patch program icon ...
         cmdLine = QString("\"%1/%2\" -addoverwrite \"%1/%3/master.exe\", \"%1/%4/%5.exe\", \"%1/%6/%5.ico\", ICONGROUP,IDI_ICON1,1033")
               .arg(sAppPath).arg(RESH_EXE).arg(PATH_BIN).arg(PATH_REL).arg(ui->lineIntName->text()).arg(PATH_ICONS);

         cmdQueue << cmdLine;

         // create customization file ...
         cmdLine = QString("\"%1/%2\" -binary -o \"%1/%3/%4.cust\" \"%1/%5/%4/cust.qrc\"")
               .arg(sAppPath).arg(RCC_EXE).arg(PATH_RES).arg(ui->lineIntName->text())
               .arg(PATH_CUST);

         cmdQueue << cmdLine;

         // run installer ...
         cmdLine = QString("\"%1/%2\" \"%1/%3/%4.nsi\"")
               .arg(sAppPath).arg(NSIS_EXE).arg(PATH_INST)
               .arg(ui->lineIntName->text());

         cmdQueue << cmdLine;

         doit();
      }
   }
}

//----------------------------------------------------------------------
//! \fn       patchAllFiles
//! \brief    patch all files stored in patch vector
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    vecPatches (const QVector<SPatch>&) patch vector
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::patchAllFiles(const QVector<SPatch> &vecPatches)
{
   for (int i = 0; i < vecPatches.count(); i++)
   {
      patchTextFile(vecPatches.at(i).src, vecPatches.at(i).trg);
   }
}

//----------------------------------------------------------------------
//! \fn       patchTextFile
//! \brief    patch text file and save if needed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    file (const QString&) in file to patch
//! \param    saveFile (const QString&) file to save as
//
//! \return   patched file content
//----------------------------------------------------------------------
QString QEasyCustDlg::patchTextFile(const QString &file, const QString& saveFile)
{
   QFile   txtFile(file);
   QString strContent;

   if (txtFile.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      strContent = QString::fromUtf8(txtFile.readAll().constData());

      QStringList sl = patchMap.keys();

      for (int i = 0; i < sl.size(); i++)
      {
         if (strContent.contains(sl.at(i)))
         {
            strContent.replace(sl.at(i), patchMap.value(sl.at(i)));
         }
      }

      txtFile.close();
   }

   if ((saveFile != "") && (strContent != ""))
   {
      txtFile.setFileName(saveFile);

      if (txtFile.open(QIODevice::WriteOnly | QIODevice::Text))
      {
         txtFile.write(strContent.toUtf8());
         txtFile.close();
      }
   }

   return strContent;
}

//----------------------------------------------------------------------
//! \fn       slotReadProcData
//! \brief    add log data send from process
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::slotReadProcData()
{
   addLog(QString::fromLocal8Bit(procBuilder->readAll()));
}

//----------------------------------------------------------------------
//! \fn       addLog
//! \brief    add log string to log widget
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    toLog (const QString&) string to add to log
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::addLog(const QString &toLog)
{
   QString str = ui->plainTextEdit->toPlainText();

   str += toLog;

   // remove app path to make log better readable ...
   str.remove(sAppPath + "/");

   ui->plainTextEdit->setPlainText(str);

   // scroll to end ...
   QTextCursor cursor =  ui->plainTextEdit->textCursor();
   cursor.movePosition(QTextCursor::End);
   ui->plainTextEdit->setTextCursor(cursor);
}

//----------------------------------------------------------------------
//! \fn       createCleanFolders
//! \brief    create and clean needed folders
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   0 --> ok, -1 --> any error
//----------------------------------------------------------------------
int QEasyCustDlg::createCleanFolders()
{
   int         iRet = 0;
   QDir        dir;
   QStringList folders, entries;
   QStringList::iterator it, it2;

   folders << QString("%1/%2/%3").arg(sAppPath).arg(PATH_CUST).arg(ui->lineIntName->text())
           << QString("%1/%2").arg(sAppPath).arg(PATH_ICONS)
           << QString("%1/%2").arg(sAppPath).arg(PATH_REL)
           << QString("%1/%2").arg(sAppPath).arg(PATH_HLP)
           << QString("%1/%2").arg(sAppPath).arg(PATH_PACK)
           << QString("%1/%2").arg(sAppPath).arg(PATH_TEMP)
           << QString("%1/%2").arg(sAppPath).arg(PATH_RES)
           << QString("%1/%2").arg(sAppPath).arg(PATH_LNG);

   for (it = folders.begin(); it != folders.end(); it++)
   {
      dir.setPath(*it);

      if (!dir.exists())
      {
         iRet |= dir.mkpath(dir.absolutePath()) ? 0 : -1;
      }
      else
      {
         if (!(*it).contains(PATH_PACK))
         {
            entries = dir.entryList(QDir::Files);

            for (it2 = entries.begin(); it2 != entries.end(); it2++)
            {
               QFile::remove(dir.absoluteFilePath(*it2));
            }
         }
      }
   }

   return iRet;
}

//----------------------------------------------------------------------
//! \fn       doit
//! \brief    process all commands stored in command queue
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::doit()
{
   QStringList::iterator it;

   if (!cmdQueue.isEmpty())
   {
      it = cmdQueue.begin();
      addLog(tr("Running:\n%1\n").arg(*it));
      procBuilder->start(*it);
      cmdQueue.removeFirst();
   }
   else
   {
      addLog(tr("All done!\n"));
      enableDisableDlg(true);

      QProcess::startDetached(QString("explorer.exe \"%1/%2\"").arg(sAppPath).arg(PATH_PACK).replace("/", "\\"));
   }
}

//----------------------------------------------------------------------
//! \fn       slotStepDone
//! \brief    one step from command queue done
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    pid (int) process id
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::slotStepDone(int pid)
{
   Q_UNUSED(pid)

   int Go = 0;

   if (procBuilder->exitStatus() != QProcess::NormalExit)
   {
      slotProcError(procBuilder->error());
   }
   else
   {
      if (procBuilder->exitCode())
      {
         addLog(tr("Error in last Command, break build!\n"));
         addLog(tr("### Break due to error! ###\n"));
         enableDisableDlg(true);
      }
      else
      {
         Go = 1;
      }
   }

   if (Go)
   {
      doit();
   }
   else
   {
      QMessageBox::critical(this, tr("Build Error!"),
                            tr("Error while customization.\nPlease have a look at the log!"));
   }
}

//----------------------------------------------------------------------
//! \fn       slotProcError
//! \brief    process sent error
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    e (QProcess::ProcessError) error enum
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::slotProcError(QProcess::ProcessError e)
{
#define mkCase(__x__) case __x__: addLog(tr("Error: %1\n").arg(#__x__)); break
   switch(e)
   {
   mkCase(QProcess::FailedToStart);
   mkCase(QProcess::Crashed);
   mkCase(QProcess::Timedout);
   mkCase(QProcess::WriteError);
   mkCase(QProcess::ReadError);
   mkCase(QProcess::UnknownError);
   default:
      break;
   }

   addLog(tr("### Break due to error! ###\n"));
   enableDisableDlg(true);

#undef mkCase
}

//----------------------------------------------------------------------
//! \fn       on_pushQuit_clicked
//! \brief    quit button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushQuit_clicked()
{
   close();
}

//----------------------------------------------------------------------
//! \fn       saveValues
//! \brief    save customization data as template
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   0 --> ok, -1 --> any error
//----------------------------------------------------------------------
int QEasyCustDlg::saveValues()
{
   QString name = QFileDialog::getSaveFileName(this, tr("Save Customization Template"),
                                               ui->lineIntName->text(),
                                               "Cust Templates (*.tmpl);;"
                                               "All Files (*.*)");

   if (!name.isEmpty())
   {
      QFile save(name);
      QString s;
      QStringList sl;

      if (save.open(QIODevice::WriteOnly | QIODevice::Text))
      {
         save.write(QString("OFF_NAME=\"%1\"\n").arg(ui->lineOffName->text()).toUtf8());
         save.write(QString("INT_NAME=\"%1\"\n").arg(ui->lineIntName->text()).toUtf8());
         save.write(QString("COMP_NAME=\"%1\"\n").arg(ui->lineCompName->text()).toUtf8());
         save.write(QString("COMP_LINK=\"%1\"\n").arg(ui->lineCompLink->text()).toUtf8());
         save.write(QString("VERSION=\"%1\"\n").arg(ui->lineVersion->text()).toUtf8());
         save.write(QString("UPD_URL=\"%1\"\n").arg(ui->lineUpdUrl->text()).toUtf8());
         save.write(QString("API_SRV=\"%1\"\n").arg(ui->lineApiSrv->text()).toUtf8());
         save.write(QString("API_XML=\"%1\"\n").arg(ui->lineApiXml->text()).toUtf8());
         save.write(QString("API_JSON=\"%1\"\n").arg(ui->lineApiJson->text()).toUtf8());
         save.write(QString("LOGO=\"%1\"\n").arg(ui->lineLogoFile->text()).toUtf8());
         save.write(QString("BACKGROUND=\"%1\"\n").arg(ui->lineBgFile->text()).toUtf8());

         for (int i = 0; i < ui->listLang->count(); i++)
         {
            if (ui->listLang->item(i)->isSelected())
            {
               sl << ui->listLang->item(i)->text();
            }
         }

         save.write(QString("LANGUAGES=\"%1\"\n").arg(sl.join(",")).toUtf8());

         settings.setValue("LastSave", name);
      }
   }

   return 0;
}

//----------------------------------------------------------------------
//! \fn       readValues
//! \brief    read customization data from template
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    load (const QString&) optional file name to load
//
//! \return   0 --> ok, -1 --> any error
//----------------------------------------------------------------------
int QEasyCustDlg::readValues(const QString& load)
{
   QRegExp rx("^([^=]+)=\"(.*)\"");

   QString name = (!load.isEmpty()) ? load : QFileDialog::getOpenFileName(this,
                                               tr("Load Customization Template"),
                                               "",
                                               "Cust Templates (*.tmpl);;"
                                               "All Files (*.*)");

   if (!name.isEmpty())
   {
      // clear entries ..
      on_pushNew_clicked();

      QFile load(name);
      QString s;

      if (load.open(QIODevice::ReadOnly | QIODevice::Text))
      {
         while (!load.atEnd())
         {
            s = QString::fromUtf8(load.readLine());

            if (rx.indexIn(s) > -1)
            {
               if (rx.cap(1) == "OFF_NAME")
               {
                  ui->lineOffName->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "INT_NAME")
               {
                  ui->lineIntName->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "COMP_NAME")
               {
                  ui->lineCompName->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "COMP_LINK")
               {
                  ui->lineCompLink->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "VERSION")
               {
                  ui->lineVersion->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "UPD_URL")
               {
                  ui->lineUpdUrl->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "API_SRV")
               {
                  ui->lineApiSrv->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "API_XML")
               {
                  ui->lineApiXml->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "API_JSON")
               {
                  ui->lineApiJson->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "LOGO")
               {
                  ui->lineLogoFile->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "BACKGROUND")
               {
                  ui->lineBgFile->setText(rx.cap(2));
               }
               else if(rx.cap(1) == "LANGUAGES")
               {
                  QStringList sl = rx.cap(2).split(",", QString::SkipEmptyParts);

                  for (int i = 0; i < ui->listLang->count(); i++)
                  {
                     if (sl.contains(ui->listLang->item(i)->text()))
                     {
                        ui->listLang->item(i)->setSelected(true);
                     }
                  }
               }
            }
         }
      }
   }

   return 0;
}

//----------------------------------------------------------------------
//! \fn       on_pushNew_clicked
//! \brief    remove all entries to start new
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushNew_clicked()
{
   ui->lineIntName->clear();
   ui->lineOffName->clear();
   ui->lineIntName->clear();
   ui->lineCompName->clear();
   ui->lineCompLink->clear();
   ui->lineVersion->clear();
   ui->lineUpdUrl->clear();
   ui->lineApiSrv->clear();
   ui->lineApiXml->clear();
   ui->lineApiJson->clear();
   ui->lineLogoFile->clear();
   ui->lineBgFile->clear();
   ui->plainTextEdit->clear();
   ui->labBg->setPixmap(QPixmap());
   ui->labLogo->setPixmap(QPixmap());

   for (int i = 0; i < ui->listLang->count(); i++)
   {
      ui->listLang->item(i)->setSelected(false);
   }
}

//----------------------------------------------------------------------
//! \fn       on_pushOpen_clicked
//! \brief    open button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushOpen_clicked()
{
   readValues();
   showPngImage(ui->labLogo, ui->lineLogoFile->text());
   showPngImage(ui->labBg, ui->lineBgFile->text());
}

//----------------------------------------------------------------------
//! \fn       on_pushSave_clicked
//! \brief    save button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushSave_clicked()
{
   saveValues();
}

//----------------------------------------------------------------------
//! \fn       showPngImage
//! \brief    show image file in QLabel
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    pLab (QLabel*) pointer to QLabel where to show image
//! \param    file (const QString&) file name of image to show
//
//! \return   0 --> ok, -1 --> any error
//----------------------------------------------------------------------
int QEasyCustDlg::showPngImage (QLabel* pLab, const QString& file)
{
   if (!file.isEmpty())
   {
      pLab->setPixmap(QPixmap(file, "png").scaled(pLab->size(), Qt::KeepAspectRatio));
   }

   return 0;
}

//----------------------------------------------------------------------
//! \fn       enableDisableDlg
//! \brief    enable / disable dialog items while process is running
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//! \param    enable (bool) enable flag
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::enableDisableDlg(bool enable)
{
   ui->lineIntName->setEnabled(enable);
   ui->lineOffName->setEnabled(enable);
   ui->lineIntName->setEnabled(enable);
   ui->lineCompName->setEnabled(enable);
   ui->lineCompLink->setEnabled(enable);
   ui->lineVersion->setEnabled(enable);
   ui->lineUpdUrl->setEnabled(enable);
   ui->lineApiSrv->setEnabled(enable);
   ui->lineApiXml->setEnabled(enable);
   ui->lineApiJson->setEnabled(enable);
   ui->lineLogoFile->setEnabled(enable);
   ui->lineBgFile->setEnabled(enable);
   ui->listLang->setEnabled(enable);
   ui->pushGetBg->setEnabled(enable);
   ui->pushGetLogo->setEnabled(enable);
   ui->pushGo->setEnabled(enable);
   ui->pushNew->setEnabled(enable);
   ui->pushOpen->setEnabled(enable);
   ui->pushSave->setEnabled(enable);
   ui->pushSaveLog->setEnabled(enable);
}

//----------------------------------------------------------------------
//! \fn       on_pushSaveLog_clicked
//! \brief    save log button was pressed
//
//! \Author   Jo2003
//! \Date     04.04.2013
//
//
//! \return   --
//----------------------------------------------------------------------
void QEasyCustDlg::on_pushSaveLog_clicked()
{
   QString name = QFileDialog::getSaveFileName(this, tr("Save Logfile"),
                                               ui->lineIntName->text(),
                                               "Logfiles (*.log);;"
                                               "All Files (*.*)");

   if (!name.isEmpty())
   {
      QFile save(name);
      QString s;

      if (save.open(QIODevice::WriteOnly | QIODevice::Text))
      {
         save.write(ui->plainTextEdit->toPlainText().toUtf8());
      }
   }
}

