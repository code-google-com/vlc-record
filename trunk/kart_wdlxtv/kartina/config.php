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

// include kartinaAPI class instance ...
require_once(dirname(__FILE__)."/_kartina_auth.php.inc");

// define column count ...
define (CHANCOLS, 8);
define (FAVCOLS, 4);

///////////////////////////////////////////////////////////////////////////////
//                            function section                               //
///////////////////////////////////////////////////////////////////////////////

/* -----------------------------------------------------------------\
|  Method: makeHeader
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: print html header
|
|  Parameters: optional title
|
|  Returns: --
\----------------------------------------------------------------- */
function makeHeader($title = "")
{
   header("Cache-Control: no-cache, must-revalidate"); // HTTP/1.1
   header("Expires: Sat, 01 Jan 2000 05:00:00 GMT");
   
   echo "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN'\n"
       ."   'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>\n" 
       ."<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>\n" 
       ."<head>\n" 
       ."<meta http-equiv='content-type' content='text/html; charset=UTF-8' />\n" 
       ."<title>".$title."</title>\n" 
       ."<style type='text/css'>\n" 
       ."<!--\n"
       ."   body {font-family: Verdana, Tahoma, Arial, sans-serif; font-size: 11px; margin: 0px; padding: 0px; text-align: left; color: #3A3A3A; background-color: #F4F4F4}\n"
       ."   table {width: 10%; background-color:#ddd;color:white;padding:0px;border:1px outset}\n"
       ."  .navitab {background-color:#333333; color:white; padding:0px; border: 1px outset;}\n"
       ."  .tdnavitab {background-color:#ddd; color:black; padding:5px; border: 0px;}\n"
       ."  .favtab  {background-color:#333333; color:white; padding:0px; border: 1px outset;}\n"
       ."  .chantab {background-color:#333333; color:white; padding:0px; border: 1px outset;}\n"
       ."   img {border: 0px;}\n"
       ."   td {width:50%;text-align:center;vertical-align:middle;border:0px;padding:2px}\n"
       ."   th {background-color:#333333;color:white; text-align: left;}\n"
       ."   td.row {background-color:white;color:black;}\n"
       ."   a:link, a:visited, a:active { text-decoration: underline; color: #444444;}\n"
       ."   a:hover { text-decoration: underline; color: #0482FE;}\n"
       ."-->\n"
       ."</style>\n" 
       ."</head>\n"
       ."<body>\n";
}

