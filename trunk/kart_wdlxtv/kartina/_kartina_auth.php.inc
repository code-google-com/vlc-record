<?php
require_once(dirname(__FILE__) . "/_defines.php.inc");
require_once(dirname(__FILE__) . "/kartinaAPI.php.inc");
require_once(dirname(__FILE__) . "/wdtvConfig.php.inc");

// ------------------------------------------------------------------
// create CWdtvConfig instance ...
// ------------------------------------------------------------------
$wdtvConf   = new CWdtvConfig (WDTV_CONF_FILE);

// initiate kartAPI class ... 
$kartAPI    = null;

// check if kartina cookie file exists ...
if (!file_exists(COOKIE_FILE)) // not authenticated ...
{
   // get username and password for kartina.tv ...
   $kartAcc  = $wdtvConf->getVal("KARTINA_ACCOUNT");
   $kartPw   = $wdtvConf->getVal("KARTINA_PASSWD");
   $eros     = $wdtvConf->getVal("KARTINA_EROTIC");

   // use default values if not stored in config file ...
   if (($kartAcc === "") || ($kartPw === ""))
   {
      $kartAcc = KARTINA_ACCOUNT;
      $kartPw  = KARTINA_PASS;
   }
   
   // ------------------------------------------------------------------
   // create kartinaAPI instance ...
   // ------------------------------------------------------------------
   $kartAPI = new kartinaAPI($kartAcc, $kartPw, KARTINA_HOST);

   // authenticate ...
   if (!$kartAPI->authenticate())
   {
      if (strtolower($eros) === "on")
      {
         $kartAPI->allowErotic(true);
      }
      else
      {
         $kartAPI->allowErotic(false);
      }
      
      $kartAPI->saveCookie();
   }
   else
   {
      // account expired ... ?
      $kartAcc = KARTINA_ACCOUNT;
      $kartPw  = KARTINA_PASS;
      
      $kartAPI->setConnectionData ($kartAcc, $kartPw, KARTINA_HOST);
      
      if (!$kartAPI->authenticate())
      {
         if (strtolower($eros) === "on")
         {
            $kartAPI->allowErotic(true);
         }
         else
         {
            $kartAPI->allowErotic(false);
         }
         
         $kartAPI->saveCookie();
      }
   }
}
else
{
   // ------------------------------------------------------------------
   // create kartinaAPI instance ...
   // ------------------------------------------------------------------
   $kartAPI = new kartinaAPI();
   
   // load cookie ...
   $kartAPI->loadCookie();
}

?>
