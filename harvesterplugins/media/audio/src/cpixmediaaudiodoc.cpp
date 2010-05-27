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


#include "cpixmediaaudiodoc.h"
#include <mdeobject.h>
#include <mdccommon.h>
#include <mdeitem.h>
#include <mdeconstants.h>
#include <mdeobjectdef.h>
#include "harvesterserverlogger.h"
#include "OstTraceDefinitions.h"
#include "csearchdocument.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cpixmediaaudiodocTraces.h"
#endif


// -----------------------------------------------------------------------------
// CCPIXDocFetcher::NewL()
// -----------------------------------------------------------------------------
//
CCPIXMediaAudioDoc* CCPIXMediaAudioDoc::NewL()
    {
    CCPIXMediaAudioDoc* self = CCPIXMediaAudioDoc::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::NewLC()
// -----------------------------------------------------------------------------
//
CCPIXMediaAudioDoc* CCPIXMediaAudioDoc::NewLC()
    {
    CCPIXMediaAudioDoc* self = new (ELeave) CCPIXMediaAudioDoc();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::~CCPIXMediaAudioDoc()
// -----------------------------------------------------------------------------
//
CCPIXMediaAudioDoc::~CCPIXMediaAudioDoc()
    {
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::CCPIXMediaAudioDoc()
// -----------------------------------------------------------------------------
//
CCPIXMediaAudioDoc::CCPIXMediaAudioDoc()
    {
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::ConstructL()
// -----------------------------------------------------------------------------
//
void CCPIXMediaAudioDoc::ConstructL()
    {
    CCPIXDocFetcher::ConstructL();//Must have
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::GetCpixDocumentL()
// -----------------------------------------------------------------------------
//
CSearchDocument* CCPIXMediaAudioDoc::GetCpixDocumentL(const CMdEObject& aObject, 
                                              const TDesC& aAppClass,
                                              CMdEObjectDef& aObjectDef)
    {
    OstTraceFunctionEntry0( CCPIXMEDIAAUDIODOC_GETCPIXDOCUMENTL_ENTRY );
    CPIXLOGSTRING("START CCPIXMediaAudioDoc::GetCpixDocumentL");
    //Get basic document
    CSearchDocument* index_item = CCPIXDocFetcher::GetCpixDocumentL(aObject,aAppClass,aObjectDef);
    //Reset Excerpt and append
    ResetExcerpt();
    TInt slashpos = GetUri().LocateReverse('\\');
    TPtrC name = GetUri().Mid( (slashpos+1) );
    AddToFieldExcerptL(name); //Add name to excerpt field
    //additional properties here 
    CMdEProperty* property(NULL);
    CMdEPropertyDef& artistPropDef1 = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KArtistProperty );
        if(aObject.Property( artistPropDef1, property ) != KErrNotFound)
           {
           //Add field to document
           CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
           AddFiledtoDocumentL(*index_item,
                                 MdeConstants::MediaObject::KArtistProperty,
                                 textProperty->Value());
           AddToFieldExcerptL(textProperty->Value());//Add artist to excerpt
           }
    CMdEPropertyDef& albumPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KAlbumProperty );
    if(aObject.Property( albumPropDef, property )!= KErrNotFound)
       {
       //Add field to document
       CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
       AddFiledtoDocumentL(*index_item,
                           MdeConstants::Audio::KAlbumProperty,
                           textProperty->Value());
       AddToFieldExcerptL(textProperty->Value());//Add Album to excerpt
       }
    CMdEPropertyDef& artistPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KAlbumArtistProperty );
    if(aObject.Property( artistPropDef, property ) != KErrNotFound)
      {
      //Add field to document
      CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
      AddFiledtoDocumentL(*index_item,
                         MdeConstants::Audio::KAlbumArtistProperty,
                         textProperty->Value());
      AddToFieldExcerptL(textProperty->Value());//Add Albumartist to excerpt
      }
    CMdEPropertyDef& origartistPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KOriginalArtistProperty );
    if(aObject.Property( origartistPropDef, property ) != KErrNotFound)
     {
     //Add field to document
     CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
     AddFiledtoDocumentL(*index_item,
                      MdeConstants::Audio::KOriginalArtistProperty,
                      textProperty->Value());
     AddToFieldExcerptL(textProperty->Value());//Add Original to excerpt
     }   
    CMdEPropertyDef& composerPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KComposerProperty );
    if(aObject.Property( composerPropDef, property ) != KErrNotFound)
    {
    //Add field to document
    CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
    AddFiledtoDocumentL(*index_item,
                  MdeConstants::Audio::KComposerProperty,
                  textProperty->Value());
    AddToFieldExcerptL(textProperty->Value());//Add composer to excerpt
    }
    index_item->AddExcerptL(*iExcerpt);   
    CPIXLOGSTRING("END CCPIXMediaAudioDoc::GetCpixDocumentL");
    OstTraceFunctionExit0( CCPIXMEDIAAUDIODOC_GETCPIXDOCUMENTL_EXIT );
    return index_item;
    }
//End of life
