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
    QVERIFY(progressiveState->mSearchHandler);

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

void SearchStateProviderTest::testonSearchComplete()
    {
//    HbMainWindow* wind = mainWindow();
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->onSearchComplete(-1, 0);
    QCOMPARE(progressiveState->mResultcount,0);
    progressiveState->onSearchComplete(0, -1);
    QCOMPARE(progressiveState->mResultcount,0);
    progressiveState->onSearchComplete(0, 10);
    QCOMPARE(progressiveState->mResultcount,10);
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

    progressiveState->onAsyncSearchComplete(-1, 10);
    QCOMPARE(progressiveState->mResultcount,0);

    progressiveState->onAsyncSearchComplete(0, -1);
    QCOMPARE(progressiveState->mResultcount,0);

    progressiveState->onAsyncSearchComplete(0, 10);
    QCOMPARE(progressiveState->mResultcount,10);

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
    progressiveState->onEntry(event);

    progressiveState->onGetDocumentComplete(0, NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);

    progressiveState->onGetDocumentComplete(-1, NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);

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
    progressiveState->onEntry(event);
    progressiveState->mSelectedCategory.insert(1, true);
    progressiveState->startNewSearch("a");
    QTest::qWait(200);
    //code for getting the model index from model
    QModelIndex index = progressiveState->mModel->index(1, 0, QModelIndex());
    progressiveState->openResultitem(index);
    delete progressiveState;
 /*//   delete wind;

       {
        SearchProgressiveState* progressiveState =
                new SearchProgressiveState();
        QEvent *event = new QEvent(QEvent::None);
        progressiveState->onEntry(event);
        progressiveState->mSelectedCategory.insert(3, true);
        progressiveState->startNewSearch("a");
        QTest::qWait(200);
        //code for getting the model index from model
        QModelIndex index = progressiveState->mModel->index(1, 0,
                QModelIndex());
        progressiveState->openResultitem(index);

        //to do
        delete progressiveState;
        }
*/    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testcreateSuggestionLink()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);

    progressiveState->createSuggestionLink(true);
    //    QModelIndex index = progressiveState->mModel->index(0, 0, QModelIndex());
    //  progressiveState->openResultitem(index);

    //progressiveState->createSuggestionLink(false);
    //QCOMPARE(progressiveState->mModel->rowCount(),0);

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
    progressiveState->onEntry(event);
    progressiveState->mSelectedCategory.insert(6, true);
    progressiveState->startNewSearch("sample");
    delete progressiveState;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsetHistory()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    QSignalSpy spy(progressiveState, SIGNAL(historyState()));
  //  progressiveState->setHistory();
    QCOMPARE(spy.count(), 1);
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
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testaboutToCloseView()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    //progressiveState->aboutToCloseView();
    delete progressiveState;
    }

void SearchStateProviderTest::test_CustomizeGoButton()
    {

    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    progressiveState->_customizeGoButton(true);
    progressiveState->_customizeGoButton(false);
    //QEvent *event = new QEvent(QEvent::None);
    // progressiveState->onEntry(event);
    // progressiveState->aboutToCloseView();
    delete progressiveState;

    }

