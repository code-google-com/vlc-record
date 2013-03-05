<?php
  // init needed variables (only $_GET supported) ...
  $_filter = isset($_GET['filter']) ? $_GET['filter'] : "native";
  $_sort   = isset($_GET['sort'])   ? $_GET['sort']   : "name";
  $_order  = isset($_GET['order'])  ? $_GET['order']  : "asc";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
     "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>
   <head>
      <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
      <title>File Index</title>
      <style type="text/css">
      <!--
        body {font-family: Verdana, Tahoma, Arial, sans-serif; font-size: 11px; margin: 0px; padding: 0px; text-align: center; color: #3A3A3A; background-color: #F4F4F4}
        table {width:50%;background-color:#333333;color:white;padding:0px;border:2px outset}
        td {text-align:center;vertical-align:middle;border:0px;padding:2px;background-color:white;color:black}
        th {background-color:#333333;color:white;font-weight:bold;}
        th a:link, th a:visited, th a:active { text-decoration: none; color: white }
        th a:hover { text-decoration: underline; color: red }
        a:link, a:visited, a:active { text-decoration: underline; color: #444444}
        a:hover { text-decoration: underline; color: #0482FE }
        pre {color: #444; background-color: #eee; border: 1px dashed gray; padding: 5px; width: 50%; text-align: left; font-family: Consolas, Courier New, monospace;}
      -->
      </style>
   </head>
   <body>
      <div align='center'>
      <h1>Beta Versions for VLC-Record</h1>
      <table class='fileinfo'>
      <tr>
         <th><a href="<?php echo $_SERVER['PHP_SELF']."?filter=".$_filter."&amp;sort=name&amp;order=".(($_order == "asc") ? "desc" : "asc"); ?>">File</a><?php sortMarker("name"); ?></th>
         <th><a href="<?php echo $_SERVER['PHP_SELF']."?filter=".$_filter."&amp;sort=arch&amp;order=".(($_order == "asc") ? "desc" : "asc"); ?>">Architechture</a><?php sortMarker("arch"); ?></th>
         <th><a href="<?php echo $_SERVER['PHP_SELF']."?filter=".$_filter."&amp;sort=size&amp;order=".(($_order == "asc") ? "desc" : "asc"); ?>">Size</a><?php sortMarker("size"); ?></th>
         <th><a href="<?php echo $_SERVER['PHP_SELF']."?filter=".$_filter."&amp;sort=date&amp;order=".(($_order == "asc") ? "desc" : "asc"); ?>">Date</a><?php sortMarker("date"); ?></th>
      </tr>
<?php

  //////////////////////////////////////////////////////////////////////
  //                     function section                             //
  //////////////////////////////////////////////////////////////////////
  
  //--------------------------------------------------------------------
  //! \brief    parse folder for supported file types
  //! \date     2013-03-04
  //! \author   Jo2003
  //! \param    $directory folder where to start
  //! \return   array with file entries 
  //--------------------------------------------------------------------
  function getDirectoryList ($directory)
  {
    $files = array();

    if ($handle = opendir($directory))
    {
      while (false !== ($file = readdir($handle)))
      {
        if ($file != "." && $file != "..")
        {
          if (!is_dir($file))
          {
            $file_parts = pathinfo($file);
            $arch       = "";

            switch($file_parts['extension'])
            {
            case "7z":
            case "exe":
              $arch = "win";
              break;
            case "deb":
              $arch = "nix";
              break;
            case "dmg":
              $arch = "mac";
              break;
            default:
              break;
            }


            if ($arch != "")
            {
                $files[] = array("name" => $file,
                                 "arch" => $arch,
                                 "size" => round(filesize($file) / (1024 * 1024), 2),
                                 "date" => filemtime($file)
                                 );
            }
          }
        }
      }
      closedir($handle);
    }

    return $files;
  }
  
  //--------------------------------------------------------------------
  //! \brief    echo sort marker if needed
  //! \date     2013-03-04
  //! \author   Jo2003
  //! \param    $link sort link name
  //! \return   --
  //--------------------------------------------------------------------
  function sortMarker ($link)
  {
    global $_order, $_sort;
    
    if ($link == $_sort)
    {
      if ($_order == "asc")
      {
        echo "&nbsp;&nbsp;<img src='images/arrow_up.gif' alt='arrow up' title='asc' />";
      }
      else
      {
        echo "&nbsp;&nbsp;<img src='images/arrow_down.gif' alt='arrow down' title='desc' />";
      }
    }
  }
  
  
  //////////////////////////////////////////////////////////////////////
  // sort callback functions used by usort()
  //////////////////////////////////////////////////////////////////////
  function name_cmp($a, $b)
  {
    return strcasecmp($a['name'], $b['name']);
  }
  
  function arch_cmp($a, $b)
  {
    return strcasecmp($a['arch'], $b['arch']);
  }
  
  function size_cmp($a, $b)
  {
    if ($a['size'] == $b['size'])
    {
      return 0;
    }
    return ($a['size'] < $b['size']) ? -1 : 1;
  }
  
  function date_cmp($a, $b)
  {
    if ($a['date'] == $b['date'])
    {
      return 0;
    }
    return ($a['date'] < $b['date']) ? -1 : 1;
  }
  
  
  function name_r_cmp($a, $b)
  {
    return strcasecmp($b['name'], $a['name']);
  }
  
  function arch_r_cmp($a, $b)
  {
    return strcasecmp($b['arch'], $a['arch']);
  }
  
  function size_r_cmp($a, $b)
  {
    if ($a['size'] == $b['size'])
    {
      return 0;
    }
    return ($b['size'] < $a['size']) ? -1 : 1;
  }
  
  function date_r_cmp($a, $b)
  {
    if ($a['date'] == $b['date'])
    {
      return 0;
    }
    return ($b['date'] < $a['date']) ? -1 : 1;
  }
  //////////////////////////////////////////////////////////////////////
  
  //--------------------------------------------------------------------
  //! \brief    should we show this row ?
  //! \date     2013-03-04
  //! \author   Jo2003
  //! \param    $name file name
  //! \param    $filter check the file name for this string
  //! \return   0 --> don't show; 1 --> show
  //--------------------------------------------------------------------
  function showRow($name, $filter)
  {
    $rv     = 0;

    if ($filter == "native")
    {
      $rv = (stripos($name, "moi") === false) ? 1 : 0;
    }
    else if (($filter == "none") || ($filter == "all"))
    {
      $rv = 1;
    }
    else if (stripos($name, $filter) !== false)
    {
      $rv = 1;
    }

    return $rv;
  }
  
  //////////////////////////////////////////////////////////////////////
  //                     end function section                         //
  //////////////////////////////////////////////////////////////////////

  $files = getDirectoryList(".");
  
  // sort array ...
  if ($_order == "asc")
  {
    usort($files, $_sort."_cmp");
  }
  else
  {
    usort($files, $_sort."_r_cmp");
  }

  for ($i = 0; $i < count($files); $i++)
  {
    if (showRow($files[$i]['name'], $_filter))
    {
      echo "      <tr>\n"
          ."         <td><a href='".$files[$i]['name']."' title='click to download'>".$files[$i]['name']."</a></td>\n"
          ."         <td><img src='images/".$files[$i]['arch'].".png' alt='".$files[$i]['arch']."' title='".$files[$i]['arch']."' /></td>\n"
          ."         <td>".$files[$i]['size']."MB</td>\n"
          ."         <td>".date ("d.m.Y H:i:s", $files[$i]['date'])."</td>\n"
          ."      </tr>\n";
    }
  }

?>
      </table>
      Use it at your own risk! As stated above - these are beta versions. Please inform me when you find a bug: olenka.joerg(at)gmail.com!
      <br /> <br />
<?php

  if (file_exists('./changelog.log'))
  {
  $content = file('./changelog.log');
    echo "      <h2>Changelog:</h2>\n"
        ."      <pre>\n"
        .htmlspecialchars(file_get_contents('./changelog.log'))
        ."      </pre>\n";
  }

?>
      <p>
         <a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a>
         <a href="http://jigsaw.w3.org/css-validator/check/referer"><img src="http://jigsaw.w3.org/css-validator/images/vcss" alt="CSS ist valide!" height="31" width="88" /></a>
      </p>
   </div>
</body>
</html>
