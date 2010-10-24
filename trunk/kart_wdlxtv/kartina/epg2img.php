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

$query_array = array();
parse_str($_SERVER['QUERY_STRING'], $query_array);

$start = $query_array['ut_start'];
$end   = $query_array['ut_end'];
$title = $query_array['title'];
$descr = $query_array['descr'];
$chan  = $query_array['channel'];

// 1280x800
$im = imagecreatefromjpeg("/osd/image/villa_bg.jpg");

header('Content-type: image/jpeg');
imagejpeg($im, NULL, 80);
imagedestroy($im);

?>