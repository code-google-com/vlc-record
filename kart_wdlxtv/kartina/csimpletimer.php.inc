<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 28.10.2010 / 9:55
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/
require_once(dirname(__FILE__) . "/_defines.php.inc");

/********************************************************************\
|  Class: CSimpleTimer
|  Date:  28.10.2010 / 9:55
|  Author: Jo2003
|  Description: implement a simple timer class
|
\********************************************************************/
class CSimpleTimer
{
   private $tStartStamp;
   private $tGmtStart;
   private $doMeasure;
   
   /* -----------------------------------------------------------------\
   |  Method: __construct
   |  Begin: 28.10.2010 / 10:50
   |  Author: Jo2003
   |  Description: constructs a class instance
   |
   |  Parameters: optional start time, if time is given, measurement 
   |              starts 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __construct($start = -1)
   {
      if ($start != -1)
      {
         $this->tGmtStart = time();
         $this->doMeasure = true;
      }
      else
      {
         $this->tGmtStart = 0;
         $this->doMeasure = false;
      }
      
      $this->tStartStamp  = $start; 
   }
   
   /* -----------------------------------------------------------------\
   |  Method: startMeasurement
   |  Begin: 28.10.2010 / 10:50
   |  Author: Jo2003
   |  Description: start time measurement
   |
   |  Parameters: start time 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function startMeasurement ($start)
   {
      $this->tStartStamp = $start;
      $this->tGmtStart   = time();
      $this->doMeasure   = true;
   }

   /* -----------------------------------------------------------------\
   |  Method: __destruct / destructor
   |  Begin: 28.10.2010 / 10:50
   |  Author: Jo2003
   |  Description: clean on destruction, store measured timestamp
   |               if measurement was started
   |  Parameters: -- 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __destruct()
   {
      if ($this->doMeasure)
      {
         // compute runtime ...
         $runtime = time() - $this->tGmtStart;
         
         // compute stop time using start and runtime ...
         $stoptime = $this->tStartStamp + $runtime;
         
         // write stoptime to file ...
         if (($fp = fopen(TIMESTAMPFILE, "w")) !== false)
         {
            fwrite ($fp, (string)$stoptime);
            fclose ($fp);
         }
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getLastStopStamp
   |  Begin: 28.10.2010 / 10:50
   |  Author: Jo2003
   |  Description: get last stored time stamp 
   |
   |  Parameters: --
   |
   |  Returns: time stamp or false on error
   \----------------------------------------------------------------- */
   public function getLastStopStamp()
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
