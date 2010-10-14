<?php

define ('ZONEDIR', "/usr/share/zoneinfo");

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
function addTimeZone (string $dir, array &$zones)
{
   $thisdir = ($dir === "") ? ZONEDIR : ZONEDIR."/".$dir;
   $zonedir = opendir($thisdir);
   
   if ($zonedir)
   {
      while (($file = readdir($zonedir)) !== false)
      {
         if (strstr($file, ".") === false)
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

?>