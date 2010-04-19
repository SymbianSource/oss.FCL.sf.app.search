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
 * Description:  Implementation of the SEARCH state provider
 *
 */

#include "searchstateprovider.h"
#include "searchprogressivestate.h"

#include "searchinitstate.h"
#include "searchsettingsstate.h"

#include <qstate.h>
#include <qdebug.h>

// constants
const char providerFileName[] = "searchstateprovider.dll";
const char initStateFileUri[] = "search.nokia.com/state/initstate";
// states
const char wizardProgressiveStateUri[] =
        "search.nokia.com/state/wizardprogressivestate";
const char wizardSettingStateUri[] =
        "search.nokia.com/state/wizardsettingstate";

// ---------------------------------------------------------------------------
// searchStateProvider::searchStateProvider()
// ---------------------------------------------------------------------------
//
SearchStateProvider::SearchStateProvider()
    {
    mInitStateToken.mLibrary = providerFileName;
    mInitStateToken.mUri = initStateFileUri;

    mWizardMenuStateToken.mLibrary = providerFileName;
    mWizardMenuStateToken.mUri = wizardProgressiveStateUri;

    mWizardActivatedStateToken.mLibrary = providerFileName;
    mWizardActivatedStateToken.mUri = wizardSettingStateUri;
    }

// ---------------------------------------------------------------------------
// SearchStateProvider::~SearchStateProvider()
// ---------------------------------------------------------------------------
//
SearchStateProvider::~SearchStateProvider()
    {
    }

// ---------------------------------------------------------------------------
// SearchStateProvider::states()
// ---------------------------------------------------------------------------
//
QList<HsStateToken> SearchStateProvider::states()
    {
    return QList<HsStateToken> () << mInitStateToken << mWizardMenuStateToken
            << mWizardActivatedStateToken;
    }

// ---------------------------------------------------------------------------
// SearchStateProvider::createState(const StateToken& aToken)
// ---------------------------------------------------------------------------
//
QState* SearchStateProvider::createState(const HsStateToken& aToken)
    {

    if (aToken.mUri == mWizardMenuStateToken.mUri)
        {
        return new SearchProgressiveState();
        }
    else if (aToken.mUri == mWizardActivatedStateToken.mUri)
        {
        return new SearchSettingsState();
        }
    else if (aToken.mUri == mInitStateToken.mUri)
        {
        return new SearchInitState();
        }

    qDebug() << "SEARCH: No state found for mUri: " << aToken.mUri;
    return NULL;

    }

#ifdef COVERAGE_MEASUREMENT
#pragma CTC SKIP
#endif //COVERAGE_MEASUREMENT
Q_EXPORT_PLUGIN2(SearchStateProvider, SearchStateProvider)
#ifdef COVERAGE_MEASUREMENT
#pragma CTC ENDSKIP
#endif //COVERAGE_MEASUREMENT

