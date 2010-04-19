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
 * Description:  Implementation of the SEARCH default runtime.
 *
 */

#include "searchruntime.h"
#include "Search_global.h"
#include "hsstatefactory.h"

#include <qstatemachine.h>
#include <qstate.h>
#include <qfinalstate.h>
#include <qdebug.h>

#include <hbmainwindow.h>

// plugin factory const
const char factoryManifestDir[] = "searchresources/plugins/stateproviders";
const char factoryPluginDir[] = "searchresources/plugins/stateproviders";
const char stateLibrary[] = "searchstateprovider.dll";

// states
const char wizardProgressiveStateUri[] =
        "search.nokia.com/state/wizardprogressivestate";
const char wizardSettingStateUri[] =
        "search.nokia.com/state/wizardsettingstate";

// ---------------------------------------------------------------------------
// searchRuntime::SearchRuntime
// ---------------------------------------------------------------------------
//
SearchRuntime::SearchRuntime(QObject* aParent) :
    HsRuntime(aParent), mStateMachine(0), mWindow(0)
    {
    mStateMachine = new QStateMachine(this);
    createGuiServiceParts();
    createStates();
    }

// ---------------------------------------------------------------------------
// SearchRuntime::~SearchRuntime()
// ---------------------------------------------------------------------------
//
SearchRuntime::~SearchRuntime()
    {
    delete mWindow;
    delete mStateMachine;
    }

// ---------------------------------------------------------------------------
// SearchRuntime::start()
// ---------------------------------------------------------------------------
//
void SearchRuntime::start()
    {
    mStateMachine->start();
    }

// ---------------------------------------------------------------------------
// SearchRuntime::stop()
// ---------------------------------------------------------------------------
//
void SearchRuntime::stop()
    {
    mStateMachine->stop();
    }

// ---------------------------------------------------------------------------
// SearchRuntime::handleStateMachineStarted()
// ---------------------------------------------------------------------------
//
void SearchRuntime::handleStateMachineStarted()
    {
    emit started();
    }

// ---------------------------------------------------------------------------
// SearchRuntime::handleStateMachineStopped()
// ---------------------------------------------------------------------------
//
void SearchRuntime::handleStateMachineStopped()
    {
    emit stopped();
    }

// ---------------------------------------------------------------------------
// SearchRuntime::createGuiServiceParts()
// ---------------------------------------------------------------------------
//
void SearchRuntime::createGuiServiceParts()
    {
    mWindow = new HbMainWindow();
    mWindow->show();
    }

// ---------------------------------------------------------------------------
// SearchRuntime::createStates()
// ---------------------------------------------------------------------------
//
void SearchRuntime::createStates()
    {
    HsStateFactory factory(factoryManifestDir, factoryPluginDir);
    HsStateToken token;

    QFinalState* finalState = new QFinalState();
    mStateMachine->addState(finalState);

    // parallel state activates all children states
    QState* parallel = new QState(QState::ParallelStates);
    mStateMachine->addState(parallel);
    parallel->addTransition(this, SIGNAL(stopStateMachine()), finalState);

    // root GUI state
    QState* guiRootState = new QState(parallel);

    QState* searchRootState = new QState(guiRootState);

    // create state based on token
    token.mLibrary = stateLibrary;
    token.mUri = wizardProgressiveStateUri;
    QState* wizardProgressiveState = factory.createState(token);
    // set state specific data
    wizardProgressiveState->setParent(searchRootState);
    wizardProgressiveState->setObjectName(token.mUri);
    token.mUri = wizardSettingStateUri;

    QState* wizardSettingState = factory.createState(token);
    wizardSettingState->setParent(searchRootState);
    wizardSettingState->setObjectName(token.mUri);

    wizardProgressiveState->addTransition(wizardProgressiveState,
            SIGNAL(settingsState()), wizardSettingState);

    // From activated back to menu
    wizardSettingState->addTransition(wizardSettingState,
            SIGNAL(backEventTriggered()), wizardProgressiveState);
    
    connect(wizardSettingState, SIGNAL(clickstatus(bool)),
                wizardProgressiveState, SLOT(settingsaction(bool)));

    connect(wizardSettingState, SIGNAL(publishSelectedCategory(int,bool)),
            wizardProgressiveState, SLOT(getSettingCategory(int,bool)));

    connect(wizardSettingState, SIGNAL(customizeGoButton(bool)),
            wizardProgressiveState, SLOT(_customizeGoButton(bool)));

    // set initial state for statemachine
    searchRootState->setInitialState(wizardProgressiveState);
    guiRootState->setInitialState(searchRootState);
    mStateMachine->setInitialState(parallel);

    connect(mStateMachine, SIGNAL(started()),
            SLOT(handleStateMachineStarted()));
    connect(mStateMachine, SIGNAL(stopped()),
            SLOT(handleStateMachineStopped()));
    connect(mStateMachine, SIGNAL(finished()),
            SLOT(handleStateMachineStopped()));

    }
