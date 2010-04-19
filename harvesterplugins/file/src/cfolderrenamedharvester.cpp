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


//  INCLUDES 
#include <e32base.h>
#include <pathinfo.h>
#include <bautils.h>

// For Logging
#include "harvesterserverlogger.h"

#include "cfileplugin.h"
#include "cfolderrenamedharvester.h"

// CONSTANTS
_LIT( KFileMask, "*.*" );
const TInt KItemsPerRun = 1;

namespace {

void AppendBackslash( TDes& aDirectory )
    {
    const TChar KBackslashChar( '\\' );
    const TInt lastChar = aDirectory.Length() - 1;

    // If it is directory which not contain backslash. 
    if ( KBackslashChar != aDirectory[lastChar] )
       {
       aDirectory.Append( KBackslashChar );
       }
    }

} // namespace

// ============================ MEMBER FUNCTIONS ===============================


// ---------------------------------------------------------------------------
// CFolderRenamedItem::NewL
// ---------------------------------------------------------------------------
//
CFolderRenamedItem * CFolderRenamedItem::NewL(const TDesC &aOldName, const TDesC &aNewName)
	{
	CFolderRenamedItem *self = new (ELeave) CFolderRenamedItem();
	CleanupStack::PushL( self );
	self->ConstructL( aOldName, aNewName );
	CleanupStack::Pop( self );
	return self;
	}


// ---------------------------------------------------------------------------
// CFolderRenamedItem::~CFolderRenamedItem
// ---------------------------------------------------------------------------
//
CFolderRenamedItem::~CFolderRenamedItem()
    {
    delete iOldName;
    delete iNewName;
    }

// ---------------------------------------------------------------------------
// CFolderRenamedItem::CFolderRenamedItem
// ---------------------------------------------------------------------------
//
CFolderRenamedItem::CFolderRenamedItem()
	{
	}


// ---------------------------------------------------------------------------
// CFolderRenamedItem::ConstructL
// ---------------------------------------------------------------------------
//
void CFolderRenamedItem::ConstructL(const TDesC &aOldName, const TDesC &aNewName)
	{
	iOldName = aOldName.AllocL();
	iNewName = aNewName.AllocL();
	}


// ---------------------------------------------------------------------------
// CFolderRenamedItem::OldName
// ---------------------------------------------------------------------------
//
const TDesC& CFolderRenamedItem::OldName()
	{
	return *iOldName;
	}


// ---------------------------------------------------------------------------
// CFolderRenamedItem::NewName
// ---------------------------------------------------------------------------
//
const TDesC& CFolderRenamedItem::NewName()
	{
	return *iNewName;
	}

// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::NewL
// ---------------------------------------------------------------------------
//
CFolderRenamedHarvester* CFolderRenamedHarvester::NewL(  CFilePlugin& aFilePlugin, RFs& aFs )
	{
	CFolderRenamedHarvester* self = new (ELeave) CFolderRenamedHarvester( aFilePlugin, aFs );
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::~CFolderRenamedHarvester
// ---------------------------------------------------------------------------
//
CFolderRenamedHarvester::~CFolderRenamedHarvester()
    {
    delete iDir;
    delete iDirscan;
    iRenamedFolders.ResetAndDestroy();
    iRenamedFolders.Close();
    Cancel();
    }

// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::CFolderRenamedHarvester

// ---------------------------------------------------------------------------
//
CFolderRenamedHarvester::CFolderRenamedHarvester(  CFilePlugin& aFilePlugin, RFs& aFs  ) 
  : CActive( CActive::EPriorityIdle ),
    iFilePlugin( aFilePlugin ),
    iFs( aFs )
    {
    CPIXLOGSTRING("ENTER CFolderRenamedHarvester::CFolderRenamedHarvester");
    CActiveScheduler::Add( this );
    CPIXLOGSTRING("END CFolderRenamedHarvester::CFolderRenamedHarvester");
    }


// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::ConstructL
// ---------------------------------------------------------------------------
//
void CFolderRenamedHarvester::ConstructL()
    {
    iDirscan = CDirScan::NewL( iFs );
    }

// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::StartL
// ---------------------------------------------------------------------------
//
TBool CFolderRenamedHarvester::StartL( const TFileName& aOldDirectoryName, 
									   const TFileName& aNewDirectoryName )
    {
    CPIXLOGSTRING("ENTER CFolderRenamedHarvester::Start");

    // Append trailing backslash if required
    TFileName oldName ( aOldDirectoryName );
    AppendBackslash( oldName );
    TFileName newName ( aNewDirectoryName );
    AppendBackslash( newName );

    CFolderRenamedItem *renameItem = CFolderRenamedItem::NewL( oldName, newName );
    iRenamedFolders.AppendL( renameItem );

	SetNextRequest( EFolderRenamedIdleState );
    CPIXLOGSTRING("END CFolderRenamedHarvester::Start");
    return ETrue;
    }

// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::GetNextFolderL
// ---------------------------------------------------------------------------
//		
void CFolderRenamedHarvester::GetNextFolderL()
    {
    CPIXLOGSTRING("ENTER CFolderRenamedHarvester::GetNextFolderL");   

    delete iDir;
    iDir = NULL;
    // Documentation: CDirScan::NextL() The caller of this function 
    // is responsible for deleting iDir after the function has returned.
    iDirscan->NextL(iDir); 

    if( iDir )
        {
        CPIXLOGSTRING("CFolderRenamedHarvester::GetNextFolderL - IF EHarvesterGetFileId"); 
        SetNextRequest( EFolderRenamedGetFileId );
        }
    else
        {
        CPIXLOGSTRING("CFolderRenamedHarvester::GetNextFolderL - IF EHarvesterIdle");

        // finished folder rename, delete the folder from the array.
        CFolderRenamedItem *item = iRenamedFolders[0];
        iRenamedFolders.Remove(0);
        delete item;

        SetNextRequest( EFolderRenamedIdleState );
        }
    CPIXLOGSTRING("END CFolderRenamedHarvester::GetNextFolderL");
    }

// ---------------------------------------------------------------------------
// CFolderRenamedHarvester::GetFileIdL
// 
// ---------------------------------------------------------------------------
//		    
void CFolderRenamedHarvester::GetFileIdL()
    {
    CPIXLOGSTRING("ENTER CFolderRenamedHarvester::GetFileId");

    if( iCurrentIndex == 0 )
        {
        TParse parse;
        parse.Set(KFileMask(), &iDirscan->FullPath(), NULL);

        // FindWildByPath assigns iDir = NULL, then allocates the memory for it.
        // Therefore must delete iDir first.
        delete iDir;
        iDir = NULL;

        TFindFile find( iFs );
        find.FindWildByPath(parse.FullName(), NULL, iDir);
        }

    if( iDir )
        {
        const TInt count(iDir->Count());
        while( ( iCurrentIndex < count ) && ( iStepNumber < KItemsPerRun ) )
            {
            TEntry entry = (*iDir)[iCurrentIndex];
            // Check if entry is a hidden or system file
            // if true -> continue until find something to index or have checked whole directory
            if( !entry.IsHidden() && !entry.IsSystem() && !entry.IsDir() )
                {
                TParse fileParser;
                fileParser.Set( iDirscan->FullPath(), &(*iDir)[iCurrentIndex].iName, NULL );

                TPtrC fileNamePtr = fileParser.FullName();
                TPtrC leaf( fileNamePtr.Right( fileNamePtr.Length() - iNewFolderName.Length() ) );
                TFileName oldFileName( iOldFolderName );
                oldFileName.Append( leaf );

                iFilePlugin.CreateFileIndexItemL( oldFileName, ECPixRemoveAction );
                iFilePlugin.CreateFileIndexItemL( fileParser.FullName(), ECPixAddAction );
                // TODO: If this is not TRAPPED, state machine breaks 
                iStepNumber++;
                }
            iCurrentIndex++;
            }
            
        iStepNumber = 0;
        
        if( iCurrentIndex >= count )
            {
            iCurrentIndex = 0;
            SetNextRequest( EFolderRenamedStartHarvest );
            }
        else
            {
            SetNextRequest( EFolderRenamedGetFileId );
            }
        }
    else
        {
        SetNextRequest( EFolderRenamedStartHarvest );
        }
    CPIXLOGSTRING("END CFolderRenamedHarvester::GetFileId"); 
    }			

      
// -----------------------------------------------------------------------------
// CFolderRenamedHarvester::DoCancel
// -----------------------------------------------------------------------------
//   
void CFolderRenamedHarvester::DoCancel()
	{
	CPIXLOGSTRING("CFolderRenamedHarvester::DoCancel");
	}

// -----------------------------------------------------------------------------
// CFolderRenamedHarvester::RunL
// -----------------------------------------------------------------------------
//   
void CFolderRenamedHarvester::RunL()
    {
    CPIXLOGSTRING("ENTER CFolderRenamedHarvester::RunL");
     // Simple Round-Robin scheduling.
    Deque();
    CActiveScheduler::Add( this );

    switch ( iHarvestState )
        {
		case EFolderRenamedIdleState:
			{
			if ( iRenamedFolders.Count() == 0 )
				{
				return; // Nothing to do.
				}

			// Some rename folder action to perform
			iOldFolderName.Copy( iRenamedFolders[0]->OldName() );
			iNewFolderName.Copy( iRenamedFolders[0]->NewName() );

			iDirscan->SetScanDataL( iRenamedFolders[0]->NewName(),
						KEntryAttMatchExclusive|KEntryAttNormal,
						ESortNone, // No need to sort data
						CDirScan::EScanDownTree );
			// No break, fall through to EFolderRenamedStartHarvest case
			}
		case EFolderRenamedStartHarvest:
			{
			GetNextFolderL();
			break;
			}
		case EFolderRenamedGetFileId:
			{
			GetFileIdL();
			break;
			}
		default:
			break;
        }
    CPIXLOGSTRING("END CFolderRenamedHarvester::RunL");
    }

// -----------------------------------------------------------------------------
// CFolderRenamedHarvester::RunError
// -----------------------------------------------------------------------------
//   
TInt CFolderRenamedHarvester::RunError(TInt aError)
	{
    CPIXLOGSTRING2("CFolderRenamedHarvester::RunError - aError: %d", aError );
    iRenamedFolders.ResetAndDestroy();
	iHarvestState = EFolderRenamedIdleState;
	return KErrNone;
	}
	

// ---------------------------------------------------------------------------
// SetNextRequest
// ---------------------------------------------------------------------------
//
void CFolderRenamedHarvester::SetNextRequest( TFileHarvesterState aState )
    {
    CPIXLOGSTRING("CFolderRenamedHarvester::SetNextRequest");
    if ( !IsActive() )
        {
        iHarvestState = aState;
        SetActive();
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    }


// End of File
