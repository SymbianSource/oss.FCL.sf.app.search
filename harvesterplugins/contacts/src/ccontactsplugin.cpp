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
* Description: 
 *
*/

#include "ccontactsplugin.h"
#include "harvesterserverlogger.h"
#include <common.h>
#include <csearchdocument.h>

#include <ccpixindexer.h>
#include <e32base.h> 

_LIT(KMimeTypeField, CPIX_MIMETYPE_FIELD);
_LIT(KMimeTypeContact, CONTACT_MIMETYPE);


/** The delay between harvesting chunks. */
const TInt KHarvestingDelay = 2000;

/** Number of contacts to process in one active scheduler cycle */
const TInt KContactsPerRunL = 1;

_LIT(KExcerptDelimiter, " ");

// -----------------------------------------------------------------------------
// CContactsPlugin::NewL()
// -----------------------------------------------------------------------------
//
CContactsPlugin* CContactsPlugin::NewL()
	{
	CContactsPlugin* instance = CContactsPlugin::NewLC();
    CleanupStack::Pop(instance);
    return instance;
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::NewLC()
// -----------------------------------------------------------------------------
//
CContactsPlugin* CContactsPlugin::NewLC()
	{
	CContactsPlugin* instance = new (ELeave)CContactsPlugin();
    CleanupStack::PushL(instance);
    instance->ConstructL();
    return instance;
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::CContactsPlugin()
// -----------------------------------------------------------------------------
//
CContactsPlugin::CContactsPlugin()
	{
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::~CContactsPlugin()
// -----------------------------------------------------------------------------
//
CContactsPlugin::~CContactsPlugin()
	{
    if (iAsynchronizer)
        iAsynchronizer->CancelCallback();
	delete iAsynchronizer;
	iContacts = NULL;
	delete iChangeNotifier;
	delete iDatabase;
	delete iIndexer;
	
	if (iExcerpt)
		delete iExcerpt;
	}
	
// -----------------------------------------------------------------------------
// CContactsPlugin::ConstructL()
// -----------------------------------------------------------------------------
//
void CContactsPlugin::ConstructL()
	{
	iDatabase = CContactDatabase::OpenL();

	// This pointer is valid until a change is made to the database or until 
	// the database's active object is allowed to run. If the array is 
	// required after one of the above two events has occurred, a copy of the 
	// array must first be made.
	iContacts = iDatabase->SortedItemsL();

    iAsynchronizer = CDelayedCallback::NewL( CActive::EPriorityIdle );
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::StartPluginL()
// -----------------------------------------------------------------------------
//
void CContactsPlugin::StartPluginL()
	{
	// Define this base application class, use default location
	User::LeaveIfError(iSearchSession.DefineVolume( _L(CONTACT_QBASEAPPCLASS), KNullDesC ));

	// Open database
	iIndexer = CCPixIndexer::NewL(iSearchSession);
	iIndexer->OpenDatabaseL( _L(CONTACT_QBASEAPPCLASS) ); 

	// Start harvester for this plugin
	iObserver->AddHarvestingQueue(this, iIndexer->GetBaseAppClass());

	// Start monitoring when plugin is started
	iChangeNotifier = CContactChangeNotifier::NewL(*iDatabase, this);	
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::StartHarvestingL()
// -----------------------------------------------------------------------------
//
void CContactsPlugin::StartHarvestingL(const TDesC& /*aQualifiedBaseAppClass*/)
    {
	iIndexer->ResetL();
	iCurrentIndex = 0;
#ifdef __PERFORMANCE_DATA
    iStartTime.UniversalTime();
#endif  
    iAsynchronizer->Start( 0, this, KHarvestingDelay );
    }

// -----------------------------------------------------------------------------
// CContactsPlugin::HandleDatabaseEventL
// -----------------------------------------------------------------------------
// 
void CContactsPlugin::HandleDatabaseEventL(TContactDbObserverEvent aEvent)
	{
	switch( aEvent.iType )
		{
		case EContactDbObserverEventContactChanged:
		case EContactDbObserverEventGroupChanged:
			CPIXLOGSTRING2("CContactsPlugin::DelayedCallbackL(): Monitored update id=%d.", aEvent.iContactId);
#ifdef __PERFORMANCE_DATA
            iStartTime.UniversalTime();
            CreateContactIndexItemL(aEvent.iContactId, ECPixUpdateAction);
            UpdatePerformaceDataL(ECPixUpdateAction);
#else			
			CreateContactIndexItemL(aEvent.iContactId, ECPixUpdateAction);
#endif			
			break;

		case EContactDbObserverEventContactDeleted:
		case EContactDbObserverEventGroupDeleted:
			CPIXLOGSTRING2("CContactsPlugin::DelayedCallbackL(): Monitored delete id=%d.", aEvent.iContactId);
#ifdef __PERFORMANCE_DATA
            iStartTime.UniversalTime();			
			CreateContactIndexItemL(aEvent.iContactId, ECPixRemoveAction);
			UpdatePerformaceDataL(ECPixRemoveAction);
#else
			CreateContactIndexItemL(aEvent.iContactId, ECPixRemoveAction);
#endif
			break;

		case EContactDbObserverEventContactAdded:
		case EContactDbObserverEventGroupAdded:
			CPIXLOGSTRING2("CContactsPlugin::DelayedCallbackL(): Monitored add id=%d.", aEvent.iContactId);
#ifdef __PERFORMANCE_DATA
            iStartTime.UniversalTime();			
			CreateContactIndexItemL(aEvent.iContactId, ECPixUpdateAction);
			UpdatePerformaceDataL(ECPixUpdateAction);
#else
			CreateContactIndexItemL(aEvent.iContactId, ECPixAddAction);
#endif
			break;

		default:
			// Ignore other events
			break;
		}
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::DelayedCallbackL
// -----------------------------------------------------------------------------
//
void CContactsPlugin::DelayedCallbackL( TInt /*aCode*/ )
    {
	if (!iContacts || !iObserver)
		return;

	// may have changed - refresh the pointer
	iContacts = iDatabase->SortedItemsL();

	// Read the next set of contacts.
	for( TInt i = 0; i < KContactsPerRunL; i++ )
		{
		// Exit the loop if no more contacts
		if (iCurrentIndex >= iContacts->Count())
			break;
		
		// Create index item
		CPIXLOGSTRING2("CContactsPlugin::DelayedCallbackL(): Harvesting id=%d.", (*iContacts)[iCurrentIndex]);
		CreateContactIndexItemL((*iContacts)[iCurrentIndex], ECPixAddAction);
		iCurrentIndex++;
		}

	if( iAsynchronizer && (iCurrentIndex < iContacts->Count()) )
	    {
	    // Launch the next RunL
        iAsynchronizer->Start(0, this, KHarvestingDelay);
        }
	else
		{
		// Harvesting was successfully completed
		Flush(*iIndexer);
#ifdef __PERFORMANCE_DATA
    UpdatePerformaceDataL();
#endif 
		iObserver->HarvestingCompleted(this, iIndexer->GetBaseAppClass(), KErrNone);
		}
	}

void CContactsPlugin::DelayedError(TInt aError)
	{
	// Harvesting was successfully completed
	Flush(*iIndexer);
	iObserver->HarvestingCompleted(this, iIndexer->GetBaseAppClass(), aError);
	}

// ---------------------------------------------------------------------------
// CContactsPlugin::AddFieldL
// Adds information field (if available)
// If the contact has multiple values for the same Field (e.g. multiple 
// PhoneNumber values), then the FieldNames of non-first values are appended a 
// number.
// ---------------------------------------------------------------------------
void CContactsPlugin::AddFieldL(CSearchDocument& aDocument, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& aFieldName)
	{
	HBufC* fieldName = HBufC::NewLC(aFieldName.Length() + 3); // +3 so can append variable 'i'.
	TPtr fieldNamePtr = fieldName->Des();

	// Find field
	TInt findpos = aFieldSet.FindNext(aFieldId, 0);
	for (TInt i = 0; findpos != KErrNotFound; i++)
		{
		fieldNamePtr.Copy(aFieldName);
		if (i>0)
			{
			fieldNamePtr.AppendNum(i);
			}
		CContactItemField& additionalField = aFieldSet[findpos];
		CContactTextField* fieldText = additionalField.TextStorage();
		if (fieldText && fieldText->Text() != KNullDesC)
	        aDocument.AddFieldL(fieldNamePtr, fieldText->Text(), CDocumentField::EStoreYes | CDocumentField::EIndexTokenized);
		else
	        aDocument.AddFieldL(fieldNamePtr, KNullDesC, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized);
		
		findpos = aFieldSet.FindNext(aFieldId, findpos+1);
		}
	CleanupStack::PopAndDestroy(fieldName);
	}


// ---------------------------------------------------------------------------
// CContactsPlugin::AddToExcerptL
// Adds more text to excerpt
// ---------------------------------------------------------------------------
void CContactsPlugin::AddToExcerptL(CSearchDocument& /*aDocument*/, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& /*aFieldName*/ )
	{
	// Find field
	TInt findpos = aFieldSet.Find( aFieldId );
	if (! (findpos < 0) || (findpos >= aFieldSet.Count() ) )
		{
		CContactItemField& additionalField = aFieldSet[ findpos ];
		CContactTextField* fieldText = additionalField.TextStorage();
		if ( fieldText && fieldText->Text() != KNullDesC )
			{
			TInt currentSize = iExcerpt->Size();
			TInt newSize = currentSize + fieldText->Text().Size() + 1;
			iExcerpt = iExcerpt->ReAllocL(newSize);
			TPtr ptr = iExcerpt->Des();
			ptr.Append(fieldText->Text());
			ptr.Append(KExcerptDelimiter);
			}
		}
	}


// ---------------------------------------------------------------------------
// CContactsPlugin::CreateMessageIndexItemL
// ---------------------------------------------------------------------------
//	    
void CContactsPlugin::CreateContactIndexItemL(TInt aContentId, TCPixActionType aActionType )
    {
	if (!iIndexer)
    	return;
    
	CPIXLOGSTRING2("CContactsPlugin::CreateContactIndexItemL(): aContentId = %d ", aContentId );
    
	// creating CSearchDocument object with unique ID for this application
	TBuf<20> docid_str;
	docid_str.AppendNum(aContentId);
	
	if (aActionType == ECPixAddAction || aActionType == ECPixUpdateAction )
		{
		CSearchDocument* index_item = CSearchDocument::NewLC(docid_str, _L(CONTACTAPPCLASS)); 
		   
	    // Add fields
		CContactItem* contact = iDatabase->ReadMinimalContactL(aContentId);
        CleanupStack::PushL( contact );
		if( contact->Type() == KUidContactGroup )
		    {
            index_item->AddFieldL(KContactsGivenNameField, static_cast<CContactGroup*>( contact )->GetGroupLabelL(), CDocumentField::EStoreYes | CDocumentField::EIndexTokenized);
            CPIXLOGSTRING2("Adding Contact Group %S", &( static_cast<CContactGroup*>( contact )->GetGroupLabelL() ) );
            index_item->AddExcerptL( static_cast<CContactGroup*>( contact )->GetGroupLabelL() );
		    }
		else//If the contact item is a regular contact.
		    {
            CContactItemFieldSet& fieldSet = contact->CardFields();
            AddFieldL( *index_item, fieldSet, KUidContactFieldGivenName, KContactsGivenNameField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldFamilyName, KContactsFamilyNameField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldCompanyName, KContactsCompanyNameField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldPhoneNumber, KContactsPhoneNumberField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldAddress, KContactsAddressField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldNote, KContactsNoteField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldJobTitle, KContactsJobTitleField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldSecondName, KContactsSecondNameField ); 
            
            AddFieldL( *index_item, fieldSet, KUidContactFieldPrefixName, KContactsPrefixField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldSuffixName, KContactsSuffixField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldAdditionalName, KContactsAdditionalNameField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldEMail, KContactsEMailField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldUrl, KContactsUrlField );
    
            AddFieldL( *index_item, fieldSet, KUidContactFieldPostOffice, KContactsPostOfficeField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldExtendedAddress, KContactsExtendedAddressField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldLocality, KContactsLocalityField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldRegion, KContactsRegionField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldPostcode, KContactsPostcodeField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldCountry, KContactsCountryField );
    
            AddFieldL( *index_item, fieldSet, KUidContactFieldSIPID, KContactsSIPIDField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldSpouse, KContactsSpouseField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldChildren, KContactsChildrenField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldClass, KContactsClassField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldFax, KContactsFaxField );				
            
            AddFieldL( *index_item, fieldSet, KUidContactFieldGivenNamePronunciation, KContactsGivenNamePronunciationField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldFamilyNamePronunciation, KContactsFamilyNamePronunciationField );
            AddFieldL( *index_item, fieldSet, KUidContactFieldCompanyNamePronunciation, KContactsCompanyNamePronunciationField );
            //left: Birthday; Anniversary (date kind of type), Picture, Logo..

            if (iExcerpt)
                {
                delete iExcerpt;
                iExcerpt = NULL;
                }
            iExcerpt = HBufC::NewL(2);
            AddToExcerptL( *index_item, fieldSet, KUidContactFieldGivenName, KContactsGivenNameField );
            AddToExcerptL( *index_item, fieldSet, KUidContactFieldFamilyName, KContactsFamilyNameField );
            AddToExcerptL( *index_item, fieldSet, KUidContactFieldPhoneNumber, KContactsPhoneNumberField );
            AddToExcerptL( *index_item, fieldSet, KUidContactFieldCompanyName, KContactsCompanyNameField );
            AddToExcerptL( *index_item, fieldSet, KUidContactFieldLocality, KContactsLocalityField );
            AddToExcerptL( *index_item, fieldSet, KUidContactFieldCountry, KContactsCountryField );
            index_item->AddExcerptL(*iExcerpt);
            }
        
    	index_item->AddFieldL(KMimeTypeField, KMimeTypeContact, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
        
		CleanupStack::PopAndDestroy( contact );
	
		// Send for indexing
		if (aActionType == ECPixAddAction)
			{
			TRAPD(err, iIndexer->AddL(*index_item));
			if (err == KErrNone)
				{
				CPIXLOGSTRING("CContactsPlugin::CreateContactIndexItemL(): Added.");
				}
			else
				{
				CPIXLOGSTRING2("CContactsPlugin::CreateContactIndexItemL(): Error %d in adding.", err);
				}			
			}
		else if (aActionType == ECPixUpdateAction)
			{
			TRAPD(err, iIndexer->UpdateL(*index_item));
			if (err == KErrNone)
				{
				CPIXLOGSTRING("CContactsPlugin::CreateContactIndexItemL(): Updated.");
				}
			else
				{
				CPIXLOGSTRING2("CContactsPlugin::CreateContactIndexItemL(): Error %d in updating.", err);
				}			
			}
		CleanupStack::PopAndDestroy(index_item); // Do not destroy
		}
	else if (aActionType == ECPixRemoveAction)
		{
		// Remove the document
		TRAPD(err, iIndexer->DeleteL(docid_str));
		if (err == KErrNone)
			{
			CPIXLOGSTRING("CContactsPlugin::CreateContactIndexItemL(): Deleted.");
			}
		else
			{
			CPIXLOGSTRING2("CContactsPlugin::CreateContactIndexItemL(): Error %d in deleting.", err);
			}			
		}
    }

// ---------------------------------------------------------------------------
// CContactsPlugin::UpdatePerformaceDataL
// ---------------------------------------------------------------------------
//	
#ifdef __PERFORMANCE_DATA
void CContactsPlugin::UpdatePerformaceDataL()
    {
    TTime now;
   
    
    iCompleteTime.UniversalTime();
    TTimeIntervalMicroSeconds timeDiff = iCompleteTime.MicroSecondsFrom(iStartTime);
    
    RFs fileSession;
    RFile perfFile;
    User::LeaveIfError( fileSession.Connect () );
    
    
    /* Open file if it exists, otherwise create it and write content in it */
    
        if(perfFile.Open(fileSession, _L("c:\\data\\ContactsPerf.txt"), EFileWrite))
                   User::LeaveIfError(perfFile.Create (fileSession, _L("c:\\data\\ContactsPerf.txt"), EFileWrite));
    
    HBufC8 *heap = HBufC8::NewL(100);
    TPtr8 ptr = heap->Des();
    now.HomeTime();
    TBuf<50> timeString;             
                
    _LIT(KOwnTimeFormat,"%:0%H%:1%T%:2%S");
    now.FormatL(timeString,KOwnTimeFormat);
    ptr.AppendNum(now.DateTime().Day());
    ptr.Append(_L("/"));
    ptr.AppendNum(now.DateTime().Month());
    ptr.Append(_L("/"));
    ptr.AppendNum(now.DateTime().Year());
    ptr.Append(_L(":"));
    ptr.Append(timeString);
    ptr.Append( _L("Time taken for Harvesting Contacts is : "));
    ptr.AppendNum(timeDiff.Int64()/1000) ;
    ptr.Append(_L(" MilliSeonds \n"));
    TInt myInt = 0;
    perfFile.Seek(ESeekEnd,myInt);
    perfFile.Write (ptr);
    perfFile.Close ();
    fileSession.Close ();
    delete heap;
    }

// ---------------------------------------------------------------------------
// CContactsPlugin::UpdatePerformaceDataL
// ---------------------------------------------------------------------------
//	
void CContactsPlugin::UpdatePerformaceDataL(TCPixActionType action)
    {
 
    iCompleteTime.UniversalTime();
    TTimeIntervalMicroSeconds timeDiff = iCompleteTime.MicroSecondsFrom(iStartTime);
    
    RFs fileSession;
    RFile perfFile;
    User::LeaveIfError( fileSession.Connect () );
    
    
    /* Open file if it exists, otherwise create it and write content in it */
    
        if(perfFile.Open(fileSession, _L("c:\\data\\ContactsPerf.txt"), EFileWrite))
                   User::LeaveIfError(perfFile.Create (fileSession, _L("c:\\data\\ContactsPerf.txt"), EFileWrite));
    
    HBufC8 *heap = HBufC8::NewL(100);
    TPtr8 ptr = heap->Des();

    switch (action) {
        case ECPixUpdateAction: ptr.Append( _L("upd "));break;
        case ECPixRemoveAction: ptr.Append( _L("del "));break;
    } 
    ptr.AppendNum(timeDiff.Int64()/1000) ;
    ptr.Append(_L("\n"));
    TInt myInt = 0;
    perfFile.Seek(ESeekEnd,myInt);
    perfFile.Write (ptr);
    perfFile.Close ();
    fileSession.Close ();
    delete heap;
    }
#endif

// End of file
