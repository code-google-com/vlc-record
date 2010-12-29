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
// config array ---->
////////////////////////////////////////////////////////////////////////////////
$info = "<img src='/umsp/plugins/kartina/images/logo1.jpg' style='float: left; margin: 10px;' alt='kartina.tv' title='kartina.tv' width='40' height='50'>\n"
       ."<b>Kartina.tv PlugIn</b> by Jo2003.<br>\n"
       ."Version: " . VERSION_INFO . "<br>\n"
       ."Newest version can always be found <a href='http://www.pristavka.de/index.php/topic,7322.0.html' target='_blank'>here</a>.\n"
       ."You also can access the old configuration script following <a href='/umsp/plugins/kartina/config.php' target='_blank'>this link</a>.";

// some plugin info ...
$wec_options['KARTINA_INFO'] = array(
   'configname'   => 'KARTINA_INFO',
   'configdesc'   => $info,
   'longdesc'     => "Plugin Info",
   'group'        => 'UMSP: Kartina.tv',
   'displaypri'   => -10,
   'type'         => WECT_DESC,
   'page'         => WECP_UMSP,
   'availval'     => array(),
   'availvalname' => array(),
   'defaultval'   => '',
   'currentval'   => '',
   'readhook'     => 'wec_hook_donothing',
   'writehook'    => 'wec_hook_donothing'
);

// account number ...
$wec_options['KARTINA_ACCOUNT'] = array(
   'configname'   => 'KARTINA_ACCOUNT',
   'configdesc'   => "Account number",
   'longdesc'     => "Account number send to you by kartina.tv",
   'group'        => 'UMSP: Kartina.tv',
   'displaypri'   => -9,
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
   'displaypri'   => -8,
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
   'displaypri'   => -7,
   'type'         => WECT_TEXT,
   'page'         => WECP_UMSP,
   'availval'     => array(),
   'availvalname' => array(),
   'defaultval'   => '/tmp/media/usb/USBX/insert/path/here',
   'currentval'   => wec_getConfigValue('KART_REC_FOLDER'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// time zone ...
$wec_options['TIMEZONE'] = array(
   'configname'   => 'TIMEZONE',
   'configdesc'   => "Timezone",
   'longdesc'     => "Choose matching timezone!",
   'group'        => 'UMSP: Kartina.tv',
   'displaypri'   => -6,
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
   'displaypri'   => -5,
   'type'         => WECT_MULTI,
   'page'         => WECP_UMSP,
   'availval'     => wec_kartinatv_getCids(), 
   'availvalname' => wec_kartinatv_getChans(),
   'defaultval'   => array(),
   'currentval'   => wec_kartinatv_getFavs(),
   'readhook'     => 'wec_hook_donothing', // we already read data ...
   'writehook'    => 'wec_kartinatv_write' // favorites at kartina ...
);

////////////////////////////////////////////////////////////////////////////////
// <---- config array
////////////////////////////////////////////////////////////////////////////////

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

/* -----------------------------------------------------------------\
|  Method: wec_kartinatv_getFavs
|  Begin: 28.12.2010 / 13:45
|  Author: Jo2003
|  Description: get an array of used favorites
|
|  Parameters: --
|
|  Returns: array with favorites
\----------------------------------------------------------------- */
function wec_kartinatv_getFavs()
{
   $api = new kartinaAPI();
   $api->loadCookie(); 
   
   // fill in current values ...
   $favsarr = $api->getFavorites();
   $favs    = array();
    
   for ($i = 0; $i < count($favsarr); $i++)
   {
      $favs[] = (string)$favsarr[$i]['cid'];
   }
   
   return $favs;
}

/* -----------------------------------------------------------------\
|  Method: wec_kartinatv_getCids
|  Begin: 28.12.2010 / 13:45
|  Author: Jo2003
|  Description: get an array of channel ids
|
|  Parameters: --
|
|  Returns: array with channel ids
\----------------------------------------------------------------- */
function wec_kartinatv_getCids ()
{
   $api = new kartinaAPI();
   $api->loadCookie();
   
   // fill in availabe values ...
   $chanarr = $api->getChannelList();
   $cids    = array();

   for ($i = 0; $i < count($chanarr); $i++)
   {
      $cids[] = (string)$chanarr[$i]['id'];
   }
   
   return $cids; 
}

/* -----------------------------------------------------------------\
|  Method: wec_kartinatv_getChans
|  Begin: 28.12.2010 / 13:45
|  Author: Jo2003
|  Description: get an array channel names
|
|  Parameters: --
|
|  Returns: array with channel names
\----------------------------------------------------------------- */
function wec_kartinatv_getChans ()
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
