echo off

set PATH=c:\QtSDK-x86_64\bin;%PATH%

echo change to C:\Users\jcl\Documents\SVN\glnemo2\trunk
cd /D C:\Users\jcl\Documents\SVN\glnemo2\trunk


echo tortoise svn up
tortoiseproc /command:update /path:. /closenoend:0

echo go to build directory
cd /D C:\Users\jcl\Documents\SVN\glnemo2\trunk\build

echo start cmake command
cmake -G "MinGW Makefiles" .. -DCMAKE_PREFIX_PATH=c:\QtSDK-x86_64

echo you must enter: mingw32-make
