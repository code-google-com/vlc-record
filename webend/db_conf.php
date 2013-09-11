<?php
include ("db_con.inc.php");

// connect to database ...
$db_access = new db_zugriff;
$db_access->appname    = "Download Help Script";
$db_access->database   = $mysqldb;
$db_access->server     = $mysqlhost;
$db_access->user       = $mysqluser;
$db_access->password   = $mysqlpassword;
$db_access->connect();

define ("DEF_DWN_TAB", "vlcr_downloads");

?>