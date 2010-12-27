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

require_once (dirname(__FILE__)."/_kartina_auth.php.inc");

$vod_tid = isset($_GET['vod_tid']) ? $_GET['vod_tid'] :  0;

// request show info XML ...
$vodInfo = $kartAPI->getVideoDetailXml ($vod_tid);
$dom     = new DomDocument();
$dom->loadXML($vodInfo);
$xp      = new DOMXpath($dom);

// get whole film info ...
$name     = $xp->query("/response/film/name")->item(0)->nodeValue;
$descr    = $xp->query("/response/film/description")->item(0)->nodeValue;
$poster   = $xp->query("/response/film/poster")->item(0)->nodeValue;
$lenght   = $xp->query("/response/film/lenght")->item(0)->nodeValue;
$year     = $xp->query("/response/film/year")->item(0)->nodeValue;
$director = $xp->query("/response/film/director")->item(0)->nodeValue;
$actors   = $xp->query("/response/film/actors")->item(0)->nodeValue;
$country  = $xp->query("/response/film/country")->item(0)->nodeValue;

// load background image ...
$im = imagecreatefromjpeg(EPGIMG);

if ($im)
{
   // avoid timeouts ...
   set_time_limit(0);
   
   header('Content-Type: image/jpeg');
   
   // add logo if path was given ...
   if ($poster != "")
   {
      addLogo($im, KARTINA_HOST.$poster);
   }
   
   // colors ...
   $white  = imagecolorallocate ($im, 255, 255, 255);
   $cyan   = imagecolorallocate ($im, 50, 198, 215);
   $yellow = imagecolorallocate ($im, 243, 246, 14);
   
   // top and left margin: 150px
   $x = 150;
   $y = 150;
   
   // the f...ing bounding box ... !
   // The height of the text we want to write 
   // isn't that easy to count ...
   // font height (40) + place for any accent above (font height / 2)
   $y += 60; 
   
   // add channel ...
   imagettftext($im, 40, 0, $x, $y, $cyan, EPGFONTBD, $name);
   
   // get next y position ...
   // font height(24) + line spacing(20) 
   // + space for "down under chars" (above font height / 2)
   // + space for accent (font height / 2)
   $y += 76;
   
   // country, year and length ...
   $info = $country." ".$year.", ".$lenght." мин.";
   imagettftext($im, 24, 0, $x, $y, $white, EPGFONT, $info);
   
   // get next y position ...
   // font height(36) + line spacing(54) 
   // + space for "down under chars" (above font height / 2)
   // + space for accent (font height / 2)
   $y += 60;
   
   // director ...
   $director = "Режиссер: ".$director;
   imagettftext($im, 24, 0, $x, $y, $white, EPGFONT, $director);
   
   // get next y position ...
   // font height(26) + line spacing(5) 
   // + space for "down under chars" (above font height / 2)
   // + space for accent (font height / 2)
   $y += 160;
   
   $dodesc = 1;
   $offset = 0;
   $actors = "В ролях: ".htmlspecialchars_decode($actors);
   
   while ($dodesc && ($y <= imagesy($im)))
   {
      $line = "";
      
      if ((mb_strlen($actors, "UTF-8") - $offset) > DESCR_LINE_LEN)
      {
         $token   = mb_substr($actors, $offset, DESCR_LINE_LEN, "UTF-8");
         $cutpos  = mb_strrpos($token, " ", 0, "UTF-8");
         $line    = mb_substr($token, 0, $cutpos, "UTF-8");
         $offset += $cutpos + 1; 
      }
      else
      {
         $dodesc  = 0;
         $line    = mb_substr($actors, $offset, mb_strlen($actors, "UTF-8") - $offset, "UTF-8"); 
      }
      
      // paint description line ...
      imagettftext($im, 26, 0, $x, $y, $yellow, EPGFONT, $line);
      
      // get next y position ...
      // font height(26) + line spacing(5) 
      // + space for "down under chars" (above font height / 2)
      // + space for accent (font height / 2)
      $y += 57;
   }
   
   $y     += 20;
   $dodesc = 1;
   $offset = 0;
   $descr  = str_replace("  ", " ", htmlspecialchars_decode($descr));
   
   while ($dodesc && ($y <= imagesy($im)))
   {
      $line = "";
      
      if ((mb_strlen($descr, "UTF-8") - $offset) > DESCR_LINE_LEN)
      {
         $token   = mb_substr($descr, $offset, DESCR_LINE_LEN, "UTF-8");
         $cutpos  = mb_strrpos($token, " ", 0, "UTF-8");
         $line    = mb_substr($token, 0, $cutpos, "UTF-8");
         $offset += $cutpos + 1; 
      }
      else
      {
         $dodesc  = 0;
         $line    = mb_substr($descr, $offset, mb_strlen($descr, "UTF-8") - $offset, "UTF-8"); 
      }
      
      // paint description line ...
      imagettftext($im, 26, 0, $x, $y, $white, EPGFONT, $line);
      
      // get next y position ...
      // font height(26) + line spacing(5) 
      // + space for "down under chars" (above font height / 2)
      // + space for accent (font height / 2)
      $y += 57;
   }
   
   
   
   imagejpeg($im, NULL, 80);
}

if ($im)
{
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
   
   if (($logo_img = imagecreatefromjpeg($logo)) !== false)
   {
      // 1. We add the logo in the upper right corner of the image.
      // 2. Top- and right margin will be set to 150 px.
      // 3. We will resize the logo to 200x320.
      $rv = imagecopyresampled($img, $logo_img, imagesx($img) - 350, 
                               150, 0, 0, 200, 320, imagesx($logo_img), 
                               imagesy($logo_img));
   
      imagedestroy($logo_img);
   }
   
   return $rv;
}

?>
