

# Introduction #

Although vlc is part of the Ubuntu distros it maybe helpful to build the player at your own because often feature updates will not be taken into the distribution.

There is already a good [wiki](http://wiki.videolan.org/UnixCompile) which describes the build process. I have taken this as source to make it more clear for Ubuntu users.

# Goal #

We want to build a vlc player which is **independent** from a maybe already installed version of the vlc player. So an existing installation will not be touched nor changed.

# Get the sources #

  * [vlc 1.0.5](http://download.videolan.org/pub/videolan/vlc/1.0.5/vlc-1.0.5.tar.bz2)
  * [LIVE555 Streaming Media library](http://www.live555.com/liveMedia/public/live555-latest.tar.gz)

# Get dependencies #

```
sudo apt-get build-dep vlc
sudo apt-get install autoconf
```

# Prepare #

Create a source folder and extract the live555 media stuff.
```
mkdir -p ~/src
cd ~/src
tar -xvzpf /path/to/live555-latest.tar.gz
```

Change the config input of the live media stuff to build position independent code (PIC).
Open file _~/src/live/config.linux_ and add option **-fPIC** to COMPILE\_OPTS line. After that it should look like this:

```
COMPILE_OPTS =          $(INCLUDES) -fPIC -I. -O2 -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
```

Close the file and create make files and make. **Don't run make install!**

```
cd ~/src/live
./genMakefiles linux
make
```

Extract the player sources:

```
cd ~/src
tar -xvjpf /path/to/vlc-1.0.5.tar.bz2
mkdir -p vlc-build
```

# Build #
Make bootstrap

```
cd vlc-1.0.5
./bootstrap
cd ..
```

Go into the build directory and configure vlc:

```
cd vlc-build
../vlc-1.0.5/configure --prefix=/opt/vlc-1.0.5 \
 --enable-live555 --with-live555-tree=$HOME/src/live \
 --enable-faad --enable-release --disable-gme
```

Compile vlc player:

```
make
```

# Install #
To be sure that you can remove the package if you don't like it, we install it using _checkinstall_.

```
sudo checkinstall
```

Make sure you don't use _vlc_ only for package name. We want to be independent from a maybe already installed version of vlc. So name it _myvlc_ or something like that:

```
Das Paket wird entsprechend dieser Vorgaben erstellt:

0 -  Maintainer: [ root@joergn-bunteku ]
1 -  Summary: [ my own special build of vlc ]
2 -  Name:    [ myvlc ]
3 -  Version: [ 1.0.5 ]
4 -  Release: [ 4 ]
5 -  License: [ GPL ]
6 -  Group:   [ checkinstall ]
7 -  Architecture: [ i386 ]
8 -  Source location: [ vlc-1.0.5 ]
9 -  Alternate source location: [  ]
10 - Requires: [  ]
11 - Provides: [ myvlc ]

Geben Sie die betreffende Nummer ein, um die Vorgaben zu ändern:
```

If you then want to remove your package, you can simply run:

```
sudo dpkg -r myvlc
```