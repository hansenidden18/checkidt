# CheckIDT

To install this module:
```
$ sudo make

$ sudo insmod checkidt.ko

$ lsmod

$ dmesg
```
Always check where is your build-essentials are kept:

```
$ find / | grep include/generated/autoconf.h
```
change the make on Makefile according to your build-essentials folder
