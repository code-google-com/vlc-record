/*------------------------------ Information ---------------------------*//**
 *
 *  $HeadURL$
 *
 *  @file     qcustparser.cpp
 *
 *  @author   Jo2003
 *
 *  @date     10.09.2012
 *
 *  $Id$
 *
 *///------------------------- (c) 2012 by Jo2003  --------------------------
#include "qcustparser.h"

extern CDirStuff *pFolders;

//---------------------------------------------------------------------------
//
//! \brief   constructs QCustParser object
//
//! \author  Jo2003
//! \date    10.09.2012
//
//! \param   parent pointer to parent object
//
//! \return  --
//---------------------------------------------------------------------------
QCustParser::QCustParser(QObject *parent) :
   QObject(parent)
{
}

bool QCustParser::loadCustResource()
{
   bool bRV       = false;
   QDir resDir    = pFolders->getResDir();
   QStringList sl = resDir.entryList(QStringList() << "*.cust");

   if (!sl.isEmpty())
   {
      if ((bRV = QResource::registerResource(QString("%1/%2").arg(pFolders->getResDir()).arg(sl[0]))))
      {
         QDir::addSearchPath("branding", ":/oem");
      }
   }

   if (!bRV)
   {
      QDir::addSearchPath("branding", ":/unbranded");
   }

   return bRV;
}

int QCustParser::parseCust()
{
   loadCustResource();

   QXmlStreamAttributes attr;
   QXmlStreamReader     xml;
   QFile                resFile("branding:cust");
   SObjCmd              cmd;

   if (resFile.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      xml.setDevice(&resFile);

      while(!xml.atEnd() && !xml.hasError())
      {
         switch (xml.readNext())
         {
         // we aren't interested in ...
         case QXmlStreamReader::StartDocument:
         case QXmlStreamReader::EndDocument:
         case QXmlStreamReader::EndElement:
            break;

         // any xml element starts ...
         case QXmlStreamReader::StartElement:
            if (xml.name() == "string")
            {
               attr = xml.attributes();
               mStrings.insert(attr.value("name").toString(), attr.value("value").toString());
               qDebug("Got value %s=%s\n", attr.value("name").toString().toLocal8Bit().constData(), attr.value("value").toString().toLocal8Bit().constData());
            }
            else if (xml.name() == "widget")
            {
               attr =  xml.attributes();
               if (!mCustCmds.contains(attr.value("class").toString()))
               {
                  mCustCmds.insert(attr.value("class").toString(), QCommandVector());
               }

               cmd.sName   =  attr.value("object").toString();
               cmd.sAction =  attr.value("action").toString();
               cmd.bValue  = (attr.value("value").toString().toLower() == "true") ? true : false;

               mCustCmds[attr.value("class").toString()].append(cmd);
            }
            break;

         default:
            break;

         } // end switch ...
      } // end while ...
   }

   return xml.hasError() ? -1 : 0;
}

const QString& QCustParser::strVal(const QString &name)
{
   return mStrings[name];
}

const QCustParser::QCommandVector& QCustParser::classCmds(const QString &name)
{
   return mCustCmds[name];
}
