<?php
require_once(dirname(__FILE__) . "/_defines.php.inc");

/* -----------------------------------------------------------------\
|  Method: createZoneinfoArray
|  Begin: 10/14/2010 / 12:42
|  Author: Jo2003
|  Description: create timezone array
|
|  Parameters: --
|
|  Returns: all timezones in an array
\----------------------------------------------------------------- */
function createZoneinfoArray ()
{
   $timeZones = array();
   
   addTimeZone("", $timeZones);
   
   sort($timeZones);
   
   return $timeZones; 
}

/* -----------------------------------------------------------------\
|  Method: addTimeZone
|  Begin: 10/14/2010 / 12:45
|  Author: Jo2003
|  Description: add timezones in given directory, 
|               ready for recursive call
|
|  Parameters: dir to add, reference to tz array
|
|  Returns: --
\----------------------------------------------------------------- */
function addTimeZone ($dir, &$zones)
{
   $thisdir = ($dir === "") ? ZONEDIR : ZONEDIR."/".$dir;
   $zonedir = opendir($thisdir);
   
   if ($zonedir)
   {
      while (($file = readdir($zonedir)) !== false)
      {
         if (isTz($file))
         {
            if (is_dir($thisdir."/".$file))
            {
               addTimeZone(($dir === "") ? $file : $dir."/".$file, $zones);
            }
            else
            {
               $zones[] = ($dir === "") ? $file : $dir."/".$file;
            }
         }
      }
      
      closedir($zonedir);
   }
}

/* -----------------------------------------------------------------\
|  Method: isTz
|  Begin: 10/14/2010 / 19:10
|  Author: Jo2003
|  Description: quick 'n' dirty check for timezone:  
|               - there shouldn't be a '.' in name
|               - first char must be uppercase 
|
|  Parameters: reference to file name
|
|  Returns: true --> timezone
|          false --> no timezone
\----------------------------------------------------------------- */
function isTz(&$file) 
{
   $rv = false;
   
   if (strstr($file, ".") === false)
   {
      $i = substr($file, 0, 1);
      
      if (strtoupper($i) === $i)
      {
         $rv = true;
      }
   }
   return $rv;
}

?>
