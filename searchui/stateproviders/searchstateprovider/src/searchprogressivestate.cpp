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
 * Description:  Implementation of the progressive search state.
 *
 */
#include "searchprogressivestate.h"
#include "indevicehandler.h"
#include <qcpixdocument.h>
#include <qcpixdocumentfield.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hbmenu.h>
#include <hbinstance.h>
#include <hblistview.h>
#include <hbdocumentloader.h>
#include <hbsearchpanel.h>
#include <hbaction.h>
#include <qstandarditemmodel.h>
#include <hbframebackground.h>
#include <hbabstractviewitem.h>
#include <hbframebackground.h>
#include <hblistviewitem.h>
#include <hbapplication.h>
#include <qsortfilterproxymodel.h>
#include <AknsUtils.h>
#include <bitdev.h> 
#include <bitstd.h>
#include <qbitmap.h>
#include <fbs.h>
#include <AknInternalIconUtils.h>
#include <AknIconUtils.h> 
#include <apgcli.h>
#include <qpluginloader.h>
#include <eventviewerplugininterface.h>
#include <noteseditor.h>
#include <w32std.h>
#include <apgtask.h>
#include <apgcli.h>
#include <AknTaskList.h>
#include <apacmdln.h>
const char *SEARCHSTATEPROVIDER_DOCML = ":/xml/searchstateprovider.docml";
const char *TOC_VIEW = "tocView";
const char *TUT_SEARCHPANEL_WIDGET = "searchPanel";
const char *TUT_LIST_VIEW = "listView";
const int totalcategories = 10;
// ---------------------------------------------------------------------------
// SearchProgressiveState::SearchProgressiveState
// ---------------------------------------------------------------------------
SearchProgressiveState::SearchProgressiveState(QState *parent) :
    QState(parent), mMainWindow(NULL), mView(NULL), mListView(NULL),
            mDocumentLoader(NULL), mModel(NULL), mSearchHandler(NULL),
            notesEditor(0), mAiwMgr(0), mRequest(0)
    {

    mMainWindow = hbInstance->allMainWindows().at(0);
    mModel = new QStandardItemModel(this);

    mAiwMgr = new XQApplicationManager;

    mDocumentLoader = new HbDocumentLoader();
    bool ok = false;
    mDocumentLoader->load(SEARCHSTATEPROVIDER_DOCML, &ok);

    QGraphicsWidget *widget = mDocumentLoader->findWidget(TOC_VIEW);
    Q_ASSERT_X(ok && (widget != 0), "TOC_VIEW", "invalid view");

    mView = qobject_cast<HbView*> (widget);
    if (mView)
        {
        mView->setTitle(hbTrId("txt_search_title_search"));
        }

    mListView = qobject_cast<HbListView *> (mDocumentLoader->findWidget(
            TUT_LIST_VIEW));
    Q_ASSERT_X(ok && (mListView != 0), "TUT_LIST_VIEW", "invalid viewocML file");

    if (mListView)
        {
        HbAbstractViewItem *prototype = mListView->itemPrototypes().first();
        HbFrameBackground frame;
        frame.setFrameGraphicsName("qtg_fr_list_normal");
        frame.setFrameType(HbFrameDrawer::NinePieces);
        prototype->setDefaultFrame(frame);

        HbListViewItem *prototypeListView = qobject_cast<HbListViewItem *> (
                prototype);
        if (prototypeListView)
            {
            HbStyle style;
            qreal x;
            style.parameter("hb-param-graphic-size-primary-large", x);
            QSizeF size(x, x);
            mListViewIconSize = size.toSize();
            prototypeListView->setTextFormat(Qt::RichText);
            }
        mListView->setModel(mModel, prototype);

        HbAbstractItemView::ItemAnimations noCreationAndRemovalAnimations =
                HbAbstractItemView::All;
        noCreationAndRemovalAnimations ^= HbAbstractItemView::Appear;
        noCreationAndRemovalAnimations ^= HbAbstractItemView::Disappear;
        mListView->setEnabledAnimations(noCreationAndRemovalAnimations);
        }

    mSearchPanel = qobject_cast<HbSearchPanel *> (
            mDocumentLoader->findWidget(TUT_SEARCHPANEL_WIDGET));
    if (mSearchPanel)
        {
        mSearchPanel->setSearchOptionsEnabled(true);
        }

    constructHandlers();

    if (mView && mMainWindow)
        {
        mMainWindow->addView(mView);
        mMainWindow->setCurrentView(mView);
        }
    mDatabasecount = 0;
    mLinkindex = 0;
    mResultcount = 0;
    mResultparser = 0;
    loadSettings = true;

    //Icon creation in array
    RArray<TUid> appUid;
    appUid.Append(TUid::Uid(0x20022EF9));//contact
    appUid.Append(TUid::Uid(0x10207C62));//media(audio)
    appUid.Append(TUid::Uid(0x200211FE));//video not assigned 
    appUid.Append(TUid::Uid(0x20000A14));//image 
    appUid.Append(TUid::Uid(0x2001FE79));//msg
    appUid.Append(TUid::Uid(0x20022F35));//email not assigned 
    appUid.Append(TUid::Uid(0x10005901));//calender
    appUid.Append(TUid::Uid(0x20029F80));//notes
    //appUid.Append(TUid::Uid(0x20022F35));//application
    appUid.Append(TUid::Uid(0x20022F35));//bookmark not assigned
    appUid.Append(TUid::Uid(0x2002BCC0));//files

    for (int i = 0; i < appUid.Count(); i++)
        {
        mIconArray.append(getAppIconFromAppId(appUid[i]));
        }
#ifdef OST_TRACE_COMPILER_IN_USE 
    //start() the timers to avoid worrying abt having to start()/restart() later
    m_categorySearchApiTime.start();
    m_categorySearchUiTime.start();
    m_totalSearchUiTime.start();
    m_categoryGetDocumentApiTime.start();
    m_getDocumentCatergoryTimeAccumulator = 0;
#endif
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::~SearchProgressiveState
// ---------------------------------------------------------------------------
SearchProgressiveState::~SearchProgressiveState()
    {
    if (mAiwMgr)
        {
        delete mAiwMgr;
        }
    if (mModel)
        {
        delete mModel;
        }
    if (mDocumentLoader)
        {
        delete mDocumentLoader;
        }
    for (int i = 0; i < mSearchHandlerList.count(); i++)
        {
        delete mSearchHandlerList.at(i);
        }
    if(notesEditor)
        {
        delete notesEditor;
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::constructHandlers
// ---------------------------------------------------------------------------
void SearchProgressiveState::constructHandlers()
    {
    InDeviceHandler* handler = NULL;
    for (int i = 0; i < totalcategories; i++)
        {
        handler = new InDeviceHandler();
        switch (i)
            {
            case 0:
                {
                handler->setCategory("");
                break;
                }
            case 1:
                {
                handler->setCategory("contact");
                break;
                }
            case 2:
                {
                handler->setCategory("media");
                break;
                }
            case 3:
                {
                handler->setCategory("msg");
                break;
                }
            case 4:
                {
                handler->setCategory("email");
                break;
                }
            case 5:
                {
                handler->setCategory("calendar");
                break;
                }
            case 6:
                {
                handler->setCategory("notes");
                break;
                }
            case 7:
                {
                handler->setCategory("applications");
                break;
                }
            case 8:
                {
                handler->setCategory("file");
                break;
                }
            case 9:
                {
                handler->setCategory("bookmark");
                break;
                }
            }
        mSearchHandlerList.append(handler);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onEntry
// ---------------------------------------------------------------------------
void SearchProgressiveState::onEntry(QEvent *event)
    {
    //  WMS_LOG << "::onEntry";
    QState::onEntry(event);
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
    if (loadSettings)
        {
        emit settingsState();
        loadSettings = false;
        }
    setSelectedCategories();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onExit
// ---------------------------------------------------------------------------
void SearchProgressiveState::onExit(QEvent *event)
    {
    QState::onExit(event);
    deActivateSignals();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::activateSignals
// ---------------------------------------------------------------------------
void SearchProgressiveState::activateSignals()
    {
    for (int i = 0; i < mSearchHandlerList.count(); i++)
        {
        connect(mSearchHandlerList.at(i),
                SIGNAL(handleAsyncSearchResult(int,int)), this,
                SLOT(onAsyncSearchComplete(int,int)));
        connect(mSearchHandlerList.at(i),
                SIGNAL(handleDocument(int,QCPixDocument*)), this,
                SLOT(onGetDocumentComplete(int,QCPixDocument*)));
        }
    if (mListView)
        {
        connect(mListView, SIGNAL(activated(const QModelIndex)), this,
                SLOT(openResultitem(const QModelIndex)));
        }
    if (mSearchPanel)
        {
        connect(mSearchPanel, SIGNAL(criteriaChanged(QString)), this,
                SLOT(startNewSearch(QString)));
        connect(mSearchPanel, SIGNAL(searchOptionsClicked()), this,
                SLOT(setSettings()));
        connect(mSearchPanel, SIGNAL(exitClicked()), this,
                SLOT(cancelSearch()));
        }
    if (mModel)
        {
        connect(mModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this,
                SLOT(getrowsInserted()));
        connect(mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this,
                SLOT(getrowsRemoved()));
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::deActivateSignals
// ---------------------------------------------------------------------------
void SearchProgressiveState::deActivateSignals()
    {
    for (int i = 0; i < mSearchHandlerList.count(); i++)
        {
        disconnect(mSearchHandlerList.at(i),
                SIGNAL(handleAsyncSearchResult(int,int)), this,
                SLOT(onAsyncSearchComplete(int,int)));
        disconnect(mSearchHandlerList.at(i),
                SIGNAL(handleDocument(int,QCPixDocument*)), this,
                SLOT(onGetDocumentComplete(int,QCPixDocument*)));
        }
    if (mListView)
        {
        disconnect(mListView, SIGNAL(activated(const QModelIndex)), this,
                SLOT(openResultitem(const QModelIndex)));
        }
    if (mSearchPanel)
        {
        disconnect(mSearchPanel, SIGNAL(criteriaChanged(QString)), this,
                SLOT(startNewSearch(QString)));
        disconnect(mSearchPanel, SIGNAL(searchOptionsClicked()), this,
                SLOT(setSettings()));
        disconnect(mSearchPanel, SIGNAL(exitClicked()), this,
                SLOT(cancelSearch()));
        }
    if (mModel)
        {
        disconnect(mModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this,
                SLOT(getrowsInserted()));
        disconnect(mModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this,
                SLOT(getrowsRemoved()));
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onAsyncSearchComplete
// ---------------------------------------------------------------------------
void SearchProgressiveState::onAsyncSearchComplete(int aError,
        int aResultCount)
    {
    PERF_CAT_API_ENDLOG
    PERF_CAT_HITS_ENDLOG
    if (aError != 0)
        {
        //some error print logs
        searchOnCategory(mSearchString);
        return;
        }
    if (aResultCount == 0)
        {
        searchOnCategory(mSearchString);
        }
    else if (aResultCount > 0)
        {
        mResultcount = aResultCount;
        mResultparser = 0;
        PERF_CAT_GETDOC_TIME_RESTART
        PERF_CAT_GETDOC_ACCUMULATOR_RESET
        mSearchHandler->getDocumentAsyncAtIndex(mResultparser);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onGetDocumentComplete
// ---------------------------------------------------------------------------
void SearchProgressiveState::onGetDocumentComplete(int aError,
        QCPixDocument* aDoc)
    {
    PERF_CAT_GETDOC_TIME_ACCUMULATE
    if (aDoc == NULL || aError != 0)
        return;
    QStringList liststr;
    QString secondrow = aDoc->excerpt();
    QString firstrow;
    //-------------- html tag creation-------------------
   /* QString htmlTagPre = QString("<u>");
    QString htmlTagPost = QString("</u>");
    int insertpt = secondrow.indexOf(mOriginalString, 0, Qt::CaseInsensitive);
    if (insertpt >= 0)
        {
        secondrow.insert(insertpt, htmlTagPre);

        secondrow.insert(insertpt + mOriginalString.length()
                + htmlTagPre.length(), htmlTagPost);
        }
*/    //--------------------Html Tag Creation completed------------
    QStandardItem* listitem = new QStandardItem();

    if (aDoc->baseAppClass().contains("contact"))
        {
        QStringList docsList = filterDoc(aDoc, "GivenName", "FamilyName");
        if (docsList.value(0, "").length())
            {
            firstrow.append(docsList.at(0));
            }
        if (docsList.value(1, "").length())
            {
            if (firstrow.length())
                firstrow.append(" ");
            firstrow.append(docsList.at(1));
            }
        if (firstrow.length() == 0)
            {
            firstrow = hbTrId("txt_phob_dblist_unnamed");
            }
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(0), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("audio"))
        {
        QStringList audioList = filterDoc(aDoc, "Title", "MediaId");
        if (audioList.value(0, "").length())
            {
            firstrow.append(audioList.at(0));
            }
        if (firstrow.length() == 0)
            {
            firstrow = hbTrId("txt_mus_dblist_val_unknown4");
            }
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(1), Qt::DecorationRole);
        if (audioList.value(1, "").length())
            {
            listitem->setData(audioList.at(1), Qt::UserRole + 2);
            }
        }
    else if (aDoc->baseAppClass().contains("video"))
        {
        QStringList videoList = filterDoc(aDoc, "Title", "MediaId","Name");
        if (videoList.value(0, "").length())
            {
            firstrow.append(videoList.at(0));
            }
        if (firstrow.length() == 0 && videoList.value(2, "").length())
            {
            firstrow.append(videoList.at(2));
            }
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(2), Qt::DecorationRole);
        if (videoList.value(1, "").length())
            {
            listitem->setData(videoList.at(1), Qt::UserRole + 2);
            }
        }
    else if (aDoc->baseAppClass().contains("image"))
        {
        QStringList imageList = filterDoc(aDoc, "Name", "MediaId");
        if (imageList.value(0, "").length())
            {
            firstrow.append(imageList.at(0));
            }
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(3), Qt::DecorationRole);
        if (imageList.value(1, "").length())
            {
            listitem->setData(imageList.at(1), Qt::UserRole + 2);
            }
        }
    else if (aDoc->baseAppClass().contains("msg"))
        {
        QStringList msgList = filterDoc(aDoc, "Subject", "Body");
        if (msgList.value(0, "").length())
            {
            firstrow.append(msgList.at(0));
            }
        else
            {
            if (msgList.value(1, "").length())
                firstrow.append(msgList.at(1));
            }
        if (firstrow.length() == 0)
            {
            firstrow = " ";// space if subject and body are missing
            }
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(4), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("email"))
        {
        firstrow.append(filterDoc(aDoc, "Subject"));
        liststr << firstrow << secondrow;
        }
    else if (aDoc->baseAppClass().contains("calendar"))
        {
        firstrow.append(filterDoc(aDoc, "Summary"));
        if (firstrow.length() == 0)
            {
            firstrow = hbTrId("txt_calendar_preview_unnamed");
            }

        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(6), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("notes"))
        {
        firstrow.append(filterDoc(aDoc, "Memo"));
        if (firstrow.length() == 0)
            {
            firstrow = hbTrId("txt_notes_dblist_unnamed");
            }
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(7), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("applications"))
        {
        firstrow.append(filterDoc(aDoc, "Name"));
        liststr << firstrow;
        bool ok;
        listitem->setData(getAppIconFromAppId(TUid::Uid(aDoc->docId().toInt(
                &ok, 16))), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("bookmark"))
        {
        firstrow.append(filterDoc(aDoc, "Name"));
        if (firstrow.length() == 0)
            {
            firstrow = "UnKnown";
            }
        liststr << firstrow << secondrow;
        }
    else if (aDoc->baseAppClass().contains("file"))
        {
        firstrow.append(filterDoc(aDoc, "Name"));
        if (firstrow.length() == 0)
            firstrow = aDoc->baseAppClass();
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(9), Qt::DecorationRole);
        }
    listitem->setData(liststr, Qt::DisplayRole);
    listitem->setData(aDoc->docId(), Qt::UserRole);
    listitem->setData(aDoc->baseAppClass(), Qt::UserRole + 1);
    mModel->appendRow(listitem);
    delete aDoc;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getSettingCategory
// ---------------------------------------------------------------------------
void SearchProgressiveState::getSettingCategory(int item, bool avalue)
    {
    mSelectedCategory.insert(item, avalue);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getrowsInserted
// ---------------------------------------------------------------------------
void SearchProgressiveState::getrowsInserted()
    {
    mResultparser++;
    if (mResultparser < mResultcount)
        {
        PERF_CAT_GETDOC_TIME_RESTART
        mSearchHandler->getDocumentAsyncAtIndex(mResultparser);
        }
    else
        {
        PERF_CAT_GETDOC_ACCUMULATOR_ENDLOG
        searchOnCategory(mSearchString);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getrowsRemoved
// ---------------------------------------------------------------------------
void SearchProgressiveState::getrowsRemoved()
    {
    if (mModel->rowCount() != 0)
        {
        mModel->removeRow(0);
        }
    else
        {
        mListView->reset();
        if (mOriginalString.length())
            {
            mDatabasecount = 0;
            /*mSearchString = "$prefix(\"";
            mSearchString += mOriginalString;
            mSearchString += "\")";*/
            mSearchString = mOriginalString;
            mSearchString.append('*');
            searchOnCategory(mSearchString);
            }
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::openResultitem
// ---------------------------------------------------------------------------
void SearchProgressiveState::openResultitem(QModelIndex index)
    {
    QStandardItem* item = mModel->itemFromIndex(index);
    QList<QVariant> args;
    bool t;
    mRequest = NULL;
    if (item->data(Qt::UserRole + 1).toString().contains("contact"))
        {
        mRequest = mAiwMgr->create("com.nokia.services.phonebookservices",
                "Fetch", "open(int)", false);

        int uid = (item->data(Qt::UserRole)).toInt(&t);
        args << uid;
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("bookmark"))
        {

        }
    else if (item->data(Qt::UserRole + 1).toString().contains("calendar"))
        {
        QDir pluginDir = QDir(QString("z:/resource/qt/plugins/calendar"));
        QPluginLoader *calAgandaViewerPluginLoader = new QPluginLoader(
                pluginDir.absoluteFilePath(QString(
                        "agendaeventviewerplugin.qtplugin")));

        calAgandaViewerPluginInstance = qobject_cast<
                EventViewerPluginInterface *> (
                calAgandaViewerPluginLoader->instance());

        connect(calAgandaViewerPluginInstance, SIGNAL(viewingCompleted()),
                this, SLOT(_viewingCompleted()));

        calAgandaViewerPluginInstance->viewEvent(
                item->data(Qt::UserRole).toInt(),
                EventViewerPluginInterface::ActionEditDelete, NULL);
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("applications"))
        {
        TRAPD(err,
                    {LaunchApplicationL(TUid::Uid((item->data(Qt::UserRole)).toString().toInt(&t, 16)));})
        if (err == KErrNone)
            {
            }
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("file"))
        {
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("video"))
        {
        mRequest = mAiwMgr->create("com.nokia.videos", "IVideoView",
                "playMedia(QString)", false);

        QString uid = getDrivefromMediaId(
                item->data(Qt::UserRole + 2).toString());
        uid.append(':');
        uid.append(item->data(Qt::UserRole).toString());
        args << uid;
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("audio"))
        {
        QString uid = getDrivefromMediaId(
                item->data(Qt::UserRole + 2).toString());
        uid.append(':');
        uid.append(item->data(Qt::UserRole).toString());
        mRequest = mAiwMgr->create("musicplayer",
                "com.nokia.symbian.IFileView", "view(QString)", false);
        args << uid;
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("image"))
        {
        QString uid = getDrivefromMediaId(
                item->data(Qt::UserRole + 2).toString());
        uid.append(':');
        uid.append(item->data(Qt::UserRole).toString());
        mRequest = mAiwMgr->create("com.nokia.services.media",
                "com.nokia.symbian.IFileView", "view(QString)", false);
        args << uid;
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("notes"))
        {
        if (!notesEditor)
            {
            notesEditor = new NotesEditor(this);
            }
        notesEditor->edit(item->data(Qt::UserRole).toInt());
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("msg"))
        {
        mRequest = mAiwMgr->create("com.nokia.services.hbserviceprovider",
                "conversationview", "view(int)", false);

        int uid = (item->data(Qt::UserRole)).toInt(&t);
        args << uid;
        }
    if (mRequest)
        {
        connect(mRequest, SIGNAL(requestOk(const QVariant&)), this,
                SLOT(handleOk(const QVariant&)));
        connect(mRequest, SIGNAL(requestError(int,const QString&)), this,
                SLOT(handleError(int,const QString&)));// Connect error handling signal or apply lastError function instead.
        mRequest->setArguments(args);
        if (!mRequest->send())// Make the request
            {
            qDebug() << "AIW-ERROR: Send failed" << mRequest->lastError();
            }
        disconnect(mRequest, SIGNAL(requestOk(const QVariant&)), this,
                SLOT(handleOk(const QVariant&)));
        disconnect(mRequest, SIGNAL(requestError(int,const QString&)), this,
                SLOT(handleError(int,const QString&)));// Connect error handling signal or apply lastError function instead.
        mRequest->deleteLater();
        }
    else
        {
        qDebug() << "AIW-ERROR: NULL request";
        return;
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::handleOk
// ---------------------------------------------------------------------------
void SearchProgressiveState::handleOk(const QVariant& var)
    {
    Q_UNUSED(var);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::handleError
// ---------------------------------------------------------------------------
void SearchProgressiveState::handleError(int ret, const QString& var)
    {
    Q_UNUSED(ret);
    Q_UNUSED(var);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::createSuggestionLink
// ---------------------------------------------------------------------------
void SearchProgressiveState::createSuggestionLink(bool aFlag)
    {
    QStandardItem* listitem = new QStandardItem();   
    if (!aFlag)
        {
        QString linkString = QString(hbTrId("txt_search_list_search_for_1").arg(mOriginalString));
        mLinkindex = mModel->rowCount();
        listitem->setData(linkString, Qt::DisplayRole);
        mModel->appendRow(listitem);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::noResultsFound
// ---------------------------------------------------------------------------
void SearchProgressiveState::noResultsFound(QString aKeyword)
    {
    if (aKeyword.length())
        {
        QStandardItem* listitem = new QStandardItem();
        QString noResultMsg = QString("<align=\"center\">" + hbTrId(
                "txt_search_list_no_match_found"));
        listitem->setData(noResultMsg, Qt::DisplayRole);
        mModel->appendRow(listitem);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::clear
// ---------------------------------------------------------------------------
void SearchProgressiveState::clear()
    {
    if (mModel)
        {
        mModel->removeRows(0, mModel->rowCount());
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::searchOnCategory
// ---------------------------------------------------------------------------
void SearchProgressiveState::searchOnCategory(const QString aKeyword)
    {
#ifdef OST_TRACE_COMPILER_IN_USE
    if( mDatabasecount != 0 )
        {//Search just started.
        PERF_CAT_UI_ENDLOG
        }
    PERF_CAT_UI_TIME_RESTART
#endif
    mResultparser = 0;
    mResultcount = 0;
    if (mDatabasecount < mTemplist.count())
        {
        if (mTemplist.at(mDatabasecount).contains("selectall"))
            {
            mSearchHandler = mSearchHandlerList.at(0);
            }
        else if (mTemplist.at(mDatabasecount).contains("contact"))
            {
            mSearchHandler = mSearchHandlerList.at(1);
            }
        else if (mTemplist.at(mDatabasecount).contains("media"))
            {
            mSearchHandler = mSearchHandlerList.at(2);
            }
        else if (mTemplist.at(mDatabasecount).contains("msg"))
            {
            mSearchHandler = mSearchHandlerList.at(3);
            }
        else if (mTemplist.at(mDatabasecount).contains("email"))
            {
            mSearchHandler = mSearchHandlerList.at(4);
            }
        else if (mTemplist.at(mDatabasecount).contains("calendar"))
            {
            mSearchHandler = mSearchHandlerList.at(5);
            }
        else if (mTemplist.at(mDatabasecount).contains("notes"))
            {
            mSearchHandler = mSearchHandlerList.at(6);
            }
        else if (mTemplist.at(mDatabasecount).contains("applications"))
            {
            mSearchHandler = mSearchHandlerList.at(7);
            }
        else if (mTemplist.at(mDatabasecount).contains("file"))
            {
            mSearchHandler = mSearchHandlerList.at(8);
            }
        else if (mTemplist.at(mDatabasecount).contains("bookmark"))
            {
            mSearchHandler = mSearchHandlerList.at(9);
            }
        // mSearchHandler->setCategory(mTemplist.at(mDatabasecount));
        mDatabasecount++;
        if (mSearchHandler->isPrepared())
            {
            PERF_CAT_API_TIME_RESTART
            mSearchHandler->searchAsync(aKeyword, "_aggregate");
            }
        }
    else if (mDatabasecount >= mTemplist.count())
        {
        PERF_TOTAL_UI_ENDLOG
        if (mModel->rowCount() == 0 && aKeyword.length() != 0)
            {
            noResultsFound(mOriginalString);
            createSuggestionLink(0);
            }
        return;
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::startNewSearch
// ---------------------------------------------------------------------------
void SearchProgressiveState::startNewSearch(const QString &aKeyword)
    {
    PERF_CAT_TOTAL_TIME_RESTART
    mOriginalString = aKeyword.trimmed();
    for (int i = 0; i < mSearchHandlerList.count(); i++)
        {
        mSearchHandlerList.at(i)->cancelLastSearch();
        }
    if (mModel->rowCount() != 0)
        {
        clear();
        }
    else if (mOriginalString.length())
        {
        mDatabasecount = 0;
        mLinkindex = 0;
        //Prefix query
        /*mSearchString = "$prefix(\"";
        mSearchString += mOriginalString;
        mSearchString += "\")";*/
        mSearchString = mOriginalString;
        mSearchString.append('*');
        searchOnCategory(mSearchString);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::setSettings
// ---------------------------------------------------------------------------
void SearchProgressiveState::setSettings()
    {
    mTempSelectedCategory = mSelectedCategory;
    emit settingsState();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::_customizeGoButton
// ---------------------------------------------------------------------------
void SearchProgressiveState::_customizeGoButton(bool avalue)
    {
    if (mSearchPanel)
        {
        if (avalue)
            {
            mSearchPanel->setProgressive(false);
            }
        else
            {
            mSearchPanel->setProgressive(true);
            }
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::settingsaction
// ---------------------------------------------------------------------------
void SearchProgressiveState::settingsaction(bool avalue)
    {
    if (avalue)
        {
        QMapIterator<int, bool> i(mTempSelectedCategory);
        QMapIterator<int, bool> j(mSelectedCategory);
        while (i.hasNext())
            {
            i.next();
            j.next();
            if (i.value() != j.value())
                {
                startNewSearch(mOriginalString);
                break;
                }
            }
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::setSelectedCategories
// ---------------------------------------------------------------------------
void SearchProgressiveState::setSelectedCategories()
    {
    mTemplist.clear();
    QMapIterator<int, bool> i(mSelectedCategory);
    bool isrootsearch = false;
    while (i.hasNext())
        {
        i.next();
        if (i.value())
            {
            switch (i.key())
                {
                case 0:
                    {
                    isrootsearch = true;
                    // mTemplist.append("selectall");
                    break;
                    }
                case 1: //Contacts
                    {
                    mTemplist.append("contact");
                    break;
                    }
                case 2://Media
                    {
                    mTemplist.append("media");
                    break;
                    }
                case 3://Messages& emails
                    {
                    mTemplist.append("msg");
                    //mTemplist.append("email");
                    break;
                    }
                case 4://Calender& notes
                    {
                    mTemplist.append("calendar");
                    mTemplist.append("notes");
                    break;
                    }
                case 5://Applications
                    {
                    mTemplist.append("applications");
                    break;
                    }
                case 6://Bookmarks
                    {
                    mTemplist.append("bookmark");
                    break;
                    }
                case 7://All other files
                    {
                    mTemplist.append("file");
                    break;
                    }
                }
            }
        }
    if (mTemplist.count() == 8)
        isrootsearch = true;
    if (isrootsearch)
        {
        mTemplist.clear();
        mTemplist.append("selectall");
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::cancelSearch
// ---------------------------------------------------------------------------
void SearchProgressiveState::cancelSearch()
    {
    for (int i = 0; i < mSearchHandlerList.count(); i++)
        {
        mSearchHandlerList.at(i)->cancelLastSearch();
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getAppIconFromAppId
// ---------------------------------------------------------------------------
HbIcon SearchProgressiveState::getAppIconFromAppId(TUid auid)
    {
    HbIcon icon;
    CAknIcon* aknIcon = NULL;
    CFbsBitmap* bitmap = NULL;
    CFbsBitmap* mask = NULL;
    QPixmap pixmap;
    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    if (skin)
        {
        TRAPD( err,
                    {
                    AknsUtils::CreateAppIconLC( skin, auid,
                            EAknsAppIconTypeList, bitmap, mask );
                    CleanupStack::Pop(2); //for trap
                    }
        );
        if (err == KErrNone)
            {
            TRAPD( err1,
                        {aknIcon = CAknIcon::NewL();
                        aknIcon->SetBitmap(bitmap);
                        aknIcon->SetMask(mask);});
            if (err1 == KErrNone)
                {
                }
            }
        }
    if (aknIcon)
        {
        //need to disable compression to properly convert the bitmap
        AknIconUtils::DisableCompression(aknIcon->Bitmap());
        AknIconUtils::SetSize(aknIcon->Bitmap(), TSize(mListViewIconSize.width(),
                mListViewIconSize.height()),
                EAspectRatioPreservedAndUnusedSpaceRemoved);
        if (aknIcon->Bitmap()->Header().iCompression == ENoBitmapCompression)
            {
            pixmap = fromSymbianCFbsBitmap(aknIcon->Bitmap());
            QPixmap mask = fromSymbianCFbsBitmap(aknIcon->Mask());
            pixmap.setAlphaChannel(mask);
            }
        else
            {
            CFbsBitmap *temp(NULL);
            TRAPD( err,
                        {temp = copyBitmapLC(aknIcon->Bitmap());
                        pixmap = fromSymbianCFbsBitmap(temp);
                        CleanupStack::PopAndDestroy();});
            if (err == KErrNone)
                {
                TRAPD( err1,
                            {temp = copyBitmapLC(aknIcon->Mask());
                            QPixmap mask = fromSymbianCFbsBitmap(temp);
                            CleanupStack::PopAndDestroy();
                            pixmap.setAlphaChannel(mask);});
                if (err1 == KErrNone)
                    {
                    }
                }
            }
        pixmap = pixmap.scaled(mListViewIconSize, Qt::KeepAspectRatioByExpanding);
        icon = HbIcon(QIcon(pixmap));
        }
    return icon;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::TDisplayMode2Format
// ---------------------------------------------------------------------------
QImage::Format SearchProgressiveState::TDisplayMode2Format(TDisplayMode mode)
    {
    QImage::Format format;
    switch (mode)
        {
        case EGray2:
            format = QImage::Format_MonoLSB;
            break;
        case EColor256:
        case EGray256:
            format = QImage::Format_Indexed8;
            break;
        case EColor4K:
            format = QImage::Format_RGB444;
            break;
        case EColor64K:
            format = QImage::Format_RGB16;
            break;
        case EColor16M:
            format = QImage::Format_RGB888;
            break;
        case EColor16MU:
            format = QImage::Format_RGB32;
            break;
        case EColor16MA:
            format = QImage::Format_ARGB32;
            break;
        case EColor16MAP:
            format = QImage::Format_ARGB32_Premultiplied;
            break;
        default:
            format = QImage::Format_Invalid;
            break;
        }
    return format;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::fromSymbianCFbsBitmap
// ---------------------------------------------------------------------------
QPixmap SearchProgressiveState::fromSymbianCFbsBitmap(CFbsBitmap *aBitmap)
    {
    aBitmap->BeginDataAccess();
    uchar *data = (uchar *) aBitmap->DataAddress();
    TSize size = aBitmap->SizeInPixels();
    TDisplayMode displayMode = aBitmap->DisplayMode();
    QImage image(data, size.iWidth, size.iHeight, TDisplayMode2Format(
            displayMode));// QImage format must match to bitmap format
    aBitmap->EndDataAccess();
    // No data copying happens because image format matches native OpenVG format.
    // So QPixmap actually points to CFbsBitmap data.
    return QPixmap::fromImage(image);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::copyBitmapLC
// ---------------------------------------------------------------------------
CFbsBitmap *SearchProgressiveState::copyBitmapLC(CFbsBitmap *input)
    {
    CFbsBitmap *bmp = new (ELeave) CFbsBitmap();
    CleanupStack::PushL(bmp);
    bmp->Create(input->SizeInPixels(), input->DisplayMode());
    CFbsBitmapDevice *bitmapDevice = CFbsBitmapDevice::NewL(bmp);
    CleanupStack::PushL(bitmapDevice);
    CFbsBitGc *bmpGc;
    bitmapDevice->CreateContext(bmpGc);
    bmpGc->BitBlt(TPoint(0, 0), input);
    delete bmpGc;
    CleanupStack::PopAndDestroy(bitmapDevice);
    return bmp;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::filterDoc
// ---------------------------------------------------------------------------
QString SearchProgressiveState::filterDoc(const QCPixDocument* aDoc,
        const QString& filter)
    {
    for (int i = 0; i < aDoc->fieldCount(); i++)
        {
        if (aDoc->field(i).name().contains(filter))
            {
            return aDoc->field(i).value();
            }
        }
    return NULL;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::filterDoc
// ---------------------------------------------------------------------------
QStringList SearchProgressiveState::filterDoc(const QCPixDocument* aDoc,
        const QString& filter1, const QString& filter2,
        const QString& filter3)
    {
    QStringList docList;
    docList.append(QString());
    docList.append(QString());
    docList.append(QString());
    bool bfilter1 = false;
    bool bfilter2 = false;
    bool bfilter3 = false;
    if (!filter3.length())
        {
        bfilter3 = true;
        }
    for (int i = 0; i < aDoc->fieldCount(); i++)
        {
        if (!bfilter1 && aDoc->field(i).name().contains(filter1))
            {
            docList.replace(0, aDoc->field(i).value());
            bfilter1 = true;
            }
        if (!bfilter2 && aDoc->field(i).name().contains(filter2))
            {
            docList.replace(1, aDoc->field(i).value());
            bfilter2 = true;
            }
        if (!bfilter3 && aDoc->field(i).name().contains(filter3))
            {
            docList.replace(2, aDoc->field(i).value());
            bfilter3 = true;
            }
        if (bfilter1 && bfilter2 && bfilter3)
            break;
        }
    return docList;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getDrivefromMediaId
// ---------------------------------------------------------------------------
QString SearchProgressiveState::getDrivefromMediaId(QString aMediaId)
    {
    TBuf<15> mediaIdBuf(aMediaId.utf16());
    if (KErrNone == iFs.Connect())
        {
        TUint mediaNum;
        TVolumeInfo vmInfo;
        TChar driveLetter;
        TLex lex(mediaIdBuf);
        lex.Val(mediaNum);
        TDriveNumber drive = TDriveNumber(KErrNotSupported);

        for (TInt i = 0; i <= EDriveZ; i++)
            {
            TInt err = iFs.Volume(vmInfo, i);// Volume() returns KErrNotReady if no volume present.       
            if (err != KErrNotReady)// In this case, check next drive number
                {
                if (vmInfo.iUniqueID == mediaNum)
                    {
                    drive = TDriveNumber(i);//Is the drive
                    if (KErrNone == iFs.DriveToChar(drive, driveLetter))
                        {
                        mediaIdBuf.Zero();
                        mediaIdBuf.Append(driveLetter);
                        mediaIdBuf.LowerCase();
                        }
                    break;
                    }
                }
            }
        iFs.Close();
        }
    return QString::fromUtf16(mediaIdBuf.Ptr(), mediaIdBuf.Length());
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::LaunchApplicationL
// ---------------------------------------------------------------------------
void SearchProgressiveState::LaunchApplicationL(const TUid aUid)
    {
    RWsSession wsSession;
    User::LeaveIfError(wsSession.Connect());
    CleanupClosePushL<RWsSession> (wsSession);
    CAknTaskList *taskList = CAknTaskList::NewL(wsSession);
    TApaTask task = taskList->FindRootApp(aUid);
    delete taskList;
    if (task.Exists())
        {
        task.BringToForeground();
        }
    else
        {
        TApaAppInfo appInfo;
        TApaAppCapabilityBuf capabilityBuf;
        RApaLsSession appArcSession;
        User::LeaveIfError(appArcSession.Connect());
        CleanupClosePushL<RApaLsSession> (appArcSession);
        User::LeaveIfError(appArcSession.GetAppInfo(appInfo, aUid));
        User::LeaveIfError(
                appArcSession.GetAppCapability(capabilityBuf, aUid));
        TApaAppCapability &caps = capabilityBuf();
        TFileName appName = appInfo.iFullName;
        CApaCommandLine *cmdLine = CApaCommandLine::NewLC();
        cmdLine->SetExecutableNameL(appName);
        if (caps.iLaunchInBackground)
            {
            cmdLine->SetCommandL(EApaCommandBackground);
            }
        else
            {
            cmdLine->SetCommandL(EApaCommandRun);
            }
        //cmdLine->SetTailEndL(aParam);
        User::LeaveIfError(appArcSession.StartApp(*cmdLine));
        CleanupStack::PopAndDestroy(cmdLine);
        CleanupStack::PopAndDestroy(&appArcSession);
        }
    CleanupStack::PopAndDestroy(&wsSession);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::_viewingCompleted
// ---------------------------------------------------------------------------
void SearchProgressiveState::_viewingCompleted()
    {
    if (calAgandaViewerPluginInstance)
        calAgandaViewerPluginInstance->deleteLater();
    }
