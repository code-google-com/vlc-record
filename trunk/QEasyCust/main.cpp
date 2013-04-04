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
#include "qeasycustdlg.h"
#include <QtPlugin>
Q_IMPORT_PLUGIN(qico)

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   QCoreApplication::setOrganizationName("Jo2003");
   QCoreApplication::setOrganizationDomain("coujo.com");
   QCoreApplication::setApplicationName("QEasyCust");

   QEasyCustDlg w;
   w.show();

   return a.exec();
}

