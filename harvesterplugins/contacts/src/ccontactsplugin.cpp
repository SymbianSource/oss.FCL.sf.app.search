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
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccontactspluginTraces.h"
#endif
 

_LIT(KMimeTypeField, CPIX_MIMETYPE_FIELD);
_LIT(KMimeTypeContact, CONTACT_MIMETYPE);


/** The delay between harvesting chunks. */
const TInt KHarvestingDelay = 2000;

/** Number of contacts to process in one active scheduler cycle */
const TInt KContactsPerRunL = 1;

_LIT(KExcerptDelimiter, " ");
_LIT(KTimeFormat, "%D%N%Y%1 %2 %3"); //Date, Month name and Year format
 
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
			OstTrace1( TRACE_NORMAL, DUP3_CCONTACTSPLUGIN_HANDLEDATABASEEVENTL, "CContactsPlugin::HandleDatabaseEventL;Monitored update id=%d", aEvent.iContactId );			
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
			OstTrace1( TRACE_NORMAL, CCONTACTSPLUGIN_HANDLEDATABASEEVENTL, "CContactsPlugin::HandleDatabaseEventL();Monitored delete id=%d", aEvent.iContactId );
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
			OstTrace1( TRACE_NORMAL, DUP1_CCONTACTSPLUGIN_HANDLEDATABASEEVENTL, "CContactsPlugin::HandleDatabaseEventL();Monitored add id=%d", aEvent.iContactId );
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
		OstTrace1( TRACE_NORMAL, CCONTACTSPLUGIN_DELAYEDCALLBACKL, "CContactsPlugin::DelayedCallbackL();Harvesting id=%d", (*iContacts)[iCurrentIndex] );
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
void CContactsPlugin::AddFieldL(CSearchDocument& aDocument, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& aFieldName, const TInt aConfig)
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
		if( additionalField.StorageType() == KStorageTypeDateTime)
		    {
            CContactDateField* fieldDate = additionalField.DateTimeStorage();
            if (fieldDate)
                {
                TBuf<30> dateString;
                fieldDate->Time().FormatL(dateString, KTimeFormat);
                aDocument.AddFieldL(fieldNamePtr, dateString, aConfig);
                }
            else
                aDocument.AddFieldL(fieldNamePtr, KNullDesC, aConfig);
		    }
		else
		    {
            CContactTextField* fieldText = additionalField.TextStorage();
            if (fieldText && fieldText->Text() != KNullDesC && aFieldId != KUidContactFieldAnniversary)
                aDocument.AddFieldL(fieldNamePtr, fieldText->Text(), aConfig);
            else if (fieldText && aFieldId == KUidContactFieldAnniversary)
                {
                TBuf<30> dateString;
                GetDateL(fieldText->Text(), dateString);
                aDocument.AddFieldL(fieldNamePtr, dateString, aConfig);
                }
            else
                aDocument.AddFieldL(fieldNamePtr, KNullDesC, aConfig);
		    }
            findpos = aFieldSet.FindNext(aFieldId, findpos+1);

		}
	CleanupStack::PopAndDestroy(fieldName);
	}


// ---------------------------------------------------------------------------
// CContactsPlugin::AddToExcerptL
// Adds more text to excerpt
// ---------------------------------------------------------------------------
void CContactsPlugin::AddToExcerptL(CSearchDocument& /*aDocument*/, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& /*aFieldName*/)
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
// Helper: Adds the field to the document AND to the exceprt field.
// This function helps avoid calling two functions explicitly for most fields.
// This function needs to be a member function as it has to call member functions.
// Improvement: AddFieldL need to be a member function - can be refactored.
// ---------------------------------------------------------------------------
void CContactsPlugin::AddFieldToDocumentAndExcerptL(CSearchDocument& aDocument, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& aFieldName, const TInt aConfig )
    {
    AddFieldL( aDocument, aFieldSet, aFieldId, aFieldName, aConfig );
    AddToExcerptL( aDocument, aFieldSet, aFieldId, aFieldName );
    }

