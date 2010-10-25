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

define ('EPGFONT',        dirname(__FILE__).'/fonts/arial.ttf');
define ('EPGFONTBD',      dirname(__FILE__).'/fonts/arialbd.ttf');
define ('EPGIMG',         dirname(__FILE__).'/images/epg_back.jpg');
define ('DESCR_LINE_LEN', 90);

require_once (dirname(__FILE__)."/_kartina_auth.php.inc");

$cid = isset($_GET['cid']) ? $_GET['cid'] :  7;
$gmt = isset($_GET['gmt']) ? $_GET['gmt'] : -1; 

// request show info array ...
$showinfo = $kartAPI->getShowInfo($cid, $gmt);

/*
$showinfo = array(
   'channel' => "2x2",
   'icon'    => "http://172.25.20.10/~joergn/pups/images/4.gif",
   'name'    => "М/с &quot;Симпсоны&quot;, 19 сезон, 19-20 эп.",
   'descr'   => "США, 1989, комедия. В ролях: Дэн Кастелланета, Джули Кэвнер, Нэнси Картрайт, Йердли Смит. Семейство Симпсонов живет в маленьком американском городке Спрингфилд. Глава семейства Гомер работает на атомной электростанции, принося предприятия больше убытков, чем пользы. Он любит пить пиво с...",
   'start'   => 1287508860,
   'end'     => 1287512400
);
*/

// load background image ...
$im = imagecreatefromjpeg(EPGIMG);

if ($im && is_array($showinfo))
{
   // avoid timeouts ...
   set_time_limit(0);
   
   $days = array(1 => "Понедельник",
                 2 => "Вторник",
                 3 => "Среда",
                 4 => "Четверг",
                 5 => "Пятница",
                 6 => "Суббота",
                 7 => "Воскресенье");
   
   header('Content-Type: image/jpeg');
   
   // add logo if path was given ...
   if ($showinfo['icon'] != "")
   {
      addLogo($im, $showinfo['icon']);
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
   imagettftext($im, 40, 0, $x, $y, $cyan, EPGFONTBD, $showinfo['channel']);
   
   // get next y position ...
   // font height(24) + line spacing(20) 
   // + space for "down under chars" (above font height / 2)
   // + space for accent (font height / 2)
   $y += 76;
   
   // create date string ...
   $date  = $days[date("N", $showinfo['start'])].", "
           .date("d.m.Y", $showinfo['start']).", "
           .date("H:i", $showinfo['start'])."ч. - "
           .date("H:i", $showinfo['end'])."ч.";
   imagettftext($im, 24, 0, $x, $y, $white, EPGFONTBD, $date);
   
   // get next y position ...
   // font height(36) + line spacing(54) 
   // + space for "down under chars" (above font height / 2)
   // + space for accent (font height / 2)
   $y += 120;
   
   // show name ...
   $showinfo['name'] = htmlspecialchars_decode($showinfo['name']);
   imagettftext($im, 36, 0, $x, $y, $yellow, EPGFONTBD, $showinfo['name']);
   
   // get next y position ...
   // font height(26) + line spacing(5) 
   // + space for "down under chars" (above font height / 2)
   // + space for accent (font height / 2)
   $y += 62;
   
   $dodesc            = 1;
   $offset            = 0;
   $showinfo['descr'] = htmlspecialchars_decode($showinfo['descr']);
   
   while ($dodesc && ($y <= imagesy($im)))
   {
      $line = "";
      
      if ((mb_strlen($showinfo['descr'], "UTF-8") - $offset) > DESCR_LINE_LEN)
      {
         $token   = mb_substr($showinfo['descr'], $offset, DESCR_LINE_LEN, "UTF-8");
         $cutpos  = mb_strrpos($token, " ", 0, "UTF-8");
         $line    = mb_substr($token, 0, $cutpos, "UTF-8");
         $offset += $cutpos + 1; 
      }
      else
      {
         $dodesc  = 0;
         $line    = mb_substr($showinfo['descr'], $offset, mb_strlen($showinfo['descr'], "UTF-8") - $offset, "UTF-8"); 
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