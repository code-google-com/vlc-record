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
   private $iArchStart;
   private $iStartTime_t;
   private $iLastPing;
   
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
   public function __construct()
   {
      $this->iArchStart   = 0;
      $this->iStartTime_t = 0;
      $this->iLastPing    = 0;
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
      $this->iArchStart   = $startTime;
      $this->iStartTime_t = time();
      $this->iLastPing    = 0;
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
      $rv   = false;
      $iNow = time();
      
      // write timestamp file only once in 5 seconds ...
      if (($iNow - $this->iLastPing) >= 5)
      {
         $this->iLastPing = $iNow;
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
		return file_put_contents(TIMESTAMPFILE, (string)($this->iArchStart + ($this->iLastPing - $this->iStartTime_t)));
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
   public function getLastStopTime ()
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
