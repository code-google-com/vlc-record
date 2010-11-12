<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 8/27/2010 / 12:20
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/

/********************************************************************\
|  Class: CWdtvConfig
|  Date:  8/27/2010 / 12:20
|  Author: Jo2003
|  Description: PHP config class for WDTV Live
|
\********************************************************************/
class CWdtvConfig
{
   private $_confFile;
   private $_settings;
   
   /* -----------------------------------------------------------------\
   |  Method: __construct / constructor
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: construct object
   |
   |  Parameters: optional config file name
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __construct($filename = "")
   {
      $this->_settings = array();
      $this->_confFile = $filename;
      
      if ($filename != "")
      {
         $this->readSettings();
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: __destruct / destructor
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: clean at destruction 
   |
   |  Parameters: --
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __destruct()
   {
      // nothing to do so far ...
   }
   
   /* -----------------------------------------------------------------\
   |  Method: setConfigFile
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: set config file name and read settings
   |
   |  Parameters: config file name
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function setConfigFile($filename)
   {
      $this->_confFile = $filename;
      $this->readSettings();
   }
   
   /* -----------------------------------------------------------------\
   |  Method: writeConf
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: write one config value to file and store cache it 
   |               internally
   |  Parameters: key name, value
   |
   |  Returns: 0 --> ok
   |   any other --> any error
   \----------------------------------------------------------------- */
   public function writeConf($key, $val)
   {
      $rv     = 0;
      $output = array();
      
      exec ("sudo config_tool -c ".$key."=".$val, $output, $rv);
      
      if ($rv === 0)
      {
         $this->_settings[$key] = $val;
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: readSettings
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: read settings from config file and cache it
   |               internally
   |  Parameters: --
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function readSettings()
   {
      $content = file($this->_confFile);
      $matched = "";
      $count   = 0;
      
      unset($this->_settings);
      $this->_settings = array();
      
      
      for ($i = 0; $i < count($content); $i ++)
      {
         $count   = 0;
         $matched = preg_replace("/^[ \t]*([a-zA-Z0-9-_]+)[ \t=]+'([^']+)'.*$/", "$1<<[:]>>$2", $content[$i], -1, $count);
         if ($count > 0)
         {
            $tmpArray = explode("<<[:]>>", $matched);
            
            $this->_settings[trim($tmpArray[0])] = trim($tmpArray[1]);
         }
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getVal
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: get value for specified key from cache
   |
   |  Parameters: key name
   |
   |  Returns: value or empty string if there is no such value
   \----------------------------------------------------------------- */
   public function getVal($key)
   {
      if (isset($this->_settings[$key]))
      {
         return $this->_settings[$key];
      }
      else
      {
         return "";
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: setTime
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: set device time to value given as param
   |
   |  Parameters: unix timestamp
   |
   |  Returns: 0 --> ok
   |          -1 --> any error
   \----------------------------------------------------------------- */
   public function setTime($timeStamp)
   {
      $output = array();
      $rv     = -1;
      
      // convert timestamp into format [YYYY.]MM.DD-hh:mm[:ss] ...
      // format -s @ doesn't work cause WDTV uses busybox ...
      $strTime = date("Y.m.d-H:i:s", $timeStamp);
      
      exec ("sudo date -s ".$strTime, $output, $rv);
         
      if ($rv != 0)
      {
         $rv = -1;
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: setTimezone
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: set devices time zone
   |
   |  Parameters: time zone shortcut (e.g. "CET")
   |
   |  Returns: 0 --> ok
   |        else --> any error
   \----------------------------------------------------------------- */
   public function setTimezone($tz)
   {
      return $this->writeConf("TIMEZONE", $tz);
   }
   
   /* -----------------------------------------------------------------\
   |  Method: isTimeSet
   |  Begin: 8/27/2010 / 15:30
   |  Author: Jo2003
   |  Description: check if time was already set
   |
   |  Parameters: --
   |
   |  Returns: true --> was set
   |          false --> wasn't set
   \----------------------------------------------------------------- */
   public function isTimeSet()
   {
      // date starts at 01.01.2000 when box starts ...
      // so test, if we're later then 2009 to find out that
      // we're already synchronized ...
      if (time() > (40 * 365 * 24 * 3600)) // ~40 years ... means somewhere in 2009
      {
         // means we are much later that 2000 (date as wdtv starts) ...
         return true;
      }
      else
      {
         return false;
      }
   }
}
?>
