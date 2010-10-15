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
* Description:  MDE object queuemanager for indexing scheduling
 *
*/

#include "mdeobjectqueuemanager.h"
#include "harvesterserverlogger.h"

// DEFINES
#define INDEXING_QUEUE_MAX 300 // Maximum number of objects in the queue
#define INDEXING_DELAY 3000000 // Nano seconds to delay the monitored object
#define MAX_RETRY_COUNT    3   //maximum number of retry on not found object


// -----------------------------------------------------------------------------
// CCPixIndexerUtils::NewL()
// -----------------------------------------------------------------------------
//
CMdeObjectQueueManager* CMdeObjectQueueManager::NewL(MMediaObjectHandler* aHandler)
    {
    CMdeObjectQueueManager* self = CMdeObjectQueueManager::NewLC(aHandler);
    CleanupStack::Pop();
    return self;
    }
// -----------------------------------------------------------------------------
// CCPixIndexerUtils::NewLC()
// -----------------------------------------------------------------------------
//
CMdeObjectQueueManager* CMdeObjectQueueManager::NewLC(MMediaObjectHandler* aHandler)
    {
    CMdeObjectQueueManager* self = new (ELeave) CMdeObjectQueueManager(aHandler);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::~CMdeObjectQueueManager()
// -----------------------------------------------------------------------------
//
CMdeObjectQueueManager::~CMdeObjectQueueManager()
    {
    Cancel();
    iTimer.Close();
    iFs.Close();
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::CMdeObjectQueueManager()
// -----------------------------------------------------------------------------
//
CMdeObjectQueueManager::CMdeObjectQueueManager(MMediaObjectHandler* aHandler):
                        CActive(CActive::EPriorityStandard),
                        iState(EStateNone),
                        iMdeObjectHandler(aHandler)
    {
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::ConstructL()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::ConstructL()
    {
    CActiveScheduler::Add(this);
    User::LeaveIfError(iFs.Connect());
    User::LeaveIfError(iTimer.CreateLocal());
    iHState = EStateResume;
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::AddMdeItemToQueueL()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::AddMdeItemToQueueL( TItemId aMsgId, 
                                                 TCPixActionType aActionType)
    {
    OverWriteOrAddToQueueL(aMsgId,aActionType);    
    ActivateAO();
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::OverWriteOrAddToQueueL()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::OverWriteOrAddToQueueL( TItemId aObjId, 
                                                     TCPixActionType aActionType)
    {
    // Overwrite or add the index to the queue
    TMdeActionRecord object;
    object.iObjectId = aObjId;
    object.iAction = aActionType;
    for (TInt i=0; i<iJobQueue.Count(); i++)
    {
        if (iJobQueue[i].iObjectId ==aObjId)
        {
            // Older version found
            iJobQueue[i] = object;
            return;
        }
    }
    
    // older not found, append
    iJobQueue.AppendL(object);    
    }

void CMdeObjectQueueManager::ActivateAO()
    {
    // Proceed only if the plugin is not in pause state 
    if ( iHState == EStateResume )
        {
        // Check the size against maximum queue size
        if (iJobQueue.Count() > INDEXING_QUEUE_MAX)
            {
            // Maximum is exceeded, force the write immediately
            if (iState == EStateWaiting)
                {
                if(IsActive())
                    iTimer.Cancel(); // RunL will be called with iStatus of KErrCancelled
                }
            else if (iState == EStateNone)
                {
                if(!IsActive())
                    {
                    SetActive();
                    TRequestStatus* status = &iStatus;
                    User::RequestComplete(status, KErrNone); // RunL will be called with iStatus of KErrNone
                    }
                }
            }
        else
            {
            // Maximum is not exceeded, keep waiting
            if (iState == EStateNone)
                {
                if(!IsActive())
                    {
                    iState = EStateWaiting;
                    iTimer.After(iStatus, INDEXING_DELAY); // Wait 5 seconds before putting this to index
                    SetActive();
                    }
                }
            }
        }
    }
// -----------------------------------------------------------------------------
// CCPixIndexerUtils::RunL()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::RunL()
    {
    CPIXLOGSTRING("START CMdeObjectQueueManager::RunL");
    
    if ( iHState == EStatePause )
        {
        iState = EStateNone;
        CPIXLOGSTRING("END CMdeObjectQueueManager::RunL as Harvester in Pause state");  
        return;          
        }
    while (iJobQueue.Count()>0  && iHState == EStateResume )
        {
        TMdeActionRecord object = iJobQueue[0];
        //iJobQueue.Remove(0);
        //Let the indexer handle this object TRAP it as it can leave
        TRAPD(err,iMdeObjectHandler->HandleMdeItemL(object.iObjectId, object.iAction));
        /*Process the Item for three time maximum if in case not able to be found from
          MDS*/
        if(KErrNone == err)
            {
            iJobQueue.Remove(0);
            }
        else if(KErrNotFound == err && object.iRetryCount < MAX_RETRY_COUNT)
                {
                iJobQueue.Remove(0); //remove existing and add new one 
                object.iRetryCount++;
                iJobQueue.AppendL(object);
                CPIXLOGSTRING3("CMdeObjectQueueManager::RunL HandleMdeItemL Item Not found err = %d, iRetry = %d",err,object.iRetryCount); 
                }
        else
            {
            iJobQueue.Remove(0);  //retry count exceeded remove it
            }
        }
#ifdef __PERFORMANCE_DATA
        iMdeObjectHandler->UpdateLogL();
#endif
        // Everything is indexed no need to be waiting anymore
        iState = EStateNone;
        
        if(iJobQueue.Count() <= 0)
            {
            if( BaflUtils::FileExists( iFs, iFilePath ))
                 BaflUtils::DeleteFile( iFs, iFilePath );
            }
        CPIXLOGSTRING("END CMdeObjectQueueManager::RunL");    
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::DoCancel()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::DoCancel()
    {
    iTimer.Cancel();
    iState = EStateNone; 
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::RunError()
// -----------------------------------------------------------------------------
//
TInt CMdeObjectQueueManager::RunError()
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCPixIndexerUtils::PauseL()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::PauseL()
    {
    iHState = EStatePause;
    }
// -----------------------------------------------------------------------------
// CCPixIndexerUtils::ResumeL()
// -----------------------------------------------------------------------------
//
void CMdeObjectQueueManager::ResumeL()
    {
    iHState = EStateResume;
    ActivateAO();       
    }

void CMdeObjectQueueManager::SetFilePath(const TDesC& aFilePath)
    {
    iFilePath.Copy(aFilePath);
    }

void CMdeObjectQueueManager::SaveQueuedItems()
    {
    if( iJobQueue.Count() <= 0)
        return;
    // Open the stream
    RFile file;
    User::LeaveIfError(file.Replace(iFs, iFilePath, EFileWrite));
    CleanupClosePushL(file);
    
    RFileWriteStream wr(file);
    wr.PushL();
    
    // Write harvester count
    wr.WriteInt32L(iJobQueue.Count());
    
    for (TInt i=0; i<iJobQueue.Count(); i++)    
        {
        wr.WriteUint32L(iJobQueue[i].iObjectId);
        wr.WriteInt16L(iJobQueue[i].iAction);        
        }    
    // Commit and cleanup
    wr.CommitL();
    CleanupStack::PopAndDestroy(2, &file);
    }

void CMdeObjectQueueManager::LoadQueuedItems()
    {
    // Open the stream
    RFile file;
    User::LeaveIfError(file.Open(iFs, iFilePath, EFileRead));
    CleanupClosePushL(file);
    RFileReadStream rd(file);
    rd.PushL();    
    
    // Read harvester count
    TInt count = rd.ReadInt32L();
    // Read the harvesters
    for (TInt i=0; i<count; i++)
        {
        TItemId iObjId= rd.ReadUint32L();
        TCPixActionType iActionType = static_cast<TCPixActionType> ( rd.ReadInt16L());        
        AddMdeItemToQueueL( iObjId, iActionType );
        }    
    // Cleanup
    CleanupStack::PopAndDestroy(2, &file);
    }
//End of file
