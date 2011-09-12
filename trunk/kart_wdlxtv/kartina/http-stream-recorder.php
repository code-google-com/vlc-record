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
require_once(dirname(__FILE__) . "/carchtimer.php.inc");

// How the stream recorder works:
// - request stream url at kartina.tv
// - open the stream
// - send stream data to player 
// - save stream data to file on disk
 
// we get channel id (cid), record flag (dorec), timestamp (gmt) 
// video flag (is_video) and record file name (recfile) ...
$query_array = array();
parse_str($_SERVER['QUERY_STRING'], $query_array);

$cid     = isset($query_array['cid'])      ? (integer)$query_array['cid']      :     7;
$dorec   = isset($query_array['dorec'])    ? (boolean)$query_array['dorec']    : false;
$gmt     = isset($query_array['gmt'])      ? (integer)$query_array['gmt']      :    -1;
$isVideo = isset($query_array['is_video']) ? (boolean)$query_array['is_video'] :  true;
$recfile = isset($query_array['recfile'])  ? (string)$query_array['recfile']   :    "";
$offset  = isset($query_array['offset'])   ? (integer)$query_array['offset']   :     0;

////////////////////////////////////////////////////////////////////////////////
// forward / backward jumping 

// 1. We want relative time jumping!
// 2. To get this, we need to measure the play time.
// 3. Measuring the play time is done using a simple timer class.
// The trick in the timer class is to use the destructor to write the
// elapsed time to a file. Without this trick we have no way to find out 
// the end of this script. 

// forward declaration for timer class ...
$sTimer = NULL;

// are we in archive play ... ?
if (!$dorec && ($gmt != -1))
{
   // create timer class ...
	$sTimer = new CArchTimer();
	
   // is any offset given ...
   if ($offset != 0)
   {
      // get last stop timestamp ...
      if (($lastStamp = $sTimer->getLastStopTime()) === false)
      {
         // can't get timestamp --> use given gmt to proceed with offset ...
         $gmt += $offset;
      }
      else
      {
         simpleLog(__FILE__.":".__LINE__." gmt: ".$gmt.", LastStop: "
            .$lastStamp.", Offset: "
            .$offset.", gmt new: ".($lastStamp + $offset));
         
         // got last timestamp --> compute new gmt 
         // using last stop stamp with offset ...
         $gmt = $lastStamp + $offset;
      }
   }
      
   // start measurement ... 
   $sTimer->start ($gmt);
}

// get stream url ...
$url = $kartAPI->getStreamUrl($cid, $gmt);

if ($url != "")
{
   $url_array = parse_url($url);
   $host      = $url_array['host'];
   $path      = isset($url_array['path'])  ? $url_array['path']          : "/";
   $port      = isset($url_array['port'])  ? (integer)$url_array['port'] : 80;
   $query     = isset($url_array['query']) ? "?".$url_array['query']     : "";
   
   $errno     = 0;
   $errstr    = "";
   
   $sock      = fsockopen($host, $port, $errno, $errstr, 30);
   
   if ($sock === false)
   {
      echo $errstr." (".$errno.")<br />\n";
   }
   else
   {
      // create / open record file if needed ...
      $recfp     = false;
      if ($dorec)
      {
         $recfolder = $wdtvConf->getVal("KART_REC_FOLDER");
         
         // do we have a rec file name ... ?
         if ($recfile == "")
         {
            $recfile  = ($gmt === -1) ? date("d_m_Y-H_i") : date("d_m_Y-H_i", $gmt);
            $recfile .= "__".$cid;
         }
         
         // open rec file ...
         $recfp = fopen($recfolder."/".$recfile.".ts", "wb");
      }
      
      // create http get request ...
      $req  = "GET ". $path . $query ." HTTP/1.1" ."\r\n";
      $req .= "User-Agent: Wget/1.12 (elf)" ."\r\n";
      $req .= "Host: " . $host . "\r\n";
      $req .= "Cache-Control: no-cache" ."\r\n";
      $req .= "Connection: Close"."\r\n"."\r\n";
      
      // send request ...
      fwrite($sock, $req);
      
      // header passed flag ....
      $header_passed = false;
      
      // read header from answer ...
      while ($header_passed === false)
      {
         $line = fgets($sock);

         // empty line tells about the end of the header ...
         if (trim($line) == "") 
         {
            $header_passed = true;
            
            if ($isVideo)
            {
               header("Content-Type: video/mpeg");
            }
            else
            {
               // header("Content-Type: audio/x-m4a"); // mp4 audio ...
               // header("Content-Type: audio/x-aac"); // aac audio ...
               header("Content-Type: audio/mpeg");     // any mpeg audio ...
            }
         }
         
         header("Content-Size: 65535");
         header("Content-Length: 65535");

         // pups_log($line);
         header($line);
      }
      
      // from now on we get binary data only ...
      $eofcnt     = 0;
      
      // file buffer help variables ...
      $chunkfile  = "";
      $chunkcount = 0;
            
      // avoid timeouts ...
      set_time_limit(0);
      
      // pass file content to player and - if needed -
      // to file on disk ...
      while ($eofcnt <= 300)
      {
         // read small chunk from socket ...
         $chunk = fread($sock, 8192);
         
         // tell the timer that we're still here ...
         if ($sTimer)
         {
            $sTimer->ping();
         }
         
         // write it to file if needed ...
         if ($recfp)
         {
            // add chunk to file buffer ...
            if ($chunkcount === 0)
            {
               $chunkfile  = $chunk;
            }
            else
            {
               $chunkfile .= $chunk;
            }
            $chunkcount ++;
         
            // don't write any small part to file ...
            // collect a good amount of data and then
            // flush at once ...
            if ($chunkcount >= 120)
            {
               fwrite($recfp, $chunkfile);
               
               $chunkcount = 0;
               $chunkfile  = "";
            }
         }
         
         // pass chunk to player ...
         echo $chunk;
         
         // simple eof check ...
         if(feof($sock))
         {
            // on eof we'll wait a second ...
            $eofcnt ++;
            usleep(1000);
         }
         else
         {
            $eofcnt = 0;
         }
      }
      
      if ($recfp)
      {
         fclose($recfp);
      }

      fclose($sock);
   }
   
   // force destruction of timer class ...
   if ($sTimer)
   {
      unset ($sTimer);
   }
}

?>
