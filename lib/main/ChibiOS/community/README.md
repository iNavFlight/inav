ChibiOS-Contrib
===============
Code under this directory is not part of the core ChibiOS project 
and the copyright is retained by the original authors. See copyright
notes in file headers.

Code is maintained via Github: https://github.com/ChibiOS/ChibiOS-Contrib  
Feel free to open pull request there.

#### Using

Default makefiles assume this repo will be cloned in the same folder as ChibiOS.  
If you use another location, you will have to edit the Makefile, or specify an alternate location to make, ie:  

```
cd ChibiOS-Contrib/testhal/STM32/STM32F3xx/TIMCAP
make CHIBIOS=~/src/ChibiOS
```

A good way to use it is to clone it as a submodule inside your project:  

```
git submodule add https://github.com/ChibiOS/ChibiOS.git
git submodule add https://github.com/ChibiOS/ChibiOS-Contrib.git
```

#### Contributing

When you submit a pull request, make sure all modified platform tests compile using the tools/build.sh script.  
There is an automated check that will report errors if it doesn't.  
Feel free to update authors.txt with your name.  

#### Useful links

https://help.github.com/  
http://git-scm.com/  
http://chibios.org/dokuwiki/doku.php?id=chibios:guides:style_guide  
http://www.chibios.com/forum/  
