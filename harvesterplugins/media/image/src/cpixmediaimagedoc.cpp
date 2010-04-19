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
* Description:  MDS to CPIX Image Document fetcher 
 *
*/


#include "cpixmediaimagedoc.h"
#include <mdeobject.h>
#include <mdccommon.h>
#include <mdeitem.h>
#include <mdeconstants.h>
#include <mdeobjectdef.h>
#include "harvesterserverlogger.h"

_LIT( KFormatDateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ");    // yyyy-mm-ddThh:mm:ssZ

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::NewL()
// -----------------------------------------------------------------------------
//
CCPIXMediaImageDoc* CCPIXMediaImageDoc::NewL()
    {
    CCPIXMediaImageDoc* self = CCPIXMediaImageDoc::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::NewLC()
// -----------------------------------------------------------------------------
//
CCPIXMediaImageDoc* CCPIXMediaImageDoc::NewLC()
    {
    CCPIXMediaImageDoc* self = new (ELeave) CCPIXMediaImageDoc();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::~CCPIXMediaImageDoc()
// -----------------------------------------------------------------------------
//
CCPIXMediaImageDoc::~CCPIXMediaImageDoc()
    {
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::CCPIXMediaImageDoc()
// -----------------------------------------------------------------------------
//
CCPIXMediaImageDoc::CCPIXMediaImageDoc()
    {
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::ConstructL()
// -----------------------------------------------------------------------------
//
void CCPIXMediaImageDoc::ConstructL()
    {
    CCPIXDocFetcher::ConstructL();//Must have
    
    }

// -----------------------------------------------------------------------------
// CCPIXDocFetcher::GetCpixDocumentL()
// -----------------------------------------------------------------------------
//
CSearchDocument* CCPIXMediaImageDoc::GetCpixDocumentL(const CMdEObject& aObject, 
                                              const TDesC& aAppClass,
                                              CMdEObjectDef& aObjectDef)
    {
    //Get basic document
    CPIXLOGSTRING("START CCPIXMediaImageDoc::GetCpixDocumentL");
    CSearchDocument* index_item = CCPIXDocFetcher::GetCpixDocumentL(aObject,aAppClass,aObjectDef);
    //URI and Excerpt is done add additional properties here 
    CMdEProperty* property(NULL);
    //Get user comment field
    CMdEPropertyDef& commentPropDef = aObjectDef.GetPropertyDefL(MdeConstants::MediaObject::KCommentProperty );
    if(aObject.Property( commentPropDef, property )!= KErrNotFound)
       {
       //Add field to document
       CMdETextProperty* textProperty = ( CMdETextProperty* ) property;
       AddFiledtoDocumentL(*index_item,
                           MdeConstants::MediaObject::KCommentProperty,
                           textProperty->Value());
       }
    CMdEPropertyDef& dateTimePropDef = aObjectDef.GetPropertyDefL(MdeConstants::Image::KDateTimeOriginalProperty);
    if(aObject.Property( dateTimePropDef, property ) != KErrNotFound)
      {
      //Add field to document
      if(EPropertyTime == property->Def().PropertyType())
          {
          CMdETimeProperty& timeProperty = static_cast < CMdETimeProperty& > (*property);
          TDateTime time = timeProperty.Value().DateTime();
          TBuf<32> buf;
          buf.Format( KFormatDateTime, time.Year(), 
                                       time.Month() + 1, 
                                       time.Day() + 1, 
                                       time.Hour(), 
                                       time.Minute(), 
                                       time.Second() );
          AddFiledtoDocumentL(*index_item,
                             MdeConstants::Image::KDateTimeOriginalProperty,
                             buf,
                             CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
          }
      }
    //Latitude error currently 
    /* TODO- Bhuvi Location is a seperate object for each image object if any such relation
     * exist it has to be searched from Relation table and from there we have to open 
     * location object and query Longitude and Latitudes
     */
    /*
    CMdEPropertyDef& latitudePropDef = aObjectDef.GetPropertyDefL(MdeConstants::Location::KLatitudeProperty);
    if(aObject.Property( latitudePropDef, property ) != KErrNotFound)
      {
      TBuf<32> buf;
      TRealFormat format(32,16); //Real format
      format.iTriLen = 0;
      //Add field to document
      if(EPropertyReal32 == property->Def().PropertyType())
          {
          CMdEReal32Property& real32Property = (CMdEReal32Property&) property;
          buf.Num( real32Property.Value(), format );
          }
      else
          if(EPropertyReal64 == property->Def().PropertyType())
          {
          CMdEReal64Property& real64Property = (CMdEReal64Property&) property;
          buf.Num( real64Property.Value(), format );
          }
      AddFiledtoDocumentL(*index_item,
                         MdeConstants::Location::KLatitudeProperty,
                         buf,
                         CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
      }
    //Longitude
    CMdEPropertyDef& longitudePropDef = aObjectDef.GetPropertyDefL(MdeConstants::Location::KLongitudeProperty);
    if(aObject.Property( longitudePropDef, property ) != KErrNotFound)
      {
      TBuf<32> buf;
      TRealFormat format(32,16); //Real format
      format.iTriLen = 0;
      //Add field to document
      if(EPropertyReal32 == property->Def().PropertyType())
          {
          CMdEReal32Property& real32Property = (CMdEReal32Property&) property;
          buf.Num( real32Property.Value(), format );
          }
      else
          if(EPropertyReal64 == property->Def().PropertyType())
          {
          CMdEReal64Property& real64Property = (CMdEReal64Property&) property;
          buf.Num( real64Property.Value(), format );
          }
      AddFiledtoDocumentL(*index_item,
                         MdeConstants::Location::KLongitudeProperty,
                         buf,
                         CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
      }
      */
    CPIXLOGSTRING("END CCPIXMediaImageDoc::GetCpixDocumentL");
    return index_item;
    }
//End of life
