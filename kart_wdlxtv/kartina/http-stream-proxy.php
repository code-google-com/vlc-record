<?php

require_once(dirname(__FILE__) . "/_kartina_auth.php.inc");

$rawURL = $_GET['itemurl'];
$parsedURL = parse_url($rawURL);
$itemHost = $parsedURL['host'];

if(isset($parsedURL['query']))
{
   $itemPath = $parsedURL['path'].'?'.$parsedURL['query'];
}
else
{
   $itemPath = $parsedURL['path'];
}

$itemPort = $parsedURL['port'];

_http_streamGet($itemHost, ($itemPath ? $itemPath : "/"), ($itemPort ? $itemPort : 80));

function _http_streamGet($prmHost, $prmPath, $prmPort) 
{
   # print "host: ".$prmHost."\n";
   # print "port: ".$prmPort."\n";
   # print "path: ".$prmPath."\n";
   $fp = fsockopen($prmHost, $prmPort, $errno, $errstr, 30);
   if (!$fp) 
   {
      echo "$errstr ($errno)<br />\n";
   } 
   else 
   {
      $out = "GET ". $prmPath ." HTTP/1.1" ."\r\n";
      $out .= "User-Agent: Wget/1.12 (elf)" ."\r\n";
      $out .= "Host: " . $prmHost . "\r\n";
      $out .= "Cache-Control: no-cache" ."\r\n";
      $out .= "Connection: Close"."\r\n"."\r\n";
      fwrite($fp, $out);
      $headerpassed = false;

      while ($headerpassed == false) 
      {
         $line = fgets($fp);
         list($tag, $value) = explode(": ", $line, 2);
         if (stristr($tag, 'Location')) 
         {
            $target_url = trim($value);
            $url_data_string = http_build_query(array('itemurl' => $target_url));
            header("Location: ".LOC_KARTINA_URL."/http-stream-proxy.php?".
                   $url_data_string);
            continue;
         }

         if (trim($line) == "") 
         {
            $headerpassed = true;
            header("Content-Type: video/mpeg");
            header("Content-Size: 65535");
            header("Content-Length: 65535");
         }

         header($line);
      }

      set_time_limit(0);
      fpassthru($fp);
      fclose($fp);
   }
}
?>