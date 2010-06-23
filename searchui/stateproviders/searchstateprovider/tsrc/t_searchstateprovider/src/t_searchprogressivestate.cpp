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
    QVERIFY(progressiveState->mTemplist.count());
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


void SearchStateProviderTest::testonAsyncSearchComplete()
    {
    //  HbMainWindow* wind = mainWindow();
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

    progressiveState->mSearchHandler
            = progressiveState->mSearchHandlerList.at(2);
   
    
    progressiveState->mSearchHandler->searchAsync("jpg*", "_aggregate");
    QTest::qWait(2000);
    QVERIFY(progressiveState->mResultcount);

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

    progressiveState->mSelectedCategory.insert(5, true);
    progressiveState->onEntry(event);
    // progressiveState->mSearchHandler = progressiveState->mSearchHandlerList.at(1);
    progressiveState->onGetDocumentComplete(0, NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);

    progressiveState->onGetDocumentComplete(-1, NULL);
    QCOMPARE(progressiveState->mModel->rowCount(),0);
    progressiveState->startNewSearch("contact");
    QTest::qWait(2000);
    int i = progressiveState->mModel->rowCount();
    QVERIFY(progressiveState->mModel->rowCount());
    delete progressiveState;
    //  delete wind;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testopenResultitem()
    {
    //    HbMainWindow* wind = mainWindow();
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);

    progressiveState->getSettingCategory(0, true);
    progressiveState->getSettingCategory(1, true);
    progressiveState->getSettingCategory(2, true);
    progressiveState->getSettingCategory(3, true);
    progressiveState->getSettingCategory(4, true);
    progressiveState->getSettingCategory(5, true);
    progressiveState->getSettingCategory(6, true);
    progressiveState->getSettingCategory(7, true);

    progressiveState->onEntry(event);
    progressiveState->startNewSearch("Creat");
    QTest::qWait(2000);

    //code for getting the model index from model
    /*QModelIndex index = progressiveState->mModel->index(0, 0);
     progressiveState->openResultitem(index);
     QTest::qWait(200);
     progressiveState->startNewSearch("jpg");
     QTest::qWait(200);
     progressiveState->cancelSearch();
     //code for getting the model index from model
     index = progressiveState->mModel->index(0, 0);
     progressiveState->openResultitem(index);
     
     QTest::qWait(200);
     progressiveState->startNewSearch("note");
     QTest::qWait(200);
     progressiveState->cancelSearch();
     //code for getting the model index from model
     index = progressiveState->mModel->index(3, 0);
     progressiveState->openResultitem(index);*/
    delete progressiveState;
    //   delete wind;    
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

    progressiveState->getSettingCategory(0, false);
    progressiveState->getSettingCategory(1, false);
    progressiveState->getSettingCategory(2, false);
    progressiveState->getSettingCategory(4, false);
    progressiveState->getSettingCategory(5, false);
    progressiveState->getSettingCategory(6, false);
    progressiveState->getSettingCategory(7, false);
    progressiveState->getSettingCategory(3, true);
    QCOMPARE(progressiveState->mSelectedCategory.count(),8);
    delete progressiveState;
    //   delete wind;
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
void SearchStateProviderTest::teststartNewSearch()
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
   
    progressiveState->startNewSearch("jpg");
    QTest::qWait(2000);
    QVERIFY(progressiveState->mModel->rowCount());
    delete progressiveState;
    }
void SearchStateProviderTest::test_customizeGoButton()
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
    QCOMPARE(progressiveState->mResultcount, 0);
    delete progressiveState;
    }
// ---------------------------------------------------------------------------
//SearchStateProviderTest::testcancelsearch()
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testcancelSearch()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    progressiveState->getSettingCategory(5, true);
    progressiveState->startNewSearch("d");
    delete progressiveState;

    }

void SearchStateProviderTest::testhandleOkError()
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

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testgetDrivefromMediaId()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->mSelectedCategory.insert(1, false);
    progressiveState->mSelectedCategory.insert(3, false);
    progressiveState->mSelectedCategory.insert(4, false);
    progressiveState->mSelectedCategory.insert(5, false);
    progressiveState->mSelectedCategory.insert(6, false);
    progressiveState->mSelectedCategory.insert(7, false);
    
    progressiveState->mSelectedCategory.insert(2, true);
    progressiveState->onEntry(event);
    progressiveState->startNewSearch("3gpp");
    QTest::qWait(200);
    QModelIndex index = progressiveState->mModel->index(1, 0);
    QStandardItem* item = progressiveState->mModel->itemFromIndex(index);
    if(item)
    QString uid = progressiveState->getDrivefromMediaId(item->data(
            Qt::UserRole + 2).toString());
    //QVERIFY(uid.length());
    delete progressiveState;
    }
void SearchStateProviderTest::testLaunchApplication()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->onEntry(event);
    // progressiveState->LaunchApplicationL(TUid::Uid(0x20011383));
    delete progressiveState;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testfilterDoc()
    {
    SearchProgressiveState* progressiveState = new SearchProgressiveState();
    QEvent *event = new QEvent(QEvent::None);
    progressiveState->mSelectedCategory.insert(5, true);
    progressiveState->onEntry(event);
    progressiveState->startNewSearch("d");
    QTest::qWait(200);
    delete progressiveState;
    }