// ---------------------------------------------------------------------------
// CContactsPlugin::CreateMessageIndexItemL
// ---------------------------------------------------------------------------
//	    
void CContactsPlugin::CreateContactIndexItemL(TInt aContentId, TCPixActionType aActionType )
    {
	if (!iIndexer)
    	return;
    
	OstTrace1( TRACE_NORMAL, CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL();aContentId=%d", aContentId );
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
            OstTraceExt1( TRACE_NORMAL, DUP1_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, ";Adding Contact Group=%S", ( static_cast<CContactGroup*>( contact )->GetGroupLabelL() ) );
            CPIXLOGSTRING2("Adding Contact Group %S", &( static_cast<CContactGroup*>( contact )->GetGroupLabelL() ) );
            index_item->AddExcerptL( static_cast<CContactGroup*>( contact )->GetGroupLabelL() );
		    }
		else//If the contact item is a regular contact.
		    {
            if (iExcerpt)
                {
                delete iExcerpt;
                iExcerpt = NULL;
                }
            iExcerpt = HBufC::NewL(2);
            
            CContactItemFieldSet& fieldSet = contact->CardFields();

            //For contacts, all fields __except__ GivenName and FamilyName should be added to excerpt.
            //See appclass-hierarchy.txt for details.
            /* The order of fields in excerpt is as below. The order in this case
             * is the order of fields shown when you 'Edit' the contact.
             */
            AddFieldL( *index_item, fieldSet, KUidContactFieldGivenName, KContactsGivenNameField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText );
            AddFieldL( *index_item, fieldSet, KUidContactFieldFamilyName, KContactsFamilyNameField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField:: EIndexFreeText );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldPhoneNumber, KContactsPhoneNumberField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldEMail, KContactsEMailField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldSIPID, KContactsSIPIDField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldCompanyName, KContactsCompanyNameField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldJobTitle, KContactsJobTitleField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldNote, KContactsNoteField );

            /* The following fields are not displayed when 'Edit'-ing the contact.
             * The order here is arbitrary.
             */
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldAddress, KContactsAddressField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldSecondName, KContactsSecondNameField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldPrefixName, KContactsPrefixField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldSuffixName, KContactsSuffixField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldAdditionalName, KContactsAdditionalNameField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText);
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldUrl, KContactsUrlField );
    
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldPostOffice, KContactsPostOfficeField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldExtendedAddress, KContactsExtendedAddressField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldLocality, KContactsLocalityField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldRegion, KContactsRegionField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldPostcode, KContactsPostcodeField );
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldCountry, KContactsCountryField );
    
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldSpouse, KContactsSpouseField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText);
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldChildren, KContactsChildrenField, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText);
            //AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldClass, KContactsClassField ); //sync field
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldFax, KContactsFaxField );
            
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldAssistant, KContactAssistant, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText);
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldDepartmentName, KContactsDepartmentName, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EIndexFreeText);
            
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldIMAddress, KContactIMAddress);
            AddFieldToDocumentAndExcerptL( *index_item, fieldSet, KUidContactFieldServiceProvider, KContactServiceProvider);
            
            AddFieldL( *index_item, fieldSet, KUidContactFieldBirthday, KContactBirthday);
            AddFieldL( *index_item, fieldSet, KUidContactFieldAnniversary, KContactAnniversary);
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
				OstTrace0( TRACE_NORMAL, DUP2_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL(): Added." );
				CPIXLOGSTRING("CContactsPlugin::CreateContactIndexItemL(): Added.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP3_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL();Error %d in adding", err );
				CPIXLOGSTRING2("CContactsPlugin::CreateContactIndexItemL(): Error %d in adding.", err);
				}			
			}
		else if (aActionType == ECPixUpdateAction)
			{
			TRAPD(err, iIndexer->UpdateL(*index_item));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP4_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL(): Updated." );
				CPIXLOGSTRING("CContactsPlugin::CreateContactIndexItemL(): Updated.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP5_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL();Error %d in updating", err );
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
			OstTrace0( TRACE_NORMAL, DUP6_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL(): Deleted." );
			CPIXLOGSTRING("CContactsPlugin::CreateContactIndexItemL(): Deleted.");
			}
		else
			{
			OstTrace1( TRACE_NORMAL, DUP7_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML, "CContactsPlugin::CreateContactIndexItemL();Error %d in deleting", err );
			CPIXLOGSTRING2("CContactsPlugin::CreateContactIndexItemL(): Error %d in deleting.", err);
			}			
		}
    }

// ---------------------------------------------------------------------------
// CContactsPlugin::GetDateL
// ---------------------------------------------------------------------------
//
void CContactsPlugin::GetDateL(const TDesC& aTime, TDes& aDateString)
    {
    TTime time;
    //sort the date string to the requried format dd/mm/yyyy from returned
    //format yyyy/mm/dd as parse API is currently not supporting japanese date format
    if( aTime.Length() >= KDateFieldLength)
        {
        aDateString.Copy(aTime.Mid( KDayPosition, KDayLength ));
        aDateString.Append(KDateSeparator);
        aDateString.Append(aTime.Mid( KMonthPosition, KDayLength ));
        aDateString.Append(KDateSeparator);
        aDateString.Append(aTime.Mid( KYearPosition, KYearLength ));
        TInt err = time.Parse(aDateString);
        if ( err >= KErrNone)
            {
            time.FormatL(aDateString, KTimeFormat);
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
