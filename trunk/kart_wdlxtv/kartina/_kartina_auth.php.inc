<?php
require_once(dirname(__FILE__) . "/_defines.php.inc");
require_once(dirname(__FILE__) . "/kartinaAPI.php.inc");
require_once(dirname(__FILE__) . "/wdtvConfig.php.inc");

// ------------------------------------------------------------------
// create CWdtvConfig instance ...
// ------------------------------------------------------------------
$wdtvConf = new CWdtvConfig (WDTV_CONF_FILE);

// initiate kartAPI class ... 
$kartAPI  = null;

// check if kartina cookie file exists ...
if (!file_exists(COOKIE_FILE)) // not authenticated ...
{
   // get username and password for kartina.tv ...
   $kartAcc  = $wdtvConf->getVal("KARTINA_ACCOUNT");
   $kartPw   = $wdtvConf->getVal("KARTINA_PASSWD");

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
      $kartAPI->saveCookie();
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

// synchronize kartinas server time with time on WDTV Live ...
if (($serverTime = $kartAPI->getServerTime()) !== FALSE)
{
   wdtvConf->setTime($serverTime);
}


// get / save channel list if not already there ...
if (!file_exists(KARTCHANLIST))
{
   $kartAPI->saveChannelList();
}
else
{
   // is file older than 5 minutes ... ?
   if ((time() - filemtime(KARTCHANLIST)) > XML_RENEW_TIME)
   {
      $kartAPI->saveChannelList();
   }
}

// get / save favorites list if not already there ...
if (!file_exists(KARTFAVLIST))
{
   $kartAPI->saveFavorites();
}

?>
