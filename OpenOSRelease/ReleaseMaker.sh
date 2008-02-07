#!/bin/bash

rm -rf OpenOS
mkdir OpenOS

cp ../AsaacInterface/ASAAC.h OpenOS;           if (test $? != 0) then exit; fi;
cp ../AsaacInterface/ASAAC_IMPL.h OpenOS;      if (test $? != 0) then exit; fi;
cp ../AsaacInterface/ASAAC_ENH.h OpenOS;       if (test $? != 0) then exit; fi;

cp ../OpenOSLib/OpenOS.h OpenOS;               if (test $? != 0) then exit; fi;
cp ../OpenOSLib/OpenOS.hh OpenOS;              if (test $? != 0) then exit; fi;

cp ../MOSLib/Debug/libMOS.so OpenOS;           if (test $? != 0) then exit; fi;
cp ../OpenOSLib/Debug/libOpenOS.so OpenOS;     if (test $? != 0) then exit; fi;
cp ../OpenOSEntity/Debug/OpenOSEntity OpenOS;  if (test $? != 0) then exit; fi;

cp ../AsaacCLib/Debug/libAsaacC.a OpenOS;      if (test $? != 0) then exit; fi;
cp ../NIILib/Debug/libNII.a OpenOS;            if (test $? != 0) then exit; fi;
cp ../PCS/Debug/PCS OpenOS;                    if (test $? != 0) then exit; fi;

cp Delivery/* OpenOS;                          if (test $? != 0) then exit; fi;

strip OpenOS/libMOS.so
strip OpenOS/libOpenOS.so
strip OpenOS/libAsaacC.a
strip OpenOS/libNII.a
strip OpenOS/OpenOSEntity
strip OpenOS/PCS

mkdir OpenOS/HelloWorld

cp ../HelloWorldConfigurationApplication/Debug/HelloWorldConfigurationApplication OpenOS/HelloWorld;   if (test $? != 0) then exit; fi;
cp ../HelloWorldConfigurationApplication/HelloWorldConfiguration                  OpenOS/HelloWorld;   if (test $? != 0) then exit; fi;
cp ../SendHelloWorldApplication/Debug/SendHelloWorldApplication                   OpenOS/HelloWorld;   if (test $? != 0) then exit; fi;
cp ../ReceiveHelloWorldApplication/Debug/ReceiveHelloWorldApplication             OpenOS/HelloWorld;   if (test $? != 0) then exit; fi;

strip OpenOS/HelloWorld/HelloWorldConfigurationApplication
strip OpenOS/HelloWorld/SendHelloWorldApplication
strip OpenOS/HelloWorld/ReceiveHelloWorldApplication

tar -cvzf Release.tgz OpenOS
rm -rf OpenOS

ftp -u ftp://upload.sourceforge.net/incoming/OpenOS-Release1.tar.gz Release.tgz

rm Release.tgz