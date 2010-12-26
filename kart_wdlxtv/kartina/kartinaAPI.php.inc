<?php
/*********************** Information *************************\
| $HeadURL$
| 
| Author: Jo2003
|
| Begin: 8/10/2010 / 4:48p
| 
| Last edited by: $Author$
| 
| $Id$
\*************************************************************/


/********************************************************************\
|  Class: kartinaAPI
|  Date:  8/10/2010 / 4:48p
|  Author: Jo2003
|  Description: implementation of kartinas Rest API to use in PHP
|
\********************************************************************/
class kartinaAPI 
{
   private $_cookie;
   private $_host;
   private $_username;
   private $_passwd;
   private $_reader;
   private $_logfile;
   private $_lastServerTime;

   /* -----------------------------------------------------------------\
   |  Method: __construct
   |  Begin: 8/10/2010 / 4:48p
   |  Author: Jo2003
   |  Description: constructs a class instance
   |
   |  Parameters: optional user, pass amd host
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __construct($username = "", $passwd = "", $host = "")
   {
      $this->_host           = $host;
      $this->_username       = $username;
      $this->_passwd         = $passwd;
      $this->_cookie         = "";
      $this->_reader         = new XMLReader();
      $this->_lastServerTime = -1;
      if (DOTRACE == "YES")
      {
        $this->_logfile = fopen("/tmp/kartina_api.log", "a+");
      }
      
      $this->log("construct ...\n");
   }

   /* -----------------------------------------------------------------\
   |  Method: __destruct
   |  Begin: 8/10/2010 / 4:50p
   |  Author: Jo2003
   |  Description: clean on destruction
   |
   |  Parameters: --
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function __destruct()
   {
      // please note: since we work with stored cookies
      // now, it wouldn't be good to logout at destruction!!!
      $this->_reader = NULL;

      if (DOTRACE == "YES")
      {
         fclose ($this->_logfile);
      }
   }

   /* -----------------------------------------------------------------\
   |  Method: setConnectionData
   |  Begin: 8/10/2010 / 4:58p
   |  Author: Jo2003
   |  Description: set connection data 
   |
   |  Parameters: optional username, password and host
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function setConnectionData ($username = "", $passwd = "", $host = "")
   {
      $this->_host     = $host;
      $this->_username = $username;
      $this->_passwd   = $passwd;
   }

   /* -----------------------------------------------------------------\
   |  Method: postReq
   |  Begin: 8/10/2010 / 5:00p
   |  Author: Jo2003
   |  Description: send a post request
   |
   |  Parameters: complete url, request string, optional headers
   |
   |  Returns: response string
   \----------------------------------------------------------------- */
   protected function postReq ($url, $request, $headerExt = "") 
   {
      $parsedUrl = parse_url($url);
      $host      = $parsedUrl['host'];
      $path      = $parsedUrl['path'];
      $errno     = 0;
      $errstr    = "";
      $reply     = "";
      
      $this->log($request);

      if ($path == "") 
      {
         $path = "/";
      }

      $header  = "POST "  .$path ." HTTP/1.1\r\n";
      $header .= "Host: " .$host ."\r\n";
      $header .= "User-Agent: Mozilla/5.0\r\n";
      $header .= "Content-Type: application/x-www-form-urlencoded\r\n";
      $header .= "Content-Length: " . strlen($request) . "\r\n";
      $header .= "Connection: Close\r\n";

      if ($this->_cookie != "")
      {
         $header .= "Cookie: " .$this->_cookie. "\r\n";
      }

      $header .= $headerExt;
      $header .= "\r\n";

      $fp = fsockopen($host, 80, $errno, $errstr, 30); 

      if (!$fp) 
      {
         die ("Cannot open $host");
      }

      fputs($fp, $header . $request);

      $reply = stream_get_contents($fp);

      fclose($fp);

      return $reply;
   }

