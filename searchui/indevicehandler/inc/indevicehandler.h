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
 * Description:  Declaration of the inDeviceHandler.
 *
 */
#ifndef INDEVICEHANDLER_H
#define INDEVICEHANDLER_H

#include <QtCore>
#include <qcoreapplication.h>
#include <qdebug.h>
#include "Search_global.h"

#ifdef BUILD_INDEVICEHANDLER
#define SEARCH_INDEVICEHANDLER Q_DECL_EXPORT
#else
#define SEARCH_INDEVICEHANDLER Q_DECL_IMPORT
#endif

#ifndef DEFAULT_SEARCH_FIELD
#define DEFAULT_SEARCH_FIELD "_aggregate"
#endif

class QCPixSearcher;
class QCPixDocument;

SEARCH_CLASS(SearchInDeviceHandlerTest)

/**
 * InDeviceHandler is an interface class for UI and
 * CPix engine.
 */
class SEARCH_INDEVICEHANDLER InDeviceHandler : public QObject
    {
Q_OBJECT
public:

    /**
     * Constructor.
     * @since S60 ?S60_version.
     */
    InDeviceHandler();

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~InDeviceHandler();

signals:

    /**
     * Signalled when search completes synchronusly 
     * error and number of results will be returned    
     */
    void handleSearchResult(int, int);

    /**
     * Signalled when search completes unsynchronusly
     * error and number of results will be returned     
     */
    void handleAsyncSearchResult(int, int);

    /**
     * Signalled when user requests for the result item
     * error and result item will be returned
     */
    void handleDocument(int aError, QCPixDocument* aDoc);

private slots:

    /**
     * slot connects to QCPixSearcher to get the status of search results 
     * @since S60 ?S60_version.
     * @param aError error code.
     * @param aEstimatedResultCount number of results
     */

    void getSearchResult(int aError, int aEstimatedResultCount);

    /**
     * slot connects to QCPixSearcher to get the result item 
     * @since S60 ?S60_version.
     * @param aError error code.
     * @param aDocument result item
     */
    void getDocumentAsync(int aError, QCPixDocument* aDocument);

public:

    /**
     * returns the result item synchronously.
     * @since S60 ?S60_version.
     * @param aIndex index 
     */
    QCPixDocument* getDocumentAtIndex(int aIndex);

    /**
     * returns the result item asynchronously.
     * @since S60 ?S60_version.
     * @param aIndex index 
     */
    void getDocumentAsyncAtIndex(int aIndex);

    /**
     * initate the search synchronously.
     * @since S60 ?S60_version.
     * @param aSearchString search keyword 
     */
    void search(QString aSearchString);

    /**
     * initate the search asynchronously.
     * @since S60 ?S60_version.
     * @param aSearchAsyncString search keyword
     * @param aDefaultSearchField search criteria
     */
    void searchAsync(QString aSearchAsyncString, QString aDefaultSearchField =
            NULL);

    /**
     * cancels the outstanding search
     * @since S60 ?S60_version.
     */
    void cancelLastSearch();

    /**
     * returns the number of results on current search
     * @since S60 ?S60_version.
     */
    int getSearchResultCount();

    /**
     * prepare the search on specified category
     * @since S60 ?S60_version.
     * @param aCategory database name
     */
    void setCategory(QString aCategory);

    /**
     * return the status whether the specified category is set 
     * and ready for search
     * @since S60 ?S60_version.
     */
    bool isPrepared();

private:

    /**
     * interface for CPix.
     * Own.
     */
    QCPixSearcher *mSearchInterface;

    /**
     * number of results on current search.        
     */
    int mSearchResultCount;

SEARCH_FRIEND_CLASS    (SearchInDeviceHandlerTest)
    };

#endif //INDEVICEHANDLER_H
