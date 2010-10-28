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

define ('TIMESTAMPFILE', '/tmp/.timestamp');

/********************************************************************\
|  Class: CSimpleTimer
|  Date:  28.10.2010 / 9:55
|  Author: Jo2003
|  Description: implement a simple timer class
|
\********************************************************************/
class CSimpleTimer
{
   private $tOffset;
   private $tGmtStart;
   private $doMeasure;
   
   /* -----------------------------------------------------------------\
   |  Method: __construct
   |  Begin: 28.10.2010 / 10:50
   |  Author: Jo2003
   |  Description: constructs a class instance
   |
   |  Parameters: optional offet time, if time is given, measurement 
   |              starts 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __construct($offset = -1)
   {
      if ($offset != -1)
      {
         $this->tGmtStart = time();
         $this->doMeasure = true;
      }
      else
      {
         $this->tGmtStart = 0;
         $this->doMeasure = false;
      }
      
      $this->tOffset   = $offset; 
   }
   
   /* -----------------------------------------------------------------\
   |  Method: startMeasurement
   |  Begin: 28.10.2010 / 10:50
   |  Author: Jo2003
   |  Description: start time measurement
   |
   |  Parameters: offset time 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function startMeasurement ($offset)
   {
      $this->tOffset   = $offset;
      $this->tGmtStart = time();
      $this->doMeasure = true;
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
         
         // compute stop time using offset and runtime ...
         $stoptime = $this->tOffset + $runtime;
         
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
      return file_get_contents(TIMESTAMPFILE);
   }
};
?>