<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 8/10/2010 / 4:48p
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/

require_once(dirname(__FILE__)."/_kartina_auth.php.inc");

/* -----------------------------------------------------------------\
|  Method: _pluginMain
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: plugin entry point 
|
|  Parameters: get request string
|
|  Returns: array of items to be displayed
\----------------------------------------------------------------- */
function _pluginMain($prmQuery) 
{
   $queryData = array();
   $items     = array();
   simpleLog(__FUNCTION__."():".__LINE__." Raw query: ".$prmQuery);
   
   parse_str($prmQuery, $queryData);

   if (!isset($queryData['action']))
   {
      $items = _pluginCreateChannelGroupList();
   }
   else
   {
      simpleLog(__FUNCTION__."():".__LINE__." Action: ".$queryData['action']);
      if ($queryData['action'] === "favorites")
      {
         $items = _pluginCreateFavList();
      }
      else if ($queryData['action'] === "channels")
      {
         simpleLog(__FUNCTION__."():".__LINE__." Changrp: ".$queryData['changrp']);
         $items = _pluginCreateChannelList($queryData['changrp']);
      }
      else if ($queryData['action'] === "arch_main")
      {
         simpleLog(__FUNCTION__."():".__LINE__." ChanID: ".$queryData['cid']);
         $items = _pluginCreateArchMainFolder ($queryData['cid']);
      }
      else if ($queryData['action'] === "archive")
      {
         simpleLog(__FUNCTION__."():".__LINE__." ChanID: ".$queryData['cid']
                  ."; Day: ".$queryData['day']);
         $items = _pluginCreateArchiveEpg ($queryData['cid'], $queryData['day']);
      }
      else if ($queryData['action'] === "chooserecplay")
      {
         $gmt     = (isset($queryData['gmt']))      ? $queryData['gmt']      : 0;
         $isVideo = (isset($queryData['is_video'])) ? $queryData['is_video'] : 1;
         
         simpleLog(__FUNCTION__."():".__LINE__." ChanID: ".$queryData['cid']
                  ."; GMT: ".$gmt."; IsVideo: ".$isVideo);
                  
         $items = _pluginChooseRecOrPlay($queryData['cid'], $gmt, $isVideo);
      }
   }
   
   return $items;
}

/* -----------------------------------------------------------------\
|  Method: _pluginCreateChannelList
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: create channel list for given channel group 
|
|  Parameters: channel group
|
|  Returns: array of items to be displayed
\----------------------------------------------------------------- */
function _pluginCreateChannelList($groupid) 
{
   $retMediaItems = array();
   $dom = new DomDocument();
   $dom->load(KARTCHANLIST);
   
   $xp        = new DOMXpath($dom);
   $groupitem = $xp->query("/response/groups/item[id='".$groupid."']");
   $group     = $groupitem->item(0); // there is only one such group ... 

   $channels  = $xp->query("channels/item/id", $group);
   $names     = $xp->query("channels/item/name", $group);
   $icons     = $xp->query("channels/item/icon", $group);
   $videoinfo = $xp->query("channels/item/is_video", $group);
   $archinfo  = $xp->query("channels/item/have_archive", $group);

   $all = $channels->length;

   for ($i = 0; $i < $all; $i++)
   {
      // check if channel has archive. If so, create a extra folder to choose
      // between live stream and archive. If not, request live stream ...
      if ((integer)$archinfo->item($i)->nodeValue === 1)
      {
         // has archive ...
         $data       = array('action' => 'arch_main', 
                             'cid'    => $channels->item($i)->nodeValue);
                             
         $dataString = http_build_query($data, "", "&amp;");
         
         $retMediaItems[] = array (
            'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
            'dc:title'       => $names->item($i)->nodeValue,
            'upnp:class'     => 'object.container',
            'upnp:album_art' => KARTINA_HOST.$icons->item($i)->nodeValue
         );
      }
      else
      {
         // prepare data to choose between rec and play ...
         $data     = array('action'   => 'chooserecplay',
                           'is_video' => (integer)$videoinfo->item($i)->nodeValue,
                           'cid'      => $channels->item($i)->nodeValue);
                    
         $dataString = http_build_query($data, "", "&amp;");
   
         $retMediaItems[] = array (
            'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
            'dc:title'       => $names->item($i)->nodeValue,
            'upnp:class'     => 'object.container',
            'upnp:album_art' => KARTINA_HOST.$icons->item($i)->nodeValue
         );
      }
   }
   
   return $retMediaItems;
}

