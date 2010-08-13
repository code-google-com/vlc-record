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
   
   parse_str($prmQuery, $queryData);
   
   if (!isset($queryData['changrp']))
   {
      $items = _pluginCreateChannelGroupList();
   }
   else
   {
      if ($queryData['changrp'] == 0) // favorites ...
      {
         $items = _pluginCreateFavList();
      }
      else
      {
         $items = _pluginCreateChannelList($queryData['changrp']);
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

   $all = $channels->length;

   for ($i = 0; $i < $all; $i++)
   {
      $url = LOC_KARTINA_URL."/stream.php?id=".$channels->item($i)->nodeValue;
      
      $url_data = array('itemurl' => $url);
      $url_data_string = http_build_query($url_data);

      $retMediaItems[] = array (
         'id'             => LOC_KARTINA_UMSP."/http-stream?".$url,
         'dc:title'       => $names->item($i)->nodeValue,
         'upnp:class'     => "object.item.videoitem",
         'res'            => LOC_KARTINA_URL."/http-stream-proxy.php?".$url_data_string,
         'protocolInfo'   => "http-get:*:*:*",
         'upnp:album_art' => KARTINA_HOST.$icons->item($i)->nodeValue,
      );
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
   $data       = array('changrp' => '0');
   $dataString = http_build_query($data, 'pluginvar_');
   
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
      $data       = array('changrp' => $groups->item($i)->nodeValue);
      $dataString = http_build_query($data, 'pluginvar_');
      
      $retMediaItems[] = array (
         'id'             => LOC_KARTINA_UMSP."/http-stream?".$dataString,
         'dc:title'       => $names->item($i)->nodeValue,
         'upnp:class'     => 'object.container',
         'upnp:album_art' => LOC_KARTINA_URL."/images/folder.png",
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
      $url      = LOC_KARTINA_URL."/stream.php?id=".$cid;
      
      $url_data = array('itemurl' => $url);
      $url_data_string = http_build_query($url_data);

      $retMediaItems[] = array (
         'id'             => LOC_KARTINA_UMSP."/http-stream?".$url,
         'dc:title'       => $name,
         'upnp:class'     => "object.item.videoitem",
         'res'            => LOC_KARTINA_URL."/http-stream-proxy.php?".$url_data_string,
         'protocolInfo'   => "http-get:*:*:*",
         'upnp:album_art' => KARTINA_HOST.$icon,
      );
   }
   
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