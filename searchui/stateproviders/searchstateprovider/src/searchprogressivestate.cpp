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
#include "qcpixdocument.h"
#include "qcpixdocumentfield.h"
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
#include <xqservicerequest.h>
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

const char *SEARCHSTATEPROVIDER_DOCML = ":/xml/searchstateprovider.docml";
const char *TOC_VIEW = "tocView";
const char *TUT_SEARCHPANEL_WIDGET = "searchPanel";
const char *TUT_LIST_VIEW = "listView";
const QSize defaultIconSize(30, 30);

// ---------------------------------------------------------------------------
// SearchProgressiveState::SearchProgressiveState
// ---------------------------------------------------------------------------
//
SearchProgressiveState::SearchProgressiveState(QState *parent) :
    QState(parent), mMainWindow(NULL), mView(NULL), mListView(NULL),
            mDocumentLoader(NULL), mModel(NULL), mSearchHandler(NULL)
    {

    mMainWindow = hbInstance->allMainWindows().at(0);
    mModel = new QStandardItemModel(this);

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(mModel);

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
            prototypeListView->setTextFormat(Qt::RichText);

            }
        mListView->setModel(mModel, prototype);
        }
    //bool bl = mListView->itemRecycling();
    // mListView->setItemRecycling(false);
    //mListView->setModel(proxyModel);

    mSearchPanel = qobject_cast<HbSearchPanel *> (
            mDocumentLoader->findWidget(TUT_SEARCHPANEL_WIDGET));
    if (mSearchPanel)
        {
        mSearchPanel->setSearchOptionsEnabled(true);
        }

    mSearchHandler = new InDeviceHandler();
    if (mView)
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

    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::~SearchProgressiveState
// ---------------------------------------------------------------------------
//
SearchProgressiveState::~SearchProgressiveState()
    {
    if (proxyModel)
        {
        delete proxyModel;
        }

    if (mModel)
        {
        delete mModel;
        }
    if (mDocumentLoader)
        {
        delete mDocumentLoader;
        }
    if (mSearchHandler)
        {
        delete mSearchHandler;
        }

    }

