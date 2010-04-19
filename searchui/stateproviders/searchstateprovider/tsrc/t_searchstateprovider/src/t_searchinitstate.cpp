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
#include "searchinitstate.h"
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testinitStateConstruction()
    {
    // HbMainWindow* wind = mainWindow();
    SearchInitState* initState = new SearchInitState();

    QVERIFY(initState != NULL);

    delete initState;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testinitStateOnEntryAndExitSignalled()
    {
    // HbMainWindow* wind = mainWindow();

    SearchInitState* initState = new SearchInitState();

    QEvent *event = new QEvent(QEvent::None);
    initState->onEntry(event);

    initState->onExit(event);

    delete initState;
    }
