/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  MDS to CPIX Audio Document fetcher 
*
*/


#include "cpixmediavideodoc.h"
#include <mdeobject.h>
#include <mdccommon.h>
#include <mdeitem.h>
#include <mdeconstants.h>
#include <mdeobjectdef.h>
#include "harvesterserverlogger.h"
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cpixmediavideodocTraces.h"
#endif


// -----------------------------------------------------------------------------
// CCPIXDocFetcher::NewL()
// -----------------------------------------------------------------------------
//
CCPIXMediaVideoDoc* CCPIXMediaVideoDoc::NewL()
    {
    CCPIXMediaVideoDoc* self = CCPIXMediaVideoDoc::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::NewLC()
// -----------------------------------------------------------------------------
//
CCPIXMediaVideoDoc* CCPIXMediaVideoDoc::NewLC()
    {
    CCPIXMediaVideoDoc* self = new (ELeave) CCPIXMediaVideoDoc();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::~CCPIXMediaAudioDoc()
// -----------------------------------------------------------------------------
//
CCPIXMediaVideoDoc::~CCPIXMediaVideoDoc()
    {
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::CCPIXMediaAudioDoc()
// -----------------------------------------------------------------------------
//
CCPIXMediaVideoDoc::CCPIXMediaVideoDoc()
    {
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::ConstructL()
// -----------------------------------------------------------------------------
//
void CCPIXMediaVideoDoc::ConstructL()
    {
    CCPIXDocFetcher::ConstructL();//Must have
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::GetCpixDocumentL()
// -----------------------------------------------------------------------------
//
CSearchDocument* CCPIXMediaVideoDoc::GetCpixDocumentL(const CMdEObject& aObject, 
                                              const TDesC& aAppClass,
                                              CMdEObjectDef& aObjectDef)
    {
    OstTraceFunctionEntry0( CCPIXMEDIAVIDEODOC_GETCPIXDOCUMENTL_ENTRY );
    //Get basic document
    CPIXLOGSTRING("CCPIXMediaVideoDoc::GetCpixDocumentL()");
    CSearchDocument* index_item = CCPIXDocFetcher::GetCpixDocumentL(aObject,aAppClass,aObjectDef);
    //URI and Excerpt is done add additional properties here 
    CMdEProperty* property(NULL);
    CMdEPropertyDef& artistPropDef = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KArtistProperty );
    if(aObject.Property( artistPropDef, property )!= KErrNotFound)
       {
       //Add field to document
       CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
       AddFiledtoDocumentL(*index_item,
                           MdeConstants::MediaObject::KArtistProperty,
                           textProperty->Value());
       }
    CMdEPropertyDef& authorPropDef = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KAuthorProperty );
    if(aObject.Property( authorPropDef, property ) != KErrNotFound)
       {
      //Add field to document
      CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
      AddFiledtoDocumentL(*index_item,
                         MdeConstants::MediaObject::KAuthorProperty,
                         textProperty->Value());
       }
    CMdEPropertyDef& copyrightPropDef = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KCopyrightProperty );
    if(aObject.Property( copyrightPropDef, property ) != KErrNotFound)
       {
     //Add field to document
     CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
     AddFiledtoDocumentL(*index_item,
                      MdeConstants::MediaObject::KCopyrightProperty,
                      textProperty->Value());
       }   
    CMdEPropertyDef& descriptionPropDef = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KDescriptionProperty );
    if(aObject.Property( descriptionPropDef, property ) != KErrNotFound)
       {
    //Add field to document
    CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
    AddFiledtoDocumentL(*index_item,
                  MdeConstants::MediaObject::KDescriptionProperty,
                  textProperty->Value());
       }
    CMdEPropertyDef& commentPropDef = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KCommentProperty );
    if(aObject.Property( commentPropDef, property ) != KErrNotFound)
       {
    //Add field to document
    CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
    AddFiledtoDocumentL(*index_item,
                   MdeConstants::MediaObject::KCommentProperty,
                   textProperty->Value());
       }
    OstTraceFunctionExit0( CCPIXMEDIAVIDEODOC_GETCPIXDOCUMENTL_EXIT );
    return index_item;
    }

