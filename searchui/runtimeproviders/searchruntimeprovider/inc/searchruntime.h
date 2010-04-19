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
 * Description:  Declaration of the SEARCH default runtime.
 *
 */

#ifndef SEARCH_RUNTIMES_DEFAULTRUNTIME_H
#define SEARCH_RUNTIMES_DEFAULTRUNTIME_H

#include "Search_global.h"
#include "hsruntime.h"
#include <qhash.h>

SEARCH_CLASS(SearchRuntimeProviderTest)

// Forward declarations
class QStateMachine;
class HbMainWindow;

/**  @ingroup group_Searchruntimeprovider
 * @brief Default implementation of the search runtime.
 *
 * Creates an execution context (EC) and populates it with
 * runtime services. States are loaded from state provider
 * plugins. Each state is given an access to the EC. States
 * are added to a state machine. Finally, the state machine
 * is started.
 *
 * @lib ?library
 * @since S60 ?S60_version
 */
class SearchRuntime : public HsRuntime
    {
Q_OBJECT

public:

    /**
     * Constructor.
     * @since S60 ?S60_version.
     * @param parent - address of parent QObject instance, by default none
     */
    SearchRuntime(QObject* = 0);

    /**
     * Destructor.
     * @since S60 ?S60_version.
     */
    ~SearchRuntime();

private:
    /**
     * Copy constructor
     */
    Q_DISABLE_COPY(SearchRuntime)

public slots:

    /**
     * @copydoc HsRuntime::start()
     */
    void start();

    /**
     * @copydoc HsRuntime::stop()
     */
    void stop();

private slots:

    /**
     * Called after the state machine has started.
     * @since S60 ?S60_version.
     */
    void handleStateMachineStarted();

    /**
     * Called after the state machine has stopped.
     * @since S60 ?S60_version.
     */
    void handleStateMachineStopped();

signals:

    /**
     * Emission of this signal initiates a transition to the final state.     
     */
    void stopStateMachine();

private:

    /**
     * Creates gui service parts
     * @since S60 ?S60_version.
     */
    void createGuiServiceParts();

    /**
     * Creates states
     * @since S60 ?S60_version.
     */
    void createStates();

private:

    /**
     * Address of state machine instance.
     * Ownership tranfered to application. Don't remove it.
     */
    QStateMachine *mStateMachine;

    /**
     * Window to visualize scene
     */
    HbMainWindow *mWindow;

    /**
     * Contains address of runtime service.
     * Ownership transfered to application. Don't remove it.
     */
    QHash<QString, QObject*> mServices;

SEARCH_FRIEND_CLASS    (SearchRuntimeProviderTest)

    };

#endif // SEARCH_RUNTIMES_DEFAULTRUNTIME_H
