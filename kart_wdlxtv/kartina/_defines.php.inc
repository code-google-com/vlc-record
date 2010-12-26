<?php

// this file holds all the defines used in this umsp plugin ...

// ------------------------------------------------------------------
// to enable tracing, set define to 'YES' ...
// ------------------------------------------------------------------
define ('DOTRACE',          "NO");

// ------------------------------------------------------------------
// default (demo) connection data ...
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
define ('KARTCHANLIST',     "/tmp/kartchanlist.xml");
define ('KARTFAVLIST',      "/tmp/kartfavorites.xml");
define ('LOC_IMG_PATH',     "/umsp/plugins/kartina/images");
define ('WDTV_CONF_FILE',   "/conf/config");
define ('COOKIE_FILE',      "/tmp/kartCookie.txt");
define ('DAY_IN_SECONDS',   3600 * 24);
define ('MAX_ARCH_DAYS',    14);
define ('MAX_EPG_DAYS',     7);
define ('ZONEDIR',          "/usr/share/zoneinfo");
define ('EPGFONT',          "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf");
define ('EPGFONTBD',        "/usr/share/fonts/truetype/msttcorefonts/Arial_Black.ttf");
define ('EPGIMG',           dirname(__FILE__).'/images/epg_back.jpg');
define ('DESCR_LINE_LEN',   90);
define ('TIMESTAMPFILE',    "/tmp/.kart_tstamp");
define ('VODPROTINFO',      "http-get:*:video/mp4:*");
?>