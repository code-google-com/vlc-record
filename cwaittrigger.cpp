/*********************** Information *************************\
| $HeadURL$
|
| Author: Jo2003
|
| Begin: 19.01.2010 / 15:51:46
|
| Last edited by: $Author$
|
| $Id$
\*************************************************************/
#include "cwaittrigger.h"

// log file functions ...
extern CLogFile VlcLog;

/* -----------------------------------------------------------------\
|  Method: CWaitTrigger / constructor
|  Begin: 19.01.2010 / 15:52:15
|  Author: Jo2003
|  Description: create object, init values
|
|  Parameters: parent pointer
|
|  Returns: --
\----------------------------------------------------------------- */
CWaitTrigger::CWaitTrigger(QObject * parent) : QThread(parent)
{
   pClient  = NULL;
   iGo      = 1;
}

/* -----------------------------------------------------------------\
|  Method: ~CWaitTrigger / destructor
|  Begin: 19.01.2010 / 15:52:51
|  Author: Jo2003
|  Description: clean at destruction
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
CWaitTrigger::~CWaitTrigger()
{
   stop();
}

/* -----------------------------------------------------------------\
|  Method: SetKartinaClient
|  Begin: 19.01.2010 / 15:53:14
|  Author: Jo2003
|  Description: set kartina client class pointer
|
|  Parameters: pointer to kartina connection
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::SetKartinaClient(CKartinaClnt *pKartinaClient)
{
   pClient = pKartinaClient;
}

/* -----------------------------------------------------------------\
|  Method: stop
|  Begin: 19.01.2010 / 15:53:52
|  Author: Jo2003
|  Description: stop thread loop
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::stop()
{
   iGo = 0;
   wait();
}

/* -----------------------------------------------------------------\
|  Method: run
|  Begin: 19.01.2010 / 15:54:11
|  Author: Jo2003
|  Description: thread loop, wait for requests and throw it
|               when client becomes ready
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::run()
{
   QVector<CommandQueue::SCmd>::iterator it;
   while (iGo)
   {
      if (!commandQueue.isEmpty ())
      {
         it = commandQueue.begin();

         // we don't block cookie requests!
         if ((*it).eReq != Kartina::REQ_COOKIE)
         {
            // wait until api becomes available ...
            while (pClient->busy () && iGo)
            {
               msleep(10);
            }
         }

         if (iGo)
         {
            switch ((*it).eReq)
            {
            case Kartina::REQ_CHANNELLIST:
               pClient->GetChannelList();
               break;
            case Kartina::REQ_COOKIE:
               pClient->GetCookie();
               break;
            case Kartina::REQ_EPG:
               pClient->GetEPG((*it).iOptArg1, (*it).iOptArg2);
               break;
            case Kartina::REQ_SERVER:
               pClient->SetServer((*it).sOptArg1);
               break;
            case Kartina::REQ_HTTPBUFF:
               pClient->SetHttpBuffer((*it).iOptArg1);
               break;
            case Kartina::REQ_STREAM:
               pClient->GetStreamURL((*it).iOptArg1);
               break;
            case Kartina::REQ_TIMERREC:
               pClient->GetStreamURL((*it).iOptArg1, true);
               break;
            case Kartina::REQ_ARCHIV:
               pClient->GetArchivURL((*it).sOptArg1);
               break;
            case Kartina::REQ_TIMESHIFT:
               pClient->SetTimeShift((*it).iOptArg1);
               break;
            case Kartina::REQ_GETTIMESHIFT:
               pClient->GetTimeShift();
               break;
            case Kartina::REQ_GET_SERVER:
               pClient->GetServer();
               break;
            case Kartina::REQ_LOGOUT:
               pClient->Logout();
               break;
            case Kartina::REQ_GETVODGENRES:
               pClient->GetVodGenres();
               break;
            case Kartina::REQ_GETVIDEOS:
               pClient->GetVideos((*it).sOptArg1);
               break;
            case Kartina::REQ_GETVIDEOINFO:
               pClient->GetVideoInfo((*it).iOptArg1);
               break;
            case Kartina::REQ_GETVODURL:
               pClient->GetVodUrl((*it).iOptArg1);
               break;
            case Kartina::REQ_GETBITRATE:
               pClient->GetBitRate();
               break;
            case Kartina::REQ_SETBITRATE:
               pClient->SetBitRate((*it).iOptArg1);
               break;
            default:
               break;
            }
         }

         commandQueue.erase (it);
      }
      else
      {
         // queue empty - wait a little ...
         msleep(5);
      }
   }
}

/* -----------------------------------------------------------------\
|  Method: TriggerRequest
|  Begin: 19.01.2010 / 15:55:06
|  Author: Jo2003
|  Description: request kartina action
|
|  Parameters: action, action params
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::TriggerRequest(Kartina::EReq req, int iArg1, int iArg2)
{
   CommandQueue::SCmd cmd;

   cmd.eReq     = req;
   cmd.iOptArg1 = iArg1;
   cmd.iOptArg2 = iArg2;
   cmd.sOptArg1 = "";
   cmd.sOptArg2 = "";
   queueIn(cmd);
}

/* -----------------------------------------------------------------\
|  Method: TriggerRequest
|  Begin: 19.01.2010 / 15:55:06
|  Author: Jo2003
|  Description: request kartina action
|
|  Parameters: action, action params
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::TriggerRequest (Kartina::EReq req, const QString &sReq1, const QString &sReq2)
{
   CommandQueue::SCmd cmd;

   cmd.eReq     = req;
   cmd.iOptArg1 = -1;
   cmd.iOptArg2 = -1;
   cmd.sOptArg1 = sReq1;
   cmd.sOptArg2 = sReq2;
   queueIn(cmd);
}

/* -----------------------------------------------------------------\
|  Method: slotReqChanList
|  Begin: 19.01.2010 / 15:55:44
|  Author: Jo2003
|  Description: channel list request, signalized by refresh timer
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::slotReqChanList()
{
   CommandQueue::SCmd cmd;

   cmd.eReq     = Kartina::REQ_CHANNELLIST;
   cmd.iOptArg1 = -1;
   cmd.iOptArg2 = -1;
   cmd.sOptArg1 = "";
   cmd.sOptArg2 = "";
   queueIn(cmd);
}

/* -----------------------------------------------------------------\
|  Method: queueIn
|  Begin: 28.09.2011
|  Author: Jo2003
|  Description: queue in new command, check for cookie and abort
|
|  Parameters: ref. to command
|
|  Returns: --
\----------------------------------------------------------------- */
void CWaitTrigger::queueIn (const CommandQueue::SCmd &cmd)
{
   // abort means delete all pending requests ...
   if (cmd.eReq == Kartina::REQ_ABORT)
   {
      commandQueue.clear ();

      if (pClient->busy ())
      {
         pClient->abort ();
      }
   }
   else if (cmd.eReq == Kartina::REQ_COOKIE)
   {
      commandQueue.clear ();

      if (pClient->busy ())
      {
         pClient->abort ();
      }

      commandQueue.append (cmd);
   }
   else
   {
      commandQueue.append (cmd);
   }
}

/************************* History ***************************\
| $Log$
\*************************************************************/

