#include "t_searchstateprovider.h"
#include "searchonlinestate.h"

void SearchStateProviderTest::teststartOnlineSearch()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->startOnlineSearch("query");
    delete onlinestate;
    }
void SearchStateProviderTest::testonlinesetSettings()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->mSearchQuery = "query";
    QSignalSpy spy(onlinestate, SIGNAL(onlineSearchQuery(QString)));
    onlinestate->setSettings();
    QCOMPARE(spy.count(), 1);
    QSignalSpy spy1(onlinestate, SIGNAL(switchOnlineToSettingsState()));
    onlinestate->setSettings();
    QCOMPARE(spy1.count(), 1);
    delete onlinestate;
    }
void SearchStateProviderTest::testonlinecancelSearch()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->cancelSearch();
    delete onlinestate;
    }
void SearchStateProviderTest::testslotbackEventTriggered()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->slotbackEventTriggered();
    delete onlinestate;
    }
void SearchStateProviderTest::testslotIndeviceQuery()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->slotIndeviceQuery("query");
    delete onlinestate;
    }
void SearchStateProviderTest::testslotlaunchLink()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->slotlaunchLink(1,"Query");
    delete onlinestate;
    }
void SearchStateProviderTest::testslotviewReady()
    {
    SearchOnlineState* onlinestate = new SearchOnlineState();
    QEvent *event = new QEvent(QEvent::None);
    onlinestate->onEntry(event);
    onlinestate->slotviewReady();
    delete onlinestate;
    }
