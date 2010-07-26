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
#include "searchuiloader.h"
#include <cpixdocument.h>
#include <cpixdocumentfield.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbicon.h>
#include <hbmenu.h>
#include <hbinstance.h>
#include <hbsearchpanel.h>
#include <hbaction.h>
#include <hbapplication.h>
#include <qsortfilterproxymodel.h>
#include <AknsUtils.h>
#include <bitdev.h> 
#include <bitstd.h>
#include <qbitmap.h>
#include <fbs.h>
#include <AknInternalIconUtils.h>
#include <AknIconUtils.h> 
#include <apaidpartner.h>
#include <qpluginloader.h>
#include <eventviewerplugininterface.h>
#include <noteseditorinterface.h>
#include <w32std.h>
#include <apgtask.h>
#include <apgcli.h>
#include <AknTaskList.h>
#include <apacmdln.h>
#include <xqconversions.h>
#include <apparc.h>
#include <qdesktopservices.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbabstractviewitem.h>
#include <hblistviewitem.h>
#include <hbactivitymanager.h>
#define hbApp qobject_cast<HbApplication*>(qApp)
const int totalcategories_normalreason = 10;
const int totalcategories_activityreasonreason = 13;
// ---------------------------------------------------------------------------
// SearchProgressiveState::SearchProgressiveState
// ---------------------------------------------------------------------------
SearchProgressiveState::SearchProgressiveState(QState *parent) :
    QState(parent), mMainWindow(NULL), mView(NULL), mListView(NULL),
            mSearchHandler(NULL), mNotesEditor(0), mAiwMgr(0), mRequest(0)
    {

    mMainWindow = hbInstance->allMainWindows().at(0);

    mAiwMgr = new XQApplicationManager;

    mUiLoader = SearchUiLoader::instance();

    mView = mUiLoader->View();
    mListView = mUiLoader->ListWidget();
    mSearchPanel = mUiLoader->SearchPanel();

    HbStyle style;
    qreal x;
    style.parameter("hb-param-graphic-size-primary-large", x);
    QSizeF size(x, x);
    mListViewIconSize = size.toSize();

    if (mSearchPanel)
        {
        mSearchPanel->setPlaceholderText(hbTrId(
                "txt_search_dialog_search_device"));
        mSearchPanel->setFocus();
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

    if (hbApp)
        {
        connect(hbApp->activityManager(), SIGNAL(activityRequested(QString)),
                this, SLOT(activityRequested(QString)));
        }

#ifdef OST_TRACE_COMPILER_IN_USE 
    //start() the timers to avoid worrying abt having to start()/restart() later
    m_categorySearchApiTime.start();
    m_categorySearchUiTime.start();
    m_totalSearchUiTime.start();
    m_categoryGetDocumentApiTime.start();
    m_getDocumentCatergoryTimeAccumulator = 0;
#endif

    //Notes Editor Interface loading 
    QDir dir(NOTES_EDITOR_PLUGIN_PATH);
    QString pluginName = dir.absoluteFilePath(NOTES_EDITOR_PLUGIN_NAME);

    // Create plugin loader.
    mNotespluginLoader = new QPluginLoader(pluginName);

    if (mNotespluginLoader)
        {
        // Load the plugin.
        mNotespluginLoader->load();

        QObject *plugin = qobject_cast<QObject*> (
                mNotespluginLoader->instance());
        mNotesEditor = qobject_cast<NotesEditorInterface*> (plugin);
        }

    mOnlineQueryAvailable = false;
    //Prepare the icons,listen of theme change    
    connect(hbInstance->theme(), SIGNAL(changeFinished()), this,
            SLOT(slotPrepareResultIcons()));
    slotPrepareResultIcons();    
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
    for (int i = 0; i < mSearchHandlerList.count(); i++)
        {
        delete mSearchHandlerList.at(i);
        }
    if (mNotespluginLoader)
        {
        mNotespluginLoader->unload();
        delete mNotespluginLoader;
        }
    SearchUiLoader::deleteinstance();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::constructHandlers
// ---------------------------------------------------------------------------
void SearchProgressiveState::constructHandlers()
    {
    int categories = totalcategories_normalreason;

    if (hbApp && hbApp->activateReason() == Hb::ActivationReasonActivity)
        {
        categories = totalcategories_activityreasonreason;
        }
    InDeviceHandler* handler = NULL;
    for (int i = 0; i < categories; i++)
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
                handler->setCategory("msg email");
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
            case 10:
                {
                handler->setCategory("media image");
                break;
                }
            case 11:
                {
                handler->setCategory("media audio");
                break;
                }
            case 12:
                {
                handler->setCategory("media video");
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

    mStateStatus = true;// used for conditional execution for the slots that are connected as transitions
    if (mSearchPanel)
        {
        mSearchPanel->setPlaceholderText(hbTrId(
                "txt_search_dialog_search_device"));
        mSearchPanel->setProgressive(true);
        }
    if (mListView)
        {
        mListView->setVisible(true);
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
    if (loadSettings)
        {
        emit switchProToSettingsState();
        loadSettings = false;
        }
    setSelectedCategories();
    if (mOnlineQueryAvailable)
        {
        mSearchPanel->setCriteria(mOriginalString);
        mOnlineQueryAvailable = false;
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onExit
// ---------------------------------------------------------------------------
void SearchProgressiveState::onExit(QEvent *event)
    {
    QState::onExit(event);
    mStateStatus = false;
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
                SIGNAL(handleDocument(int,CpixDocument*)), this,
                SLOT(onGetDocumentComplete(int,CpixDocument*)));
        }
    if (mListView)
        {
        connect(mListView, SIGNAL(activated(HbListWidgetItem *)), this,
                SLOT(openResultitem(HbListWidgetItem *)));
        }
    if (mSearchPanel)
        {
        connect(mSearchPanel, SIGNAL(criteriaChanged(QString)), this,
                SLOT(startNewSearch(QString)));
        connect(mSearchPanel, SIGNAL(searchOptionsClicked()), this,
                SLOT(setSettings()));
        }
    connect(mMainWindow, SIGNAL(viewReady()), this, SLOT(viewReady()));
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
                SIGNAL(handleDocument(int,CpixDocument*)), this,
                SLOT(onGetDocumentComplete(int,CpixDocument*)));
        }
    if (mListView)
        {
        disconnect(mListView, SIGNAL(activated(HbListWidgetItem *)), this,
                SLOT(openResultitem(HbListWidgetItem *)));
        }
    if (mSearchPanel)
        {
        disconnect(mSearchPanel, SIGNAL(criteriaChanged(QString)), this,
                SLOT(startNewSearch(QString)));
        disconnect(mSearchPanel, SIGNAL(searchOptionsClicked()), this,
                SLOT(setSettings()));
        }
    disconnect(mMainWindow, SIGNAL(viewReady()), this, SLOT(viewReady()));
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
        CpixDocument* aDoc)
    {
    PERF_CAT_GETDOC_TIME_ACCUMULATE
    if (aDoc == NULL || aError != 0)
        return;
    QString secondrow = aDoc->excerpt();
    QString firstrow;
    HbListWidgetItem* listitem = new HbListWidgetItem();

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
        listitem->setData(mIconArray.at(1), Qt::DecorationRole);
        if (audioList.value(1, "").length())
            {
            listitem->setData(audioList.at(1), Qt::UserRole + 2);
            }
        }
    else if (aDoc->baseAppClass().contains("video"))
        {
        QStringList videoList = filterDoc(aDoc, "Title", "MediaId", "Name");
        if (videoList.value(0, "").length())
            {
            firstrow.append(videoList.at(0));
            }
        if (firstrow.length() == 0 && videoList.value(2, "").length())
            {
            firstrow.append(videoList.at(2));
            }
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
        listitem->setData(mIconArray.at(3), Qt::DecorationRole);
        if (imageList.value(1, "").length())
            {
            listitem->setData(imageList.at(1), Qt::UserRole + 2);
            }
        }
    else if (aDoc->baseAppClass().contains("msg email"))
        {
        QStringList emailList = filterDoc(aDoc, "Sender", "MailBoxId",
                "FolderId");
        firstrow.append(emailList.at(0));
        listitem->setData(emailList.at(1), Qt::UserRole + 2);
        listitem->setData(emailList.at(2), Qt::UserRole + 3);
        listitem->setData(mIconArray.at(5), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("msg"))
        {
        QStringList msgList = filterDoc(aDoc, "Folder", "To", "From");
        if (msgList.value(0).contains("Inbox"))
            {
            firstrow.append(msgList.at(2));
            }
        else
            {
            if (msgList.value(1, "").length())
                firstrow.append(msgList.at(1));
            }
        listitem->setData(mIconArray.at(4), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("calendar"))
        {
        firstrow.append(filterDoc(aDoc, "Summary"));
        if (firstrow.length() == 0)
            {
            firstrow = hbTrId("txt_calendar_preview_unnamed");
            }
        listitem->setData(mIconArray.at(6), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("notes"))
        {
        firstrow.append(filterDoc(aDoc, "Memo"));
        if (firstrow.length() == 0)
            {
            firstrow = hbTrId("txt_notes_dblist_unnamed");
            }
        listitem->setData(mIconArray.at(7), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("applications"))
        {
        firstrow.append(filterDoc(aDoc, "Name"));
        bool ok;
        TRAP_IGNORE(listitem->setData(getAppIconFromAppIdL(TUid::Uid(aDoc->docId().toInt(
                                                &ok, 16))), Qt::DecorationRole));
        }
    else if (aDoc->baseAppClass().contains("bookmark"))
        {
        firstrow.append(filterDoc(aDoc, "Name"));
        if (firstrow.length() == 0)
            {
            firstrow = "UnKnown";
            }
        listitem->setData(secondrow, Qt::UserRole + 2);
        listitem->setData(mIconArray.at(8), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("file folder"))
        {
        bool ok;
        QStringList fileList = filterDoc(aDoc, "Name", "IsFolder",
                "Extension");
        firstrow = fileList.at(0);
        if (fileList.at(1).toInt(&ok) == 1) // folder result icon map 
            {
            listitem->setData(mIconArray.at(13), Qt::DecorationRole);
            }
        else
            {
            if (fileList.at(2).contains("sis", Qt::CaseInsensitive)
                    || fileList.at(1).contains("sisx", Qt::CaseInsensitive))
                {
                listitem->setData(mIconArray.at(10), Qt::DecorationRole);
                }
            else if (fileList.at(2).contains("java", Qt::CaseInsensitive)
                    || fileList.at(2).contains("jar", Qt::CaseInsensitive)
                    || fileList.at(2).contains("jad", Qt::CaseInsensitive))
                {
                listitem->setData(mIconArray.at(11), Qt::DecorationRole);
                }
            else if (fileList.at(2).contains("swf", Qt::CaseInsensitive))
                {
                listitem->setData(mIconArray.at(12), Qt::DecorationRole);
                }
            else
                {
                listitem->setData(mIconArray.at(14), Qt::DecorationRole);
                }
            }
        }
    else if (aDoc->baseAppClass().contains("file"))
        {
        QStringList fileList = filterDoc(aDoc, "Name", "Extension");
        firstrow = fileList.at(0);
        if (firstrow.length() == 0)
            firstrow = aDoc->baseAppClass();
        if (fileList.at(1).contains("txt", Qt::CaseInsensitive))
            {
            listitem->setData(mIconArray.at(9), Qt::DecorationRole);
            }
        else
            {
            listitem->setData(mIconArray.at(14), Qt::DecorationRole);
            }
        }
    listitem->setText(firstrow);
    listitem->setSecondaryText(secondrow);
    listitem->setData(aDoc->docId(), Qt::UserRole);
    listitem->setData(aDoc->baseAppClass(), Qt::UserRole + 1);
    mListView->addItem(listitem);

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
// SearchProgressiveState::openResultitem
// ---------------------------------------------------------------------------
void SearchProgressiveState::openResultitem(HbListWidgetItem * item)
    {
    if (item == NULL)
        return;PERF_RESULT_ITEM_LAUNCH_TIME_RESTART
    QList<QVariant> args;
    bool t;
    mRequest = NULL;
    if (item->data(Qt::UserRole + 1).toString().contains("contact"))
        {
        PERF_RESULT_ITEM_FOR_LAUNCHING("contact")
        mRequest = mAiwMgr->create("com.nokia.services.phonebookservices",
                "Fetch", "open(int)", true);

        int uid = (item->data(Qt::UserRole)).toInt(&t);
        args << uid;
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("bookmark"))
        {
        PERF_RESULT_ITEM_FOR_LAUNCHING("bookmark")
        QDesktopServices::openUrl(item->data(Qt::UserRole + 2).toString());
PERF_RESULT_ITEM_LAUNCH_TIME_ENDLOG        ("")

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
        PERF_RESULT_ITEM_FOR_LAUNCHING("applications")
        TRAP_IGNORE(LaunchApplicationL(TUid::Uid((item->data(Qt::UserRole)).toString().toInt(&t, 16))));
        PERF_RESULT_ITEM_LAUNCH_TIME_ENDLOG("")
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("file"))
        {
        PERF_RESULT_ITEM_FOR_LAUNCHING("file")
        QString uid = item->data(Qt::UserRole).toString();
        QFile file(uid);
        mRequest = mAiwMgr->create(file, true);
        args << file.fileName();
        }
    else if ((item->data(Qt::UserRole + 1).toString().contains("video"))
            || (item->data(Qt::UserRole + 1).toString().contains("audio"))
            || (item->data(Qt::UserRole + 1).toString().contains("image")))
        {
        PERF_RESULT_ITEM_FOR_LAUNCHING("media")
        QString uid = getDrivefromMediaId(
                item->data(Qt::UserRole + 2).toString());
        uid.append(':');
        uid.append(item->data(Qt::UserRole).toString());
        QFile file(uid);
        mRequest = mAiwMgr->create(file, true);
        args << file.fileName();
        }

    else if (item->data(Qt::UserRole + 1).toString().contains("notes"))
        {
        PERF_RESULT_ITEM_FOR_LAUNCHING("notes")
        if(mNotesEditor)
            {
            mNotesEditor->edit(item->data(Qt::UserRole).toInt());
            }
        PERF_RESULT_ITEM_LAUNCH_TIME_ENDLOG("")
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("msg email"))
        {

        mRequest = mAiwMgr->create("nmail",
                "com.nokia.symbian.IEmailMessageView",
                "viewMessage(QVariant,QVariant,QVariant)", true);

        args << item->data(Qt::UserRole + 2).toULongLong(&t) << item->data(
                Qt::UserRole + 3).toULongLong(&t) << item->data(Qt::UserRole).toULongLong(&t);
        }
    else if (item->data(Qt::UserRole + 1).toString().contains("msg"))
        {
        PERF_RESULT_ITEM_FOR_LAUNCHING("msg")
        mRequest = mAiwMgr->create("messaging",
                "com.nokia.symbian.IMessageView", "view(int)", true);

        int uid = (item->data(Qt::UserRole)).toInt(&t);
        args << uid;
        }
    else if(item->data(Qt::UserRole + 1).toString().contains("links"))
        {
        emit launchLink((item->data(Qt::UserRole)).toInt(&t),mOriginalString);
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
PERF_RESULT_ITEM_LAUNCH_TIME_ENDLOG    ("")
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
void SearchProgressiveState::createSuggestionLink()
    {
    QString linkString = QString(hbTrId("txt_search_list_search_for_1").arg(
            mOriginalString));
    QMapIterator<int, HbIcon> i(mISprovidersIcon);
    while (i.hasNext())
        {
        i.next();
        HbListWidgetItem* listitem = new HbListWidgetItem();
        listitem->setText(linkString);
        listitem->setData(i.key(), Qt::UserRole);
        listitem->setData(i.value(), Qt::DecorationRole);
        listitem->setData("links", Qt::UserRole + 1);
        mListView->addItem(listitem);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::noResultsFound
// ---------------------------------------------------------------------------
void SearchProgressiveState::noResultsFound(QString aKeyword)
    {
    if (aKeyword.length())
        {
        HbListWidgetItem* listitem = new HbListWidgetItem();
        QString noResultMsg = QString("<align=\"center\">" + hbTrId(
                "txt_search_list_no_match_found"));
        listitem->setText(noResultMsg);
        mListView->addItem(listitem);

        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::clear
// ---------------------------------------------------------------------------
void SearchProgressiveState::clear()
    {
    mListView->clear();
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
        else if (mTemplist.at(mDatabasecount).contains("image"))
            {
            mSearchHandler = mSearchHandlerList.at(10);
            }
        else if (mTemplist.at(mDatabasecount).contains("audio"))
            {
            mSearchHandler = mSearchHandlerList.at(11);
            }
        else if (mTemplist.at(mDatabasecount).contains("video"))
            {
            mSearchHandler = mSearchHandlerList.at(12);
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
        if (mSearchHandler != NULL && mSearchHandler->isPrepared())
            {
            PERF_CAT_API_TIME_RESTART
            mSearchHandler->searchAsync(aKeyword, "_aggregate");

            }
        else
            {
            searchOnCategory(mSearchString);
            }
        }
    else if (mDatabasecount >= mTemplist.count())
        {
        PERF_TOTAL_UI_ENDLOG
        if (mListView->count() == 0 && aKeyword.length() != 0)
            {
            noResultsFound(mOriginalString);
            createSuggestionLink();
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
    if (mListView->count() != 0)
        {
        clear();
        }
    if (mOriginalString.length())
        {
        mDatabasecount = 0;
        mLinkindex = 0;
        //Prefix query
        mSearchString = "$prefix(\"";
        mSearchString += mOriginalString;
        mSearchString += "\")";
        /*mSearchString = mOriginalString;
         mSearchString.append('*');*/
        searchOnCategory(mSearchString);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::setSettings
// ---------------------------------------------------------------------------
void SearchProgressiveState::setSettings()
    {
    mTempSelectedCategory = mSelectedCategory;
    if (mOriginalString.length())
        emit inDeviceSearchQuery(mOriginalString);
    emit switchProToSettingsState();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::settingsaction
// ---------------------------------------------------------------------------
void SearchProgressiveState::settingsaction(bool avalue)
    {
    if (avalue && mStateStatus)
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
                    mTemplist.append("email");
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
    if (mTemplist.count() == 9)
        isrootsearch = true;
    if (isrootsearch)
        {
        mTemplist.clear();
        mTemplist.append("selectall");
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getAppIconFromAppId
// ---------------------------------------------------------------------------
HbIcon SearchProgressiveState::getAppIconFromAppIdL(TUid auid)
    {
    HbIcon icon;
    RApaLsSession apaLsSession;
    CleanupClosePushL(apaLsSession);
    User::LeaveIfError(apaLsSession.Connect());
    CApaAppServiceInfoArray* skinArray(NULL);
    TRAPD( err, skinArray = apaLsSession.GetAppServiceOpaqueDataLC(auid, TUid::Uid(0x2002DCF3));
            if (err == KErrNone && skinArray )
                {
                TArray<TApaAppServiceInfo> tmpArray( skinArray->Array() );
                if ( tmpArray.Count() )
                    {
                    TPtrC8 opaque(tmpArray[0].OpaqueData());
                    const TPtrC16 iconName((TText16*) opaque.Ptr(),(opaque.Length()+1)>>1);
                    icon = HbIcon( XQConversions:: s60DescToQString( iconName ) );
                    }
                }
            CleanupStack::PopAndDestroy(skinArray);
    );
    if (icon.isNull() || !(icon.size().isValid()))
        {
        TSize iconSize(mListViewIconSize.width(), mListViewIconSize.height());
        CApaMaskedBitmap* apaMaskedBitmap = CApaMaskedBitmap::NewLC();
        TInt err = apaLsSession.GetAppIcon(auid, iconSize, *apaMaskedBitmap);
        TInt iconsCount(0);
        apaLsSession.NumberOfOwnDefinedIcons(auid, iconsCount);
        QPixmap pixmap;
        if ((err == KErrNone) && (iconsCount > 0))
            {
            fromBitmapAndMaskToPixmapL(apaMaskedBitmap,
                    apaMaskedBitmap->Mask(), pixmap);
            pixmap = pixmap.scaled(mListViewIconSize,
                    Qt::KeepAspectRatioByExpanding);
            icon = HbIcon(QIcon(pixmap));
            }
        else
            {
            HBufC* fileNameFromApparc;
            TInt err2 = apaLsSession.GetAppIcon(auid, fileNameFromApparc);
            CleanupStack::PushL(fileNameFromApparc);
            if (err2 == KErrNone)
                {
                QString fileName = XQConversions::s60DescToQString(
                        fileNameFromApparc->Des());
                if (fileName.contains(QString(".mif")))
                    {
                    TPtr ptr(fileNameFromApparc->Des());
                    GetPixmapByFilenameL(ptr, mListViewIconSize, pixmap);
                    pixmap = pixmap.scaled(mListViewIconSize,
                            Qt::KeepAspectRatioByExpanding);
                    icon = HbIcon(QIcon(pixmap));
                    }
                }
            CleanupStack::Pop(fileNameFromApparc);
            }
        CleanupStack::PopAndDestroy(apaMaskedBitmap);
        }
    CleanupStack::PopAndDestroy(&apaLsSession);
    if (icon.isNull() || !(icon.size().isValid()))
        icon = HbIcon("qtg_large_application");
    return icon;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::GetPixmapByFilenameL
// ---------------------------------------------------------------------------
void SearchProgressiveState::GetPixmapByFilenameL(TDesC& fileName,
        const QSize &size, QPixmap& pixmap)
    {
    CFbsBitmap *bitamp(0);
    CFbsBitmap *mask(0);
    if (AknIconUtils::IsMifFile(fileName))
        {
        // SVG icon
        // SVG always has only one icon
        TInt bitmapIndex = 0;
        TInt maskIndex = 1;
        AknIconUtils::ValidateLogicalAppIconId(fileName, bitmapIndex,
                maskIndex);

        AknIconUtils::CreateIconLC(bitamp, mask, fileName, bitmapIndex,
                maskIndex);
        }
    AknIconUtils::DisableCompression(bitamp);
    AknIconUtils::SetSize(bitamp, TSize(size.width(), size.height()),
            EAspectRatioPreservedAndUnusedSpaceRemoved);
    AknIconUtils::DisableCompression(mask);
    AknIconUtils::SetSize(mask, TSize(size.width(), size.height()),
            EAspectRatioPreservedAndUnusedSpaceRemoved);
    fromBitmapAndMaskToPixmapL(bitamp, mask, pixmap);
    // bitmap and icon, AknsUtils::CreateIconLC doesn't specify the order
    CleanupStack::Pop(2);
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
// SearchProgressiveState::fromBitmapAndMaskToPixmapL
// ---------------------------------------------------------------------------
void SearchProgressiveState::fromBitmapAndMaskToPixmapL(
        CFbsBitmap* fbsBitmap, CFbsBitmap* fbsMask, QPixmap& pixmap)
    {
    if (fbsBitmap->Header().iCompression == ENoBitmapCompression)
        {
        pixmap = fromSymbianCFbsBitmap(fbsBitmap);
        QPixmap mask = fromSymbianCFbsBitmap(fbsMask);
        pixmap.setAlphaChannel(mask);
        }
    else
        { // we need special handling for icons in 9.2 (NGA)
        // let's hope that in future it will be in QT code
        CFbsBitmap *temp(NULL);
        temp = copyBitmapLC(fbsBitmap);
        pixmap = fromSymbianCFbsBitmap(temp);
        CleanupStack::PopAndDestroy();
        temp = copyBitmapLC(fbsMask);
        QPixmap mask = fromSymbianCFbsBitmap(temp);
        CleanupStack::PopAndDestroy();
        pixmap.setAlphaChannel(mask);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::filterDoc
// ---------------------------------------------------------------------------
QString SearchProgressiveState::filterDoc(const CpixDocument* aDoc,
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
QStringList SearchProgressiveState::filterDoc(const CpixDocument* aDoc,
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
PERF_RESULT_ITEM_LAUNCH_TIME_ENDLOG    ("")
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::_viewingCompleted
// ---------------------------------------------------------------------------
void SearchProgressiveState::_viewingCompleted()
    {
    if (calAgandaViewerPluginInstance)
        calAgandaViewerPluginInstance->deleteLater();
PERF_RESULT_ITEM_LAUNCH_TIME_ENDLOG    ("")
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::viewReady
// ---------------------------------------------------------------------------
void SearchProgressiveState::viewReady()
    {

    if (hbApp)
        {
        if (hbApp->activateReason() == Hb::ActivationReasonActivity)
            {
            QVariantHash params = hbApp->activateParams();
            QString searchKey = params.value("query").toString();
            params.remove("query");
            params.remove("activityname");
            QList<QVariant> list = params.values();
            mTemplist.clear();
            for (int i = 0; i < list.count(); i++)
                {
                QString str = list.at(i).toString();
                if (!str.isNull())
                    mTemplist.append(str);
                }
            if (searchKey.length() > 0)
                mSearchPanel->setCriteria(searchKey);
            }
        }PERF_APP_LAUNCH_END("SearchAppplication View is ready");
    emit applicationReady();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::slotOnlineQuery
// ---------------------------------------------------------------------------
void SearchProgressiveState::slotOnlineQuery(QString str)
    {
    if (mOriginalString != str)
        {
        mOriginalString = str;
        mOnlineQueryAvailable = true;
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::slotISProvidersIcon
// ---------------------------------------------------------------------------
void SearchProgressiveState::slotISProvidersIcon(int id, HbIcon icon)
    {
    mISprovidersIcon.insert(id, icon);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::slotPrepareResultIcons
// ---------------------------------------------------------------------------
void SearchProgressiveState::slotPrepareResultIcons()
    {
    //Icon creation in array
    QStringList icons;
    icons << "qtg_large_phonebook" << "qtg_large_tone" << "qtg_large_video"
            << "qtg_large_photos" << "qtg_large_message" << "qtg_large_email"
            << "qtg_large_calendar" << "qtg_large_notes"
            << "qtg_large_web_link" << "qtg_large_text" << "qtg_large_sisx"
            << "qtg_large_java" << "qtg_large_flash" << "qtg_large_folder"
            << "qtg_large_query";
    mIconArray.clear();
    for (int i = 0; i < icons.count(); i++)
        {
        HbIcon icon(icons.at(i));
        if (icon.isNull() || !(icon.size().isValid()))
            icon = HbIcon("qtg_large_application");
        mIconArray.append(icon);
        }
    /*QList<TUid> appUid;
     appUid.append(TUid::Uid(0x20022EF9));//contact
     appUid.append(TUid::Uid(0x10207C62));//audio
     appUid.append(TUid::Uid(0x200211FE));//video 
     appUid.append(TUid::Uid(0x20000A14));//image 
     appUid.append(TUid::Uid(0x2001FE79));//msg
     appUid.append(TUid::Uid(0x200255BA));//email 
     appUid.append(TUid::Uid(0x10005901));//calender
     appUid.append(TUid::Uid(0x20029F80));//notes
     //appUid.Append(TUid::Uid(0x20022F35));//application
     appUid.append(TUid::Uid(0x10008D39));//bookmark
     appUid.append(TUid::Uid(0x2002BCC0));//files

     for (int i = 0; i < appUid.count(); i++)
     {
     TRAP_IGNORE(mIconArray.append(getAppIconFromAppIdL(appUid.at(i))));
     }*/
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::activityRequested
// ---------------------------------------------------------------------------
void SearchProgressiveState::activityRequested(const QString &name)
    {
    /* when search application is launched in normal and then supporting for activity uri
     * for normal search launching "media" is used instead of "media image","media audio","media video" 
     */
    if (mSearchHandlerList.count() != totalcategories_activityreasonreason)
        {
        InDeviceHandler* handler = NULL;
        handler->setCategory("media image");
        mSearchHandlerList.append(handler);
        handler->setCategory("media audio");
        mSearchHandlerList.append(handler);
        handler->setCategory("media video");
        mSearchHandlerList.append(handler);
        }
    if (name == "SearchDeviceQueryView")
        {
        QVariantHash params = hbApp->activateParams();
        QString searchKey = params.value("query").toString();
        int i = params.count();
        params.remove("query");
        params.remove("activityname");
        QList<QVariant> list = params.values();
        int j = list.count();
        mTemplist.clear();
        for (int i = 0; i < list.count(); i++)
            {
            QString str = list.at(i).toString();
            if (!str.isNull())
                mTemplist.append(str);
            }
        if (searchKey.length() > 0)
            mSearchPanel->setCriteria(searchKey);
        }
    }
