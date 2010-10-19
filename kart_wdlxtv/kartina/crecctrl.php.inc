<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 19.10.2010 / 8:35
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/

// -------------------------------------------------------------------
//  defines section ...
// -------------------------------------------------------------------
define ('PIDFILE', '/var/run/kartrec.pid');
define ('LOGFILE', '/tmp/CRecCtrl.log');
define ('WGETLOG', '/tmp/kart_wget.log');


/********************************************************************\
|  Class: CRecCtrl
|  Date:  19.10.2010 / 8:35
|  Author: Jo2003
|  Description: (very) )simple record control for WDTV Live 
|
\********************************************************************/
class CRecCtrl 
{
   private $_logfile;
   private $_recpid;

   /* -----------------------------------------------------------------\
   |  Method: __construct
   |  Begin: 19.10.2010 / 8:35
   |  Author: Jo2003
   |  Description: constructs a class instance
   |
   |  Parameters: --
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __construct()
   {
      $this->_recpid = -1;
      
      if (DOTRACE == "YES")
      {
        $this->_logfile = fopen(LOGFILE, "a+");
      }
      
      $this->log("construct ...\n");
   }

   /* -----------------------------------------------------------------\
   |  Method: __destruct
   |  Begin: 19.10.2010 / 8:35
   |  Author: Jo2003
   |  Description: clean on destruction
   |
   |  Parameters: --
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __destruct()
   {
      $this->log("destruct ...\n");
      $this->stopRec();
      
      if (DOTRACE == "YES")
      {
         fclose ($this->_logfile);
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: stopRec
   |  Begin: 19.10.2010 / 9:25
   |  Author: Jo2003
   |  Description: stop record process
   |
   |  Parameters: --
   |
   |  Returns: 0 ==> ok
   |          -1 ==> any error
   \----------------------------------------------------------------- */
   public function stopRec($sig = SIGTERM)
   {
      $rv = -1;
      if ($this->_recpid > -1)
      {
         $this->log("Stopping record with pid ".$this->_recpid);
         
         // send term signal ...
         if (posix_kill($this->_recpid, $sig))
         {
            $this->_recpid = -1;
            @unlink (PIDFILE);
            $rv = 0;
         }
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: startRec
   |  Begin: 19.10.2010 / 9:10
   |  Author: Jo2003
   |  Description: clean on destruction
   |
   |  Parameters: file name to save stream to, url for download
   |
   |  Returns: 0 ==> ok
   |          -1 ==> any error
   \----------------------------------------------------------------- */
   public function startRec($fileName, $srcUrl)
   {
      $rv  = -1; 
      // create command line ...
      $cmd = "wget ".((DOTRACE == "YES") ? "-o ".WGETLOG." " : "")
            ."-O \"".$fileName."\" \"".$srcUrl
            ."\" >/dev/null 2>&1 &; WGETPID=\$!; "
            ."echo \${WGETPID} >".PIDFILE;
      
      $this->log("Starting Download using following command line:\n --> ".$cmd);
            
      // execute command ...
      @shell_exec($cmd);
      
      // get process id from started process ...
      $tmpPid = file_get_contents(PIDFILE);
      
      if ($tmpPid)
      {
         $this->_recpid = (integer)trim($tmpPid);
         
         if ($this->_recpid > -1)
         {
            $this->log("Record process started with pid ".$this->_recpid);
            $rv = 0;
         }
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: log
   |  Begin: 8/11/2010 / 9:24a
   |  Author: Jo2003
   |  Description: write string to logfile
   |
   |  Parameters: string to log
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function log($str)
   {
      if (DOTRACE == "YES")
      {
         fwrite($this->_logfile, $str."\n");
      }   
   }
   
   /* -----------------------------------------------------------------\
   |  Method: isRunning
   |  Begin: 19.10.2010 / 8:40
   |  Author: Jo2003
   |  Description: check if record is still running
   |
   |  Parameters: --
   |
   |  Returns: TRUE ==> is running
   |          FALSE ==> isn't running
   \----------------------------------------------------------------- */
   public function isRunning()
   {
      $rv = FALSE;
      
      if ($this->_recpid > -1)
      {
         $rv = posix_kill($this->_recpid, 0);
      }
      
      return $rv;
   }
};
?>