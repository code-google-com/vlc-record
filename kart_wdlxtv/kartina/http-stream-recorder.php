<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 18.10.2010 / 14:30
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/

require_once(dirname(__FILE__) . "/_kartina_auth.php.inc");
require_once(dirname(__FILE__) . "/crecctrl.php.inc"); 

/*
 * How the stream recorder works:
 * - request stream url at kartina.tv
 * - use wget to save this stream to disk
 * - open the file wget is saving right now 
 *   for playing
 */
 
// get requested itemurl ...
$rawURL     = $_GET['itemurl'];
$parsedURL  = parse_url($rawURL);
$query      = $parsedURL['query'];
$isVideo    = (isset($_GET['is_video'])) ? (integer)$_GET['is_video'] : 1;
$queryitems = array();

parse_str($query, $queryitems);

// get channel id and timestamp ...
$cid        = (isset($queryitems['id']))  ? $queryitems['id']  :  7;
$gmt        = (isset($queryitems['gmt'])) ? $queryitems['gmt'] : -1;

// request stream url at kartina.tv ...
$strurl     = $kartAPI->getStreamUrl($cid, $gmt);

if ($strurl != "")
{
   // create output file name ...
   $folder   = $wdtvConf->getVal("KART_REC_FOLDER");
   
   // for now we create a simple file name
   // we'll maybe better next time ...
   $recfile  = ($gmt === -1) ? date("d_m_Y-H_i") : date("d_m_Y-H_i", $gmt);
   $recfile .= "__".$cid.".ts";
   
   // Some thoughts:
   // The problem here isn't the starting of the record but the stopping!
   // Starting wget in background doesn't stop it when this script ends.
   // So we try to use the destructor of a class here: The destructor
   // will be called when the last script which uses the class instance
   // ends. Since we create this instance here it should be destroyed
   // when this script ends ... theoretically.
   $ctrl = new CRecCtrl();
   
   if (!$ctrl->startRec($folder."/".$recfile, $strurl))
   {
      // avoid timeouts ...
      set_time_limit(0);
      
      // fake http answer ...
      if ($isVideo)
      {
         header("Content-Type: video/mpeg");
      }
      else
      {
         header("Content-Type: audio/mpeg"); // any mpeg audio ...
      }
      
      header("Content-Size: unknown");
      header("Content-Length: unknown");
      
      // give wget the time to start download ... 
      sleep(10);
      
      // open new generated output file ...
      if (file_exists($folder."/".$recfile))
      {
         $fp = fopen ($folder."/".$recfile , "rb");
         
         if ($fp)
         {
            // pass file content to player ...
            // Don't try to read all at once! It will stop
            // the player shortly.
            while (!feof($fp))
            {
               echo fread($fp, 8192);
               flush();
            }

            fclose($fp);
         }
      }
   }
} 

?>