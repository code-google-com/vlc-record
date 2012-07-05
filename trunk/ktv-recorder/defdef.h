/*********************** Information *************************\
| $HeadURL$
|
| Author: Jo2003
|
| Begin: 18.01.2010 / 09:19:48
|
| Last edited by: $Author$
|
| $Id$
\*************************************************************/
#ifndef __011810__DEFDEF_H
   #define __011810__DEFDEF_H

#include <QtGlobal>
#include "customization.h"

#define APP_NAME          "KTV-Recorder"

#ifdef Q_OS_WIN32
   #define DATA_DIR_ENV   "APPDATA"
   #define DATA_DIR       APP_NAME
#else
   #define DATA_DIR_ENV   "HOME"
   #define DATA_DIR       "." APP_NAME
#endif

#define COMPANY_NAME "Kartina.TV"
#define COMPANY_LINK "<a href='http://www.kartina.tv'>" COMPANY_NAME "</a>"

#ifdef __GNUC__
   #define __UNUSED        __attribute__ ((unused))
#else
   #define __UNUSED
#endif

#define APP_INI_FILE      "ktv-rcd.ini"
#define APP_LOG_FILE      "ktv-recorder.log"
#define PLAYER_LOG_FILE   "player.log"
#define MOD_DIR           "modules"
#define LANG_DIR          "language"
#define LANG_DIR_QT       "translations"
#define LOGO_DIR          "logos"
#define VOD_DIR           "vod"
#define KARTINA_HOST      "iptv.kartina.tv"
#define KARTINA_API_PATH  "/api/xml/"
#define LOGO_URL          "/img/ico/24"
// #define DEF_TIME_FORMAT   "MMM dd, yyyy hh:mm:ss"
#define DEF_TIME_FORMAT   "dd.MM.yyyy hh:mm:ss"
#define DEF_TZ_STEP       1800            // time zone step is min. 30 minutes (1800 sec.) ...
#define DEF_MAX_DIFF      600             // accept system clock inaccuracy up too 600 sec
#define EPG_NAVBAR_HEIGHT 24              // default height for EPG navbar
#define TIMER_REC_OFFSET  300             // 5 minutes in seconds
#define INVALID_ID        0xFFFFFFFF      // mark an id as invalid
#define TIMER_STBY_TIME   30              // 30 sec. before we should start record
#define MAX_NAME_LEN      10              // max. length of show name
#define ARCHIV_OFFSET     900             // 15 minutes after show start, archiv should be available
#define MAX_ARCHIV_AGE    1209000         // < 2 weeks in seconds
#define MAX_NO_FAVOURITES 10              // max. number of favourites ...
#define MIN_CACHE_SIZE    5000000         // < 5 MB ...
#define TIME_OFFSET       (35 * 365 * 24 * 3600) // make the slider handle gmt
#define VIDEOS_PER_SITE   20              // number of videos / site
#define GPU_ACC_TOKEN     ":ffmpeg-hw"    // mrl option to use GPU acceleration
#define UPD_CHECK_URL     "http://code.google.com/p/vlc-record/downloads/list"
#define MAX_CHANNEL_ACTS  200             // max number of channel actions
#define MAX_CHANNEL_GROUPS 20             // max number of channel groups
#define MAX_RECENT_CHANNELS 10            // max number of recent channels
#define MAX_ASPECTS         7             // max number of aspect ratios
#define MAX_CROPS           10            // max number of crop ratios

// a namespace for translators ...
namespace Translators
{
    enum ETranslators
    {
        TRANS_QT,
        TRANS_OWN,
        TRANS_MAX
    };
}

#endif /* __011810__DEFDEF_H */
/************************* History ***************************\
| $Log$
\*************************************************************/


