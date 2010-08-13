<?php

require_once(dirname(__FILE__) . "/kartinaAPI.php.inc");

// ------------------------------------------------------------------
// insert connectioin data here ...
// ------------------------------------------------------------------
define (KARTINA_ACCOUNT,  "144");
define (KARTINA_PASS,     "441");

// ------------------------------------------------------------------
// don't touch anything beyond this line ...
// ------------------------------------------------------------------
define (KARTINA_HOST,     "http://iptv.kartina.tv");
define (API_PATH,         "/api/xml/");
define (LOC_KARTINA_URL,  "http://127.0.0.1/umsp/plugins/kartina");
define (LOC_KARTINA_UMSP, "umsp://plugins/kartina");
define (DOTRACE,          "NO");
define (KARTCHANLIST,     "/tmp/kartchanlist.xml");
define (KARTFAVLIST,      "/tmp/kartfavorites.xml");
define (LOC_IMG_PATH,     "/umsp/plugins/kartina/images");

// ------------------------------------------------------------------
// create kartinaAPI instance ...
// ------------------------------------------------------------------
$kartAPI = new kartinaAPI(KARTINA_ACCOUNT, KARTINA_PASS, KARTINA_HOST);

// authenticate ...
$kartAPI->authenticate();

// get / save channel list if not already there ...
if (!file_exists(KARTCHANLIST))
{
   $kartAPI->saveChannelList();
}

// get / save favorites list if not already there ...
if (!file_exists(KARTFAVLIST))
{
   $kartAPI->saveFavorites();
}

?>