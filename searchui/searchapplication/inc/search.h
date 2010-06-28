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
 * Description:  search application main class.
 *
 */

#ifndef SEARCHAPPLICATION_H
#define SEARCHAPPLICATION_H

#include "search_global.h"

#include <qobject.h>

class QStateMachine;

/**
 * @ingroup group_searchsearchlication
 * @brief Search application main class.
 * 
 * Loads a runtime from a runtime provider plugin. Manages 
 * the runtime execution.
 *
 * @lib ?library
 * @since S60 ?S60_version
 */
class Search : public QObject
    {
Q_OBJECT

public:

    /**
     * Constructor.
     * @since S60 ?S60_version.     
     * @param aParent Parent object.
     */
    Search(QObject* aParent = 0);

    /**
     * Destructor.
     * @since S60 ?S60_version.     
     */
    virtual ~Search();

private:   
    
    Q_DISABLE_COPY(Search)

signals:

    /**
     * Emitted when the search application needs to exit.
     * @since S60 ?S60_version.     
     */
    void exit();

public slots:

    /**
     * Starts the runtime.
     * @since S60 ?S60_version. 
     */
    void start();

    /**
     * Stops the runtime.
     * @since S60 ?S60_version.     
     */
    void stop();

private slots:

    /**
     * Called after the runtime has started.
     * @since S60 ?S60_version.     
     */
    void handleRuntimeStarted();

    /**
     * Called after the runtime has stopped.
     * @since S60 ?S60_version.     
     */
    void handleRuntimeStopped();

    /**
     * Called after the runtime has faulted.
     * @since S60 ?S60_version.     
     */
    void handleRuntimeFaulted();

private:

    /**
     * Runtime.
     */
    QStateMachine* mRuntime;
    };

#endif//SEARCHAPPLICATION_H
