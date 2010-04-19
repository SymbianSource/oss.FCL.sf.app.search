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
 * Description:  
 *
 */

#ifndef SEARCH_STATES_SETTINGS_H
#define SEARCH_STATES_SETTINGS_H

#include <qstate.h>
#include <qabstractitemmodel.h>
#include "Search_global.h"

SEARCH_CLASS( SearchStateProviderTest)

class QGraphicsWidget;
class HbMainWindow;
class HbLabel;
class HbStackedWidget;
class HbView;
class HbDocumentLoader;
class HbAction;
class HbGridView;
class QGraphicsLinearLayout;
class SettingsWidget;

/** @ingroup group_searchstateprovider
 * @brief The state handles wizard activation and interaction.
 *
 * @see StateMachine
 *
 * @lib ?library
 * @since S60 ?S60_version
 */
class SearchSettingsState : public QState
    {
Q_OBJECT
public:
    /*
     * The plugin's display mode enumeration.
     * FullScreen means menustrip is not shown.
     * PartialScreen means menustrip is shown.
     */
    enum PluginDisplayMode
        {
        FullScreen, PartialScreen
        };

public:

    /**
     * Constructor.
     * @since S60 ?S60_version.     
     * @param aParent Owner.
     */
    SearchSettingsState(QState *aParent = 0);

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    virtual ~SearchSettingsState();

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
     * Returns the main window handle.
     * @since S60 ?S60_version.
     * @return The pointer to the main window.
     */
    //  HbMainWindow* mainWindow();

    /**
     * Connects signals and slots for active wizard.
     * @since S60 ?S60_version.
     */

    void isInternetOn();
public slots:

    void getItemCategory(int, bool);

    /**
     * Signalled when back action is triggerd from toolbaar.
     * @since S60 ?S60_version.
     */
    void handleBackEvent(bool);

    /**
     * Function to customize the progressive state.
     * @since S60 ?S60_version.
     */
    //  void customizeGoButton();
signals:

    /**
     * Emitted when the menu state is to be activated.
     * @since S60 ?S60_version.
     */
    void backEventTriggered();
    
    /**
     * Emitted when ok/cancel clicked on settings page
     * @since S60 ?S60_version.
     */
    void clickstatus(bool);

    /**
     * Emitted when categories selected for searching
     * @since S60 ?S60_version.
     */
    void publishSelectedCategory(int, bool);

    /**
     * Emitted when internet search is selectead 
     * @since S60 ?S60_version.
     */
    void customizeGoButton(bool);

private:

    /**
     * HbMainWindow instance.
     */
    HbMainWindow* mMainWindow;

    /**
     * HbDocumentLoader instance.
     */
    HbDocumentLoader* mDocumentLoader;

    /**
     * settings widget
     * 
     */
    SettingsWidget* mWidget;

    bool minitialCount;
    /**
     * friend class for unit testing
     */
SEARCH_FRIEND_CLASS    (SearchStateProviderTest)
    };

#endif //SEARCH_STATES_SETTINGS_H
