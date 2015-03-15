# Problem #

In case we get segfault when VLC-Record based on Qt5 loads the libVLC we should think about that on most Linux systems VLC is built with Qt4 support. When our Qt5 application loads the player it too loads the GUI of the player which is based on **Qt4**. This leads to segfault!

# Workaround #

Make a copy of the plugins and remove the gui folder inside. When creating the libVLC instance use additional parameter _--plugin-path=./plugin\_copy_