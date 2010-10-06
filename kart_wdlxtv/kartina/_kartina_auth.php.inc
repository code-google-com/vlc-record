<?php

require_once(dirname(__FILE__) . "/kartinaAPI.php.inc");
require_once(dirname(__FILE__) . "/wdtvConfig.php.inc");

// ------------------------------------------------------------------
// default (demo) connectioin data ...
// ------------------------------------------------------------------
define ('KARTINA_ACCOUNT',  "144");
define ('KARTINA_PASS',     "441");

// ------------------------------------------------------------------
// don't touch anything beyond this line ...
// ------------------------------------------------------------------
define ('KARTINA_HOST',     "http://iptv.kartina.tv");
define ('API_PATH',         "/api/xml/");
define ('LOC_KARTINA_URL',  "http://127.0.0.1/umsp/plugins/kartina");
define ('LOC_KARTINA_UMSP', "umsp://plugins/kartina");
define ('DOTRACE',          "NO");
define ('KARTCHANLIST',     "/tmp/kartchanlist.xml");
define ('KARTFAVLIST',      "/tmp/kartfavorites.xml");
define ('LOC_IMG_PATH',     "/umsp/plugins/kartina/images");
define ('WDTV_CONF_FILE',   "/conf/config");
define ('COOKIE_FILE',      "/tmp/kartCookie.txt");
define ('XML_RENEW_TIME',   300); // 5 min.
define ('DAY_IN_SECONDS',   3600 * 24);
define ('MAX_ARCH_DAYS',    14);

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
   $kartAPI = new kartinaAPI("", "", KARTINA_HOST);
   
   // load cookie ...
   $kartAPI->loadCookie();
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