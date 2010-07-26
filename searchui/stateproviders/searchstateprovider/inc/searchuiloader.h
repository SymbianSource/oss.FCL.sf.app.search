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
 * Description:  Declaration of the Ui Loader .
 *
 */
#ifndef SEARCH_CONTROLLER_H
#define SEARCH_CONTROLLER_H

#include "search_global.h"
#include <qobject.h>
#include <qmutex.h>

class HbDocumentLoader;
class HbView;
class HbListWidget;
class HbSearchPanel;
class TsTaskSettings;
class HbMainWindow;
class HbShrinkingVkbHost;

SEARCH_CLASS( SearchStateProviderTest)

class SearchUiLoader : public QObject
    {
Q_OBJECT
public:

    /**
     * Singleton construtor
     * @since S60 ?S60_version.
     */
    static SearchUiLoader* instance()
        {
        static QMutex mutex;
        if (!m_instance)
            {
            mutex.lock();
            m_instanceCounter++;
            if (!m_instance)
                {
                m_instance = new SearchUiLoader;
                }
            mutex.unlock();
            }

        return m_instance;
        }

    /**
     * Singleton destructor
     * @since S60 ?S60_version.
     */
    static void deleteinstance()
        {
        m_instanceCounter--;
        if ((m_instanceCounter <= 0) && (m_instance))
            {
            delete m_instance;
            m_instance = 0;
            }
        }
public slots:

    /**
     * Slot to send the application to background fake exit
     * @since S60 ?S60_version.
     */
    void slotsendtobackground();   

public:

    /**
     * Function returns the HbListWidget object 
     *  @param aUid Unique app Id.
     */
    HbView* View()
        {
        return mView;
        }

    /**
     * Function returns the HbListWidget object
     *  @param aUid Unique app Id.
     */
    HbListWidget* ListWidget()
        {
        return mListWidget;
        }

    /**
     * Function returns the HbSearchPanel object
     *  @param aUid Unique app Id.
     */
    HbSearchPanel* SearchPanel()
        {
        return mSearchPanel;
        }

    /**
     * Function to listen the applicaition foreground event
     *  @param aUid Unique app Id.
     */
    bool eventFilter(QObject *, QEvent *);

private:
    /**
     * Constructor.
     * @since S60 ?S60_version.     
     * @param aParent Owner.
     */
    SearchUiLoader();
    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~SearchUiLoader();
private:

    /**
     * static instance to make single instance
     * Own.
     */
    static SearchUiLoader *m_instance;

    /**
     * counter for number for instance
     * Own.
     */
    static int m_instanceCounter;

    /**
     * Doucument loader API
     * Own.
     */
    HbDocumentLoader* mDocumentLoader;

    /**
     * Application view 
     * Own.
     */
    HbView* mView;

    /**
     * Listwidget to show the search results
     * Own.
     */
    HbListWidget* mListWidget;

    /**
     * searchpanel for query
     * Own.
     */
    HbSearchPanel* mSearchPanel;

    /**
     * to hide for task switcher
     * Own.
     */
    TsTaskSettings* mClient;

    /**
     * application main window
     */
    HbMainWindow* mMainWindow;

    /**
     * vkbhost to resize the result screen 
     * Own.
     */
    HbShrinkingVkbHost* mVirtualKeyboard;
private:
SEARCH_FRIEND_CLASS    ( SearchStateProviderTest)
    };

#endif //SEARCH_CONTROLLER_H
