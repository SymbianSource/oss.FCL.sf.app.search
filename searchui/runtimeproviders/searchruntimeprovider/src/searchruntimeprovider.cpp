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
 * Description:  Implementation of the SEARCH default runtime provider.
 *
 */

#include "searchruntimeprovider.h"
#include "searchruntime.h"

// constants
const char providerFileName[] = "searchruntimeprovider.dll";
const char providerFileUri[] = "search.nokia.com/runtime/defaultruntime";

// ---------------------------------------------------------------------------
// SearchRuntimeProvider::SearchRuntimeProvider()
// ---------------------------------------------------------------------------
//
SearchRuntimeProvider::SearchRuntimeProvider()
    {
    mRuntimeToken.mLibrary = providerFileName;
    mRuntimeToken.mUri = providerFileUri;
    }

// ---------------------------------------------------------------------------
// SearchRuntimeProvider::runtimes()
// ---------------------------------------------------------------------------
//
QList<HsRuntimeToken> SearchRuntimeProvider::runtimes()
    {
    return QList<HsRuntimeToken> () << mRuntimeToken;
    }

// ---------------------------------------------------------------------------
// SearchRuntimeProvider::createRuntime()
// ---------------------------------------------------------------------------
//
HsRuntime* SearchRuntimeProvider::createRuntime(const HsRuntimeToken& aToken)
    {
    return (aToken.mUri == providerFileUri ? new SearchRuntime() : 0);
    }

#ifndef COVERAGE_MEASUREMENT
Q_EXPORT_PLUGIN2(Searchruntimeprovider, SearchRuntimeProvider)
#endif //COVERAGE_MEASUREMENT
