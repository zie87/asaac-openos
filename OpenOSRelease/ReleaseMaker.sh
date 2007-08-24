#!/bin/bash

rm -rf OpenOS
mkdir OpenOS

cp ../ASAAC\ IF/ASAAC.h OpenOS;                if (test $? != 0) then exit; fi;
cp ../ASAAC\ IF/ASAAC_IMPL.h OpenOS;           if (test $? != 0) then exit; fi;
cp ../ASAAC\ IF/ASAAC_ENH.h OpenOS;            if (test $? != 0) then exit; fi;

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

tar -cvzf Release.tgz OpenOS
rm -rf OpenOS

ftp -u ftp://upload.sourceforge.net/incoming/OpenOS-Beta2.tar.gz Release.tgz

rm Release.tgz