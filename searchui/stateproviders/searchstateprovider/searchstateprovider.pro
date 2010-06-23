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

DEFINES += BUILD_SEARCHSTATEPROVIDER

symbian {
    CONFIG(debug, debug|release) {
      DESTDIR = debug      
    } else {
      DESTDIR = release
    }
}

win32 {
    CONFIG(debug, debug|release) {
      SUBDIRPART = debug
    } else {
      SUBDIRPART = release
    }
    DESTDIR = $$PWD/../../../bin/$$SUBDIRPART/searchresources/plugins/stateproviders
    LIBS += -L$$PWD/../../../bin/debug
}

LIBS += -lsearchindevicehandler
LIBS += -lxqservice 
LIBS += -lqcpixsearchclient
LIBS += -laknskins 
LIBS += -lfbscli
LIBS += -laknicon
LIBS += -lapgrfx
LIBS += -lbitgdi
#LIBS += -lnoteseditor
LIBS += -lxqutils
LIBS += -lapparc
LIBS += -lefsrv
LIBS += -lapgrfx
LIBS += -lws32
LIBS += -lavkon

coverage:DEFINES += COVERAGE_MEASUREMENT

CONFIG += hb

QT += sql xml


nft:DEFINES += NFT

DEPENDPATH += ./inc \
              ./src

INCLUDEPATH += ./inc \
               ../../inc \
               ../../indevicehandler/inc
               


symbian {
    TARGET.UID2 = 0x20004C45
    TARGET.UID3 = 0x2002C37A
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = ALL -TCB -DRM
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE 
    INCLUDEPATH += $$APP_LAYER_SYSTEMINCLUDE
    
    defBlock = \      
	"$${LITERAL_HASH}if defined(EABI)" 	\
	"DEFFILE  ../eabi/" 				\
    "$${LITERAL_HASH}else" 				\
    "DEFFILE  ../bwins/" 				\
    "$${LITERAL_HASH}endif"
	
	MMP_RULES += defBlock
    #include(searchstateprovider_installs_symbian.pri)
   # BLD_INF_RULES.prj_exports += "./inc/searchstateprovider.h |../../inc/"

}
win32:include(searchstateprovider_installs_win32.pri)
symbian:unix:include(searchstateprovider_installs_unix.pri)
include(searchstateprovider.pri)
