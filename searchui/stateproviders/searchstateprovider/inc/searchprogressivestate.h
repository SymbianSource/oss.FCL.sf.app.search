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
 * Description:  Declaration of the progressive search state.
 *
 */

#ifndef PROGRESSIVE_SEARCH_STATE_H
#define PROGRESSIVE_SEARCH_STATE_H

#include <qabstractitemmodel.h>
#include <qstate.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <hbicon.h>
#include <displaymode.h>
#include "Search_global.h"

class HbMainWindow;
class HbView;
class HbListView;
class HbDocumentLoader;
class QStandardItemModel;
class HbSearchPanel;
class QSortFilterProxyModel;

class CFbsBitmap;

class InDeviceHandler;
class QCPixDocument;

SEARCH_CLASS( SearchStateProviderTest)
/** @ingroup group_searchstateprovider
 * @brief The state where progressive search state is shown
 *
 * @see StateMachine
 *
 * @lib ?library
 * @since S60 ?S60_version
 */
class SearchProgressiveState : public QState
    {
Q_OBJECT

public:

    /**
     * Constructor.
     * @since S60 ?S60_version.     
     * @param aParent Owner.
     */
    SearchProgressiveState(QState *aParent = 0);

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    virtual ~SearchProgressiveState();

protected:

    /**
     * @copydoc QState::onEntry()
     */
    void onEntry(QEvent *event);

    /**
     * @copydoc QState::onExit()
     */
    void onExit(QEvent *event);

private:
    /**
     * deactivates the signals .
     * @since S60 ?S60_version.
     */
    void deActivateSignals();

    /**
     * activates the signals .
     * @since S60 ?S60_version.
     */
    void activateSignals();

    /**
     * get the application icon  .
     * @since S60 ?S60_version.
     */
    HbIcon getAppIconFromAppId(TUid auid);

    /**
     * resizing the symbain icon  .
     * @since S60 ?S60_version.
     */
    CFbsBitmap *copyBitmapLC(CFbsBitmap *input);
public slots:

    /**
     * slot connects to CSearchHandler to get the status of search result synchronously 
     * @since S60 ?S60_version.
     * @param aError error code.
     * @param aResultCount number of results
     */
    void onSearchComplete(int aError, int aResultCount);

    /**
     * slot connects to CSearchHandler to get the status of search result asynchronously 
     * @since S60 ?S60_version.
     * @param aError error code.
     * @param aResultCount number of results
     */
    void onAsyncSearchComplete(int aError, int aResultCount);

    /**
     * slot connects to CSearchHandler to get the result item asynchronously 
     * @since S60 ?S60_version.
     * @param aError error code.
     * @param aDoc result item
     */
    void onGetDocumentComplete(int aError, QCPixDocument* aDoc);

    /**
     * slot connects to list view to launch the respective application
     * @since S60 ?S60_version.
     * @param aIndex index of the activated item.
     */
    void openResultitem(QModelIndex aIndex);

    /**
     * slot connects to settings state to get the selected category information
     * @since S60 ?S60_version.
     * @param aCategory category(database) name.
     * @param aStatus whether the category selected or not
     */
    void getSettingCategory(int, bool);

    /**
     * slot connects to action to change the current state to settings state
     * @since S60 ?S60_version. 
     */
    void setSettings();

    /**
     * slot connects to search panel to initiate the fresh search
     * @since S60 ?S60_version.
     * @param aKeyword search keyword.
     */
    void startNewSearch(const QString &aKeyword);

    /**
     * slot connects to search state  for internet search
     * @since S60 ?S60_version.
     */
    void _customizeGoButton(bool avalue);

    /**
     * slot connects to search state  for internet search
     * @since S60 ?S60_version.
     */
    void settingsaction(bool avalue);

    /**
     * slot connects to search state  for internet search
     * @since S60 ?S60_version.
     */
    void cancelSearch();

private:

    /**
     * initiate the fresh search for selected category separately 
     * 
     * @param aKeyword search keyword.
     */
    void searchOnCategory(const QString aKeyword);

    /**
     * clears the model  
     *              
     */
    void clear();

    /**
     * prepares the item on result list view for no result case 
     * 
     * @param aKeyword search keyword.
     */
    void noResultsFound(QString aKeyword);

    /**
     * prepares the suggestion link item on result list view to provide suggestion links     
     * 
     * @param aKeyword search keyword.
     */
    void createSuggestionLink(bool aFlag);

    /**
     * Function to include corrrect Qimage format to be taken from bitmap
     *  @param mode Bitmap display mode.
     */
    QImage::Format TDisplayMode2Format(TDisplayMode mode);

    /**
     * Function to convert the s60 based CFbsBitmap into Qt specific QPixmap
     *  @param aBitmap Bitmap to be converted.
     */
    QPixmap fromSymbianCFbsBitmap(CFbsBitmap *aBitmap);

signals:

    /**
     * Signalled when user selects an to switch the settings state
     * setting state will be  activated.
     */
    void settingsState();

private:

    HbMainWindow* mMainWindow;

    /**
     * main view.
     * Own.
     */
    HbView* mView;

    /**
     * The List View widget.
     * Own.
     */
    HbListView* mListView;

    /**
     * Document handler to load .docml.
     * Own.
     */
    HbDocumentLoader* mDocumentLoader;

    /**
     * The searchpanel widget.
     * Own.
     */
    HbSearchPanel* mSearchPanel;
    /**
     * proxymodel for list view
     * Own.
     */
    QSortFilterProxyModel *proxyModel;

    /**
     * model for list view
     * Own.
     */
    QStandardItemModel* mModel;

    /**
     * qt interface for CPix engine
     * Own.
     */
    InDeviceHandler* mSearchHandler;

    /**
     * Search Keyword                        
     */

    QString mSearchString;

    /**
     * Search Keyword                        
     */

    QString mOriginalString;

    /**
     * selected categories on a map                        
     */
    QMap<int, bool> mSelectedCategory;

    /**
     * save the previous selected categories,decision to search again                        
     */
    QMap<int, bool> mTempSelectedCategory;

    /**
     * temporary list of selected categories
     */
    QStringList mTemplist;

    /**
     * number of categories selected
     * 
     */
    int mDatabasecount;

    /**
     * index of link item
     */
    int mLinkindex;

    /**
     * number of hits
     */
    int mResultcount;

    /**
     * result parser 
     * 
     */
    int mResultparser;

    /**
     * setting loaded or not variable
     * 
     */
    bool loadSettings;
    QList<HbIcon> mIconArray;
    HbIcon mIcon;

SEARCH_FRIEND_CLASS    (SearchStateProviderTest)

    };

#endif //PROGRESSIVE_SEARCH_STATE_H
