<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 24.10.2010 / 11:00
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/

define ('EPGFONT',   dirname(__FILE__).'/fonts/arial.ttf');
define ('EPGFONTBD', dirname(__FILE__).'/fonts/arialbd.ttf');
define ('EPGIMG',    dirname(__FILE__).'/images/villa_bg.png');

$query_array = array();
parse_str($_SERVER['QUERY_STRING'], $query_array);

$start = isset($query_array['ut_start']) ? $query_array['ut_start'] : -1;
$end   = isset($query_array['ut_end'])   ? $query_array['ut_end']   : $start + 3600;
$title = isset($query_array['title'])    ? $query_array['title']    : "";
$descr = isset($query_array['descr'])    ? $query_array['descr']    : "";
$chan  = isset($query_array['channel'])  ? $query_array['channel']  : "";
$icon  = isset($query_array['icon'])     ? $query_array['icon']     : "";

// load background image ...
$im = imagecreatefrompng(EPGIMG);

if ($im)
{
   // avoid timeouts ...
   set_time_limit(0);
   
   header('Content-Type: image/jpeg');
   
   // add logo if path was given ...
   if ($icon !== "")
   {
      addLogo($im, $icon);
   }
   
   $color = imagecolorallocate ($im, 255, 255, 255); // white ...
   
   // top and left margin: 50px
   $x = 150;
   $y = 150;
   
   // get text size ...
   $coords = imagettfbbox(40, 0, EPGFONTBD, $chan);
   
   // add channel ...
   $y += $coords[1];
   imagettftext($im, 40, 0, $x, $y, $color, EPGFONTBD, $chan);
   
   // get next y position ...
   $y += 15; // add place between lines ...
   
   // create time string ...
   $times  = date("H:i", $start)." - ".date("H:i", $end);
   
   // get text size ...
   $coords = imagettfbbox(40, 0, EPGFONTBD, $times);
   
   // add times ...
   $y += $coords[1];
   imagettftext($im, 40, 0, $x, $y, $color, EPGFONTBD, $times);
   
   // get next y position ...
   $y += 15; // add place between lines ...
   
   // show name ...
   $title = str_replace("&quot;", "\"", $title);
   
   // get text size ...
   $coords = imagettfbbox(40, 0, EPGFONTBD, $title);
   
   // add title ...
   $y += $coords[1];
   imagettftext($im, 40, 0, $x, $y, $color, EPGFONTBD, $title);
   
   // get next y position ...
   $y += 5; // add place between lines ...
   
   imagejpeg($im, NULL, 85);
   imagedestroy($im);
}

/* -----------------------------------------------------------------\
|  Method: imageResize
|  Begin: 24.10.2010 / 15:30
|  Author: Jo2003
|  Description: resize image 
|
|  Parameters: ref. to img resource, new width, new height
|
|  Returns: new image resource
\----------------------------------------------------------------- */
function imageResize(&$img, $width, $height)
{
   // create new image ...
   $new_image  = imagecreatetruecolor($width, $height);

   // resize background to match Full HD resolution ...
   if (imagecopyresampled($new_image, $img, 0, 0, 0, 0, $width, $height, imagesx($img), imagesy($img)))
   {
      return $new_image;
   }
   else
   {
      return false;
   }
}

/* -----------------------------------------------------------------\
|  Method: addLogo
|  Begin: 24.10.2010 / 15:30
|  Author: Jo2003
|  Description: add a gif logo into upper right corner of image  
|
|  Parameters: ref. to img resource, path togif image
|
|  Returns: true --> ok
|          false --> any error
\----------------------------------------------------------------- */
function addLogo (&$img, $logo)
{
   $rv = false;
   
   if (($logo_img = imagecreatefromgif($logo)) !== false)
   {
      // 1. We add the logo in the upper right corner of the image.
      // 2. Top- and right margin will be set to 150 px.
      // 3. We will resize the logo to 200x200.
      $rv = imagecopyresampled($img, $logo_img, imagesx($img) - 350, 
                               150, 0, 0, 200, 200, imagesx($logo_img), 
                               imagesy($logo_img));
   
      imagedestroy($logo_img);
   }
   
   return $rv;
}

?>