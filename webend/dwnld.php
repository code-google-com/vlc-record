<?php
// --------------------------------------------------------------------
// A simple download script to replace the download stuff
// in google code.
// (c)2013 by Jo2003
// This script is free software! Nevertheless I don't think it's
// worth to use it outsite from my site ...
// --------------------------------------------------------------------
include ("db_conf.php");

//----------------------------------------------------------------------
//! \brief    get value from $_POST and $_GET variables
//
//! \Author   Jo2003
//! \Date     11.09.2013
//
//! \param    name (string) variable name
//
//! \return   string with value
//----------------------------------------------------------------------
function grabVal($name)
{
    $var = isset($_POST[$name]) ? $_POST[$name] : (isset($_GET[$name]) ? $_GET[$name] : "");
    return $var;
}

//----------------------------------------------------------------------
//! \brief    build a simple hash code to block link stealing
//!           hash will be good for the day it was created
//
//! \Author   Jo2003
//! \Date     11.09.2013
//
//! \param    action (string) action
//! \param    id (int) file id
//
//! \return   hash code
//----------------------------------------------------------------------
function buildHash($action, $id)
{
    return md5($action . $id . $id . ($id * $id) . "08/15" . date("Ydm"));
}

//----------------------------------------------------------------------
//! \brief    get image html code for architecture
//
//! \Author   Jo2003
//! \Date     11.09.2013
//
//! \param    arch (string) architecture
//
//! \return   html code
//----------------------------------------------------------------------
function archImg($arch)
{
    $rv = "";
    
    if (($arch === "lin32") || ($arch === "lin64"))
    {
        $rv = "<img src='images/nix.png' alt='".$arch."' title='".$arch."' />";
    }
    else if ($arch === "win")
    {
        $rv = "<img src='images/win.png' alt='".$arch."' title='".$arch."' />";
    }
    else if ($arch === "mac")
    {
        $rv = "<img src='images/mac.png' alt='".$arch."' title='".$arch."' />";
    }
    else
    {
        $rv = $arch;
    }
    
    return $rv;
}

/// global variables ...
$action  = grabVal("action");
$sort    = grabval("sort");
$id      = grabVal("id");
$filter  = grabVal("filter");
$hash    = grabVal("hash");
$arch    = grabVal("arch");
$count   = grabVal("count");
$ver     = grabVal("ver");
$link    = $_SERVER['PHP_SELF'] . "?action=";

$content = "";

// download file ...
if ($action === "dwnld")
{
    if (is_numeric($id) && ($hash === buildHash($action, $id)))
    {
        $res = $db_access->query_first("SELECT link, downloads from " . DEF_DWN_TAB . " WHERE id=".$id);
        if (is_array($res))
        {
            $db_access->query("UPDATE ". DEF_DWN_TAB . " SET downloads=".($res['downloads'] + 1)." WHERE id=".$id);
            header("Location: ". $res['link']);
        }
        else
        {
            $content = "<h3>Error!</h3><br /> <br /><span class='error'>Can't find file to download!</span>";
        }
    }
    else
    {
        $content = "<h3>Error!</h3><br /> <br /><span class='error'>Either download id not set or hash incorrect!</span>";
    }
}
else if(($action == "list") || ($action == "")) // list files ...
{
    $sql   = "SELECT * FROM " . DEF_DWN_TAB;
    $where = "";
    $limit = "";
    $sort  = ($sort != "") ? " ".$sort : " desc";
    
    if ($filter != "")
    {
        $where = "descr like '%" . $filter ."%'";
    }
    
    if ($arch != "")
    {
        $where .= ($where != "") ? " AND " : "";
        $where .= "arch like '%" . $arch . "%'";
    }
    
    if ($ver != "")
    {
        $where .= ($where != "") ? " AND " : "";
        $where .= "ver='" . $ver . "'";
    }
    
    if ($count != "")
    {
        $limit = " LIMIT 0,".$count;
    }
    
    if ($where != "")
    {
        $sql .= " WHERE " .$where;
    }
    
    $sql .= " ORDER BY ver".$sort.$limit;
    
    $res = $db_access->query($sql);
    
    $content = "<h1>Downloads</h1>\n<table>\n<tr><th>File</th><th>Arch</th><th>Version</th><th>Date</th><th>md5</th><th>Downloads</th></tr>\n";
    
    while (($entry = $db_access->fetch_array($res)) !== false)
    {
        $content .= "<tr>"
                   ."<td><a href='".$link."dwnld&amp;id=".$entry['id']."&amp;hash=".buildHash("dwnld", $entry['id'])."' title='".$entry['name']."'>".$entry['descr']."</a></td>"
                   ."<td>".archImg($entry['arch'])."</td>"
                   ."<td>".$entry['ver']."</td>"
                   ."<td>".$entry['date']."</td>"
                   ."<td>".$entry['checksum']."</td>"
                   ."<td>".$entry['downloads']."</td>"
                   ."</tr>\n";
    }
    
    $content .= "</table>";
}


