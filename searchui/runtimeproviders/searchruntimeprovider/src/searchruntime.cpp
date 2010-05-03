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
#include "search_global.h"
#include "searchstateprovider.h"

#include <qstatemachine.h>
#include <qstate.h>
#include <qfinalstate.h>
#include <qdebug.h>

#include <hbmainwindow.h>



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
QStateMachine(aParent),mWindow(0)
    {
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
  //  delete mStateMachine;
    }

// ---------------------------------------------------------------------------
// SearchRuntime::handleStateMachineStarted()
// ---------------------------------------------------------------------------
//
void SearchRuntime::handleStateMachineStarted()
    {
   // emit started();
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

    SearchStateProvider stateProvider;
    
    QFinalState* finalState = new QFinalState();
    this->addState(finalState);

    // parallel state activates all children states
    QState* parallel = new QState(QState::ParallelStates);
    this->addState(parallel);
    parallel->addTransition(this, SIGNAL(stopStateMachine()), finalState);

    // root GUI state
    QState* guiRootState = new QState(parallel);

    QState* searchRootState = new QState(guiRootState);


    QState* wizardProgressiveState = stateProvider.createState(wizardProgressiveStateUri);
    // set state specific data
    wizardProgressiveState->setParent(searchRootState);
    wizardProgressiveState->setObjectName(wizardProgressiveStateUri);


    QState* wizardSettingState = stateProvider.createState(wizardSettingStateUri);
    wizardSettingState->setParent(searchRootState);
    wizardSettingState->setObjectName(wizardSettingStateUri);

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
    this->setInitialState(parallel);

    connect(this, SIGNAL(started()),
            SLOT(handleStateMachineStarted()));
    connect(this, SIGNAL(stopped()),
            SLOT(handleStateMachineStopped()));
    connect(this, SIGNAL(finished()),
            SLOT(handleStateMachineStopped()));

    }
