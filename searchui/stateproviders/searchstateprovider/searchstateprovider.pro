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
    INCLUDEPATH += \              
              $$PWD/../../../homescreensrv/homescreensrv_plat/ftuwizardmodel_api/inc \
              $$PWD/../../../homescreensrv/homescreensrv_plat/statemodel_api
    LIBS += -L$$PWD/../../../bin/debug
}


LIBS += -lstatemodel
LIBS += -lsearchindevicehandler
LIBS += -lxqservice 
LIBS += -lqcpixsearchclient
LIBS += -laknskins 
LIBS += -lfbscli
LIBS += -laknicon
LIBS += -lapgrfx
LIBS += -lbitgdi


CONFIG += plugin debug_and_release

coverage:DEFINES += COVERAGE_MEASUREMENT

CONFIG += hb

QT += xml

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
    include(searchstateprovider_installs_symbian.pri)

}

symbian:unix:include(searchstateprovider_installs_unix.pri)
include(searchstateprovider.pri)

