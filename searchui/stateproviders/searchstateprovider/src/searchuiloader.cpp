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
 * Description:  Implementation of the uicontroller.
 *
 */

#include "searchuiloader.h"
#include <hbdocumentloader.h>
#include <hbview.h>
#include <hblistwidget.h>
#include <hbsearchpanel.h>
#include <hbabstractviewitem.h>
#include <hbframebackground.h>
#include <hblistviewitem.h>
#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbinstance.h>
#include <tstasksettings.h>
#include <hbshrinkingvkbhost.h>
#include <qinputcontext.h>

const char *SEARCHSTATEPROVIDER_DOCML = ":/xml/searchstateprovider.docml";
const char *TOC_VIEW = "tocView";
const char *TUT_SEARCHPANEL_WIDGET = "searchPanel";
const char *TUT_LIST_VIEW = "listView";

SearchUiLoader *SearchUiLoader::m_instance = 0;
int SearchUiLoader::m_instanceCounter = 0;

// ---------------------------------------------------------------------------
// SearchUiLoader::SearchUiLoader
// ---------------------------------------------------------------------------
SearchUiLoader::SearchUiLoader() :
    mDocumentLoader(NULL), mView(NULL), mListWidget(NULL),
            mSearchPanel(NULL), mClient(NULL), mMainWindow(NULL)
    {
    bool ok = false;

    mDocumentLoader = new HbDocumentLoader();

    mDocumentLoader->load(SEARCHSTATEPROVIDER_DOCML, &ok);

    QGraphicsWidget *widget = mDocumentLoader->findWidget(TOC_VIEW);

    Q_ASSERT_X(ok && (widget != 0), "TOC_VIEW", "invalid view");

    mView = qobject_cast<HbView*> (widget);

    if (mView)
        {
        mView->setTitle(hbTrId("txt_search_title_search"));
        }

    mListWidget = qobject_cast<HbListWidget *> (mDocumentLoader->findWidget(
            TUT_LIST_VIEW));

    Q_ASSERT_X(ok && (mListWidget != 0), "TUT_LIST_VIEW", "invalid viewocML file");

    if (mListWidget)
        {
        HbAbstractViewItem *prototype = mListWidget->itemPrototypes().first();
        HbFrameBackground frame;
        frame.setFrameGraphicsName("qtg_fr_list_normal");
        frame.setFrameType(HbFrameDrawer::NinePieces);
        prototype->setDefaultFrame(frame);
        HbListViewItem *prototypeListView = qobject_cast<HbListViewItem *> (
                prototype);
        prototypeListView->setGraphicsSize(HbListViewItem::LargeIcon);
        if (prototypeListView)
            {
            prototypeListView->setTextFormat(Qt::RichText);
            }
        HbAbstractItemView::ItemAnimations noCreationAndRemovalAnimations =
                HbAbstractItemView::All;
        noCreationAndRemovalAnimations ^= HbAbstractItemView::Appear;
        noCreationAndRemovalAnimations ^= HbAbstractItemView::Disappear;
        mListWidget->setEnabledAnimations(noCreationAndRemovalAnimations);
        }

    mSearchPanel = qobject_cast<HbSearchPanel *> (
            mDocumentLoader->findWidget(TUT_SEARCHPANEL_WIDGET));
    if (mSearchPanel)
        {
        Qt::InputMethodHints hints = mSearchPanel->inputMethodHints();
        hints |= Qt::ImhNoPredictiveText;
        mSearchPanel->setInputMethodHints(hints);
        mSearchPanel->setSearchOptionsEnabled(true);
        mSearchPanel->setCancelEnabled(false);
        }

    mMainWindow = new SearchMainWindow();
    connect(mMainWindow, SIGNAL(bringvkb()), this, SLOT(slotbringvkb()));

    HbAction *action = new HbAction(Hb::DoneNaviAction);
    connect(action, SIGNAL(triggered()), this, SLOT(slotsendtobackground()));
    mView->setNavigationAction(action);

    mVirtualKeyboard = new HbShrinkingVkbHost(mView);

    mBringtoForground = true;

    }
// ---------------------------------------------------------------------------
// SearchUiLoader::~SearchUiLoader
// ---------------------------------------------------------------------------
SearchUiLoader::~SearchUiLoader()
    {
    delete mMainWindow;
    delete mDocumentLoader;
    delete mClient;
    }
// ---------------------------------------------------------------------------
// SearchUiLoader::slotsendtobackground
// ---------------------------------------------------------------------------
void SearchUiLoader::slotsendtobackground()
    {
    if (!mClient)
        mClient = new TsTaskSettings;
    mClient->setVisibility(false);
    mListWidget->clear();
    mSearchPanel->setCriteria(QString());
    mMainWindow->lower();
    mBringtoForground = true;
    }
// ---------------------------------------------------------------------------
// SearchUiLoader::slotbringvkb
// ---------------------------------------------------------------------------
void SearchUiLoader::slotbringvkb()
    {
    if (mBringtoForground)
        {
        if (!mClient)
            mClient = new TsTaskSettings;
        mClient->setVisibility(true);
        mSearchPanel->setFocus();
        QInputContext *ic = qApp->inputContext();
        if (ic)
            {
            mBringtoForground = false;
            QEvent *event = new QEvent(QEvent::RequestSoftwareInputPanel);
            ic->filterEvent(event);
            delete event;
            }
        }
    }

// ---------------------------------------------------------------------------
// SearchMainWindow::slotViewReady
// ---------------------------------------------------------------------------
void SearchMainWindow::slotViewReady()
    {
    emit bringvkb();
    }
