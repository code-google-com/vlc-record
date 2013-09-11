#!/usr/bin/python

import glob
from os import path
import pprint
import httplib2
import io

##----------------------------------------------------------------------
# @brief    try to get architecture from file name
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fname (string) file name
#
# @return   string with architecture
#-----------------------------------------------------------------------
def getArch(fname):
   arch = "unkwn";
   if path.isfile(fname):
        if fname.find("win") != -1:
            arch = "win";
        elif fname.find("mac") != -1:
            arch = "mac";
        elif fname.find("amd64") != -1:
            arch = "lin64";
        elif fname.find("i386") != -1:
            arch = "lin32";
        else:
            arch = "unkwn";
      
   return arch;

##----------------------------------------------------------------------
# @brief    get date from file name (parser)
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fname (string) file name
#
# @return   string with date (if found)
#-----------------------------------------------------------------------
def getDate(fname):
    v   = getVer(fname);
    idx = fname.find(v) + len(v) + 1;
    return fname[idx:idx + 8];
    
    
##----------------------------------------------------------------------
# @brief    try to get version from file name
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fname (string) file name
# @param    ext (int) flag for extended format
#
# @return   string with version
#-----------------------------------------------------------------------
def getVer(fname, ext = 0):
    idx = len(getProgName(fname)) + 1;
    l   = fname.find("-", idx);
    v   = fname[idx:l];
    
    if ext != 0:
        if v.lower().find("B") > -1:
            v += " (Beta)";
        elif v.lower().find("rc") > -1:
            v += " (Release Candidate)";

    return v;

##----------------------------------------------------------------------
# @brief    get file extension from file name
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fname (string) file name
#
# @return   string with extension
#-----------------------------------------------------------------------
def fileExt(fname):
    l = fname.split(".");
    i = len(l);
    if (i > 0):
        return l[i - 1];
    else:
        return "";

##----------------------------------------------------------------------
# @brief    get mime type from file extension
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fext (string) file extension
#
# @return   string with mime type
#-----------------------------------------------------------------------
def mimeType(fext):
    fext = fext.lower();
    mt   = "";
    if fext == "7z":
        mt = "application/x-7z-compressed";
    elif fext == "zip":
        mt = "application/zip";
    elif fext == "deb":
        mt = "application/x-deb";
    elif fext == "dmg":
        mt = "application/x-apple-diskimage";
    elif fext == "exe":
        mt = "application/octet-stream";
        
    return mt

##----------------------------------------------------------------------
# @brief    create program description from file name
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fname (string) file name
#
# @return   dictionary as used for google drive file upload
#-----------------------------------------------------------------------
def createDescr(fname):
    body = {
        'title': fname,
        'description': getProgName(fname) + " " + getVer(fname, 1) + " for " + getOS(getArch(fname)) + " built at " + getDate(fname),
        'mimeType': mimeType(fileExt(i)),
        'parents': [{
            'kind': 'drive#fileLink',
            'id': '0Bz0YtDApAonAYzFGdlZGYV9FbXM'
        }]
    };
    return body;

##----------------------------------------------------------------------
# @brief    get OS name from architecture
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    arch (string) architecture shortcut
#
# @return   string with OS name
#-----------------------------------------------------------------------
def getOS(arch):
    os = "";
    if arch == "win":
        os = "Windows";
    elif arch == "mac":
        os = "MacOS";
    elif arch == "lin32":
        os = "Linux (x86)";
    elif arch == "lin64":
        os = "Linux (amd64)";

    return os;

##----------------------------------------------------------------------
# @brief    recognize program name from file name
#
# @Author   Jo2003
# @Date     10.09.2013
#
# @param    fname (string) file name
#
# @return   string with program name
#-----------------------------------------------------------------------
def getProgName(fname):
    fname = fname.lower();
    prg   = "";
    
    if fname.find("polsky") > -1:
        prg = "Polsky.TV";
    elif fname.find("record") > -1:
        prg = "VLC-Record";
    elif fname.find("kartina") > -1:
        prg = "Kartina.TV";

    return prg;

# ---------------------------------------------------
# /function section 
# ---------------------------------------------------

from apiclient.discovery import build
from apiclient.http import MediaFileUpload
from oauth2client.client import OAuth2WebServerFlow


# Copy your credentials from the APIs Console
CLIENT_ID = '111122223333444455556666777788889999.apps.googleusercontent.com'
CLIENT_SECRET = 'aaabbbcccdddeeefffggg'
REDIR_URI = 'urn:ietf:wg:oauth:2.0:oob'


# Check https://developers.google.com/drive/scopes for all available scopes
OAUTH_SCOPE = 'https://www.googleapis.com/auth/drive'

# Run through the OAuth flow and retrieve credentials
flow = OAuth2WebServerFlow(CLIENT_ID, CLIENT_SECRET, OAUTH_SCOPE, REDIR_URI)
authorize_url = flow.step1_get_authorize_url()
print 'Go to the following link in your browser: ' + authorize_url
code = raw_input('Enter verification code: ').strip()
credentials = flow.step2_exchange(code)


# Create an httplib2.Http object and authorize it with our credentials
http = httplib2.Http()
http = credentials.authorize(http)


drive_service = build('drive', 'v2', http=http)

files = glob.glob("*.*")

ainfo = []
for i in files:
    a = getArch(i);
    if (a != "unkwn"):
        # Insert a file
        media_body = MediaFileUpload(i, mimetype=mimeType(fileExt(i)), resumable=True);
        body       = createDescr(i);
        print "Upload: " + body["description"];
        file = drive_service.files().insert(body=body, media_body=media_body).execute();
        
        finfo = {'fname' : i,
                 'arch'  : getArch(i),
                 'mime'  : mimeType(fileExt(i)),
                 'date'  : getDate(i),
                 'name'  : body["description"],
                 'md5'   : file.get("md5Checksum"),
                 'link'  : file.get("webContentLink"),
                 'ver'   : getVer(i)};
        
        ainfo.append(finfo);

        print "-----------------------------------------------";

f = open("update.sql", "w");

for i in range(0, len(ainfo)):
    f.write("INSERT INTO vlcr_downloads (name, link, descr, ver, date, arch, mime, checksum, downloads) VALUES ("
            + "'" + ainfo[i]['fname'] + "', "
            + "'" + ainfo[i]['link' ] + "', "
            + "'" + ainfo[i]['name' ] + "', "
            + "'" + ainfo[i]['ver'  ] + "', "
            + "'" + ainfo[i]['date' ] + "', "
            + "'" + ainfo[i]['arch' ] + "', "
            + "'" + ainfo[i]['mime' ] + "', "
            + "'" + ainfo[i]['md5'  ] + "', 0);\n");
    
f.close();
