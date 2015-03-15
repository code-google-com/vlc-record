# Intro #
Running VLC-Record 2.xx under Ubuntu isn't that easy as I liked it to ...
But because for me it's really important to have a "full" Linux-Version, I tried to figure out how to make it. Don't expect a 100% clean way. I haven't the time to do so ...

# What we need ... #
  * We need [a "special" vlc package](http://vlc-record.googlecode.com/files/vlc_1.0.5-1_i386.deb) with last stable version of the vlc player.
  * We need the [package of VLC-Record](http://vlc-record.googlecode.com/files/vlc-record_2.23-1_i386.deb).
  * We need a (very) little knowledge about Ubuntu.

# Let's go #
  * Uninstall the old vlc as well as dependent packages.
```
sudo apt-get remove vlc libvlc2 libvlccore2 vlc-data
```

  * Download and install the new vlc package
```
sudo dpkg -i /path/to/vlc_1.0.5-1_i386.deb
```

  * tell ld where to look for libvlc
```
sudo sh -c "cat >/etc/ld.so.conf.d/libvlc.conf << EOF
# where to look for libvlc ...
/opt/vlc-1.0.5/lib
EOF"
```

  * run ldconfig
```
sudo ldconfig
```

  * Download and install VLC-Record
```
sudo dpkg -i /path/to/vlc-record_2.23-1_i386.deb
```
  * Done!

# Where to find what? #
There are no menu entries for the new installed software. Nevertheless you can use it ;-)
The new vlc player is now located in _/opt/vlc-1.0.5_. So if you want to start it, simply run _/opt/vlc-1.0.5/bin/vlc_. VLC-Record is located in _/opt/vlc-record_. Run it using following command line: _/opt/vlc-record/vlc-record_. Create the starter you like where you like. If all works well VLC-Record now uses the updated libVLC and you can enjoy the new version.

# Howto get rid of this shit? #
You don't like what you have now? Why ...? Nevertheless there is a 100% clean way to get rid of the new installed stuff.

  * Remove VLC-Record
```
sudo dpkg -r vlc-record
```
  * Remove the special vlc player package
```
sudo dpkg -r vlc
```
  * Remove the entry in _/etc/ld.so.conf.d_
```
sudo rm /etc/ld.so.conf.d/libvlc.conf
```
  * Update linker cache
```
sudo ldconfig
```