/* -----------------------------------------------------------------\
|  Method: _pluginCreateChannelGroupList
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: create channel group list with favorite folder 
|
|  Parameters: --
|
|  Returns: array of items to be displayed
\----------------------------------------------------------------- */
function _pluginCreateChannelGroupList()
{
   $retMediaItems = array();
   $dom           = new DomDocument();
   
   // first add favorites folder ...
   $data       = array('action' => 'favorites');

   $dataString = http_build_query($data, "", "&amp;");
   
   $retMediaItems[] = array (
      'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
      'dc:title'       => "Фавориты",
      'upnp:class'     => 'object.container',
      'upnp:album_art' => LOC_KARTINA_URL."/images/favorite.png",
   );
         
   $dom->load(KARTCHANLIST);
   
   $xp         = new DOMXpath($dom);
      
   $groups     = $xp->query("/response/groups/item/id");
   $names      = $xp->query("/response/groups/item/name");

   $all = $groups->length;

   for ($i = 0; $i < $all; $i++)
   {
      $data       = array('action'  => 'channels',
                          'changrp' => $groups->item($i)->nodeValue);
                          
      $dataString = http_build_query($data, "", "&amp;");
      
      $retMediaItems[] = array (
         'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
         'dc:title'       => $names->item($i)->nodeValue,
         'upnp:class'     => 'object.container',
         'upnp:album_art' => LOC_KARTINA_URL."/images/folder.png"
      );
   }

   return $retMediaItems;
}

/* -----------------------------------------------------------------\
|  Method: _pluginCreateFavList
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: create favorites list 
|
|  Parameters: --
|
|  Returns: array of items to be displayed
\----------------------------------------------------------------- */
function _pluginCreateFavList()
{
   $retMediaItems = array();
   $domFavList    = new DOMDocument();
   $domChanList   = new DOMDocument();
   
   $domFavList->load(KARTFAVLIST);
   $domChanList->load(KARTCHANLIST);
   
   $xpfav  = new DOMXpath($domFavList);
   $xpchan = new DOMXpath($domChanList);
   
   $favorites = $xpfav->query("/response/favorites/item/channel_id");
   $favcount  = $favorites->length;

   for ($i = 0; $i < $favcount; $i ++)
   {
      $cid      = $favorites->item($i)->nodeValue;
      $chanitem = $xpchan->query("/response/groups/item/channels/item[id='".$cid."']");
      $chan     = $chanitem->item(0); // there is only one such item ...
      
      $icon     = $xpchan->query("icon", $chan)->item(0)->nodeValue;
      $name     = $xpchan->query("name", $chan)->item(0)->nodeValue; 
      $isvideo  = (integer)$xpchan->query("is_video", $chan)->item(0)->nodeValue;
      $hasarch  = (integer)$xpchan->query("have_archive", $chan)->item(0)->nodeValue;
      
      // check if channel has archive. If so, create a extra folder to choose
      // between live stream and archive. If not, request live stream ...
      if ($hasarch === 1)
      {
         // has archive ...
         $data       = array('action' => 'arch_main', 
                             'cid'    => $cid);
                             
         $dataString = http_build_query($data, "", "&amp;");
         
         $retMediaItems[] = array (
            'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
            'dc:title'       => $name,
            'upnp:class'     => 'object.container',
            'upnp:album_art' => KARTINA_HOST.$icon
         );
      }
      else
      {
         // prepare data to choose between rec and play ...
         $data     = array('action'   => 'chooserecplay',
                           'is_video' => $isvideo,
                           'cid'      => $cid);
                    
         $dataString = http_build_query($data, "", "&amp;");
   
         $retMediaItems[] = array (
            'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
            'dc:title'       => $name,
            'upnp:class'     => 'object.container',
            'upnp:album_art' => KARTINA_HOST.$icon
         );
      }
   }
   
   return $retMediaItems;
}

