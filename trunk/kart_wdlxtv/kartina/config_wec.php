<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 28.12.2010 / 14:00
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/

require_once(dirname(__FILE__)."/_timezones.php.inc");
require_once(dirname(__FILE__)."/_kartina_auth.php.inc");

////////////////////////////////////////////////////////////////////////////////
// config array ...
////////////////////////////////////////////////////////////////////////////////

// account number ...
$wec_options['KARTINA_ACCOUNT'] = array(
   'configname'   => 'KARTINA_ACCOUNT',
   'configdesc'   => "Account number",
   'longdesc'     => "Account number send to you by kartina.tv",
   'group'        => 'UMSP: Kartina.tv',
   'type'         => WECT_TEXT,
   'page'         => WECP_UMSP,
   'availval'     => array(),
   'availvalname' => array(),
   'defaultval'   => '144',
   'currentval'   => wec_getConfigValue('KARTINA_ACCOUNT'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// password ...
$wec_options['KARTINA_PASSWD'] = array(
   'configname'   => 'KARTINA_PASSWD',
   'configdesc'   => "Password",
   'longdesc'     => "Password send to you by kartina.tv",
   'group'        => 'UMSP: Kartina.tv',
   'type'         => WECT_TEXT,
   'page'         => WECP_UMSP,
   'availval'     => array(),
   'availvalname' => array(),
   'defaultval'   => '441',
   'currentval'   => wec_getConfigValue('KARTINA_PASSWD'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// record folder ...
$wec_options['KART_REC_FOLDER'] = array(
   'configname'   => 'KART_REC_FOLDER',
   'configdesc'   => "Target Folder",
   'longdesc'     => "Where do you want your records to be stored?",
   'group'        => 'UMSP: Kartina.tv',
   'type'         => WECT_TEXT,
   'page'         => WECP_UMSP,
   'availval'     => array(),
   'availvalname' => array(),
   'defaultval'   => '/tmp/media/usb/USBX/insert/path/here',
   'currentval'   => wec_getConfigValue('KART_REC_FOLDER'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// time zone (may not work because used in other config areas)...
$wec_options['TIMEZONE'] = array(
   'configname'   => 'TIMEZONE',
   'configdesc'   => "Timezone",
   'longdesc'     => "Choose matching timezone!",
   'group'        => 'UMSP: Kartina.tv',
   'type'         => WECT_SELECT,
   'page'         => WECP_UMSP,
   'availval'     => createZoneinfoArray(),
   'availvalname' => createZoneinfoArray(),
   'defaultval'   => 'Europe/Berlin',
   'currentval'   => wec_getConfigValue('TIMEZONE'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// favorites ...
$wec_options['KARTINA_FAVORITES'] = array(
   'configname'   => 'KARTINA_FAVORITES',
   'configdesc'   => "Favorites",
   'longdesc'     => "Choose favorites you like to have in favorites folder!",
   'group'        => 'UMSP: Kartina.tv',
   'type'         => WECT_MULTI,
   'page'         => WECP_UMSP,
   'availval'     => wec_getCids(), 
   'availvalname' => wec_getChans(),
   'defaultval'   => array(),
   'currentval'   => wec_getFavs(),
   'readhook'     => NULL,
   'writehook'    => 'wec_kartinatv_write'
);

/* -----------------------------------------------------------------\
|  Method: wec_kartinatv_read
|  Begin: 28.12.2010 / 13:45
|  Author: Jo2003
|  Description: hook read function, at this time only used for 
|               favorites stuff
|
|  Parameters: option array
|
|  Returns: --
\----------------------------------------------------------------- */
function wec_getFavs()
{
   $api = new kartinaAPI();
   $api->loadCookie(); 
   
   // fill in current values ...
   $favsarr = $api->getFavorites();
   $favs    = array();
    
   for ($i = 0; $i < count($favsarr); $i++)
   {
      $favs[] = $favsarr[$i]['cid'];
   }
   
   return $favs;
}

/* -----------------------------------------------------------------\
|  Method: wec_kartinatv_write
|  Begin: 28.12.2010 / 13:45
|  Author: Jo2003
|  Description: hook write function, at this time only used for 
|               favorites stuff
|
|  Parameters: option array, new value
|
|  Returns: --
\----------------------------------------------------------------- */
function wec_kartinatv_write($wec_option_array, $value)
{
   $api = new kartinaAPI();
   $api->loadCookie();

   if ($wec_option_array['configname'] === "KARTINA_FAVORITES")
   {
      for ($i = 1; $i <= 12; $i++)
      {
         $cid = isset($value[$i - 1]) ? $value[$i - 1] : 0;
         $api->setFavorite($i, $cid);
      }
   }
}


function wec_getCids ()
{
   $api = new kartinaAPI();
   $api->loadCookie();
   
   // fill in availabe values ...
   $chanarr = $api->getChannelList();
   $cids    = array();

   for ($i = 0; $i < count($chanarr); $i++)
   {
      $cids[] = $chanarr[$i]['id'];
   }
   
   return $cids; 
}

function wec_getChans ()
{
   $api = new kartinaAPI();
   $api->loadCookie();
   
   // fill in availabe values ...
   $chanarr = $api->getChannelList();
   $chans   = array();

   for ($i = 0; $i < count($chanarr); $i++)
   {
      $chans[] = $chanarr[$i]['name'];
   }

   return $chans; 
}

?>