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

#include "ccalendarplugin.h"
#include "harvesterserverlogger.h"
#include <common.h>
#include <csearchdocument.h>

#include <ccpixindexer.h>
#include <e32base.h> 
#include <calsession.h>
#include <calentry.h>
#include <caliterator.h>
#include <calentryview.h>
#include <mmfcontrollerpluginresolver.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "ccalendarpluginTraces.h"
#endif
 // CleanupResetAndDestroyPushL

_LIT(KMimeTypeField, CPIX_MIMETYPE_FIELD);
_LIT(KMimeTypeCalendar, CALENDAR_MIMETYPE);

_LIT(KExcerptDelimiter, " ");

/** The delay between harvesting chunks. */
const TInt KHarvestingDelay = 2000;
_LIT(KCalendarTimeFormat,"%04d %02d %02d %02d %02d");
// ---------------------------------------------------------------------------
// CMessagePlugin::NewL
// ---------------------------------------------------------------------------
//  
CCalendarPlugin* CCalendarPlugin::NewL()
	{
	CCalendarPlugin* instance = CCalendarPlugin::NewLC();
    CleanupStack::Pop(instance);
    return instance;
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::NewLC
// ---------------------------------------------------------------------------
//  
CCalendarPlugin* CCalendarPlugin::NewLC()
	{
	CCalendarPlugin* instance = new (ELeave) CCalendarPlugin();
    CleanupStack::PushL(instance);
    instance->ConstructL();
    return instance;
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::CCalendarPlugin
// ---------------------------------------------------------------------------
//  
CCalendarPlugin::CCalendarPlugin()
	{
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::~CCalendarPlugin
// ---------------------------------------------------------------------------
//  
CCalendarPlugin::~CCalendarPlugin()
	{
    if (iAsynchronizer)
        iAsynchronizer->CancelCallback();
	delete iAsynchronizer;
	delete iIndexer;

	delete iEntryView;
	delete iCalIterator;
	if( iSession )
		{
		iSession->StopChangeNotification();
		}
	delete iSession;
	}
	
// ---------------------------------------------------------------------------
// CCalendarPlugin::ConstructL
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::ConstructL()
	{
	iAsynchronizer = CDelayedCallback::NewL( CActive::EPriorityIdle );
	iSession = CCalSession::NewL();	
	TRAPD ( err , iSession->OpenL( iSession->DefaultFileNameL() ) );
	if ( err == KErrNotFound)
	    {
	    iSession->CreateCalFileL( iSession->DefaultFileNameL() );
	    iSession->OpenL( iSession->DefaultFileNameL() );
	    }
	iCalIterator = CCalIter::NewL( *iSession );
	iEntryView = CCalEntryView::NewL( *iSession, *this );
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::StartPluginL
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::StartPluginL()
	{
	// Define this base application class, use default location
	User::LeaveIfError(iSearchSession.DefineVolume( _L(CALENDAR_QBASEAPPCLASS), KNullDesC ));

	// Open the database
	iIndexer = CCPixIndexer::NewL(iSearchSession);
	iIndexer->OpenDatabaseL( _L(CALENDAR_QBASEAPPCLASS) ); 

	// Start harvester for this plugin
	iObserver->AddHarvestingQueue(this, iIndexer->GetBaseAppClass());
	
	// Start monitoring calendar events
	TTime startTime( TDateTime( 1980, EJanuary, 0, 0, 0, 0, 0 ) );
	TTime endTime( startTime + TTimeIntervalYears( 40 ) );
	TCalTime startTimeCal;
	TCalTime endTimeCal;
	startTimeCal.SetTimeUtcL( startTime );
	endTimeCal.SetTimeUtcL( endTime );
	CCalChangeNotificationFilter* filter = CCalChangeNotificationFilter::NewL( MCalChangeCallBack2::EChangeEntryAll, ETrue, CalCommon::TCalTimeRange( startTimeCal, endTimeCal ) );
	iSession->StartChangeNotification( *this, *filter );
	delete filter;
	}	

// ---------------------------------------------------------------------------
// CCalendarPlugin::StartHarvestingL
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::StartHarvestingL(const TDesC& /*aQualifiedBaseAppClass*/)
    {
    iIndexer->ResetL();
    iStartHarvesting = ETrue;
#ifdef __PERFORMANCE_DATA
    iStartTime.UniversalTime();
#endif  
    if (iFirstEntry)
        {
        iAsynchronizer->Start( 0, this, KHarvestingDelay );
        }
    }

// -----------------------------------------------------------------------------
// CCalendarPlugin::Progress
// -----------------------------------------------------------------------------
//
void CCalendarPlugin::Progress( TInt /*aPercentageCompleted*/ )
	{
	// No implementation needed
	}

// -----------------------------------------------------------------------------
// CCalendarPlugin::Completed
// -----------------------------------------------------------------------------
//
void CCalendarPlugin::Completed( TInt aError )
	{
	// No error code and harvesting is needed star harvesting.
	iFirstEntry = ETrue;
	if (aError == KErrNone && iStartHarvesting)
		{
		// Calendar entry view constructed successfully, start harvesting
		iAsynchronizer->Start( 0, this, KHarvestingDelay );
		}
	}

// -----------------------------------------------------------------------------
// CCalendarPlugin::NotifyProgress
// -----------------------------------------------------------------------------
//
TBool CCalendarPlugin::NotifyProgress()
	{
	return EFalse;
	}

// -----------------------------------------------------------------------------
// CContactsPlugin::DelayedCallbackL
// -----------------------------------------------------------------------------
//
void CCalendarPlugin::DelayedCallbackL( TInt /*aCode*/ )
    {
    // Harvest items on each call
	TPtrC8 uid( KNullDesC8 );

	if( iFirstEntry )
		{
		uid.Set( iCalIterator->FirstL() );
		iFirstEntry = EFalse;
		}
	else
		{
		uid.Set( iCalIterator->NextL() );
		}

	if( uid != KNullDesC8 )
		{
		RPointerArray<CCalEntry> entryArray;
		CleanupResetAndDestroyPushL(entryArray);
		iEntryView->FetchL( uid, entryArray );
		// Handle only the first (i.e. parent entry)
		if( entryArray.Count() > 0 )
			{
			CCalEntry* entry = (CCalEntry*)entryArray[ 0 ];
			OstTrace1( TRACE_NORMAL, CCALENDARPLUGIN_DELAYEDCALLBACKL, "CCalendarPlugin::DelayedCallbackL();Harvesting id=%d", entry->LocalUidL() );
			CPIXLOGSTRING2("CCalendarPlugin::DelayedCallbackL(): Harvesting id=%d.", entry->LocalUidL());
			CreateEntryL( entry->LocalUidL(), ECPixAddAction );
			}
		CleanupStack::PopAndDestroy(&entryArray);

		// Request next entry.
		iAsynchronizer->Start( 0, this, KHarvestingDelay );
		}
	else
		{
		// Harvesting was successfully completed
		iFirstEntry = ETrue; // Make sure we can harvest next time as well...
		Flush(*iIndexer);
#ifdef __PERFORMANCE_DATA
    UpdatePerformaceDataL();
#endif
		iObserver->HarvestingCompleted(this, iIndexer->GetBaseAppClass(), KErrNone);		
		}
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::DelayedError
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::DelayedError(TInt aError)
	{
	// Harvesting was completed
	iFirstEntry = ETrue; // Make sure we can harvest next time as well...
	Flush(*iIndexer);
	iObserver->HarvestingCompleted(this, iIndexer->GetBaseAppClass(), aError);
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::CalChangeNotification
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::CalChangeNotification( RArray< TCalChangeEntry >& aChangeItems )
	{
	const TInt count(aChangeItems.Count());
	OstTrace1( TRACE_NORMAL, CCALENDARPLUGIN_CALCHANGENOTIFICATION, "CCalendarPlugin::CalChangeNotification();changed item count=%d", count );
	CPIXLOGSTRING2("CCalendarPlugin::CalChangeNotification(): changed item count =%d.", count);
	for( TInt i = 0; i < count; ++i )
		{
		TCalChangeEntry changedEntry = aChangeItems[ i ];
		TRAP_IGNORE(HandleChangedEntryL(changedEntry));
		}
	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::HandleChangedEntryL
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::HandleChangedEntryL(const TCalChangeEntry& changedEntry)
	{
	switch( changedEntry.iChangeType )
		{		
		case EChangeAdd:
			{
			OstTrace1( TRACE_NORMAL, CCALENDARPLUGIN_HANDLECHANGEDENTRYL, "CCalendarPlugin::HandleChangedEntryL();Monitored add id=%d", changedEntry.iEntryId );
			CPIXLOGSTRING2("CCalendarPlugin::HandleChangedEntryL(): Monitored add id=%d.", changedEntry.iEntryId);
#ifdef __PERFORMANCE_DATA
            iStartTime.UniversalTime();
			CreateEntryL( changedEntry.iEntryId, ECPixAddAction );
			UpdatePerformaceDataL(ECPixAddAction);
#else
			CreateEntryL( changedEntry.iEntryId, ECPixAddAction );
#endif
			break;
			}

		case EChangeDelete:
			{	
			OstTrace1( TRACE_NORMAL, DUP1_CCALENDARPLUGIN_HANDLECHANGEDENTRYL, "CCalendarPlugin::HandleChangedEntryL();Monitored delete id=%d", changedEntry.iEntryId );
			CPIXLOGSTRING2("CCalendarPlugin::HandleChangedEntryL(): Monitored delete id=%d.", changedEntry.iEntryId);
#ifdef __PERFORMANCE_DATA
            iStartTime.UniversalTime();
			CreateEntryL( changedEntry.iEntryId, ECPixAddAction );
			UpdatePerformaceDataL(ECPixAddAction);
#else
			CreateEntryL( changedEntry.iEntryId, ECPixRemoveAction );
#endif
			break;
			}

		case EChangeModify:
			{
			OstTrace1( TRACE_NORMAL, DUP2_CCALENDARPLUGIN_HANDLECHANGEDENTRYL, "CCalendarPlugin::HandleChangedEntryL();Monitored update id=%d", changedEntry.iEntryId );
			CPIXLOGSTRING2("CCalendarPlugin::HandleChangedEntryL(): Monitored update id=%d.", changedEntry.iEntryId);
#ifdef __PERFORMANCE_DATA
            iStartTime.UniversalTime(); 
			CreateEntryL( changedEntry.iEntryId, ECPixUpdateAction );
			UpdatePerformaceDataL(ECPixUpdateAction);
#else
			CreateEntryL( changedEntry.iEntryId, ECPixUpdateAction );
#endif
			break;
			}
			
		/* TCalChangeEntry documentation:
			If iChangeType is EChangeUndefined or EChangeOverflowError, iEntryId and
			iEntryType are undefined and should not be used by clients.
		 */
		case EChangeUndefined:
			{
			OstTrace0( TRACE_NORMAL, DUP3_CCALENDARPLUGIN_HANDLECHANGEDENTRYL, "CCalendarPlugin::HandleChangedEntryL(): EChangeUndefined." );
			CPIXLOGSTRING("CCalendarPlugin::HandleChangedEntryL(): EChangeUndefined.");
			// This event could be related to synchronization.
			// Mark harvesting as cancelled.
			// Remove it from the harvesting queue to cause it to enter 
			// EHarvesterStatusHibernate state.
			// Now add it to the harvesting queue and force a reharvest.

			iFirstEntry = ETrue; // Make sure we can harvest next time as well...
			Flush(*iIndexer);
			iObserver->HarvestingCompleted(this, iIndexer->GetBaseAppClass(), KErrCancel);
			iObserver->RemoveHarvestingQueue(this, iIndexer->GetBaseAppClass());
			iObserver->AddHarvestingQueue(this, iIndexer->GetBaseAppClass(), ETrue);
			break;
			}
		default:
			// Ignore other events
			break;
		}	
	}
	
// ---------------------------------------------------------------------------
// CCalendarPlugin::CreateEntryL
// ---------------------------------------------------------------------------
//  
void CCalendarPlugin::CreateEntryL( const TCalLocalUid& aLocalUid, TCPixActionType aActionType )
	{
	if (!iIndexer)
    	return;
	

	OstTrace1( TRACE_NORMAL, CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL();Uid=%d", aLocalUid );
	CPIXLOGSTRING2("CCalendarPlugin::CreateEntryL():  Uid = %d.", aLocalUid);
	
	// creating CSearchDocument object with unique ID for this application
	TBuf<20> docid_str;
	docid_str.AppendNum(aLocalUid);
	
	if (aActionType == ECPixAddAction || aActionType == ECPixUpdateAction)
		{
		CSearchDocument* index_item = CSearchDocument::NewLC(docid_str, _L(CALENDARAPPCLASS)); 
		
		// Return the entry that has been fetched, this will be NULL if there are 
		// no entries with the cal unique id
		CCalEntry* entry = iEntryView->FetchL(aLocalUid);
		if ( entry == NULL ) {
            CleanupStack::PopAndDestroy(index_item);
            return;
		}
		// TODO leave if entry is NULL
		CleanupStack::PushL(entry);
		// TODO Uncomment below portion of code when the latest Organiser code in MCL
		if( CCalEntry::ENote == entry->EntryTypeL() )
		    {
            CleanupStack::PopAndDestroy(entry);
            CleanupStack::PopAndDestroy(index_item);
            OstTrace0( TRACE_NORMAL, DUP1_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL(): Donot harvest Note item." );
            CPIXLOGSTRING("CCalendarPlugin::CreateEntryL(): Donot harvest Note item.");
            return;
		    }

		// Add fields
		index_item->AddFieldL(KCalendarSummaryField, entry->SummaryL());
		index_item->AddFieldL(KCalendarDescriptionField, entry->DescriptionL());
		index_item->AddFieldL(KCalendarLocationField, entry->LocationL());
		
		TUint priority = entry->PriorityL();
		
		switch(priority)
		    {
		    case 1:
		        index_item->AddFieldL(KCalendarPriorityField, KCalendarPriorityHigh);
		        break;
		    case 2:
		        index_item->AddFieldL(KCalendarPriorityField, KCalendarPriorityMedium);
                break;
		    case 3:
		        index_item->AddFieldL(KCalendarPriorityField, KCalendarPriorityLow);
                break;
		    default:
	            index_item->AddFieldL(KCalendarPriorityField, KNullDesC);
	            break;
		    }

		TBuf<30> dateString;
		TDateTime datetime = entry->StartTimeL().TimeUtcL().DateTime();       
		dateString.Format( KCalendarTimeFormat, datetime.Year(),
		                                     TInt(datetime.Month()+ 1),
		                                     datetime.Day() + 1,
		                                     datetime.Hour()+ 1,
		                                     datetime.Minute());
		index_item->AddFieldL(KCalendarStartTimeField, dateString, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);

		TDateTime endTime = entry->EndTimeL().TimeUtcL().DateTime();		
		dateString.Format( KCalendarTimeFormat, endTime.Year(),
                                                TInt(endTime.Month()+ 1),
                                                endTime.Day() + 1,
                                                endTime.Hour()+ 1,
                                                endTime.Minute());
		index_item->AddFieldL(KCalendarEndTimeField, dateString, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
		
		TTime completedTime = entry->CompletedTimeL().TimeUtcL();
		if( completedTime != Time::NullTTime() && CCalEntry::ETodo == entry->EntryTypeL())
		    {
            TDateTime compTime = completedTime.DateTime();
            dateString.Format( KCalendarTimeFormat, compTime.Year(),
                                                TInt(compTime.Month()+ 1),
                                                compTime.Day() + 1,
                                                compTime.Hour()+ 1,
                                                compTime.Minute());
            index_item->AddFieldL(KCalenderCompletedField, dateString, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
		    }
		index_item->AddFieldL(KMimeTypeField, KMimeTypeCalendar, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);

    	TInt excerptLength = 1 /*single 1-character delimiters*/ + entry->DescriptionL().Length() + entry->LocationL().Length();
		HBufC* excerpt = HBufC::NewLC(excerptLength);
		TPtr excerptDes = excerpt->Des();
		excerptDes.Append(entry->DescriptionL());
		excerptDes.Append(KExcerptDelimiter);
		excerptDes.Append(entry->LocationL());

        index_item->AddExcerptL(*excerpt);
        CleanupStack::PopAndDestroy(excerpt);
        CleanupStack::PopAndDestroy(entry);

		/*
		RPointerArray<CCalAttendee>& attendees = iEntry.AttendeesL();
		for( TInt i = 0; i < attendees.Count(); i++ )
			{
			CCalAttendee* attendee = (CCalAttendee*)attendees[ i ];
			TInt err = KErrNone;
		    TRAP( err, AddContent( CIndexContent::NewL( attendee->CommonName(), ECalendarAttendee ) ) );
			}
		attendees.ResetAndDestroy();
		*/
		
		// Send for indexing
		if (aActionType == ECPixAddAction)
			{
			TRAPD(err, iIndexer->AddL(*index_item));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP2_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL(): Added." );
				CPIXLOGSTRING("CCalendarPlugin::CreateEntryL(): Added.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP3_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL();Error %d in adding", err );
				CPIXLOGSTRING2("CCalendarPlugin::CreateEntryL(): Error %d in adding.", err);
				}			
			}
		else if (aActionType == ECPixUpdateAction)
			{
			TRAPD(err, iIndexer->UpdateL(*index_item));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP4_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL(): Updated." );
				CPIXLOGSTRING("CCalendarPlugin::CreateEntryL(): Updated.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP5_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL();Error %d in updating", err );
				CPIXLOGSTRING2("CCalendarPlugin::CreateEntryL(): Error %d in updating.", err);
				}			
			}
		CleanupStack::PopAndDestroy(index_item);
		}
	else if (aActionType == ECPixRemoveAction)
		{
		TRAPD(err, iIndexer->DeleteL(docid_str));
		if (err == KErrNone)
			{
			OstTrace0( TRACE_NORMAL, DUP6_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL(): Deleted." );
			CPIXLOGSTRING("CCalendarPlugin::CreateEntryL(): Deleted.");
			}
		else
			{
			OstTrace1( TRACE_NORMAL, DUP7_CCALENDARPLUGIN_CREATEENTRYL, "CCalendarPlugin::CreateEntryL();Error %d in deleting", err );
			CPIXLOGSTRING2("CCalendarPlugin::CreateEntryL(): Error %d in deleting.", err);				
			}
		}

	}

// ---------------------------------------------------------------------------
// CCalendarPlugin::UpdatePerformaceDataL
// ---------------------------------------------------------------------------
//
#ifdef __PERFORMANCE_DATA
void CCalendarPlugin::UpdatePerformaceDataL()
    {
    TTime now;
   
    
    iCompleteTime.UniversalTime();
    TTimeIntervalMicroSeconds timeDiff = iCompleteTime.MicroSecondsFrom(iStartTime);
    
    RFs fileSession;
    RFile perfFile;
    User::LeaveIfError( fileSession.Connect () );
    
    
    /* Open file if it exists, otherwise create it and write content in it */
    
        if(perfFile.Open(fileSession, _L("c:\\data\\CalenderPerf.txt"), EFileWrite))
                   User::LeaveIfError(perfFile.Create (fileSession, _L("c:\\data\\CalenderPerf.txt"), EFileWrite));
    
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
    ptr.Append( _L("Time taken for Harvesting Calendar is : "));
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
// CCalendarPlugin::UpdatePerformaceDataL
// ---------------------------------------------------------------------------
//
void CCalendarPlugin::UpdatePerformaceDataL(TCPixActionType action)
    {
   
    iCompleteTime.UniversalTime();
    TTimeIntervalMicroSeconds timeDiff = iCompleteTime.MicroSecondsFrom(iStartTime);
    
    RFs fileSession;
    RFile perfFile;
    User::LeaveIfError( fileSession.Connect () );
    
    
    /* Open file if it exists, otherwise create it and write content in it */
    
        if(perfFile.Open(fileSession, _L("c:\\data\\CalenderPerf.txt"), EFileWrite))
                   User::LeaveIfError(perfFile.Create (fileSession, _L("c:\\data\\CalenderPerf.txt"), EFileWrite));
    
    HBufC8 *heap = HBufC8::NewL(100);
    TPtr8 ptr = heap->Des();

    switch (action) {
        case ECPixAddAction: ptr.Append( _L("add "));break;
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
