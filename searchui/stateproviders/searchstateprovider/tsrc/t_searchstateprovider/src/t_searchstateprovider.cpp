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
 * Description:  Main test class for searchstateprovider library.
 *
 */

#include "t_searchstateprovider.h"
//#include "searchruntimeprovider.h"
#include "searchstateprovider.h"

#include <hsstatefactory.h>

#include <qmetatype.h>
#include <qstate.h>

#include <hbinstance.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbgraphicsscene.h>

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//

SearchStateProviderTest::SearchStateProviderTest()
    {

    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
SearchStateProviderTest::~SearchStateProviderTest()
    {
    delete mMainWindow;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::init()
    {
    
   // delete mMainWindow;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::initTestCase()
    {
    mMainWindow = mainWindow();
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::cleanupTestCase()
    {
    
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testCreateAndDestructSearchStateProvider()
    {
    SearchStateProvider provider;

 /*   QVERIFY(provider.mWizardMenuStateToken.mLibrary == "searchstateprovider.dll");
    QVERIFY(provider.mWizardMenuStateToken.mUri == "search.nokia.com/state/wizardprogressivestate");

    QVERIFY(provider.mWizardActivatedStateToken.mLibrary == "searchstateprovider.dll");
    QVERIFY(provider.mWizardActivatedStateToken.mUri == "search.nokia.com/state/wizardsettingstate");

    QVERIFY(provider.mInitStateToken.mLibrary == "searchstateprovider.dll");
    QVERIFY(provider.mInitStateToken.mUri == "search.nokia.com/state/initstate");
 */   }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testStates()
    {
    /*  SearchStateProvider provider;
   QList<HsStateToken> states = provider.states();

    QCOMPARE(states.count(), 3);

    QVERIFY(findStateInStateList(states, "searchstateprovider.dll","search.nokia.com/state/wizardprogressivestate"));
    QVERIFY(findStateInStateList(states, "searchstateprovider.dll","search.nokia.com/state/wizardsettingstate"));
    QVERIFY(findStateInStateList(states, "searchstateprovider.dll","search.nokia.com/state/initstate"));
   */ }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testCreateProgressiveState()
    {
    
    QVERIFY(verifyStateCreation("search.nokia.com/state/wizardprogressivestate"));
   // delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testCreateSettingsState()
    {
    //HbMainWindow* wind = mainWindow();   
    QVERIFY(verifyStateCreation("search.nokia.com/state/wizardsettingstate"));
   // delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testCreateinitState()
    {
    //HbMainWindow* wind = mainWindow();   
    QVERIFY(verifyStateCreation("search.nokia.com/state/initstate"));
  //  delete wind;
    }


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
bool SearchStateProviderTest::verifyStateCreation(const QString& uri)
    {
    SearchStateProvider provider;   
    QState* state = provider.createState(uri);
    bool result = (state != NULL);
    if (state)
        {
        delete state;
        }
    return result;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
bool SearchStateProviderTest::findStateInStateList(
        const QList<HsStateToken>& states, const QString& library,
        const QString& uri)
    {
    foreach(HsStateToken token, states)
            {
            if (token.mLibrary == library && token.mUri == uri)
                {
                return true;
                }
            }
    return false;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
HbMainWindow* SearchStateProviderTest::mainWindow()
    {
    int count = HbInstance::instance()->allMainWindows().count();

    for (int i = count - 1; i > -1; i--)
        {
        delete HbInstance::instance()->allMainWindows().at(i);
        }
    return new HbMainWindow;
    }

#ifdef Q_OS_SYMBIAN

//QTEST_MAIN corrected since crashes if TRAP not in correct place.
//Will be corrected in later (estimate 4.6.0) Qt release for Symbian.
int main(int argc, char *argv[])
    {
    QApplication app(argc, argv);
    int error;
    TRAPD(err,
            QTEST_DISABLE_KEYPAD_NAVIGATION
            SearchStateProviderTest tc;
            error = QTest::qExec(&tc, argc, argv););
    return error;
    }
#else //Q_OS_SYMBIAN
QTEST_MAIN(SearchStateProviderTest)
#endif //Q_OS_SYMBIAN
