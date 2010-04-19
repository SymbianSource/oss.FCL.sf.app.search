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


// INCLUDE FILES
#include "cmessagemonitor.h"
#include "harvesterserverlogger.h"

#include <smut.h>
#include <mmsvattachmentmanager.h>	// MMsvAttachmentManager
#include <msventry.h>	// CMsvEntry
#include <msvuids.h>
#include <mmsconst.h>
#include <mtclreg.h>
#include <smsclnt.h>
#include <senduiconsts.h>


// DEFINES
#define INDEXING_QUEUE_MAX 100 // Maximum number of messages in the queue
#define INDEXING_DELAY 5000000 // Nano seconds to delay the monitored messages 

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMessageMonitor::NewL
// -----------------------------------------------------------------------------
//
CMessageMonitor* CMessageMonitor::NewL(  CMessagePlugin& aMessagePlugin, CMsvSession& aSession )
	{
	CMessageMonitor* self = new ( ELeave ) CMessageMonitor( aMessagePlugin, aSession );
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
	}

// -----------------------------------------------------------------------------
// CMessageMonitor::CMessageMonitor
// -----------------------------------------------------------------------------
//
CMessageMonitor::CMessageMonitor( CMessagePlugin& aMessagePlugin, CMsvSession& aSession )
	: CActive(CActive::EPriorityStandard),
	iState(EStateNone),
	iMsvSession(aSession),
	iMessagePlugin(aMessagePlugin)
	{
	CPIXLOGSTRING("ENTER CMessageMonitor::CMessageMonitor");

	CActiveScheduler::Add(this);

	CPIXLOGSTRING("END CMessageMonitor::CMessageMonitor");
	}

// -----------------------------------------------------------------------------
// CMessageMonitor::ConstructL
// -----------------------------------------------------------------------------
//
void CMessageMonitor::ConstructL() 
	{
	CPIXLOGSTRING("ENTER CMessageMonitor::ConstructL");

	// Create timer 
	User::LeaveIfError(iTimer.CreateLocal());
	
	CPIXLOGSTRING("END CMessageMonitor::ConstructL");
	}
	


// -----------------------------------------------------------------------------
// CMessageMonitor::~CMessageMonitor
// -----------------------------------------------------------------------------
//
CMessageMonitor::~CMessageMonitor() 
	{
	CPIXLOGSTRING("ENTER ~CMessageMonitor");
	Cancel();
	iMessages.Close();
	iTimer.Close();
	CPIXLOGSTRING("END ~CMessageMonitor");
	}

// -----------------------------------------------------------------------------
// CMessageMonitor::HandleMsgMovedL
// -----------------------------------------------------------------------------
//
void CMessageMonitor::HandleMsgMovedL( const TMsvId aNewFolderId, 
									   const TMsvId /* aOldFolderId */, 
								   	   const CMsvEntrySelection& aSelection )
	{
	CPIXLOGSTRING("ENTER CMessageMonitor::HandleMsgMovedL");
	
	const TInt count( aSelection.Count() );
	// cycle through every message in the CMsvEntrySelection
	for ( TInt i=0; i < count; ++i )
		{
		TMsvId msgId = aSelection[i];
		CPIXLOGSTRING2("msgId: %d", msgId );
        MessageItemL( msgId, ECPixUpdateAction, aNewFolderId );
		}
	CPIXLOGSTRING("END CMessageMonitor::HandleMsgMovedL");
	}


