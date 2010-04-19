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
* Description: 
*
*/
#include "t_searchstateprovider.h"
#include "searchsettingsstate.h"

// ---------------------------------------------------------------------------
//SearchStateProviderTest::testsettingsStateConstruction()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsettingsStateConstruction()
    {
    // HbMainWindow* wind = mainWindow();
    SearchSettingsState* settingsState = new SearchSettingsState();

    QVERIFY(settingsState != NULL);
    QVERIFY(settingsState->mWidget);

    delete settingsState;

    }
// ---------------------------------------------------------------------------
//SearchStateProviderTest::testsettingsStateOnEntryAndExitSignalled()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsettingsStateOnEntryAndExitSignalled()
    {
    // HbMainWindow* wind = mainWindow();

    SearchSettingsState* settingsState = new SearchSettingsState();

    QEvent *event = new QEvent(QEvent::None);
    settingsState->onEntry(event);

    settingsState->onExit(event);

    delete settingsState;
    }

// ---------------------------------------------------------------------------
//SearchStateProviderTest::testhandleBackEvent()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testhandleBackEvent()
    {
    SearchSettingsState* settingsState = new SearchSettingsState();

    QSignalSpy spy(settingsState, SIGNAL(backEventTriggered()));
    settingsState->handleBackEvent(true);
    QCOMPARE(spy.count(), 1);

    delete settingsState;
    }
// ---------------------------------------------------------------------------
//SearchStateProviderTest::test_selectedcategory()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testgetItemCategory()
    {
    SearchSettingsState* settingsState = new SearchSettingsState();

    QSignalSpy spy(settingsState,
            SIGNAL(publishSelectedCategory(int, bool)));
    settingsState->getItemCategory(1, true);
    settingsState->getItemCategory(1, false);
    settingsState->getItemCategory(6, true);
    settingsState->getItemCategory(6, true);
    QCOMPARE(spy.count(), 4);
    delete settingsState;
    }

// ---------------------------------------------------------------------------
//SearchStateProviderTest::testisInternetOn()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testisInternetOn()
    {
    SearchSettingsState* settingsState = new SearchSettingsState();
    QSignalSpy spy(settingsState,
                SIGNAL(customizeGoButton(bool)));
    
    settingsState->isInternetOn();    
    
    QCOMPARE(spy.count(), 1);
    delete settingsState;
    }
