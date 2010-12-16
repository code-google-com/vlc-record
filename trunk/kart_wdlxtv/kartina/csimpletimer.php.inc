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
require_once(dirname(__FILE__) . "/ctimer.php");

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
   private $doMeasure;
   private $timer;
   
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
      $this->timer = new Timer();
      
      if ($start != -1)
      {
         $this->doMeasure = true;
         $this->timer->start();
      }
      else
      {
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
      $this->doMeasure   = true;
      $this->timer->start();
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
      $this->getAndSaveTime();
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getAndSaveTime
   |  Begin: 08.12.2010 / 10:20
   |  Author: Jo2003
   |  Description: get measured time, compute stop stamp, save to file
   |               if measurement was started
   |  Parameters: -- 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function getAndSaveTime ()
   {
      if ($this->doMeasure)
      {
         // compute runtime ...
         $this->timer->stop();
         $runTime  = (integer)round($this->timer->get(Timer::SECONDS));
         
         // compute stop time using start and runtime ...
         $stoptime = $this->tStartStamp + $runTime;
         
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