/* -----------------------------------------------------------------\
|  Method: _pluginCreateArchMainFolder
|  Begin: 10/5/2010 / 1:56p
|  Author: Jo2003
|  Description: create the archive main folder with live stream and 
|               day folders
|  Parameters: channel id
|
|  Returns: array of media items
\----------------------------------------------------------------- */
function _pluginCreateArchMainFolder ($cid)
{
   $retMediaItems = array();
   
   $days = array(1 => "Понедельник",
                 2 => "Вторник",
                 3 => "Среда",
                 4 => "Четверг",
                 5 => "Пятница",
                 6 => "Суббота",
                 7 => "Воскресенье");

   // first item is always the live stream ...

   // get info / url from given channel ...
   $domChanList = new DOMDocument();
   $domChanList->load(KARTCHANLIST);

   $xpchan   = new DOMXpath($domChanList);

   $chanitem = $xpchan->query("/response/groups/item/channels/item[id='".$cid."']");
   $chan     = $chanitem->item(0); // there is only one such item ...

   $icon     = $xpchan->query("icon", $chan)->item(0)->nodeValue;
   $epgname  = $xpchan->query("epg_progname", $chan)->item(0)->nodeValue;
   $epgstart = (integer)$xpchan->query("epg_start", $chan)->item(0)->nodeValue;    
   $epgend   = (integer)$xpchan->query("epg_end", $chan)->item(0)->nodeValue;
   $isvideo  = (integer)$xpchan->query("is_video", $chan)->item(0)->nodeValue;
   
   // cut prog name cause it makes trouble when it's too long ...
   // Note: UTF-8 uses up too 4 byte for one char ...
   // So we take max. useable chars and count backward 
   // to find a good place to cut the string.
   // Cutting the string inside an UTF-8 block
   // will break the whole output ...
   if (strlen($epgname) > 100)
   {
      for ($i = 100; $i > 0; $i --)
      {
         if (substr($epgname, $i, 1) === " ")
         {
            $epgname = substr($epgname, 0, $i)." ...";
            break;
         }
      }
   }
   
   // build title ...
   $title    = date("H:i", $epgstart)."-".date("H:i", $epgend)." ".$epgname;
   
   // prepare data to choose between rec and play ...
   $data     = array('action'   => 'chooserecplay',
                     'is_video' => $isvideo,
                     'cid'      => $cid);
                    
   $dataString = http_build_query($data, "", "&amp;");
   
   $retMediaItems[] = array (
      'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
      'dc:title'       => $title,
      'upnp:class'     => 'object.container',
      'upnp:album_art' => KARTINA_HOST.$icon
   );
   
   // make folders for all 14 days of the archive ... 
   $now       = time();
   $archstart = $now - MAX_ARCH_DAYS * DAY_IN_SECONDS;

   for ($i = $archstart; $i <= $now; $i += DAY_IN_SECONDS)
   {
      // first add favorites folder ...
      $data       = array('action' => 'archive',
                          'day'    => date ("dmy", $i),
                          'cid'    => $cid);
                          
      $dataString = http_build_query($data, "", "&amp;");
   
      $retMediaItems[] = array (
         'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
         'dc:title'       => "Архив - " .$days[date("N", $i)]. ", " .date("d.m.Y", $i),
         'upnp:class'     => 'object.container',
         'upnp:album_art' => LOC_KARTINA_URL."/images/archive.png"
      );
   }
  
   return $retMediaItems;
}

/* -----------------------------------------------------------------\
|  Method: _pluginCreateArchiveEpg
|  Begin: 10/5/2010 / 1:56p
|  Author: Jo2003
|  Description: create the archive epg for one channel / day
|               day folders
|  Parameters: channel id, day in form ddmmyy
|
|  Returns: array of media items
\----------------------------------------------------------------- */
function _pluginCreateArchiveEpg ($cid, $day)
{
   // Please note that all global variables
   // are wiped out! Therefore we have to instantiate
   // a local instance here .... 
   // global $kartAPI;
   
   // don't break your head about login / logout at kartina!
   // we will load the cookie from file so no authentication
   // is needed here ... we also don't need username and PW ...
   $tmpKartAPI = new kartinaAPI();
   
   // load cookie ...
   $tmpKartAPI->loadCookie();

   $retMediaItems = array();

   $epg = $tmpKartAPI->getDayEpg($cid, $day);
   
   $all = count($epg);
   
   simpleLog(__FUNCTION__."():".__LINE__." We found ".$all." EPG entries...");
   

   for ($i = 0; $i < $all; $i ++)
   {
      // display this show if it is in archive ...
      if (inArchive($epg[$i]['timestamp']))
      {
         
         // prepare data to choose between rec and play ...
         $data       = array('action'   => 'chooserecplay',
                             'gmt'      => $epg[$i]['timestamp'],
                             'is_video' => 1,
                             'cid'      => $cid);
                          
         $dataString = http_build_query($data, "", "&amp;");
         
         $tok = strtok($epg[$i]['programm'], "\n");
   
         $retMediaItems[] = array (
            'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
            'dc:title'       => date("H:i" , $epg[$i]['timestamp']) . " - " . $tok,
            'upnp:class'     => 'object.container',
            'upnp:album_art' => LOC_KARTINA_URL."/images/play.png"
         );
      }
   }

   return $retMediaItems;
}

