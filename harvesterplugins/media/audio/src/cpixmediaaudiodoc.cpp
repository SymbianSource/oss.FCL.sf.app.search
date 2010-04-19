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
    CPIXLOGSTRING("START CCPIXMediaAudioDoc::GetCpixDocumentL");
    //Get basic document
    CSearchDocument* index_item = CCPIXDocFetcher::GetCpixDocumentL(aObject,aAppClass,aObjectDef);
    //URI and Excerpt is done add additional properties here 
    CMdEProperty* property(NULL);
    CMdEPropertyDef& albumPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KAlbumProperty );
    if(aObject.Property( albumPropDef, property )!= KErrNotFound)
       {
       //Add field to document
       CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
       AddFiledtoDocumentL(*index_item,
                           MdeConstants::Audio::KAlbumProperty,
                           textProperty->Value());
       }
    CMdEPropertyDef& artistPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KAlbumArtistProperty );
    if(aObject.Property( artistPropDef, property ) != KErrNotFound)
      {
      //Add field to document
      CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
      AddFiledtoDocumentL(*index_item,
                         MdeConstants::Audio::KAlbumArtistProperty,
                         textProperty->Value());
      }
    CMdEPropertyDef& origartistPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KOriginalArtistProperty );
    if(aObject.Property( origartistPropDef, property ) != KErrNotFound)
     {
     //Add field to document
     CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
     AddFiledtoDocumentL(*index_item,
                      MdeConstants::Audio::KOriginalArtistProperty,
                      textProperty->Value());
     }   
    CMdEPropertyDef& composerPropDef = aObjectDef.GetPropertyDefL(MdeConstants::Audio::KComposerProperty );
    if(aObject.Property( composerPropDef, property ) != KErrNotFound)
    {
    //Add field to document
    CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
    AddFiledtoDocumentL(*index_item,
                  MdeConstants::Audio::KComposerProperty,
                  textProperty->Value());
    }
    CMdEPropertyDef& artistPropDef1 = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KArtistProperty );
    if(aObject.Property( artistPropDef1, property ) != KErrNotFound)
       {
       //Add field to document
       CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
       AddFiledtoDocumentL(*index_item,
                             MdeConstants::MediaObject::KArtistProperty,
                             textProperty->Value());
       }   
    CPIXLOGSTRING("END CCPIXMediaAudioDoc::GetCpixDocumentL");
    return index_item;
    }
//End of life
