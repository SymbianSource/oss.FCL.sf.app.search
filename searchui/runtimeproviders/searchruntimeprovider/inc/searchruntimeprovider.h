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
 * Description:  Declaration of the search default runtime provider.
 *
 */

#ifndef SEARCH_RUNTIMEPROVIDERS_DEFAULTRUNTIMEPROVIDER_H
#define SEARCH_RUNTIMEPROVIDERS_DEFAULTRUNTIMEPROVIDER_H

#include "hsiruntimeprovider.h"
#include "hsistateprovider.h"

/**
 * @ingroup group_Searchruntimeprovider
 * @brief Provides a default implementation of the SEARCH runtime.
 *
 * This provider includes a default implementation of the SEARCH runtime.
 * The runtime is described in the Searchruntimeprovider.manifest file.
 *
 * @since S60 ?S60_version.
 */
class SearchRuntimeProvider : public QObject, public IHsRuntimeProvider
    {
Q_OBJECT

Q_INTERFACES(IHsRuntimeProvider)

public:

    /**
     * Default constructor.
     *
     * @since S60 ?S60_version.
     */
    SearchRuntimeProvider();

public:

    /**
     * Returns contained runtimes as a list of tokens.
     *
     * @return Contained runtimes as a list of tokens.
     * @since S60 ?S60_version.
     */
    QList<HsRuntimeToken> runtimes();

    /**
     * Creates a runtime based on the given token.
     *
     * @param aToken Identifies the runtime to be created.
     * @since S60 ?S60_version.
     */
    HsRuntime* createRuntime(const HsRuntimeToken& aToken);

private:

    /**
     * Token that describes the default runtime.
     */
    HsRuntimeToken mRuntimeToken;

    };

#endif //SEARCH_RUNTIMEPROVIDERS_DEFAULTRUNTIMEPROVIDER_H
