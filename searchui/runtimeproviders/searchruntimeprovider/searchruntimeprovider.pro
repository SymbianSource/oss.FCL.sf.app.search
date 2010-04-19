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
# Description:  SEARCH default runtime provider project file
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
    DESTDIR = $$PWD/../../../bin/$$SUBDIRPART/searchresources/plugins/runtimeproviders
    INCLUDEPATH += \
              $$PWD/../../../homescreensrv/homescreensrv_plat/appruntimemodel_api \
              $$PWD/../../../homescreensrv/homescreensrv_plat/statemodel_api
    LIBS += -L$$PWD/../../../bin/debug
}

LIBS +=  -lappruntimemodel
LIBS +=  -lstatemodel

    
CONFIG += plugin debug_and_release

CONFIG += hb

QT += xml sql

DEFINES += SEARCHRUNTIMEPROVIDER_LIB

coverage:DEFINES += COVERAGE_MEASUREMENT

DEPENDPATH +=   ./inc \
                ./src

INCLUDEPATH += ./inc \
               ../../inc \

symbian {
    TARGET.UID3 = 0x2002C37D
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = ALL -TCB
    appkey:DEFINES += S60APP_KEY
    INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE

    include(searchruntimeprovider_installs_symbian.pri)
}

win32 {
include(searchruntimeprovider_installs_win32.pri)
}

symbian:unix:include(searchruntimeprovider_installs_unix.pri)

include(searchruntimeprovider.pri)
