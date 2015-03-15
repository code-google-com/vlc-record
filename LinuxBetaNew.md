# Summary #
There is a new **Beta version** of VLC-Record for Ubuntu **10.10 x86**. This version uses the vlc player which comes with Ubuntu 10.10.

## Requirements ##
Your package manager should take care about the dependencies.

## Installation ##
Use the package manager to install the package. A double click on the package will do this job.

## Removing ##
Use the package manager to remove vlc-record. You can also use following command line:

```
sudo apt-get remove vlc-record
```

## Start the program ##
The binary is located in _/usr/local/bin_. This should be in the path. So if you want to start it, create a shortcut with following command line:

```
vlc-record
```

... or type in this command line in your terminal.

## Known Limitations ##
  * If the vlc player which comes with Ubuntu is crippled, vlc-record will have the same problems.
  * There are problems with the shortcuts. I have no idea how to fix it. After pressing some key combinations often, shortcuts will not work until you restart VLC-Record.
  * When changing aspect ratio, video frame is sometimes not shrinked / positioned as needed.
  * Programm might freeze when you close it the first time.