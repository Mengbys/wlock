# wlock
wlock is a simple console locker, which is basically built on the work of [vlock](https://github.com/WorMzy/vlock). wlock can only lock the current terminal console, but it can show a fancy ui appearance (designed by myself) once locked. I pretty love it although you may think it is a meaningless work.


![gif](https://github.com/Mengbys/wlock/blob/main/img/1.gif)


## Usage
Once wlock is installed, you can type ```wlock``` in the command line and press Enter to lock your console. If you want to unlock it, you need to press Enter and input the password for current user or root. The whole work flow of wlock is shown in the gif above.

## Installation
### Pre-built Release
Pre-built binary file can be found [here](https://github.com/Mengbys/wlock/releases), which is built on gcc (version 9.4.0) and make (version 4.2.1). And it serves for linux platform (maybe MacOS?).

After downloading, you should do some things extra to make wlock work:
```bash
# with sudo privilege
$ chown root ./wlock
$ chgrp root ./wlock
$ chmod 7411 ./wlock
$ mv ./wlock /usr/sbin
```
You can simply remove wlock by ```sudo rm /usr/sbin/wlock```.

### Build from Source
#### Requirement
- gcc
- make
- [Linux-PAM](https://github.com/linux-pam/linux-pam)
#### Compile and Install
Follow these steps:
```bash
$ git clone https://github.com//mengbys/wlock.git
$ cd wlock
$ make
$ sudo make install
```
#### Uninstall
Just change directory into wlock and:
```shell
$ sudo make uninstall
$ make clean
```

## License
GPL-2.0
