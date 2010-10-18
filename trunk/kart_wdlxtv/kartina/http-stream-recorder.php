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
   
   // create wget command ...
   // some thoughts:
   // 1. This php thread is the parent of the created process.
   // 2. The started wget is send to background so exec returns immediately.
   // 3. Stopping play / record will end this thread and therefore end wget.
   // 4. This is all theoretical ;-)
   $cmd      = "wget -o /tmp/kart_stream_rec.log -O \"".$folder."/".$recfile."\" ".$strurl." >/dev/null 2>&1 &";

   exec($cmd);
   
   // give wget the time to start download ... 
   sleep(5);
   
   // open new generated output file ...
   if (file_exists($folder."/".$recfile))
   {
      $fp = fopen ($folder."/".$recfile , "r");
      
      if ($fp)
      {
         // fake http answer ...
         if ($isVideo)
         {
            header("Content-Type: video/mpeg");
         }
         else
         {
            header("Content-Type: audio/mpeg"); // any mpeg audio ...
         }
         
         header("Content-Size: 65535");
         header("Content-Length: 65535");
   
         // unset time limit ...
         set_time_limit(0);
         
         // pass file content to player ...
         fpassthru($fp);
         fclose($fp);
      }
   }
} 

?>