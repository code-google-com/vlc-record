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

$gmt   = $query_array['gmt'];
$title = $query_array['title'];
$descr = $query_array['descr'];
$chan  = $query_array['channel'];

$im = imagecreatefrompng("/path/to/img");

header('Content-type: image/jpeg');
imagejpeg($im, NULL, 80);
imagedestroy($im);

?>