// -----------------------------------------------------------------------------
// CMessageMonitor::0
// -----------------------------------------------------------------------------
//
void CMessageMonitor::HandleMsgCreatedChangedL( const CMsvEntrySelection& aSelection,
                                          	    const TMsvId aFolderId,
                                          	    const MMsvSessionObserver::TMsvSessionEvent /*aEvent*/ )
	{
	CPIXLOGSTRING("ENTER CMessageMonitor::HandleMsgCreatedChangedL");	
	const TInt count( aSelection.Count() );
	//MMsvSessionObserver::TMsvSessionEvent theEvent = aEvent;
    // cycle through every message in the CMsvEntrySelection
	for ( TInt i=0; i < count; ++i )
		{
		// extract the message server entry from the parameters
		TMsvId msgId = aSelection[i];
		CPIXLOGSTRING2("msgId: %d", msgId );							
        TMsvId service = 0;
        TMsvEntry entry;
        const TInt err = iMsvSession.GetEntry( msgId, service, entry );
        if ( ( !err ) && 
            ( KUidMsvMessageEntry == entry.iType ) && 
            ( entry.Visible() && entry.Complete() && !entry.InPreparation() ) &&
            ( iMessagePlugin.CalculateMessageType( entry ) != EMsgTypeInvalid ) )
            {
            CPIXLOGSTRING("CMessageMonitor::HandleMsgCreatedChangedL Called # MonitorEvent #");
            MessageItemL( msgId, ECPixUpdateAction, aFolderId );
            }   
        CPIXLOGSTRING2(" Folder TYPE %x", aFolderId );	     
        CPIXLOGSTRING2(" Entry TYPE %x", entry.iType );	           
        CPIXLOGSTRING2(" Entry VISIBLE %d", entry.Visible() );	          
        CPIXLOGSTRING2(" Entry COMPLETE %d", entry.Complete() );
        CPIXLOGSTRING2(" Entry INPREPARATION %d", entry.InPreparation() );
        CPIXLOGSTRING2(" Message TYPE %d", iMessagePlugin.CalculateMessageType( entry ));
        }
	CPIXLOGSTRING("END CMessageMonitor::HandleMsgCreatedChangedL");	        
	}
	
// -----------------------------------------------------------------------------
// CMessageMonitor::HandleMsgDeletedL
// -----------------------------------------------------------------------------
//
void CMessageMonitor::HandleMsgDeletedL( const CMsvEntrySelection& aSelection )
	{
	CPIXLOGSTRING("ENTER CMessageMonitor::HandleMsgDeletedL");
	
	const TInt count( aSelection.Count() );
	// cycle through every message in the CMsvEntrySelection
	for( TInt i = 0; i < count; ++i )
		{
		TMsvId msgId = aSelection[i];	
		CPIXLOGSTRING2("msgId: %d", msgId );								
		MessageItemL( msgId, ECPixRemoveAction, NULL);
		}
	CPIXLOGSTRING("END CMessageMonitor::HandleMsgDeletedL");	
	}	


void CMessageMonitor::OverwriteOrAddToQuereL(TMsvId aMsvId, TCPixActionType aAction, TMsvId aFolderId)
{
	// Overwrite or add the index to the queue
	TMessageRecord message;
	message.iMsvId = aMsvId;
	message.iAction = aAction;
	message.iFolderId = aFolderId;
	for (TInt i=0; i<iMessages.Count(); i++)
	{
		if (iMessages[i].iMsvId==aMsvId)
		{
			// Older version found
			iMessages[i] = message;
			return;
		}
	}
	
	// older not found, append
	iMessages.AppendL(message);
}

void CMessageMonitor::MessageItemL(TMsvId aMsvId, TCPixActionType aAction, TMsvId aFolderId)
	{	
		// Overwrite or add the index to the queue
		OverwriteOrAddToQuereL(aMsvId, aAction, aFolderId);
			
		// Check the size against maximum queue size
		if (iMessages.Count() > INDEXING_QUEUE_MAX)
		{
			// Maximum is exceeded, force the write immediately
			if (iState == EStateWaiting)
			{
				iTimer.Cancel(); // RunL will be called with iStatus of KErrCancelled
			}
			else if (iState == EStateNone)
			{
		        SetActive();
		        TRequestStatus* status = &iStatus;
		        User::RequestComplete(status, KErrNone); // RunL will be called with iStatus of KErrNone
			}
		}
		else
		{
			// Maximum is not exceeded, keep waiting
			if (iState == EStateNone)
			{
				iState = EStateWaiting;
				iTimer.After(iStatus, INDEXING_DELAY); // Wait 5 seconds before putting this to index
				SetActive();
			}
		}
	}

void CMessageMonitor::RunL()
	{
		// Index the current queue	
		while (iMessages.Count()>0)
		{
			TMessageRecord message = iMessages[0];
			iMessages.Remove(0);
			iMessagePlugin.MessageItemL(message.iMsvId, message.iAction, message.iFolderId);
		}
		
		// Everything is indexed no need to be waiting anymore
		iState = EStateNone;
	}

void CMessageMonitor::DoCancel()
	{
		iTimer.Cancel();
		iState = EStateNone;
	}

TInt CMessageMonitor::RunError(TInt /* aError */)
	{
		// TODO handle indexing errors here
		return KErrNone; // Don't panic
	}

// End Of File

