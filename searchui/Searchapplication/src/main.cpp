/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  Search application main implementation.
 *
 */

#include "search.h"
#include "search_global.h"

#include <QtGui>
#include <hbapplication.h>
#include <qtranslator.h>
#include <qlocale.h>

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
//
int main(int argc, char *argv[])
    {
    SEARCH_FUNC_ENTRY("SEARCH::Search::main");

    HbApplication app(argc, argv);

    QTranslator translator;
    QString lang = QLocale::system().name();
    QString path = "Z:/resource/qt/translations/";
    translator.load("searchsw_" + lang, path);
    app.installTranslator(&translator);

    QTranslator commonTranslator;
    commonTranslator.load("common_" + lang, path);
    app.installTranslator(&commonTranslator);

    QTranslator musicTranslator;
    musicTranslator.load("musicplayer_" + lang, path);
    app.installTranslator(&musicTranslator);

    QTranslator contactsTranslator;
    contactsTranslator.load("contacts_" + lang, path);
    app.installTranslator(&contactsTranslator);

    QTranslator calenderTranslator;
    calenderTranslator.load("calendar_" + lang, path);
    app.installTranslator(&calenderTranslator);

    QTranslator notesTranslator;
    notesTranslator.load("notes_" + lang, path);
    app.installTranslator(&notesTranslator);

    Search search;
#ifdef ROM
    QDir::setCurrent("Z:/");
#else
    QDir::setCurrent("C:/");
    SEARCHDEBUG("main() - SearchApplication's current dir set to C:/");
#endif //ROM
    QObject::connect(&app, SIGNAL(aboutToQuit()), &search, SLOT(stop()));
    QObject::connect(&search, SIGNAL(exit()), &app, SLOT(quit()),
            Qt::QueuedConnection);
    search.start();
    int ret = app.exec();
    SEARCH_FUNC_EXIT("SEARCH::Search::main");
    return ret;
    }
