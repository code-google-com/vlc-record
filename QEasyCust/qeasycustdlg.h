/*------------------------------ Information ---------------------------*//**
*
*  $HeadURL$
*
*  @file     qeasycustdlg.h
*
*  @author   Jo2003
*
*  @date     04.04.2013
*
*  $Id$
*
*/ //----------------- (c) 2013 Jo2003 --------------------------------------
#ifndef __040413__QEASYCUSTDLG_H
   #define __040413__QEASYCUSTDLG_H

#include <QDialog>
#include <QMap>
#include <QProcess>
#include <QStringList>
#include <QLabel>
#include <QSettings>
#include <QShowEvent>
#include <QKeyEvent>

/// \name define a patch map
typedef QMap<QString, QString> QPatchMap;

namespace Ui {
   class QEasyCustDlg;
}

//----------------------------------------------------------------------
//! \class    QEasyCustDlg
//! \brief    Dialog for customization
//
//! \Author   Jo2003
//! \Date     04.04.2013
//----------------------------------------------------------------------
class QEasyCustDlg : public QDialog
{
   Q_OBJECT

public:
   explicit QEasyCustDlg(QWidget *parent = 0, const QString& tmpl = QString());
   ~QEasyCustDlg();

   struct SPatch
   {
      QString src;
      QString trg;
   };

protected:
   QString patchTextFile(const QString &file, const QString& saveFile = QString());
   void addLog(const QString& toLog);
   int createCleanFolders();
   void doit();
   void patchAllFiles(const QVector<SPatch> &vecPatches);
   int saveValues();
   int readValues(const QString& load = QString());
   int showPngImage (QLabel* pLab, const QString& file);
   virtual void showEvent(QShowEvent *e);
   virtual void keyPressEvent(QKeyEvent *e);
   void enableDisableDlg(bool enable);

private slots:
   void on_pushGetLogo_clicked();
   void on_pushGetBg_clicked();
   void on_pushGo_clicked();
   void slotReadProcData();
   void slotStepDone(int pid);
   void slotProcError(QProcess::ProcessError e);
   void on_pushQuit_clicked();
   void on_pushNew_clicked();
   void on_pushOpen_clicked();
   void on_pushSave_clicked();
   void on_pushSaveLog_clicked(const QString& fName = QString());

private:
   Ui::QEasyCustDlg *ui;
   QPatchMap patchMap;
   QString sAppPath;
   QProcess *procBuilder;
   QStringList cmdQueue;
   QSettings settings;
   QString   s2Process;
   bool      bCLMode;
};

#endif // __040413__QEASYCUSTDLG_H
