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
 * Description:  Search application main class.
 *
 */

#include "searchapplication.h"
#include <qstatemachine.h>
#include <searchruntimeprovider.h>
#include <searchruntime.h>

// ---------------------------------------------------------------------------
// SearchApplication::SearchApplication
// ---------------------------------------------------------------------------
//
SearchApplication::SearchApplication(QObject* aParent) :
    QObject(aParent), mRuntime(NULL)
    {
    SEARCH_FUNC_ENTRY("SEARCH::SearchApplication::SearchApplication");

    SearchRuntimeProvider *interface = new SearchRuntimeProvider();
    mRuntime = interface->createPlugin();
    if (mRuntime)
        {
        mRuntime->setParent(this);
        connect(mRuntime, SIGNAL(started()),this, SLOT(handleRuntimeStarted()));
        connect(mRuntime, SIGNAL(stopped()),this, SLOT(handleRuntimeStopped()));
        connect(mRuntime, SIGNAL(faulted()),this, SLOT(handleRuntimeFaulted()));
        } 
		SEARCH_FUNC_EXIT("SEARCH::SearchApplication::SearchApplication");
    }

// ---------------------------------------------------------------------------
// SearchApplication::~SearchApplication()
// ---------------------------------------------------------------------------
//
SearchApplication::~SearchApplication()
    {
    if (mRuntime)
        {
        disconnect(mRuntime, SIGNAL(started()), this,
                SLOT(handleRuntimeStarted()));
        disconnect(mRuntime, SIGNAL(stopped()), this,
                SLOT(handleRuntimeStopped()));
        disconnect(mRuntime, SIGNAL(faulted()), this,
                SLOT(handleRuntimeFaulted()));

        delete mRuntime;
        }
    }

// ---------------------------------------------------------------------------
// SearchApplication::start()
// ---------------------------------------------------------------------------
//
void SearchApplication::start()
    {
    SEARCH_FUNC_ENTRY("SEARCH::Search::start");

    if (mRuntime)
        {
        mRuntime->start();
        }
    else
        {
        emit exit();
        }

    SEARCH_FUNC_EXIT("SEARCH::Search::start");
    }

// ---------------------------------------------------------------------------
// SearchApplication::stop()
// ---------------------------------------------------------------------------
//
void SearchApplication::stop()
    {
    SEARCH_FUNC_ENTRY("SEARCH::Search::stop");
    if (mRuntime)
        {
        mRuntime->stop();
        }

    SEARCH_FUNC_EXIT("SEARCH::Search::stop");
    }

// ---------------------------------------------------------------------------
// SearchApplication::handleRuntimeStarted()
// ---------------------------------------------------------------------------
//
void SearchApplication::handleRuntimeStarted()
    {
    }

// ---------------------------------------------------------------------------
// SearchApplication::handleRuntimeStopped()
// ---------------------------------------------------------------------------
//
void SearchApplication::handleRuntimeStopped()
    {
    emit exit();
    }

// ---------------------------------------------------------------------------
// SearchApplication::handleRuntimeFaulted()
// ---------------------------------------------------------------------------
//
void SearchApplication::handleRuntimeFaulted()
    {
    emit exit();
    }
