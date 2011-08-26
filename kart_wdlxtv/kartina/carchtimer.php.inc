<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 26.08.2011 / 10:35
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/
require_once(dirname(__FILE__) . "/_defines.php.inc");

/********************************************************************\
|  Class: CArchTimer
|  Date:  26.08.2011 / 10:35
|  Author: Jo2003
|  Description: implement a simple timer class which needs to be
|               triggered
|
\********************************************************************/
class CArchTimer
{
   private $_iStartStamp;
   private $_iStartGmt;
   private $_iLastTriggered;
   
   /* -----------------------------------------------------------------\
   |  Method: __construct
   |  Begin: 26.08.2011 / 11:45
   |  Author: Jo2003
   |  Description: constructs a class instance
   |
   |  Parameters: optional start time, if time is given, measurement 
   |              starts 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __contruct($startTime = -1)
   {
      if ($startTime !== -1)
      {
         $this->_iStartStamp    = $startTime;
         $this->_iStartGmt      = time();
         $this->_iLastTriggered = 0;
      }
      else
      {
         $this->_iStartStamp    = 0;
         $this->_iStartGmt      = 0;
         $this->_iLastTriggered = 0;
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: __destruct
   |  Begin: 26.08.2011 / 11:45
   |  Author: Jo2003
   |  Description: destroys the class instance
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
   |  Method: start
   |  Begin: 26.08.2011 / 11:45
   |  Author: Jo2003
   |  Description: start measurement
   |
   |  Parameters: start Timestamp
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function start ($startTime)
   {
      $this->_iStartStamp    = $startTime;
      $this->_iStartGmt      = time();
      $this->_iLastTriggered = 0;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: ping
   |  Begin: 26.08.2011 / 11:45
   |  Author: Jo2003
   |  Description: update timestamp file (if needed)
   |
   |  Parameters: --
   |
   |  Returns: true --> ok
   |          false --> error
   \----------------------------------------------------------------- */
   public function ping()
   {
      $rv  = false;
      $now = time();
      
      // write timestamp file only once in 5 seconds ...
      if (($time - $this->_iLastTriggered) >= 5)
      {
         $this->_iLastTriggered = $time;
         $rv = $this->writeTimeStamp();
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: writeTimeStamp
   |  Begin: 26.08.2011 / 11:45
   |  Author: Jo2003
   |  Description: write timestamp file
   |
   |  Parameters: --
   |
   |  Returns: true --> ok
   |          false --> error
   \----------------------------------------------------------------- */
   protected function writeTimeStamp ()
   {
      $rv = false;
      
      if (($fp = fopen (TIMESTAMPFILE, "w")) !== false)
      {
         $stopStamp = $this->_iStartStamp + ($this->_iLastTriggered - $this->_iStartGmt);
         
         $fwrite ($fp, (string)$stopStamp);
         
         $rv = true;
         
         fclose ($fp);
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getLastStopTime
   |  Begin: 26.08.2011 / 11:45
   |  Author: Jo2003
   |  Description: read timestamp file
   |
   |  Parameters: --
   |
   |  Returns: false --> error
   |            else --> last stop timestamp
   \----------------------------------------------------------------- */
   public getLastStopTime ()
   {
      $rv = false;
      
      if (file_exists(TIMESTAMPFILE))
      {
         $rv = file_get_contents(TIMESTAMPFILE);
      }
      
      return $rv;
   }
};

?>
