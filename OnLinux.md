# VLC-Record for Linux #

## At this time ##

... the Ubuntu package for the 2.xx version doesn't work without some handmade stuff. Have a look at the NewVersionOnUbuntu Wiki. Hopefully this will help you.

## Intro ##

For versions newer than x.20 I decided to build packages for the actual Ubuntu only. So there is no static linked application but it uses the qt4 stuff which comes with Ubuntu. The 2.xx version (the so called new version) also uses the libVLC which comes with Ubuntu. At this time this is libvlc2. Because kartina.tv changed the archiv access to http protocol the "old" version of Ubuntu is no problem.

## Other Linux Distributions ##

If you need VLC-Record for another Linux distribution feel free to build it at your own. Simply install the qt4dev package from your distribution and install the QtCreator from [Nokia](http://qt.nokia.com). libvlc and libvlccore including the plugins are also needed. If you need help building vlc-record, [have a look](BuildVLCRecord.md) or simply ask.

## Installation ##

Install the deb packages as usual. There will be **NO** menu entry for this. You'll find the installed programms at _/opt/vlc-record_ or _/opt/vlc-rcd-classic_. You can create a menu entry / shortcut at your own.

## Differences to the Windows Version ##

For the classic version there are no differences to the Windows version. For the new version there are: Shortcuts for fullscreen and aspect ratio aren't working so far in fullscreen mode. To leave the fullscreen mode, double click at the video window.

## Download ##

You can download the linux packages [here](http://code.google.com/p/vlc-record/downloads/list)