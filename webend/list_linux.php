<?php
  $_filter   = isset($_GET['filter']) ? $_GET['filter'] : "vlc";
  
  // build projects array
  $_projects = array("VLC-Record", "Kartina.TV", "Polsky.TV", "Moi-Dom.TV");
  
  // prepare link ...
  $_link32   = "http://code.google.com/p/vlc-record/downloads/list?can=3&amp;q=filename%3A".$_filter."+label%3AUbuntu-x86";
  $_link64   = "http://code.google.com/p/vlc-record/downloads/list?can=3&amp;q=filename%3A".$_filter."+label%3AUbuntu-amd64";
  
  //--------------------------------------------------------------------
  //! \brief    get project name depending on $_filter
  //! \date     2013-03-08
  //! \author   Jo2003
  //! \param    --
  //! \return   project name
  //--------------------------------------------------------------------
  function getProjectName ()
  {
    global $_projects, $_filter;
    
    $match    = 0;
    $i        = 0;
    $project  = "";
  
    for ($i = 0; $i < count($_projects); $i ++)
    {
      if (stripos ($_projects[$i], $_filter) !== false)
      {
        $project = $_projects[$i];
        $match ++;      
      }
    }
    
    if (($match == 0) || ($match > 1))
    {
      $project = "VLC-Record";
    }
    return $project;
  }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
     "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>
   <head>
      <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
      <title><?php echo getProjectName(); ?> Linux-Downloads</title>
      <style type="text/css">
      <!--
        body {font-family: Verdana, Tahoma, Arial, sans-serif; font-size: 11px; margin: 0px; padding: 0px; text-align: center; color: #3A3A3A; background-color: #F4F4F4}
        h1 {color: #036;}
        li {margin: 10px;}
        ul {border: 1px dotted #040404; width: 600px; }
        .red {color: red;}
        a:link, a:visited, a:active { text-decoration: underline; color: #444444}
        a:hover { text-decoration: underline; color: #0482FE }
        .mainframe {text-align: left; margin: 50px; padding: 50px; width: 80%; }
      -->
      </style>
   </head>
   <body>
      <div class='mainframe'>
      <h1><?php echo getProjectName(); ?> Linux-Downloads</h1>
      Please choose the download matching your architecture!
      <ul>
         <li>Ubuntu <b class='red'>32 bit</b> (i386): <a href="<?php echo $_link32; ?>" title="32bit">http://code.google.com/p/vlc-record/down ... i386.deb</a></li>
         <li>Ubuntu <b class='red'>64 bit</b> (amd64): <a href="<?php echo $_link64; ?>" title="64bit">http://code.google.com/p/vlc-record/down ... amd64.deb</a></li>
      </ul>
      </div>
   </body>
</html>