/* -----------------------------------------------------------------\
|  Method: _pluginChooseRecOrPlay
|  Begin: 18.10.2010 / 16:16
|  Author: Jo2003
|  Description: make folder with rec and play item  
|
|  Parameters: channel id, gmt, video flag
|
|  Returns: array of media items ...
\----------------------------------------------------------------- */
function _pluginChooseRecOrPlay ($cid, $gmt = 0, $isVideo = 1)
{
   $retMediaItems = array();
   
   $url = LOC_KARTINA_URL."/stream.php?id=".$cid.(($gmt) ? "&gmt=".$gmt : "");
         
   simpleLog(__FUNCTION__."():".__LINE__." Request URL: ".$url);

   $url_data = array(
      'itemurl'  => $url,
      'is_video' => $isVideo
   );

   $url_data_string = http_build_query($url_data);

   // add play item ...
   $retMediaItems[] = array (
      'id'             => LOC_KARTINA_UMSP."/http-stream?".$url,
      'dc:title'       => "Просмотр",
      'upnp:class'     => ($isVideo) ? "object.item.videoitem" : "object.item.audioitem",
      'res'            => LOC_KARTINA_URL."/http-stream-proxy.php?".$url_data_string,
      'protocolInfo'   => "http-get:*:*:*",
      'upnp:album_art' => LOC_KARTINA_URL."/images/play.png"
   );
   
   // add кусщкв item ...
   $retMediaItems[] = array (
      'id'             => LOC_KARTINA_UMSP."/http-stream?".$url,
      'dc:title'       => "Запись",
      'upnp:class'     => ($isVideo) ? "object.item.videoitem" : "object.item.audioitem",
      'res'            => LOC_KARTINA_URL."/http-stream-recorder.php?".$url_data_string,
      'protocolInfo'   => "http-get:*:*:*",
      'upnp:album_art' => LOC_KARTINA_URL."/images/record.png"
   );
   
   return $retMediaItems;
}

/* -----------------------------------------------------------------\
|  Method: printItems
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: debug function to print all available items ... 
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
function printItems()
{
   $items = _pluginCreateChannelGroupList();
   echo "<h3>Groups</h3>\n";

   for ($i = 0; $i < count($items); $i++)
   {
      echo ($i + 1)."<br />\n";
      echo $items[$i]['dc:title'] . " (".$items[$i]['id'].")<br />\n";
      echo "<img src='".$items[$i]['upnp:album_art']."' alt='' /><br />\n";
      echo "<hr /><br />\n";
   }
   
   $items = _pluginCreateFavList();
   echo "<h3>Favorites</h3>\n";

   for ($i = 0; $i < count($items); $i++)
   {
      echo ($i + 1)."<br />\n";
      echo $items[$i]['dc:title'] . " (".$items[$i]['id'].")<br />\n";
      echo "<img src='".$items[$i]['upnp:album_art']."' alt='' /><br />\n";
      echo "<hr /><br />\n";
   }
   
   $items = _pluginCreateChannelList(1);
   echo "<h3>Channels in Group 1</h3>\n";

   for ($i = 0; $i < count($items); $i++)
   {
      echo ($i + 1)."<br />\n";
      echo $items[$i]['dc:title'] . " (".$items[$i]['id'].")<br />\n";
      echo "<img src='".$items[$i]['upnp:album_art']."' alt='' /><br />\n";
      echo "<hr /><br />\n";
   }
}

/* -----------------------------------------------------------------\
|  Method: inArchive
|  Begin: 10/5/2010 / 3:46p
|  Author: Jo2003
|  Description: check if show is in archive time
|
|  Parameters: timestamp
|
|  Returns: 0 ==> ok
|          -1 ==> any error
\----------------------------------------------------------------- */
function inArchive ($gmt)
{
   $now = time();

   if (($gmt >= ($now - MAX_ARCH_DAYS * DAY_IN_SECONDS)) // 14 days ...
      && ($gmt <= ($now - 600)))                         // 10 minutes ...
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

/* -----------------------------------------------------------------\
|  Method: simpleLog
|  Begin: 10/07/2010 / 14:10
|  Author: Jo2003
|  Description: write log to log file
|
|  Parameters: log entry
|
|  Returns: --
\----------------------------------------------------------------- */
function simpleLog($str)
{
   if (DOTRACE == "YES")
   {
      $f = fopen("/tmp/kart_umsp.log", "a+");
      if ($f)
      {
         fwrite($f, date("d.m.y H:i:s").": ".$str."\n");
         fclose($f);
      }
   }
}
//////////////////////////////////////////////////////////////////
// for debug only                                               //
//////////////////////////////////////////////////////////////////
if (isset($_GET['trace']))
{
   if ($_GET['trace'] === "yes")
   {
      printItems();
   }

   @unlink(KARTFAVLIST);
   @unlink(KARTCHANLIST);
}

?>