/* -----------------------------------------------------------------\
|  Method: makeFooter
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: print html footer
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
function makeFooter()
{
   echo "</body>\n"
       ."</html>";
}

/* -----------------------------------------------------------------\
|  Method: makeFavTab
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: create favotites table
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
function makeFavTab()
{
   $colcount      = 1;
   $domFavList    = new DOMDocument();
   $domChanList   = new DOMDocument();
   
   $domFavList->load(KARTFAVLIST);
   $domChanList->load(KARTCHANLIST);
   
   $xpfav  = new DOMXpath($domFavList);
   $xpchan = new DOMXpath($domChanList);
   
   $favarray  = array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
   $favorites = $xpfav->query("/response/favorites/item/channel_id");
   
   echo "<table class='favtab' border='0' cellspacing='1' cellpadding='1'>\n"
       ."<colgroup span='".FAVCOLS."' />\n"
       ."<tr><th colspan='".FAVCOLS."'>Фавориты</th></tr>\n";
   
   for ($i = 0; $i < count($favarray); $i ++)
   {
      if (($colcount % FAVCOLS) === 1)
      {
         echo "<tr>\n";
      }
      
      // check if this place is used ...
      $favitem  = $xpfav->query("/response/favorites/item[place='".$favarray[$i]."']");
      
      if ($favitem->length)
      {
         $cid      = $xpfav->query("channel_id", $favitem->item(0))->item(0)->nodeValue;
         $place    = $xpfav->query("place", $favitem->item(0))->item(0)->nodeValue;

         $chanitem = $xpchan->query("/response/groups/item/channels/item[id='".$cid."']");
         $chan     = $chanitem->item(0); // there is only one such item ...
         
         $icon     = $xpchan->query("icon", $chan)->item(0)->nodeValue;
         $name     = $xpchan->query("name", $chan)->item(0)->nodeValue;
         
         echo "<td><a href='".$_SERVER['PHP_SELF']."?act=fav&amp;del=".$place."'>"
             ."<img src='".KARTINA_HOST.$icon."' alt='channel icon' title='удалить ".$name." от фаворитов' />"
             ."</a></td>\n";
      }
      else
      {
         echo "<td><img src='".LOC_KARTINA_URL."/images/help.png' alt='empty favorite' title='пустое место для фаворита' />"
             ."</td>\n";
      }
          
      if (($colcount % FAVCOLS) === 0)
      {
         echo "</tr>\n";
      }

      $colcount ++;
   }
   
   $finalcols = FAVCOLS - (($colcount - 1) % FAVCOLS);

   if ($finalcols != FAVCOLS)
   {
      echo "<td colspan='".$finalcols."'>&nbsp;</td>\n"
          ."</tr>\n";
   }
   echo "</table>\n";
}

/* -----------------------------------------------------------------\
|  Method: makeChanTab
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: create channel table
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
function makeChanTab ()
{
   $colcount      = 1;
   $domFavList    = new DOMDocument();
   $domChanList   = new DOMDocument();
   
   $domFavList->load(KARTFAVLIST);
   $domChanList->load(KARTCHANLIST);
   
   $xpfav  = new DOMXpath($domFavList);
   $xpchan = new DOMXpath($domChanList);
   
   $channels = $xpchan->query("/response/groups/item/channels/item/id");
   $names    = $xpchan->query("/response/groups/item/channels/item/name");
   $icons    = $xpchan->query("/response/groups/item/channels/item/icon");
   
   $chancount  = $channels->length;
   
   if ($chancount > 0)
   {
      echo "<table class='chantab' border='0' cellspacing='1' cellpadding='1'>\n"
          ."<colgroup span='".CHANCOLS."' />\n"
          ."<tr><th colspan='".CHANCOLS."'>Список каналов</th></tr>\n";
   }

   for ($i = 0; $i < $chancount; $i ++)
   {
      $cid      = $channels->item($i)->nodeValue;
      $icon     = $icons->item($i)->nodeValue;
      $name     = $names->item($i)->nodeValue;
      
      // is channel there as favorite ... ?
      $fav      = $xpfav->query("/response/favorites/item[channel_id='".$cid."']")->length;
      
      if (!$fav)
      {
         if (($colcount % CHANCOLS) === 1)
         {
            echo "<tr>\n";
         }
      
         // only display channel if it isn't already a favorite ...
         echo "<td><a href='".$_SERVER['PHP_SELF']."?act=fav&amp;add=".$cid."'>"
             ."<img src='".KARTINA_HOST.$icon."' alt='channel icon' title='добавить ".$name." в фавориты' />"
             ."</a></td>\n";

         if (($colcount % CHANCOLS) === 0)
         {
            echo "</tr>\n";
         }
         
         $colcount ++;
      }
   }
   
   if ($chancount > 0)
   {
      $finalcols = CHANCOLS - (($colcount - 1) % CHANCOLS);

      if ($finalcols != CHANCOLS)
      {
         echo "<td colspan='".$finalcols."'>&nbsp;</td>\n"
             ."</tr>\n";
      }
      
      echo "</table>\n";
   }
}

/* -----------------------------------------------------------------\
|  Method: makeNavi
|  Begin: 8/13/2010 / 1:24p
|  Author: Jo2003
|  Description: create navigation table
|
|  Parameters: --
|
|  Returns: --
\----------------------------------------------------------------- */
function makeNavi()
{
   echo "<table class='navitab' border='0' cellpadding='1' cellspacing='1'>\n<tr>\n"
       ."<td class='tdnavitab' nowrap='nowrap'>[<a href='".$_SERVER['PHP_SELF']."'>Домой</a>]</td>\n"
       ."<td class='tdnavitab' nowrap='nowrap'>[<a href='".$_SERVER['PHP_SELF']."?act=fav'>Фавориты</a>]</td>\n"
       ."<td class='tdnavitab' nowrap='nowrap'>[<a href='".$_SERVER['PHP_SELF']."?act=delxml'>Удалить файлы XML</a>]</td>\n"
       ."<td class='tdnavitab' nowrap='nowrap'>[<a href='".$_SERVER['PHP_SELF']."?act=timeshift'>Задержка времени</a>]</td>\n"
       ."</tr>\n</table>\n";
}

///////////////////////////////////////////////////////////////////////////////
//                            request handling                               //
///////////////////////////////////////////////////////////////////////////////
if (isset($_GET['act']))
{
   // favorite request ...
   if ($_GET['act'] === "fav")
   {
      // delete favorite ...
      if (isset($_GET['del']))
      {
         $kartAPI->setFavorite($_GET['del'], 0);
         header("Location: ".$_SERVER['PHP_SELF']."?act=fav");
      }
      
      // add favorite ...
      if (isset($_GET['add']))
      {
         $favs     = $kartAPI->getFavorites();
         $emptyfav = array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
         
         for ($i = 0; $i < count($favs); $i ++)
         {
            $tmpfav = array();
            for ($j = 0; $j < count($emptyfav); $j++)
            {
               if ($favs[$i]['place'] != $emptyfav[$j])
               {
                  $tmpfav[] = $emptyfav[$j]; 
               }
            }
            $emptyfav = $tmpfav;
         }
         
         if (isset($emptyfav[0]))
         {
            $kartAPI->setFavorite($emptyfav[0], $_GET['add']);
            header("Location: ".$_SERVER['PHP_SELF']."?act=fav");
         }
      }
   }
   
   // delete XML request ...
   if ($_GET['act'] === "delxml")
   {
      @unlink(KARTCHANLIST);
      @unlink(KARTFAVLIST);
      header("Location: ".$_SERVER['PHP_SELF']);
   }
} 

// create document header ...
makeHeader(isset($_GET['act']) ? $_GET['act'] : "Установка");

// show navi bar ...
makeNavi();

// show fav tables ...
if (isset($_GET['act']))
{
   if ($_GET['act'] === "fav")
   {
      echo "<table>\n<tr>\n<td style='vertical-align: top;'>\n";
      makeFavTab();
      echo "</td>\n<td>\n";
      makeChanTab();
      echo "</td>\n</tr>\n</table>\n";
   }
}
else
{
   echo "Kartina Config Panel: Choose from actions above!";
}

// close document ...
makeFooter();
?>