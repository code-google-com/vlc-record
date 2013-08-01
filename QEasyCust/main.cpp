/*------------------------------ Information ---------------------------*//**
*
*  $HeadURL$
*
*  @file     main.cpp
*
*  @author   Jo2003
*
*  @date     04.04.2013
*
*  $Id$
*
*/ //----------------- (c) 2013 Jo2003 --------------------------------------
#include <QApplication>
#include <QStringList>
#include <QByteArray>
#include "qeasycustdlg.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(qico)

QString tmplFile(const QStringList& sl)
{
   QString tmpl = "";

   if (sl.count() == 2)
   {
      tmpl = sl.at(1);
   }
   else if(sl.count() > 2)
   {
      if (sl.at(1).contains("-t")
          || sl.at(1).contains("/t")
          || sl.at(1).contains("-T")
          || sl.at(1).contains("/T"))
      {
         tmpl = sl.at(2);
      }
   }

   return tmpl;
}

int main(int argc, char *argv[])
{
   QStringList sl;
   int         i, iRet = -1;

   // grab all command line parameters
   // not using arguments() from QApplication
   for (i = 0; i < argc; i++)
   {
      sl << QString::fromLocal8Bit(argv[i]);
   }

   QApplication a(argc, argv);

   QCoreApplication::setOrganizationName("Jo2003");
   QCoreApplication::setOrganizationDomain("coujo.com");
   QCoreApplication::setApplicationName("QEasyCust");

   QEasyCustDlg *pDlg = new QEasyCustDlg(NULL, tmplFile(sl));

   if (pDlg)
   {
      pDlg->show();

      iRet = a.exec();

      delete pDlg;
   }

   return iRet;
}

