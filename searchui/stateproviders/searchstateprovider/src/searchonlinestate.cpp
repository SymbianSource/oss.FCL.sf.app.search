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
 * Description:  Implements the wizard activation and interactions.
 *
 */

#include "searchonlinestate.h"
#include "onlinehandler.h"
#include <hbdocumentloader.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbstackedwidget.h>
#include <hbicon.h>
#include <hbinstance.h>
#include <hbaction.h>
#include <qstandarditemmodel.h>
#include <qdebug.h>
#include <qgraphicswidget.h>
#include <qdir.h>
#include <hbsearchpanel.h>
#include <hbapplication.h>
#define hbApp qobject_cast<HbApplication*>(qApp)
const char *SEARCHONLINESTATE_DOCML = ":/xml/searchstateprovider.docml";
const char *ONLINE_TOC_VIEW = "tocView";
const char *ONLINE_TUT_SEARCHPANEL_WIDGET = "searchPanel";

// ---------------------------------------------------------------------------
// SearchOnlineState::SearchOnlineState
// ---------------------------------------------------------------------------
//
SearchOnlineState::SearchOnlineState(QState *parent) :
    QState(parent), mMainWindow(NULL), mView(NULL), mSearchPanel(NULL),
            mDocumentLoader(NULL)
    {

    mInternetHandler = new OnlineHandler();
    mSearchReady = true;
    mIndeviceQueryAvailable = false;
    mIsUICreated = false;
    //activateSignals();
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::~SearchOnlineState
// ---------------------------------------------------------------------------
//
SearchOnlineState::~SearchOnlineState()
    {
    delete mDocumentLoader;
    delete mInternetHandler;
    }
void SearchOnlineState::createui()
    {
    mMainWindow = hbInstance->allMainWindows().at(0);
    mDocumentLoader = new HbDocumentLoader();
    bool ok = false;
    mDocumentLoader->load(SEARCHONLINESTATE_DOCML, &ok);

    QGraphicsWidget *widget = mDocumentLoader->findWidget(ONLINE_TOC_VIEW);
    Q_ASSERT_X(ok && (widget != 0), "ONLINE_TOC_VIEW", "invalid view");

    mView = qobject_cast<HbView*> (widget);
    if (mView)
        {
        mView->setTitle(hbTrId("txt_search_title_search"));
        }

    mSearchPanel = qobject_cast<HbSearchPanel *> (
            mDocumentLoader->findWidget(ONLINE_TUT_SEARCHPANEL_WIDGET));
    if (mSearchPanel)
        {
        mSearchPanel->setSearchOptionsEnabled(true);

        mSearchPanel->setProgressive(false);

        mSearchPanel->setPlaceholderText(hbTrId(
                "txt_search_dialog_search_internet"));

        mSearchPanel->setCancelEnabled(false);          
        }
    if (mView && mMainWindow)
        {
        mMainWindow->addView(mView);
        mMainWindow->setCurrentView(mView);
        }
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::onEntry
// ---------------------------------------------------------------------------
//
void SearchOnlineState::onEntry(QEvent *event)
    {
    qDebug() << "search:SearchOnlineState::onEntry";
    QState::onEntry(event);
    if (!mIsUICreated)
        {
        createui();
        mIsUICreated = true;
        }

    activateSignals();
    // If this is not the current view, we're getting back from plugin view  
    if (mMainWindow)
        {
        if (mMainWindow->currentView() != mView)
            {
            mMainWindow->setCurrentView(mView, true);
            }
        mMainWindow->show();
        }
    if (mIndeviceQueryAvailable)
        {
        mSearchPanel->setCriteria(mSearchQuery);
        mIndeviceQueryAvailable = false;
        }

    }
// ---------------------------------------------------------------------------
// SearchOnlineState::onExit
// ---------------------------------------------------------------------------
//
void SearchOnlineState::onExit(QEvent *event)
    {
    QState::onExit(event);
    deActivateSignals();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::activateSignals
// ---------------------------------------------------------------------------
void SearchOnlineState::activateSignals()
    {
    if (mSearchPanel)
        {
        connect(mSearchPanel, SIGNAL(criteriaChanged(QString)), this,
                SLOT(startOnlineSearch(QString)));
        connect(mSearchPanel, SIGNAL(searchOptionsClicked()), this,
                SLOT(setSettings()));
        connect(mSearchPanel, SIGNAL(exitClicked()), this,
                SLOT(cancelSearch()));
        }
    if (mInternetHandler)
        {

        connect(mInternetHandler, SIGNAL(pluginIntialized(bool)), this,
                SLOT(slotpluginIntialized(bool)));
        connect(mInternetHandler, SIGNAL(pluginShutdown(bool)), this,
                SLOT(slotpluginShutdown(bool)));
        connect(mInternetHandler, SIGNAL(backEventTriggered()), this,
                SLOT(slotbackEventTriggered()));
        }
    connect(mMainWindow, SIGNAL(viewReady()), this, SLOT(slotviewReady()));
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::deActivateSignals
// ---------------------------------------------------------------------------
void SearchOnlineState::deActivateSignals()
    {
    if (mSearchPanel)
        {
        disconnect(mSearchPanel, SIGNAL(criteriaChanged(QString)), this,
                SLOT(startOnlineSearch(QString)));
        disconnect(mSearchPanel, SIGNAL(searchOptionsClicked()), this,
                SLOT(setSettings()));
        disconnect(mSearchPanel, SIGNAL(exitClicked()), this,
                SLOT(cancelSearch()));
        }
    if (mInternetHandler)
        {
        disconnect(mInternetHandler, SIGNAL(pluginIntialized(bool)), this,
                SLOT(slotpluginIntialized(bool)));
        disconnect(mInternetHandler, SIGNAL(pluginShutdown(bool)), this,
                SLOT(slotpluginShutdown(bool)));
        disconnect(mInternetHandler, SIGNAL(backEventTriggered()), this,
                SLOT(slotbackEventTriggered()));
        }
    disconnect(mMainWindow, SIGNAL(viewReady()), this, SLOT(slotviewReady()));
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::startOnlineSearch
// ---------------------------------------------------------------------------
void SearchOnlineState::startOnlineSearch(const QString &query)
    {
    mSearchQuery = query;
    //    if (mSearchReady)
        {
        mInternetHandler->readSettings();
        mInternetHandler->initializePlugin();
        }
    // load online handlers
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::setSettings
// ---------------------------------------------------------------------------
void SearchOnlineState::setSettings()
    {
    if (mSearchQuery.length())
        emit onlineSearchQuery(mSearchQuery);
    emit switchOnlineToSettingsState();
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::cancelSearch
// ---------------------------------------------------------------------------
void SearchOnlineState::cancelSearch()
    {
    // unload handlers
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::slotpluginIntialized
// ---------------------------------------------------------------------------
void SearchOnlineState::slotpluginIntialized(bool status)
    {
    if (status)
        mInternetHandler->activatePlugin(mSearchQuery);
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::slotpluginShutdown
// ---------------------------------------------------------------------------
void SearchOnlineState::slotpluginShutdown(bool status)
    {
    mSearchReady = status;
    if (!mSearchReady)
        mInternetHandler->unloadPluginandExit();
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::slotbackEventTriggered
// ---------------------------------------------------------------------------
void SearchOnlineState::slotbackEventTriggered()
    {
    mInternetHandler->unloadPluginandExit();
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::slotIndeviceQuery
// ---------------------------------------------------------------------------
void SearchOnlineState::slotIndeviceQuery(QString str)
    {
    mSearchQuery = str;
    mIndeviceQueryAvailable = true;
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::slotlaunchLink
// ---------------------------------------------------------------------------
void SearchOnlineState::slotlaunchLink(int id, QString query)
    {
    mInternetHandler->loadISPlugin(id, query);
    }
// ---------------------------------------------------------------------------
// SearchOnlineState::slotviewReady
// ---------------------------------------------------------------------------
void SearchOnlineState::slotviewReady()
    {
    if (hbApp)
        {
        if (hbApp->activateReason() == Hb::ActivationReasonActivity)
            {
            QVariantHash params = hbApp->activateParams();
            QString searchKey = params.value("query").toString();
            if (searchKey.length() > 0)
                {
                mSearchPanel->setCriteria(searchKey);
                startOnlineSearch(searchKey);
                }
            }
        }//PERF_APP_LAUNCH_END("SearchAppplication View is ready");
    }
