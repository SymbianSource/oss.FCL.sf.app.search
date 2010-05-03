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
#include "t_indevicehandler.h"
#include "indevicehandler.h"
#include <qsignalspy.h>
#include <qcpixdocument.h>
SearchInDeviceHandlerTest::SearchInDeviceHandlerTest()
    {
    }
SearchInDeviceHandlerTest::~SearchInDeviceHandlerTest()
    {
    }
void SearchInDeviceHandlerTest::init()
    {
    }
void SearchInDeviceHandlerTest::initTestCase()
    {
    }
void SearchInDeviceHandlerTest::cleanupTestCase()
    {
    }

void SearchInDeviceHandlerTest::testCreateAndDestructIndeviceHandler()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();

    QVERIFY(indevicehandler != NULL);

    delete indevicehandler;
    }

void SearchInDeviceHandlerTest::testhandleAsyncSearchResult()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");
    QSignalSpy
            spy(indevicehandler, SIGNAL(handleAsyncSearchResult(int, int)));
    indevicehandler->searchAsync("txt");
    QTest::qWait(200);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;

    }
void SearchInDeviceHandlerTest::testhandleDocument()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");

    indevicehandler->searchAsync("a");
    QTest::qWait(200);

    QSignalSpy spy(indevicehandler,
            SIGNAL(handleDocument(int, QCPixDocument*)));
    indevicehandler->getDocumentAsyncAtIndex(0);
    QTest::qWait(200);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;

    }
void SearchInDeviceHandlerTest::testgetSearchResult()
    {
    testhandleAsyncSearchResult();
    }
void SearchInDeviceHandlerTest::testgetDocumentAsync()
    {
    testhandleDocument();
    }

void SearchInDeviceHandlerTest::testgetDocumentAsyncAtIndex()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");

    indevicehandler->searchAsync("txt");
    QTest::qWait(200);
    QSignalSpy spy(indevicehandler,
            SIGNAL(handleDocument(int, QCPixDocument*)));
    indevicehandler->getDocumentAsyncAtIndex(0);
    QTest::qWait(200);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;
    }

void SearchInDeviceHandlerTest::testsearchAsync()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");
    QSignalSpy spy(indevicehandler, SIGNAL(handleAsyncSearchResult(int,int)));
    indevicehandler->searchAsync(NULL);
    QTest::qWait(200);
    QCOMPARE(spy.count(), 0);
    indevicehandler->searchAsync("txt");
    QTest::qWait(200);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;
    }
void SearchInDeviceHandlerTest::testcancelLastSearch()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");
    indevicehandler->searchAsync("txt");
    indevicehandler->cancelLastSearch();
    delete indevicehandler;
    }
void SearchInDeviceHandlerTest::testgetSearchResultCount()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    QCOMPARE(indevicehandler->getSearchResultCount(),0);
    indevicehandler->setCategory("file");
    indevicehandler->searchAsync("txt");
    QTest::qWait(200);
    QVERIFY(indevicehandler->getSearchResultCount());
    delete indevicehandler;

    }
void SearchInDeviceHandlerTest::testsetCategory()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");
    QVERIFY(indevicehandler->mSearchInterface);
    indevicehandler->setCategory("");
    QVERIFY(indevicehandler->mSearchInterface);
    delete indevicehandler;
    }
void SearchInDeviceHandlerTest::testisPrepared()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("file");
    QVERIFY(indevicehandler->isPrepared());
    indevicehandler->setCategory("");
    QVERIFY(indevicehandler->isPrepared());
    delete indevicehandler;

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
            SearchInDeviceHandlerTest tc;
            error = QTest::qExec(&tc, argc, argv););
    return error;
    }
#else //Q_OS_SYMBIAN
QTEST_MAIN(SearchInDeviceHandlerTest)
#endif //Q_OS_SYMBIAN    
    
