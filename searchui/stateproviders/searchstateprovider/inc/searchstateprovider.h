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
 * Description:  Declaration of the Search state provider
 *
 */

#ifndef SEARCH_STATEPROVIDERS_STATEPROVIDER_H
#define SEARCH_STATEPROVIDERS_STATEPROVIDER_H

#include "Search_global.h"

#include "hsistateprovider.h"

class SearchStateProviderTest;

class QState;

/**
 * @ingroup group_Searchstateprovider
 * @brief Provides a default implementation for each Search state.
 *
 * This provider includes a default implementation for each Search state
 * States are described in the Searchstateprovider.manifest file.
 *
 * @lib Searchstateprovider
 * @since S60 ?S60_version.
 */
class SearchStateProvider : public QObject, public IHsStateProvider
    {

Q_OBJECT
Q_INTERFACES(IHsStateProvider)

public:

    /**
     * Default constructor.
     *
     * @since S60 ?S60_version.
     */
    SearchStateProvider();

    /**
     * Destructor.
     *
     * @since S60 ?S60_version.
     */
    virtual ~SearchStateProvider();

public:

    /**
     * Returns contained states as a list of tokens.
     *
     * @return Contained states as a list of tokens.
     * @since S60 ?S60_version.
     */
    QList<HsStateToken> states();

    /**
     * Creates a state based on the given token.
     *
     * @param aToken Identifies the state to be created.
     * @since S60 ?S60_version.
     */
    QState* createState(const HsStateToken& aToken);

private:

    /**
     * Token that describes the wizard menu state.
     */
    HsStateToken mInitStateToken;

    HsStateToken mWizardMenuStateToken;

    HsStateToken mWizardActivatedStateToken;

    friend class SearchStateProviderTest;
    };

#endif //SEARCH_STATEPROVIDERS_STATEPROVIDER_H
