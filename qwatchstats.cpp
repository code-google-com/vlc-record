/*------------------------------ Information ---------------------------*//**
 *
 *  $HeadURL$
 *
 *  @file     qwatchstats.cpp
 *
 *  @author   Jo2003
 *
 *  @date     15.10.2014
 *
 *  $Id$
 *
 *///------------------------- (c) 2014 by Jo2003  --------------------------
#include "qwatchstats.h"
#include "version_info.h"

// external stuuf can't be included from our include header since we
// are global as well ...
#include "qcustparser.h"
#include "qdatetimesyncro.h"

// global syncronized time ...
extern QDateTimeSyncro tmSync;

// global customization class ...
extern QCustParser *pCustomization;

// global showinfo class ...
extern CShowInfo showInfo;

//---------------------------------------------------------------------------
//
//! \brief   constructs QWatchStats object
//
//! \author  Jo2003
//! \date    15.10.2014
//
//! \param   pParent [in] (QObject *) pointer to parent object
//
//---------------------------------------------------------------------------
QWatchStats::QWatchStats(QObject *pParent) : QObject(pParent), mpSettings(NULL), mbStartSet(false)
{
}

//---------------------------------------------------------------------------
//
//! \brief   destroys QWatchStats object
//
//! \author  Jo2003
//! \date    15.10.2014
//
//---------------------------------------------------------------------------
QWatchStats::~QWatchStats()
{
}

//---------------------------------------------------------------------------
//
//! \brief   import settings class
//
//! \author  Jo2003
//! \date    15.10.2014
//
//! \param   pSet [in] (CSettingsDlg *) pointer to settings class
//
//---------------------------------------------------------------------------
void QWatchStats::setSettings(CSettingsDlg *pSet)
{
   mpSettings = pSet;
}

//---------------------------------------------------------------------------
//
//! \brief   a stream should be started, grab needed informations
//
//! \author  Jo2003
//! \date    15.10.2014
//
//! \param   sUrl [in] (const QString &) stream url
//
//---------------------------------------------------------------------------
void QWatchStats::playStarts(const QString &sUrl)
{
   mbStartSet = true;

   // clean entry ...
   mEntry.clear();

   mEntry["stream_url"]    = sUrl;
   mEntry["play_start"]    = tmSync.currentDateTimeSync().toTime_t();
   mEntry["prog_type"]     = CShowInfo::progType2String(showInfo.showType());
   mEntry["stream_server"] = mpSettings->getStreamServer();
   mEntry["buffering"]     = mpSettings->GetBufferTime();

   switch (showInfo.showType())
   {
   case ShowInfo::Archive:
      // archive needs gmt, cid, timeshift, bitrate ...
      mEntry["bitrate"]    = mpSettings->GetBitRate();
      mEntry["timeshift"]  = mpSettings->getTimeShift();
      mEntry["cid"]        = showInfo.channelId();
      mEntry["gmt"]        = showInfo.starts() + showInfo.lastJump();
      break;

   case ShowInfo::Live:
      // live needs gmt, cid, timeshift, bitrate ...
      mEntry["bitrate"]    = mpSettings->GetBitRate();
      mEntry["timeshift"]  = mpSettings->getTimeShift();
      mEntry["cid"]        = showInfo.channelId();
      mEntry["gmt"]        = -1;
      break;

   case ShowInfo::VOD:
      // vod needs movie id and file id
      mEntry["movie_id"]   = showInfo.videoId();
      mEntry["file_id"]    = showInfo.vodFileId();
      break;

   default:
      mbStartSet = false;
      break;
   }
}

//---------------------------------------------------------------------------
//
//! \brief   player ends, store stats record
//
//! \author  Jo2003
//! \date    15.10.2014
//
//! \param   iErrCount [in] (int) error count
//
//---------------------------------------------------------------------------
void QWatchStats::playEnds(int iErrCount)
{
   if (mbStartSet)
   {
      if ((tmSync.currentDateTimeSync().toTime_t() - mEntry["play_start"].toUInt()) > 10)
      {
         // more than 10 seconds play time ...
         mEntry["play_end"]    = tmSync.currentDateTimeSync().toTime_t();
         mEntry["error_count"] = iErrCount;
         mEntryList.append(mEntry);
      }
      mbStartSet = false;
   }
}

//---------------------------------------------------------------------------
//
//! \brief   JSON serialize statistics
//
//! \author  Jo2003
//! \date    15.10.2014
//
//! \param   devId [in] (const QString&) unique device id
//
//---------------------------------------------------------------------------
QString QWatchStats::serialize(const QString& devId)
{
   WStats::StatsEntry entry;

   entry["server_address"]   = mpSettings->GetAPIServer();
   entry["account_number"]   = mpSettings->GetUser();
   entry["device_id"]        = devId;
   entry["device_type"]      = QString("%1%2-%3").arg(pCustomization->strVal("APPLICATION_SHORTCUT")).arg(OP_SYS).arg(SOFTID_DEVELOPER);
   entry["firmware_version"] = QString(__MY__VERSION__);

   if (!mEntryList.isEmpty())
   {
      entry["play_statistics"]  = mEntryList;
   }

   QString ret = QtJson::serialize(entry);
   mEntryList.clear();
   mbStartSet = false;

   return ret;
}
