#include "t_indevicehandler.h"
#include "indevicehandler.h"
#include <qsignalspy.h>
#include <cpixdocument.h>
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
    indevicehandler->setCategory("media");
    QSignalSpy
            spy(indevicehandler, SIGNAL(handleAsyncSearchResult(int, int)));
    indevicehandler->searchAsync("jpg*");
    QTest::qWait(2000);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;

    }
void SearchInDeviceHandlerTest::testhandleDocument()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("media");

    indevicehandler->searchAsync("jpg*");
    QTest::qWait(2000);

    QSignalSpy spy(indevicehandler,
            SIGNAL(handleDocument(int, CpixDocument*)));
    indevicehandler->getDocumentAsyncAtIndex(0);
    QTest::qWait(2000);
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
    indevicehandler->setCategory("media");

    indevicehandler->searchAsync("jpg*");
    QTest::qWait(2000);
    QSignalSpy spy(indevicehandler,
            SIGNAL(handleDocument(int, CpixDocument*)));
    indevicehandler->getDocumentAsyncAtIndex(0);
    QTest::qWait(2000);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;
    }

void SearchInDeviceHandlerTest::testsearchAsync()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("media");
    QSignalSpy spy(indevicehandler, SIGNAL(handleAsyncSearchResult(int,int)));
    indevicehandler->searchAsync(NULL);
    QTest::qWait(2000);
    QCOMPARE(spy.count(), 0);
    indevicehandler->searchAsync("jpg*");
    QTest::qWait(2000);
    QCOMPARE(spy.count(), 1);
    delete indevicehandler;
    }
void SearchInDeviceHandlerTest::testcancelLastSearch()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    indevicehandler->setCategory("media");
    indevicehandler->searchAsync("jpg*");
    indevicehandler->cancelLastSearch();
    delete indevicehandler;
    }
void SearchInDeviceHandlerTest::testgetSearchResultCount()
    {
    InDeviceHandler* indevicehandler = new InDeviceHandler();
    QCOMPARE(indevicehandler->getSearchResultCount(),0);
    indevicehandler->setCategory("media");
    indevicehandler->searchAsync("jpg*");
    QTest::qWait(2000);
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
    QCoreApplication app(argc, argv);
#ifdef __WINSCW__
    char *new_argv[3];
    QString str = "C:\\data\\" + QFileInfo(
            QCoreApplication::applicationFilePath()).baseName() + ".log";
    QByteArray bytes = str.toAscii();
    char arg1[] = "-o";
    new_argv[0] = argv[0];
    new_argv[1] = arg1;
    new_argv[2] = bytes.data();
    SearchInDeviceHandlerTest tc;
    return QTest::qExec(&tc, 3, new_argv);
#else
    SearchInDeviceHandlerTest tc;
    return QTest::qExec(&tc, argc, argv);
#endif

    }
#else //Q_OS_SYMBIAN
QTEST_MAIN(SearchInDeviceHandlerTest)
#endif //Q_OS_SYMBIAN    
    
