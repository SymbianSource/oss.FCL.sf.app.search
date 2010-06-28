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

#include "search.h"
#include <qstatemachine.h>
#include <searchruntimeprovider.h>
#include <searchruntime.h>

// ---------------------------------------------------------------------------
// Search::Search
// ---------------------------------------------------------------------------
//
Search::Search(QObject* aParent) :
    QObject(aParent), mRuntime(NULL)
    {
    SEARCH_FUNC_ENTRY("SEARCH::Search::Search");

    SearchRuntimeProvider *interface = new SearchRuntimeProvider();
    mRuntime = interface->createPlugin();
    if (mRuntime)
        {
        mRuntime->setParent(this);
        connect(mRuntime, SIGNAL(started()),this, SLOT(handleRuntimeStarted()));
        connect(mRuntime, SIGNAL(stopped()),this, SLOT(handleRuntimeStopped()));
        connect(mRuntime, SIGNAL(faulted()),this, SLOT(handleRuntimeFaulted()));
        } 
		SEARCH_FUNC_EXIT("SEARCH::Search::Search");
    }

// ---------------------------------------------------------------------------
// Search::~Search()
// ---------------------------------------------------------------------------
//
Search::~Search()
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
// Search::start()
// ---------------------------------------------------------------------------
//
void Search::start()
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
// Search::stop()
// ---------------------------------------------------------------------------
//
void Search::stop()
    {
    SEARCH_FUNC_ENTRY("SEARCH::Search::stop");
    if (mRuntime)
        {
        mRuntime->stop();
        }

    SEARCH_FUNC_EXIT("SEARCH::Search::stop");
    }

// ---------------------------------------------------------------------------
// Search::handleRuntimeStarted()
// ---------------------------------------------------------------------------
//
void Search::handleRuntimeStarted()
    {
    }

// ---------------------------------------------------------------------------
// Search::handleRuntimeStopped()
// ---------------------------------------------------------------------------
//
void Search::handleRuntimeStopped()
    {
    emit exit();
    }

// ---------------------------------------------------------------------------
// Search::handleRuntimeFaulted()
// ---------------------------------------------------------------------------
//
void Search::handleRuntimeFaulted()
    {
    emit exit();
    }
