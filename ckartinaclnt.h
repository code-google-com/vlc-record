/*=============================================================================\
| $HeadURL$
|
| Author: Jo2003
|
| last changed by: $Author$
|
| Begin: Monday, January 04, 2010 16:11:14
|
| $Id$
|
\=============================================================================*/
#ifndef __201004161114_CKARTINACLNT_H
   #define __201004161114_CKARTINACLNT_H

#include <QHttp>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QBuffer>
#include <QString>
#include <QMessageBox>
#include <QtNetwork>
#include <QDate>
#include <QFile>
#include <QTimer>

#include "clogfile.h"
#include "defdef.h"
#include "version_info.h"


namespace Kartina {
   enum EReq
   {
      REQ_COOKIE,
      REQ_CHANNELLIST,
      REQ_STREAM,
      REQ_TIMESHIFT,
      REQ_EPG,
      REQ_SERVER,
      REQ_HTTPBUFF,
      REQ_ARCHIV,
      REQ_TIMERREC,
      REQ_GET_SERVER,
      REQ_LOGOUT,
      REQ_GETTIMESHIFT,
      REQ_GETVODGENRES,
      REQ_GETVIDEOS,
      REQ_GETVIDEOINFO,
      REQ_GETVODURL,
      REQ_ABORT,
      REQ_GETBITRATE,
      REQ_SETBITRATE,
      REQ_CHANLIST_ALL,
      REQ_SETCHAN_HIDE,
      REQ_SETCHAN_SHOW,
      REQ_GET_VOD_MANAGER,
      REQ_SET_VOD_MANAGER,
      REQ_ADD_VOD_FAV,
      REQ_REM_VOD_FAV,
      REQ_GET_VOD_FAV,
      REQ_GET_VOD_FAV_IDS,
      REQ_UNKNOWN = 255
   };
}

/*=============================================================================\
| Class:       CKartinaClnt
|
| Author:      Jo2003
|
| Begin:       Monday, January 04, 2010 16:12:28
|
| Description: a class to communicate with kartina.tv services
|
\=============================================================================*/
class CKartinaClnt : public QHttp
{
   Q_OBJECT

public:
   CKartinaClnt(const QString &host, const QString &usr, const QString &pw,
                const QString &sEPw = QString(), bool bAllowErotic = false);

   CKartinaClnt();
   ~CKartinaClnt();

   void SetData(const QString &host, const QString &usr, const QString &pw,
                const QString &sEPw = QString(), bool bAllowErotic = false);

   void GetCookie ();
   void Logout ();
   void GetChannelList (const QString &secCode = QString());
   void SetTimeShift (int iHours);
   void GetTimeShift ();
   void GetStreamURL (int iChanID, bool bTimerRec = false);
   void GetArchivURL (const QString &prepared);
   void GetVodUrl (int iVidId, const QString &secCode);
   void GetVodGenres ();
   void SetServer (const QString& sIp);
   void GetServer ();
   void SetBitRate (int iRate);
   void GetBitRate ();
   void SetHttpBuffer (int iTime);
   void GetEPG (int iChanID, int iOffset = 0);
   void GetVideos (const QString &sPrepared);
   void GetVideoInfo (int iVodID, const QString &secCode = QString());
   void SetCookie (const QString &cookie);
   bool busy ();
   bool cookieSet();
   void setChanHide (const QString &cids, const QString &secCode);
   void setChanShow (const QString &cids, const QString &secCode);
   void getVodManager (const QString &secCode);
   void setVodManager (const QString &rules, const QString &secCode);
   void addVodFav (int iVidID, const QString &secCode);
   void remVodFav (int iVidID, const QString &secCode);
   void getVodFav (bool bIDs = false);

protected:
   void PostRequest (Kartina::EReq req, const QString &path, const QString &content,
                     const QString &sBrowser = APP_NAME " " __MY__VERSION__);
   void GetRequest (Kartina::EReq req, const QString &sRequest,
                    const QString &sBrowser = APP_NAME " " __MY__VERSION__);

private:
   Kartina::EReq eReq;
   QString       sUsr;
   QString       sPw;
   QString       sHost;
   QString       sErosPw;
   bool          bEros;
   QString       sCookie;
   QByteArray    baPageContent;
   QBuffer       bufReq;
   int           iReq;

private slots:
   void handleEndRequest (int id, bool err);

signals:
   void sigError (QString str);
   void sigHttpResponse(QString str, int iReq);
};

#endif /* __201004161114_CKARTINACLNT_H */
/*=============================================================================\
|                                    History:
| ---------------------------------------------------------------------------
| 04.Jan.2010 - communication API for kartina.tv (inspired by conros)
\=============================================================================*/

