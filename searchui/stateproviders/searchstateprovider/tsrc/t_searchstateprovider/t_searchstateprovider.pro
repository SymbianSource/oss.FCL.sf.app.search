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
# Description:  Search stateprovider unit test project file
#

TEMPLATE = app


LIBS += -lstatemodel
#LIBS += -searchstateprovider.dll
#LIBS += -searchruntimeprovider.dll
LIBS += -lsearchindevicehandler
LIBS += -lxqservice 
LIBS += -lqcpixsearchclient
LIBS += -laknskins 
LIBS += -lfbscli
LIBS += -laknicon
LIBS += -lapgrfx
LIBS += -lbitgdi
    
CONFIG += qtestlib 
CONFIG += hb

QT += xml \
      sql
DEFINES += SEARCH_UNITTESTING

DEPENDPATH += .\
              ./src \
              ./inc \
              ../../src \
              ../../inc

INCLUDEPATH += .\
               ./inc \
               ../../inc \
               ../../../../inc \
               ../../../../indevicehandler/inc \
	       ../../../../runtimeproviders/searchruntimeprovider/inc
symbian {         
CONFIG += symbian_test               
TARGET.UID2 = 0x100039CE
TARGET.UID3 = 0x20026F9A
TARGET.CAPABILITY = CAP_APPLICATION AllFiles
TARGET.EPOCSTACKSIZE = 0x14000 // 80kB
TARGET.EPOCHEAPSIZE = 0x20000 0x1000000 // 128kB - 16MB
INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE 
}

include(t_searchstateprovider.pri)

