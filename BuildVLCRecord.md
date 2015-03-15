

# Introduction #
Since Linux is such a wide field it's impossible to provide you with all possible packages so that you find the version that you need. So what we can do? Of course - make it on your own ;-)

# What do we need? #
At least we need subversion, a compiler (gcc with g++), the qt4 dev framework, the libvlc-dev package, make and in addition checkinstall. To get these things on Ubuntu we run:

```
sudo apt-get install libqt4-dev libx11-dev vlc-nox libvlc-dev build-essential gawk gtk2-engines-pixbuf subversion
```

This also will install some additional packages our packages depend on.

# Prepare #
Create a directory where we'll put the sources of vlc-record. We could make it that way:
```
mkdir -p ~/src
```

Now we go into the created _src_ folder and check out the vlc-record sources.

```
cd ~/src
svn checkout http://vlc-record.googlecode.com/svn/trunk/vlc-record vlc-record
```

# Build #
Go into the source directory and create the make files using the qmake command:

```
cd ~/src/vlc-record
qmake vlc-record.pro
```

Cause we have the make files, let's start to build vlc-records binary.

```
make release
```

Hopefully all went well!

# Install #
I have prepared some simple install scripts. These scripts will install vlc-record for you. To use these scripts, type following command:

```
sudo make -f install.mak
```

After that vlc-record should be installed. In Ubuntu you can search for vlc-record in DASH. An application shortcut is also created, but you have to add it to the menu at your own.

[![](https://www.paypal.com/de_DE/DE/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=11286909)