   /* -----------------------------------------------------------------\
   |  Method: getReq
   |  Begin: 8/10/2010 / 5:00p
   |  Author: Jo2003
   |  Description: send a get request
   |
   |  Parameters: complete url, optional request string,
   |              optional headers
   |
   |  Returns: response string
   \----------------------------------------------------------------- */
   protected function getReq ($url, $request = "", $headerExt = "") 
   {
      $parsedUrl = parse_url($url);
      $host      = $parsedUrl['host'];
      $path      = $parsedUrl['path'];
      $reply     = "";
      
      $this->log($request);

      if ($path == "") 
      {
         $path = "/";
      }

      $header  = "GET "   .$path .(($request != "") ? "?".$request : ""). " HTTP/1.1\r\n";
      $header .= "Host: " .$host ."\r\n";
      $header .= "User-Agent: Mozilla/5.0\r\n";
      $header .= "Connection: Close\r\n";

      if ($this->_cookie != "")
      {
         $header .= "Cookie: " .$this->_cookie. "\r\n";
      }

      $header .= $headerExt;
      $header .= "\r\n";

      $fp = fsockopen($host, 80, $errno, $errstr, 30); 

      if (!$fp) 
      {
         die ("Cannot open $host");
      }

      fputs($fp, $header);

      $reply = stream_get_contents($fp);

      fclose($fp);

      return $reply;
   }

   /* -----------------------------------------------------------------\
   |  Method: authenticate
   |  Begin: 8/10/2010 / 5:02p
   |  Author: Jo2003
   |  Description: authenticate at kartina ...
   |
   |  Parameters: --
   |
   |  Returns: 0 ==> ok
   |          -1 ==> any error
   \----------------------------------------------------------------- */
   public function authenticate ()
   {
      $this->_cookie = "";
      $sid           = "";
      $sid_name      = "";
      $reply         = $this->postReq($this->_host . API_PATH . "login", 
                                     "login=" . $this->_username . "&pass=" . $this->_passwd);
      $reply         = $this->cleanReply($reply);

      $rv            = 0;
      
      if ($this->noError($reply))
      {
         $this->_reader->XML($reply);

         while ($this->_reader->read())
         {
            switch($this->_reader->nodeType)
            {
            case XMLReader::ELEMENT:
               if ($this->_reader->localName == "sid")
               {
                  if($this->_reader->read())
                  {
                     if ($this->_reader->nodeType == XMLReader::TEXT)
                     {
                        $sid = $this->_reader->value;
                     }
                  }
               }
               else if ($this->_reader->localName == "sid_name")
               {
                  if($this->_reader->read())
                  {
                     if ($this->_reader->nodeType == XMLReader::TEXT)
                     {
                        $sid_name = $this->_reader->value;
                     }
                  }
               }
               break;

            default:
               break;
            }
         }

         $this->_reader->close();
      }
      
      if (($sid != "") && ($sid_name != ""))
      {
         $this->_cookie = $sid_name ."=". $sid;
      }
      else
      {
         $rv = -1;
      }
      
      $this->log("Cookie: ".$this->_cookie."\n");

      return $rv;
   }

   /* -----------------------------------------------------------------\
   |  Method: logout
   |  Begin: 8/10/2010 / 5:03p
   |  Author: Jo2003
   |  Description: logout from host
   |
   |  Parameters: --
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function logout ()
   {
      if ($this->_cookie != "")
      {
         $this->getReq($this->_host . API_PATH . "logout");
         
         // since we're logged out now, make sure 
         // to delete the cookie also!
         @unlink(COOKIE_FILE);
         
         // delete stored files ...
         @unlink (KARTCHANLIST);
         @unlink (KARTFAVLIST);
      }
      $this->_cookie = "";
   }

   /* -----------------------------------------------------------------\
   |  Method: isAuthenticated
   |  Begin: 8/10/2010 / 5:04p
   |  Author: Jo2003
   |  Description: are we authenticated?
   |
   |  Parameters: --
   |
   |  Returns: true --> yes
   |          false --> no
   \----------------------------------------------------------------- */
   public function isAuthenticated ()
   {
      if ($this->_cookie != "")
      {
         return true;
      }
      else
      {
         return false;
      }
   }