// ---------------------------------------------------------------------------
// SearchProgressiveState::onEntry
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::onEntry(QEvent *event)
    {
    //  WMS_LOG << "::onEntry";
    QState::onEntry(event);

    activateSignals();

    // If this is not the current view, we're getting back from plugin view  

    if (mMainWindow->currentView() != mView)
        {
        mMainWindow->setCurrentView(mView, true);
        }
    mMainWindow->show();

    if (loadSettings)
        {
        emit settingsState();
        loadSettings = false;
        }

    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onExit
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::onExit(QEvent *event)
    {
    QState::onExit(event);
    deActivateSignals();
    }

// ---------------------------------------------------------------------------
// SearchProgressiveState::activateSignals
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::activateSignals()
    {
    if (mSearchHandler)
        {
        connect(mSearchHandler, SIGNAL(handleSearchResult(int,int)), this,
                SLOT(onSearchComplete(int,int)));
        connect(mSearchHandler, SIGNAL(handleAsyncSearchResult(int,int)),
                this, SLOT(onAsyncSearchComplete(int,int)));
        connect(mSearchHandler, SIGNAL(handleDocument(int,QCPixDocument*)),
                this, SLOT(onGetDocumentComplete(int,QCPixDocument*)));
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
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::deActivateSignals
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::deActivateSignals()
    {
    if (mSearchHandler)
        {
        disconnect(mSearchHandler, SIGNAL(handleSearchResult(int,int)), this,
                SLOT(onSearchComplete(int,int)));
        disconnect(mSearchHandler, SIGNAL(handleAsyncSearchResult(int,int)),
                this, SLOT(onAsyncSearchComplete(int,int)));
        disconnect(mSearchHandler,
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
        }
    }

// ---------------------------------------------------------------------------
// SearchProgressiveState::onSearchComplete
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::onSearchComplete(int aError, int aResultCount)
    {
    if (aError != 0)
        {
        //Error found
        return;
        }
    if (aResultCount > 0)
        {
        mResultcount = aResultCount;
        mResultparser = 0;
        mSearchHandler->getDocumentAsyncAtIndex(mResultparser);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onAsyncSearchComplete
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::onAsyncSearchComplete(int aError,
        int aResultCount)
    {

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

        mSearchHandler->getDocumentAsyncAtIndex(mResultparser);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::onGetDocumentComplete
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::onGetDocumentComplete(int aError,
        QCPixDocument* aDoc)
    {
    if (aDoc == NULL || aError != 0)
        return;
    QStringList liststr;
    QString secondrow = aDoc->excerpt();
    QString baseclass = aDoc->baseAppClass();
    QString Uid = aDoc->docId();
    QString firstrow;

    //-------------- html tag creation-------------------
    QString htmlTagPre = QString("<u>");
    QString htmlTagPost = QString("</u>");
    int insertpt = secondrow.indexOf(mOriginalString, 0, Qt::CaseInsensitive);
    if (insertpt >= 0)
        {
        int preTagLen = htmlTagPre.length();

        secondrow.insert(insertpt, htmlTagPre);

        secondrow.insert(insertpt + mOriginalString.length() + preTagLen,
                htmlTagPost);
        //--------------------Html Tag Creation completed------------
        }

    QStandardItem* listitem = new QStandardItem();

    if (aDoc->baseAppClass().contains("contact"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("GivenName"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(0), Qt::DecorationRole);

        }
    else if (aDoc->baseAppClass().contains("audio"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Name"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(1), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("video"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Name"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(2), Qt::DecorationRole);

        }
    else if (aDoc->baseAppClass().contains("image"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Name"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(3), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("msg"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Subject"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(4), Qt::DecorationRole);

        }
    else if (aDoc->baseAppClass().contains("email"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("GivenName"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        }
    else if (aDoc->baseAppClass().contains("calendar"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Description"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(6), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("notes"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Memo"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        }
    else if (aDoc->baseAppClass().contains("applications"))
        {

        liststr << secondrow;
        bool ok;
        listitem->setData(getAppIconFromAppId(TUid::Uid(aDoc->docId().toInt(
                &ok, 16))), Qt::DecorationRole);
        }
    else if (aDoc->baseAppClass().contains("bookmark"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("Name"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        }
    else if (aDoc->baseAppClass().contains("file"))
        {
        int index = aDoc->fieldCount();
        for (int i = 0; i < index; i++)
            {
            if (aDoc->field(i).name().contains("FullName"))
                {
                firstrow = aDoc->field(i).value();
                break;
                }
            }
        if (firstrow.length() == 0)
            firstrow = baseclass;
        liststr << firstrow << secondrow;
        listitem->setData(mIconArray.at(9), Qt::DecorationRole);
        }

    listitem->setData(liststr, Qt::DisplayRole);
    listitem->setData(aDoc->docId(), Qt::UserRole);
    listitem->setData(aDoc->baseAppClass(), Qt::UserRole + 1);

    mModel->appendRow(listitem);
    mResultparser++;
    if (mResultparser < mResultcount)
        {

        mSearchHandler->getDocumentAsyncAtIndex(mResultparser);
        }
    else
        {

        searchOnCategory(mSearchString);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::_selectedcategory
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::getSettingCategory(int item, bool avalue)
    {

    mSelectedCategory.insert(item, avalue);

    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::openResultitem
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::openResultitem(QModelIndex index)
    {
    QStandardItem* item = mModel->itemFromIndex(index);
    QString baseclass = item->data(Qt::UserRole + 1).toString();
    bool t;
    int uid = (item->data(Qt::UserRole)).toInt(&t);

    if (index.row() == mLinkindex)
        {
        // emit linkItemClicked();
        }
    else
        {
        XQServiceRequest *mSndEdit;
        QVariant retValue;

        if (baseclass.contains("contact"))
            {
            mSndEdit = new XQServiceRequest(
                    "com.nokia.services.phonebookservices.Fetch",
                    "open(int)", false);
            *mSndEdit << uid;
            retValue = mSndEdit->send();
            }
        else if (baseclass.contains("msg"))
            {
            mSndEdit = new XQServiceRequest(
                    "com.nokia.services.hbserviceprovider.conversationview",
                    "openConversationView(qint64,int)", false);
            *mSndEdit << uid << 0;
            retValue = mSndEdit->send();
            }
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::createSuggestionLink
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::createSuggestionLink(bool aFlag)
    {
    QStandardItem* listitem = new QStandardItem();
    /*   QString displaystr;
     QString searchStr(mSearchString);*/
    QString htmlKeyword = QString("<u>%1</u>").arg(mOriginalString);
    /*  QString htmlServiceProvider = QString("in Google");*/

    if (!aFlag)
        {
        QString linkString = QString(hbTrId(
                "txt_search_list_search_for_1_on_2").arg(htmlKeyword));
        mLinkindex = mModel->rowCount();
        listitem->setData(linkString, Qt::DisplayRole);
        /*   HbFrameBackground* background = new HbFrameBackground(
         "qtg_fr_button_function_latched", HbFrameDrawer::NinePieces);
         listitem->setData(*background, Qt::BackgroundRole);
         */
        mModel->appendRow(listitem);
        }

    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::noResultsFound
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::noResultsFound(QString aKeyword)
    {
    if (aKeyword.length())
        {
        QStandardItem* listitem = new QStandardItem();
        QString noResultMsg = QString("<align=\"center\">" + hbTrId(
                "txt_search_results_no_match_found"));
        listitem->setData(noResultMsg, Qt::DisplayRole);
        mModel->appendRow(listitem);
        }

    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::noResultsFound
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::clear()
    {
    if (mModel)
        {
        mModel->clear();
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::noResultsFound
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::searchOnCategory(const QString aKeyword)
    {
    mResultparser = 0;
    mResultcount = 0;
    if (mDatabasecount < mTemplist.count())
        {
        QString str = mTemplist.at(mDatabasecount);
        mSearchHandler->setCategory(mTemplist.at(mDatabasecount));
        mDatabasecount++;
        if (mSearchHandler->isPrepared())
            {

            mSearchHandler->searchAsync(aKeyword, "_aggregate");
            }
        }
    else if (mDatabasecount >= mTemplist.count())
        {
        if (mListView->indexCount() == 0 && aKeyword.length() != 0)
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
//
void SearchProgressiveState::startNewSearch(const QString &aKeyword)
    {

    if (mSearchHandler)
        {
        mSearchHandler->cancelLastSearch();
        clear();
        }    
    mOriginalString = aKeyword.trimmed();
    if (mOriginalString.length())
        {
        mDatabasecount = 0;
        mTemplist.clear();
        mSearchString = mOriginalString;
        mSearchString.append('*');
        QMapIterator<int, bool> i(mSelectedCategory);
        while (i.hasNext())
            {
            i.next();
            if (i.value())
                {
                switch (i.key())
                    {
                    case 0:
                        {
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
                        //mTemplist.append("notes");
                        break;
                        }
                    case 5://Applications
                        {
                        mTemplist.append("applications");
                        break;
                        }
                    case 6://All other files
                        {
                        mTemplist.append("file");
                        mTemplist.append("bookmark");
                        break;
                        }
                    }
                }
            }
        searchOnCategory(mSearchString);
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::setSettings
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::setSettings()
    {
    mTempSelectedCategory = mSelectedCategory;
    emit settingsState();
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::_customizeGoButton
// ---------------------------------------------------------------------------
//
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
//
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
            if (i.value() == j.value())
                {

                }
            else
                {
                startNewSearch(mOriginalString);
                break;
                }
            }
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::cancelSearch
// ---------------------------------------------------------------------------
//
void SearchProgressiveState::cancelSearch()
    {
    if(mSearchHandler)
        {
        mSearchHandler->cancelLastSearch();
        }
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::getAppIconFromAppId
// ---------------------------------------------------------------------------
//
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
            aknIcon = CAknIcon::NewL();
            aknIcon->SetBitmap(bitmap);
            aknIcon->SetMask(mask);
            }
        }
    if (aknIcon)
        {
        CleanupStack::PushL(aknIcon);

        //need to disable compression to properly convert the bitmap
        AknIconUtils::DisableCompression(aknIcon->Bitmap());

        AknIconUtils::SetSize(aknIcon->Bitmap(), TSize(
                defaultIconSize.width(), defaultIconSize.height()),
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
            temp = copyBitmapLC(aknIcon->Bitmap());
            pixmap = fromSymbianCFbsBitmap(temp);
            CleanupStack::PopAndDestroy();

            temp = copyBitmapLC(aknIcon->Mask());
            QPixmap mask = fromSymbianCFbsBitmap(temp);
            CleanupStack::PopAndDestroy();

            pixmap.setAlphaChannel(mask);
            }

        pixmap = pixmap.scaled(defaultIconSize,
                Qt::KeepAspectRatioByExpanding);
        CleanupStack::PopAndDestroy(aknIcon);
        icon = HbIcon(QIcon(pixmap));
        }
    return icon;
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::TDisplayMode2Format
// ---------------------------------------------------------------------------
//
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
//
QPixmap SearchProgressiveState::fromSymbianCFbsBitmap(CFbsBitmap *aBitmap)
    {
    aBitmap->BeginDataAccess();
    uchar *data = (uchar *) aBitmap->DataAddress();
    TSize size = aBitmap->SizeInPixels();
    TDisplayMode displayMode = aBitmap->DisplayMode();

    // QImage format must match to bitmap format
    QImage image(data, size.iWidth, size.iHeight, TDisplayMode2Format(
            displayMode));
    aBitmap->EndDataAccess();

    // No data copying happens because image format matches native OpenVG format.
    // So QPixmap actually points to CFbsBitmap data.
    return QPixmap::fromImage(image);
    }
// ---------------------------------------------------------------------------
// SearchProgressiveState::copyBitmapLC
// ---------------------------------------------------------------------------
//
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
