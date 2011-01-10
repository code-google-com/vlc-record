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
require_once(dirname(__FILE__)."/info.php");

////////////////////////////////////////////////////////////////////////////////
// config array ---->
////////////////////////////////////////////////////////////////////////////////
$info = "<img src='".$pluginInfo['thumb']."' style='float: left; margin: 10px;' alt='".$pluginInfo['id']."' title='".$pluginInfo['id']."' width='40' height='50'>\n"
       ."<b>".$pluginInfo['desc']."</b> by ".$pluginInfo['author'].".<br>\n"
       ."Version: ".$pluginInfo['version']." / ".$pluginInfo['date']."<br>\n"
       ."Newest version can always be found <a href='".$pluginInfo['url']."' target='_blank'>here</a>.<br>\n"
       ."You also can access the old configuration script following <a href='/umsp/plugins/kartina/config.php' target='_blank'>this link</a>.";
       
// some plugin info ...
$wec_options['KARTINA_INFO'] = array(
   'configname'   => 'KARTINA_INFO',
   'configdesc'   => $info,
   'longdesc'     => "Plugin Info",
   'group'        => $pluginInfo['name'],
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

// enable disable ...
$wec_options[$pluginInfo['id']] = array(
   'configname'   => $pluginInfo['id'],
   'configdesc'   => 'Enable '.$pluginInfo['name'].' UMSP plugin',
   'longdesc'     => '',
   'group'        => $pluginInfo['name'],
   'type'         => WECT_BOOL,
   'page'         => WECP_UMSP,
   'displaypri'   => -9,
   'availval'     => array('off','on'),
   'availvalname' => array(),
   'defaultval'   => '',
   'currentval'   => wec_getConfigValue($pluginInfo['id']),
   'readhook'     => wec_umspwrap_read,
   'writehook'    => wec_umspwrap_write,
   'backuphook'   => NULL,
   'restorehook'  => NULL
);

// account number ...
$wec_options['KARTINA_ACCOUNT'] = array(
   'configname'   => 'KARTINA_ACCOUNT',
   'configdesc'   => "Account number",
   'longdesc'     => "Account number send to you by kartina.tv",
   'group'        => $pluginInfo['name'],
   'displaypri'   => -8,
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
   'group'        => $pluginInfo['name'],
   'displaypri'   => -7,
   'type'         => WECT_TEXT,
   'page'         => WECP_UMSP,
   'availval'     => array(),
   'availvalname' => array(),
   'defaultval'   => '441',
   'currentval'   => wec_getConfigValue('KARTINA_PASSWD'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// allow erotic channels ...
$wec_options['KARTINA_EROTIC'] = array(
   'configname'   => 'KARTINA_EROTIC',
   'configdesc'   => "Allow Erotic Channels",
   'longdesc'     => "Do you want to enable erotic channels?",
   'group'        => $pluginInfo['name'],
   'displaypri'   => -6,
   'type'         => WECT_BOOL,
   'page'         => WECP_UMSP,
   'availval'     => array('off','on'),
   'availvalname' => array(),
   'defaultval'   => 'off',
   'currentval'   => wec_getConfigValue('KARTINA_EROTIC'),
   'readhook'     => NULL,
   'writehook'    => NULL
);

// record folder ...
$wec_options['KART_REC_FOLDER'] = array(
   'configname'   => 'KART_REC_FOLDER',
   'configdesc'   => "Target Folder",
   'longdesc'     => "Where do you want your records to be stored?",
   'group'        => $pluginInfo['name'],
   'displaypri'   => -5,
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
   'group'        => $pluginInfo['name'],
   'displaypri'   => -4,
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
   'group'        => $pluginInfo['name'],
   'displaypri'   => -3,
   'type'         => WECT_MULTI,
   'page'         => WECP_UMSP,
   'availval'     => wec_kartinatv_getCids(), 
   'availvalname' => wec_kartinatv_getChans(),
   'defaultval'   => wec_kartinatv_getFavs(),
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