   /* -----------------------------------------------------------------\
   |  Method: getChannelList
   |  Begin: 8/10/2010 / 5:05p
   |  Author: Jo2003
   |  Description: get channel list from kartina
   |
   |  Parameters: --
   |
   |  Returns: array of arrays(id, name, icon)
   \----------------------------------------------------------------- */
   public function getChannelList ()
   {
      $reply    = $this->getReq($this->_host . API_PATH . "channel_list");
      $reply    = $this->cleanReply($reply);
      $entries  = array();
      
      if ($this->noError($reply))
      {
         $dom      = new DomDocument();
         $dom->loadXML($reply);
         $xp       = new DOMXpath($dom);
         $entries  = array();
   
         $channels = $xp->query("/response/groups/item/channels/item/id");
         $names    = $xp->query("/response/groups/item/channels/item/name");
         $icons    = $xp->query("/response/groups/item/channels/item/icon");
   
         $all = $channels->length;
   
         for ($i = 0; $i < $all; $i++)
         {
            $entries[] = array(
               'id'   => $channels->item($i)->nodeValue,
               'name' => $names->item($i)->nodeValue,
               'icon' => $icons->item($i)->nodeValue
            );
         }
      }

      return $entries;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getChannelListXml
   |  Begin: 15.11.2010 / 09:36
   |  Author: Jo2003
   |  Description: get channel, don't touch anything
   |
   |  Parameters: --
   |
   |  Returns: false --> error
   |            else --> channel list xml
   \----------------------------------------------------------------- */
   public function getChannelListXml()
   {
      $reply = $this->getReq($this->_host . API_PATH . "channel_list");
      $reply = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         return $reply;
      }
      else
      {
         return false;
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: saveChannelList
   |  Begin: 8/11/2010 / 5:05p
   |  Author: Jo2003
   |  Description: get and save channel list from kartina
   |
   |  Parameters: --
   |
   |  Returns: 0 --> ok
   |          -1 --> any error
   \----------------------------------------------------------------- */
   public function saveChannelList ()
   {
      $rv       = -1;
      $reply    = $this->getReq($this->_host . API_PATH . "channel_list");
      $reply    = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         $fp = fopen(KARTCHANLIST, "w+");
      
         if ($fp)
         {
            fwrite ($fp, $reply);
            fclose ($fp);
            $rv = 0;
         }
      }
      
      return $rv;
   }

   /* -----------------------------------------------------------------\
   |  Method: getStreamUrl
   |  Begin: 8/10/2010 / 5:06p
   |  Author: Jo2003
   |  Description: get live stream url from given channel
   |
   |  Parameters: channel id
   |
   |  Returns: stream url
   \----------------------------------------------------------------- */
   public function getStreamUrl ($cid, $gmt = -1)
   {
      $req   = "cid=".$cid."&protect_code=".$this->_passwd;

      if ($gmt !== -1)
      {
         $req .= "&gmt=".$gmt;
      }

      $reply = $this->postReq($this->_host . API_PATH . "get_url", $req);
      $reply = $this->cleanReply($reply);
      $url   = "";
      
      if ($this->noError($reply))
      {
         $this->_reader->XML($reply);

         while ($this->_reader->read())
         {
            switch ($this->_reader->nodeType)
            {
            // element start ...
            case XMLReader::ELEMENT:
               if ($this->_reader->localName == "url")
               {
                  if($this->_reader->read())
                  {
                     if ($this->_reader->nodeType == XMLReader::TEXT)
                     {
                        $url = $this->_reader->value;
   
                        // replace buffer values ...
                        $url = preg_replace('/^.*(\/\/[^ ]*).*$/', 'http:$1', $url); 
                     }
                  }
               }
               break;

            default:
               break;
            }
         }
         
         $this->_reader->close();
      }

      return $url;
   }

   /* -----------------------------------------------------------------\
   |  Method: cleanReply
   |  Begin: 8/10/2010 / 5:07p
   |  Author: Jo2003
   |  Description: remove http header and remove chunked transmission
   |
   |  Parameters: response string
   |
   |  Returns: cleaned response without header
   \----------------------------------------------------------------- */
   protected function cleanReply($reply)
   {
      list($header, $html) = explode("\r\n\r\n", $reply);

      if (strpos(strtolower($header), "transfer-encoding: chunked") !== FALSE) 
      {
         $html = $this->unchunkHttp11($html);
      }
      
      // remove anything which comes before xml open tags ...
      $html = strstr($html, "<?xml");
      
      $this->log($header.$html);
      
      // extract timestamp from answer ...
      $this->scanServerTime ($html);

      return $html;
   }

   /* -----------------------------------------------------------------\
   |  Method: unchunkHttp11
   |  Begin: 8/10/2010 / 5:08p
   |  Author: Jo2003
   |  Description: remove chunked transmission stuff
   |
   |  Parameters: response string to clean
   |
   |  Returns: cleaned response
   \----------------------------------------------------------------- */
   protected function unchunkHttp11($data) 
   {
      $fp = 0;
      $outData = "";
      while ($fp < strlen($data)) 
      {
         $rawnum   = substr($data, $fp, strpos(substr($data, $fp), "\r\n") + 2);
         $num      = hexdec(trim($rawnum));
         $fp      += strlen($rawnum);
         $chunk    = substr($data, $fp, $num);
         $outData .= $chunk;
         $fp      += strlen($chunk);
      }
      return $outData;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: log
   |  Begin: 8/11/2010 / 9:24a
   |  Author: Jo2003
   |  Description: write string to logfile
   |
   |  Parameters: string to log
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function log($str)
   {
      if (DOTRACE == "YES")
      {
         fwrite($this->_logfile, date("d.m.y H:i:s").": ".$str."\n");
      }   
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getFavorites
   |  Begin: 8/12/2010 / 9:24a
   |  Author: Jo2003
   |  Description: get stored favorites from kartina
   |
   |  Parameters: --
   |
   |  Returns: array of arrays(place, cid)
   \----------------------------------------------------------------- */
   public function getFavorites()
   {
      $favorites = array();
      $place     = 0;
      $cid       = 0;
      $reply     = $this->getReq($this->_host.API_PATH."/favorites");
      $reply     = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         $dom       = new DOMDocument();
         $dom->loadXML($reply);
         
         $xp        = new DOMXPath($dom);
         
         $places    = $xp->query("/response/favorites/item/place");
         $cids      = $xp->query("/response/favorites/item/channel_id");
         
         $count     = $places->length;
         
         for ($i = 0; $i < $count; $i ++)
         {
            $favorites[] = array(
               'place' => $places->item($i)->nodeValue, 
               'cid'   => $cids->item($i)->nodeValue
            );
         }
      }
      
      return $favorites;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getFavoritesXML
   |  Begin: 15.11.2010 / 10:55
   |  Author: Jo2003
   |  Description: get stored favorites from kartina (XML)
   |
   |  Parameters: --
   |
   |  Returns: false --> error
   |            else --> favorites as XML
   \----------------------------------------------------------------- */
   public function getFavoritesXML()
   {
      $reply = $this->getReq($this->_host.API_PATH."/favorites");
      $reply = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         return $reply;
      }
      else
      {
         return false;
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: saveFavorites
   |  Begin: 8/12/2010 / 9:24a
   |  Author: Jo2003
   |  Description: get and save favorites to file
   |
   |  Parameters: --
   |
   |  Returns: 0 --> ok
   |          -1 --> error writing file
   \----------------------------------------------------------------- */
   public function saveFavorites()
   {
      $rv    = -1;
      $reply = $this->getReq($this->_host.API_PATH."/favorites");
      $reply = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         $fp = fopen(KARTFAVLIST, "w+");
      
         if ($fp)
         {
            fwrite($fp, $reply);
            fclose($fp);
            $rv = 0;
         }
      }
      
      return $rv;
   }

   /* -----------------------------------------------------------------\
   |  Method: setFavorite
   |  Begin: 8/12/2010 / 9:24a
   |  Author: Jo2003
   |  Description: set 1 favorite
   |
   |  Parameters: place (1 - 12), channel id (o to delete) 
   |
   |  Returns: --
   \----------------------------------------------------------------- */
   public function setFavorite($place, $cid)
   {
      $this->postReq($this->_host.API_PATH."/favorites_set", "place=".$place."&cid=".$cid);
   }

   /* -----------------------------------------------------------------\
   |  Method: getDayEpg
   |  Begin: 10/05/2010 / 3:30p
   |  Author: Jo2003
   |  Description: get epg for cid and day
   |
   |  Parameters: channel id, day in form ddmmyy
   |
   |  Returns: array of arrays(id, name, icon)
   \----------------------------------------------------------------- */
   public function getDayEpg ($cid, $day)
   {
      $reply    = $this->getReq($this->_host . API_PATH . "epg", "cid=".$cid."&day=".$day);
      $reply    = $this->cleanReply($reply);
      $entries  = array();
      
      if ($this->noError($reply))
      {
         $dom     = new DomDocument();
         $dom->loadXML($reply);
         $xp      = new DOMXpath($dom);
         $entries = array();
   
         $start   = $xp->query("/response/epg/item/ut_start");
         $names   = $xp->query("/response/epg/item/progname");
   
         $all     = $start->length;
   
         for ($i = 0; $i < $all; $i++)
         {
            $entries[] = array(
               'timestamp' => $start->item($i)->nodeValue,
               'programm'  => $names->item($i)->nodeValue
            );
         }
      }

      return $entries;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getShowInfo
   |  Begin: 25.10.2010 / 10:30
   |  Author: Jo2003
   |  Description: get show info for one show
   |
   |  Parameters: channel id, optional unix timestamp
   |
   |  Returns: showinfo array with following keys:
   |        channel, icon, start, end, name, descr
   \----------------------------------------------------------------- */
   public function getShowInfo ($cid, $gmt = -1)
   {
      $showinfo = array();
      
      // get channel name and channel icon ...
      if (($channelList = $this->getChannelListXml ()) !== FALSE)
      {
         $domChanList = new DOMDocument();
         $domChanList->loadXML($channelList);
      
         $xpchan   = new DOMXpath($domChanList);
         $chanitem = $xpchan->query("/response/groups/item/channels/item[id='".$cid."']");
         $chan     = $chanitem->item(0); // there is only one such item ...
      
         // name ...
         $showinfo['channel'] = $xpchan->query("name", $chan)->item(0)->nodeValue;
      
         // icon ...
         $showinfo['icon']    = KARTINA_HOST.$xpchan->query("icon", $chan)->item(0)->nodeValue;
      
         // is gmt given here ... ?
         if ($gmt == -1)
         {
            // not given --> get info from channel list ...
         
            // start ...
            $showinfo['start'] = $xpchan->query("epg_start", $chan)->item(0)->nodeValue;
         
            // end ...
            $showinfo['end'] = $xpchan->query("epg_end", $chan)->item(0)->nodeValue;
         
            // get cut position ...
            $name = $xpchan->query("epg_progname", $chan)->item(0)->nodeValue;
         
            // cut between name and decription ...
            $cutpos = strpos($name, "\n");
         
            if ($cutpos !== false)
            {
               // show name ...
               $showinfo['name']  = substr($name, 0, $cutpos);
         
               // show description ...
               $showinfo['descr'] = substr($name, $cutpos + 1);
            }
            else
            {
               // show name ...
               $showinfo['name']  = $name;
         
               // no show description ...
               $showinfo['descr'] = "";
            }
         }
         else
         {
            // gmt is set, get epg for this day ...
            $epg = $this->getDayEpg($cid, date("dmy", $gmt));
         
            if (is_array($epg))
            {
               $count = count($epg);
            
               for ($i = 0; $i < $count; $i ++)
               {
                  if ($epg[$i]['timestamp'] == $gmt)
                  {
                     // found entry ...
                  
                     // get start time ...
                     $showinfo['start'] = $epg[$i]['timestamp'];
                  
                     // is there a next entry ... ?
                     if ($i < ($count - 1))
                     {
                        // start of next show is end of this show ...
                        $showinfo['end'] = $epg[$i + 1]['timestamp'];
                     }
                     else
                     {
                        // no info --> use one hour ...
                        $showinfo['end'] = $showinfo['start'] + 3600;
                     }
                  
                     // get cut position ...
                     $cutpos = strpos($epg[$i]['programm'], "\n");
                  
                     if ($cutpos !== false)
                     {
                        // show name ...
                        $showinfo['name']  = substr($epg[$i]['programm'], 0, $cutpos);
                  
                        // show description ...
                        $showinfo['descr'] = substr($epg[$i]['programm'], $cutpos + 1);
                     }
                     else
                     {
                        // show name ...
                        $showinfo['name']  = $epg[$i]['programm'];
                  
                        // no show description ...
                        $showinfo['descr'] = "";
                     }

                     // we found what we're looking for --> break for loop ...
                     break;
                  }
               }
            }
         }
      }
      
      return $showinfo;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: noError
   |  Begin: 8/12/2010 / 9:24a
   |  Author: Jo2003
   |  Description: check reply for no error
   |
   |  Parameters: reply 
   |
   |  Returns: 1 --> no error
   |           0 --> error
   \----------------------------------------------------------------- */
   protected function noError($reply)
   {
      if (strstr($reply, "<message>") && strstr($reply, "<code>"))
      {
         // on error log out ...
         $this->logout();
         return 0;
      } 
      else 
      {
         return 1;
      }
   }
   
   /* -----------------------------------------------------------------\
   |  Method: saveCookie
   |  Begin: 06.10.2010 / 15:10
   |  Author: Jo2003
   |  Description: save cookie to file
   |
   |  Parameters: --
   |
   |  Returns: 0 --> no error
   |          -1 --> error
   \----------------------------------------------------------------- */
   public function saveCookie()
   {
      $rv = -1;
      if ($this->_cookie != "")
      {
         $cookieData = array('cookie'  => $this->_cookie,
                             'account' => $this->_username,
                             'passwd'  => $this->_passwd,
                             'host'    => $this->_host);
                          
         $f = fopen (COOKIE_FILE, "w+");
         
         if ($f)
         {
            if (fwrite ($f, serialize($cookieData)))
            {
               $rv = 0;
               $this->log("Cookie successfully saved to file!");
            }
            
            fclose ($f);
         }
      }

      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: loadCookie
   |  Begin: 06.10.2010 / 15:30
   |  Author: Jo2003
   |  Description: load cookie from file
   |
   |  Parameters: --
   |
   |  Returns: 0 --> no error
   |          -1 --> error
   \----------------------------------------------------------------- */
   public function loadCookie ()
   {
      $rv        = -1;
      $tmpCookie = file_get_contents (COOKIE_FILE);
      
      if ($tmpCookie)
      {
         $cookieData = unserialize($tmpCookie);
         
         $this->_cookie   = $cookieData['cookie'];
         $this->_host     = $cookieData['host'];
         $this->_username = $cookieData['account'];
         $this->_passwd   = $cookieData['passwd'];
         $rv              = 0;
         $this->log("Cookie successfully read from file!");
      }
      
      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getServerTime
   |  Begin: 10.11.2010 / 16:45
   |  Author: Jo2003
   |  Description: get server time from kartina server ...
   |
   |  Parameters: --
   |
   |  Returns: false --> error
   |            else --> server time as unix time stamp ...
   \----------------------------------------------------------------- */
   public function getServerTime ()
   {
      // to get server time use a simple settings call.
      // server time is included into every answer...
      $reply      = $this->getReq($this->_host . API_PATH . "settings", "var=timezone");
      $reply      = $this->cleanReply($reply);
      
      $serverTime = false;
      
      $this->_reader->XML($reply);

      while ($this->_reader->read())
      {
         switch ($this->_reader->nodeType)
         {
         // element start ...
         case XMLReader::ELEMENT:
            if ($this->_reader->localName == "servertime")
            {
               if($this->_reader->read())
               {
                  if ($this->_reader->nodeType == XMLReader::TEXT)
                  {
                     // get server time ...
                     $serverTime = $this->_reader->value;
                  }
               }
            }
            break;

         default:
            break;
         }
      }
      
      $this->_reader->close();

      $this->log("Got following server time: ".$serverTime); 

      return $serverTime;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: scanServerTime
   |  Begin: 15.11.2010 / 09:30
   |  Author: Jo2003
   |  Description: get server time from xml answer
   |
   |  Parameters: xml answer
   |
   |  Returns: 0 --> ok
   |          -1 --> any error
   \----------------------------------------------------------------- */
   protected function scanServerTime($xmlCode)
   {
      $rv = -1;
      
      $this->_reader->XML($xmlCode);

      // scn xml for servertime element ...
      while ($this->_reader->read())
      {
         switch ($this->_reader->nodeType)
         {
         // element start ...
         case XMLReader::ELEMENT:
            if ($this->_reader->localName == "servertime")
            {
               if($this->_reader->read())
               {
                  if ($this->_reader->nodeType == XMLReader::TEXT)
                  {
                     // cache server time value ...
                     $this->_lastServerTime = $this->_reader->value;
                     $rv                    = 0;
                  }
               }
            }
            break;

         default:
            break;
         }
      }
      
      $this->_reader->close();

      return $rv;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getLastServerTime
   |  Begin: 15.11.2010 / 09:35
   |  Author: Jo2003
   |  Description: get last stored server time
   |
   |  Parameters: --
   |
   |  Returns: -1 --> no server time stored yet
   |         else --> last cached server time
   \----------------------------------------------------------------- */
   public function getLastServerTime()
   {
      return $this->_lastServerTime;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getVodGenres
   |  Begin: 16.12.2010 / 11:35
   |  Author: Jo2003
   |  Description: get vod genres from kartina.tv
   |               This function will change as soon as kartinas API
   |               supports this.
   |
   |  Parameters: --
   |
   |  Returns: array with genres array(id => numb, name => genre name)
   \----------------------------------------------------------------- */
   public function getVodGenres()
   {
      $genres  = array();
      $matches = array();

      // in respone we search for 
      // <div class="filter" rel="[genre id]">[genre name]</div>
      $filter = '|<div[ \t]+class="filter"[ \t]+rel="([0-9]+)">([^<]+)</div>|Ui';
      
      // until now we get no xml but html here ...
      $resp   = $this->getReq($this->_host."/", "m=vod&act=home");
      
      list($header, $html) = explode("\r\n\r\n", $resp);

      if (strpos(strtolower($header), "transfer-encoding: chunked") !== FALSE) 
      {
         $html = $this->unchunkHttp11($html);
      }
      
      $html = strstr($html, '<div class="filter"');
      
      $this->log($html); 
      
      preg_match_all($filter, $html, $matches, PREG_PATTERN_ORDER);
      
      
      for ($i = 0; $i < count($matches[1]); $i++)
      {
         $genres[] = array(
            'id'   => $matches[1][$i], 
            'name' => mb_strtoupper($matches[2][$i], "UTF-8") // uppercase looks simply better ...
         );
      }
      
      return $genres;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getGenreVideos
   |  Begin: 16.12.2010 / 12:35
   |  Author: Jo2003
   |  Description: get genre videos
   |
   |  Parameters: genre id
   |
   |  Returns: array with videos array(id      => numb, 
   |                                   name    => video name,
   |                                   img     => thumbnail,
   |                                   year    => production year
   |                                   country => filmed in)
   \----------------------------------------------------------------- */
   public function getGenreVideos ($gid)
   {
      $videos = array();
      $reply  = $this->getReq($this->_host.API_PATH."vod_list", "type=last&genre=".$gid);
      $reply  = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         $dom       = new DomDocument();
         $dom->loadXML($reply);
         $xp        = new DOMXpath($dom);
   
         $ids       = $xp->query("/response/rows/item/id");
         $names     = $xp->query("/response/rows/item/name");
         $imgs      = $xp->query("/response/rows/item/poster");
         $years     = $xp->query("/response/rows/item/year");
         $countries = $xp->query("/response/rows/item/country");
         
         $all       = $ids->length;
   
         for ($i = 0; $i < $all; $i++)
         {
            $videos[] = array(
               'id'      => $ids->item($i)->nodeValue,
               'name'    => $names->item($i)->nodeValue,
               'img'     => $imgs->item($i)->nodeValue,
               'year'    => $years->item($i)->nodeValue,
               'country' => $countries->item($i)->nodeValue
            );
         }
      }
      
      return $videos;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getVideoTracks
   |  Begin: 16.12.2010 / 13:05
   |  Author: Jo2003
   |  Description: get video tracks
   |
   |  Parameters: video id
   |
   |  Returns: array(name    => video name,
   |                 ids     => array with track ids)
   |
   \----------------------------------------------------------------- */
   public function getVideoTracks ($vid)
   {
      $video  = array();
      $reply  = $this->getReq($this->_host.API_PATH."vod_info", "id=".$vid);
      $reply  = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         $dom           = new DomDocument();
         $dom->loadXML($reply);
         $xp            = new DOMXpath($dom);
   
         $ids           = $xp->query("/response/film/videos/item/id");
         $video['name'] = $xp->query("/response/film/name")->item(0)->nodeValue;
         
         
         $all           = $ids->length;
   
         for ($i = 0; $i < $all; $i++)
         {
            $video['ids'][] = $ids->item($i)->nodeValue;
         }
      }
      
      return $video;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getVodUrl
   |  Begin: 16.12.2010 / 13:35
   |  Author: Jo2003
   |  Description: get url for vod track
   |
   |  Parameters: vod track id
   |
   |  Returns: url
   |
   \----------------------------------------------------------------- */
   public function getVodUrl($vod_tid)
   {
      $reply = $this->postReq($this->_host.API_PATH."vod_geturl", "fileid=".$vod_tid);
      $reply = $this->cleanReply($reply);
      $url   = "";
      
      if ($this->noError($reply))
      {
         $this->_reader->XML($reply);

         while ($this->_reader->read())
         {
            switch ($this->_reader->nodeType)
            {
            // element start ...
            case XMLReader::ELEMENT:
               if ($this->_reader->localName == "url")
               {
                  if($this->_reader->read())
                  {
                     if ($this->_reader->nodeType == XMLReader::TEXT)
                     {
                        $url = $this->_reader->value;
   
                        // replace buffer values ...
                        $url = preg_replace('/^.*(\/\/[^ ]*).*$/', 'http:$1', $url); 
                     }
                  }
               }
               break;

            default:
               break;
            }
         }
         
         $this->_reader->close();
      }

      return $url;
   }
   
   /* -----------------------------------------------------------------\
   |  Method: getVideoDetailXml
   |  Begin: 16.12.2010 / 15:35
   |  Author: Jo2003
   |  Description: get video details
   |
   |  Parameters: video id
   |
   |  Returns: XML reply
   |
   \----------------------------------------------------------------- */
   public function getVideoDetailXml ($vid)
   {
      $rv    = false;
      
      $reply = $this->getReq($this->_host.API_PATH."vod_info", "id=".$vid);
      $reply = $this->cleanReply($reply);
      
      if ($this->noError($reply))
      {
         $rv = $reply;
      }
      
      return $rv;
   }
};

?>
