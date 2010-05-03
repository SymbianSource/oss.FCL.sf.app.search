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
#include "searchprogressivestate.h"
#include <qsignalspy.h>
#include "indevicehandler.h"
#include <qstandarditemmodel.h>
#include <fbs.h>
#include <AknsUtils.h>
#include <bitdev.h> 
#include <bitstd.h>
#include <qbitmap.h>
#include <fbs.h>
#include <AknInternalIconUtils.h>
#include <AknIconUtils.h> 
#include <apgcli.h>
#include <hbsearchpanel.h>

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//

void SearchStateProviderTest::testProgressiveStateConstruction()
    {
    // HbMainWindow* wind = mainWindow();

    SearchProgressiveState* progressiveState = new SearchProgressiveState();

    QVERIFY(progressiveState != NULL);
    QVERIFY(progressiveState->mView);
    QVERIFY(progressiveState->mListView);
    QVERIFY(progressiveState->mDocumentLoader);
    QVERIFY(progressiveState->mSearchPanel);
    QVERIFY(progressiveState->mModel);
    //QVERIFY(progressiveState->mSearchHandler);

    delete progressiveState;

    //  delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//

void SearchStateProviderTest::testProgressiveStateOnEntryAndExitSignalled()
    {
    //    HbMainWindow* wind = mainWindow();

    SearchProgressiveState* progressiveState = new SearchProgressiveState();

    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);

    progressiveState->onExit(event);

    delete progressiveState;

    //  delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//


void SearchStateProviderTest::testonAsyncSearchComplete()
    {
    //  HbMainWindow* wind = mainWindow();
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->mSearchHandler
            = progressiveState->mSearchHandlerList.at(1);
    progressiveState->onAsyncSearchComplete(-1, 10);
    QCOMPARE(progressiveState->mResultcount,0);

    progressiveState->onAsyncSearchComplete(0, -1);
    QCOMPARE(progressiveState->mResultcount,0);
    progressiveState->mSearchHandler->searchAsync("a", "_aggregate");
    QTest::qWait(100);
    //QCOMPARE(progressiveState->mResultcount,10);

    delete progressiveState;
    //  delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testonGetDocumentComplete()
    {
    //    HbMainWindow* wind = mainWindow();
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);

    progressiveState->mSelectedCategory.insert(1, true);
    progressiveState->onEntry(event);
    // progressiveState->mSearchHandler = progressiveState->mSearchHandlerList.at(1);
    progressiveState->onGetDocumentComplete(0, NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);

    progressiveState->onGetDocumentComplete(-1, NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);
    progressiveState->startNewSearch("a");
    QTest::qWait(2000);
    int i =progressiveState->mModel->rowCount();
    QVERIFY(progressiveState->mModel->rowCount());
    delete progressiveState;
    //  delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testgetSettingCategory()
    {
    //    HbMainWindow* wind = mainWindow();
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);

    progressiveState->getSettingCategory(3, true);
    QCOMPARE(progressiveState->mSelectedCategory.count(),1);
    delete progressiveState;
    //   delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testopenResultitem()
    {
    //    HbMainWindow* wind = mainWindow();
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);

    progressiveState->mSelectedCategory.insert(1, true);
    progressiveState->onEntry(event);
    progressiveState->startNewSearch("a");
    QTest::qWait(100);
    progressiveState->cancelSearch();
    //code for getting the model index from model
    QModelIndex index = progressiveState->mModel->index(0, 0);
    progressiveState->openResultitem(index);
    delete progressiveState;
    //   delete wind;    
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testcreateSuggestionLink()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);

    progressiveState->createSuggestionLink(true);
    QCOMPARE(progressiveState->mModel->rowCount(), 0);
    progressiveState->createSuggestionLink(false);
    QCOMPARE(progressiveState->mModel->rowCount(), 1);
    delete progressiveState;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testnoResultsFound()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);

    progressiveState->noResultsFound(NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);

    progressiveState->noResultsFound("aaa");
    QCOMPARE(progressiveState->mModel->rowCount(),1);

    delete progressiveState;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testclear()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);

    progressiveState->noResultsFound("aaa");
    QCOMPARE(progressiveState->mModel->rowCount(),1);
    progressiveState->clear();
    QCOMPARE(progressiveState->mModel->rowCount(),0);

    delete progressiveState;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsearchOnCategory()
    {

    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::teststartNewSearch()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->mSelectedCategory.insert(1, true);
    progressiveState->onEntry(event);
    progressiveState->startNewSearch("a");
    delete progressiveState;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsetSettings()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    QSignalSpy spy(progressiveState, SIGNAL(settingsState()));
    progressiveState->setSettings();
    QCOMPARE(spy.count(), 1);
    delete progressiveState;
    }

void SearchStateProviderTest::test_CustomizeGoButton()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->_customizeGoButton(true);
    QVERIFY(!(progressiveState->mSearchPanel->isProgressive()));
    progressiveState->_customizeGoButton(false);
    QVERIFY(progressiveState->mSearchPanel->isProgressive());
    delete progressiveState;
    }

void SearchStateProviderTest::testgetAppIconFromAppId()
    {

    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    for (int i = 0; i < progressiveState->mIconArray.count(); i++)
        {
        QVERIFY(!(progressiveState->mIconArray.at(i).isNull()));
        }
    progressiveState->mIconArray.clear();
    delete progressiveState;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsettingsaction()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->getSettingCategory(5, true);
    progressiveState->mOriginalString = "a";
    progressiveState->settingsaction(false);
    QCOMPARE(progressiveState->mResultcount, 0);
    progressiveState->settingsaction(true);
    QTest::qWait(50);
    progressiveState->cancelSearch();
    QCOMPARE(progressiveState->mResultcount, 0);
    delete progressiveState;
    }
// ---------------------------------------------------------------------------
//SearchStateProviderTest::testcancelsearch()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testcancelsearch()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->getSettingCategory(5, true);
    progressiveState->startNewSearch("a");
    progressiveState->cancelSearch();
    delete progressiveState;

    }
void SearchStateProviderTest::testhandleokanderror()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    QVariant var;
    progressiveState->handleOk(var);
    int ret = 0;
    progressiveState->handleError(ret, QString());
    delete progressiveState;
    }
void SearchStateProviderTest::testTDisplayMode2Format()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    TDisplayMode mode = EGray2;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor256;
    progressiveState->TDisplayMode2Format(mode);
    mode = EGray256;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor4K;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor64K;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor16M;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor16MU;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor16MA;
    progressiveState->TDisplayMode2Format(mode);
    mode = EColor16MAP;
    progressiveState->TDisplayMode2Format(mode);
    mode = ENone;
    progressiveState->TDisplayMode2Format(mode);

    delete progressiveState;
    }
void SearchStateProviderTest::testsetSelectedCategories()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->getSettingCategory(0, true);
    progressiveState->getSettingCategory(1, true);
    progressiveState->getSettingCategory(2, true);
    progressiveState->getSettingCategory(3, true);
    progressiveState->getSettingCategory(4, true);
    progressiveState->getSettingCategory(5, true);
    progressiveState->getSettingCategory(6, true);
    progressiveState->getSettingCategory(7, true);
    progressiveState->setSelectedCategories();
    delete progressiveState;
    }

void SearchStateProviderTest::testfilterdoc()
    {
    
        SearchProgressiveState* progressiveState = new SearchProgressiveState();
        QEvent *event = new QEvent(QEvent::None);
        progressiveState->onEntry(event);
        progressiveState->filterDoc(NULL,"Name");
        delete progressiveState;  
    
    }
