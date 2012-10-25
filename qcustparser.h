/*------------------------------ Information ---------------------------*//**
 *
 *  $HeadURL$
 *
 *  @file     qcustparser.h
 *
 *  @author   Jo2003
 *
 *  @date     10.09.2012
 *
 *  $Id$
 *
 *///------------------------- (c) 2012 by Jo2003  --------------------------
#ifndef __20120910_QCUSTPARSER_H
   #define __20120910_QCUSTPARSER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QDir>
#include <QResource>

#include "cdirstuff.h"

class QCustParser : public QObject
{
   Q_OBJECT

   struct SObjCmd
   {
      QString sName;
      QString sAction;
      bool    bValue;
   };

   typedef QVector<SObjCmd> QCommandVector;

public:
   QCustParser(QObject *parent = 0);
   int parseCust();
   const QString& strVal(const QString &name);
   const QCommandVector& classCmds(const QString &name);

private:
   QMap<QString, QString>        mStrings;
   QMap<QString, QCommandVector> mCustCmds;

protected:
   bool loadCustResource();

signals:

public slots:

};

#endif // __20120910_QCUSTPARSER_H