if ($content == "")
{
    // nothing to tell ...
    die;
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
     "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>
   <head>
      <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
      <title>Downloads</title>
      <style type="text/css">
      <!--
        body {font-family: Verdana, Tahoma, Arial, sans-serif; font-size: 11px; margin: 0px; padding: 0px; text-align: center; color: #3A3A3A; background-color: #F4F4F4}
        table {width:50%;background-color:#333333;color:white;padding:0px;border:2px outset}
        td {text-align:center;vertical-align:middle;border:0px;padding:2px;background-color:white;color:black;white-space: nowrap;}
        th {background-color:#333333;color:white;font-weight:bold; font-size: 14px;white-space: nowrap;}
        th a:link, th a:visited, th a:active { text-decoration: none; color: white }
        th a:hover { text-decoration: underline; color: red }
        a:link, a:visited, a:active { text-decoration: underline; color: #444444}
        a:hover { text-decoration: underline; color: #0482FE }
        pre {color: #444; background-color: #eee; border: 1px dashed gray; padding: 5px; width: 50%; text-align: left; font-family: Consolas, Courier New, monospace;}
        .error {color: red; font-weight: bold;}
        .note {color: #800; background-color: #fc0; border: 1px dashed red; padding: 15px; width: 50%; text-align: left; font-family: Consolas, Courier New, monospace; font-size: 14px;}
        .links {color: #444; background-color: white; border: 1px solid green; padding: 15px; width: 50%; text-align: left; font-family: Consolas, Courier New, monospace; font-size: 14px;}
      -->
      </style>
   </head>
   <body>
      <!--
      Following variables are allowed for sorting and filtering:
      - sort: "asc" or "desc"
      - filter: search in description for given text
      - arch: search in architecture for given text; architectures are: "win", "mac", "lin32", "lin64"
      - count: show not more than x entries
      - ver: show entries with excactly this version
      Example: "<?php echo $link; ?>list&filter=polsky&arch=lin&ver=2.64" will list all entries of Polsky.TV in version 2.64 for Linux (x86 and amd64).
      -->
      <div align="center">
         <?php echo $content ."\n"; ?>
         <br /> <br />
         <div class='links'>
            <b>[</b> <a href='http://rt.coujo.de'>Here</a> <b>]</b> is an alternate download location. Please use only if your download above doesn't work.<br />
            <b>[</b> <a href='https://code.google.com/p/vlc-record/downloads/list?can=4'>Here</a> <b>]</b> you'll find <b>former (deprecated) program versions</b> (up to 2.64).
         </div>
         <br /> <br />
         <div class='note'>
            <h3>Please note:</h3>
            Since Google Code deprecated downloads all files are now located at my Google-Drive.
            When clicking a file to download you might get a warning that the file is to large
            so that it couldn't be tested for viruses. <b>This doesn't mean that there is a virus
            in that file!</b> This only means the file isn't checked by Google.
            <b>All files are checked and free from viruses, ad- and spy-ware!</b>
         </div>
         <p>
            <a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a>
            <a href="http://jigsaw.w3.org/css-validator/check/referer"><img src="http://jigsaw.w3.org/css-validator/images/vcss" alt="CSS ist valide!" height="31" width="88" /></a>
         </p>
      </div>
   </body>
</html>
