#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:  SEARCH state provider project file
#

TEMPLATE = lib
TARGET = searchindevicehandler

   
   LIBS += -lqcpixsearchclient
   LIBS += -lhbcore
   
DEFINES += BUILD_INDEVICEHANDLER 

DEPENDPATH += ./inc \
              ./src

INCLUDEPATH += ./inc \
               ../inc 

               
#TRANSLATIONS=searchstateprovider.ts

symbian {
    TARGET.UID2 = 0x20004C45
    TARGET.UID3 = 0x2002C37B
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = ALL -TCB -DRM
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
    
    
}


include(indevicehandler.pri)

