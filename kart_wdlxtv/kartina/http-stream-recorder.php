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

// get stream url ...
$url     = $kartAPI->getStreamUrl($cid, $gmt);

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
         $recfp = fopen($recfolder."/".$recfile.".ts", "ab");
         // $recfp = fopen("/tmp/".$recfile.".ts", "ab");
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
            
            header("Content-Size: 65535");
            header("Content-Length: 65535");
         }

         header($line);
      }
      
      // from now on we get binary data only ...
      $eofcnt = 0;
            
      // avoid timeouts ...
      set_time_limit(0);
      
      // pass file content to player and - if needed -
      // to file on disk ...
      while ($eofcnt <= 300)
      {
         // read small chunk from socket ...
         $chunk = fread($sock, 8192);
         
         // write it to file if needed ...
         if ($recfp)
         {
            if (fwrite($recfp, $chunk) === false)
            {
               fclose($recfp);
               $recfp = false;
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
}

?>
