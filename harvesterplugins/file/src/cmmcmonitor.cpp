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
* Description:  Mmc monitor
*
*/


// INCLUDE FILES
#include "cmmcmonitor.h"
#include "cfileplugin.h"
#include "harvesterserverlogger.h"

#include <pathinfo.h>
#include <s32file.h>
#include <f32file.h> // TDriveNumber
#include <driveinfo.h> // TDriveInfo

#include <uikoninternalpskeys.h>

// CONSTANTS
// 

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CMMCMonitor::NewL
// -----------------------------------------------------------------------------
//
CMMCMonitor* CMMCMonitor::NewL( CFilePlugin& aFilePlugin, RFs* aFsSession )
    {
    CMMCMonitor* self = new ( ELeave ) CMMCMonitor( aFilePlugin );
    CleanupStack::PushL( self );
    self->ConstructL( aFsSession );
    CleanupStack::Pop( self );
    return self;
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::CMMCMonitor
// -----------------------------------------------------------------------------
//
CMMCMonitor::CMMCMonitor( CFilePlugin& aFilePlugin )
    : CActive( CActive::EPriorityStandard ),
      iFilePlugin( aFilePlugin )
    {
    CPIXLOGSTRING("ENTER CMMCMonitor::CMMCMonitor");
    CActiveScheduler::Add( this );
    CPIXLOGSTRING("END CMMCMonitor::CMMCMonitor");
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::~CMMCMonitor
// -----------------------------------------------------------------------------
//
CMMCMonitor::~CMMCMonitor()
    {
    CPIXLOGSTRING("ENTER ~CMMCMonitor");

    Cancel();
    iProperty.Close();
    CPIXLOGSTRING("END ~CMMCMonitor");
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::ConstructL
// -----------------------------------------------------------------------------
//
void CMMCMonitor::ConstructL( RFs* aFsSession )
    {
    CPIXLOGSTRING("ENTER CMMCMonitor::ConstructL Foobar");
    iFsSession = aFsSession;
    
    TInt error = iProperty.Attach( KPSUidUikon, KUikMMCInserted );
    if ( error != KErrNone ) CPIXLOGSTRING("END CMMCMonitor::Attach to MMCInserted failed");
    
    error = iProperty.Get( KPSUidUikon, KUikMMCInserted, iMmcStatus );
    if ( error != KErrNone ) 
    	{
		CPIXLOGSTRING("CMMCMonitor::Get MMCInserted failed");
    	} 
    else if ( iMmcStatus ) 	 
    	{
    	CPIXLOGSTRING("CMMCMonitor::MMC card is in");
    	}
    else 
    	{
    	CPIXLOGSTRING("CMMCMonitor::no MMC card");
    	}

    // The CFilePlugin::StartMonitoring() will call
    // CMMCMonitor::StartMonitoring() which will call
    // CMMCMonitor::RunL().
    //
    CPIXLOGSTRING("END CMMCMonitor::ConstructL");
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::StartMonitoring
// -----------------------------------------------------------------------------
//
TBool CMMCMonitor::StartMonitoring()
    {
    CPIXLOGSTRING("ENTER CMMCMonitor::StartMonitoring");
    TRAP_IGNORE( RunL() ); // Need to TRAP this rather than use RunError
    CPIXLOGSTRING("END CMMCMonitor::StartMonitoring");
    return ETrue;
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::MmcStatus
// -----------------------------------------------------------------------------
//
TBool CMMCMonitor::MmcStatus( TInt aDriveNumber )
    {
    TBool isMmcPresent(EFalse);

    if ( iFsSession->IsValidDrive( aDriveNumber ) )
        {
        TUint drvStatus( 0 );
        TInt err = DriveInfo::GetDriveStatus( *iFsSession, aDriveNumber, drvStatus );
        if ( err )
            {
            return EFalse;
            }
        // MMC drives are removable and user visible
        if ( ( drvStatus & DriveInfo::EDriveRemovable ) &&
             ( drvStatus & DriveInfo::EDriveUserVisible ) )
                {
                CPIXLOGSTRING2("CMMCMonitor::MmcStatus Drive Number %d", aDriveNumber);
                isMmcPresent = ETrue;
                }
        }
    return isMmcPresent;
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::RunError
// -----------------------------------------------------------------------------
//
TInt CMMCMonitor::RunError( TInt aError )
    {
    CPIXLOGSTRING2("CMMCMonitor::RunError Error:",aError);
    return KErrNone;
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::DoCancel
// -----------------------------------------------------------------------------
//
void CMMCMonitor::DoCancel()
    {
    CPIXLOGSTRING("ENTER CMMCMonitor::DoCancel");
    iProperty.Cancel();
    CPIXLOGSTRING("END CMMCMonitor::DoCancel");
    }


// -----------------------------------------------------------------------------
// CMMCMonitor::RunL
// -----------------------------------------------------------------------------
//
void CMMCMonitor::RunL()
    {
    CPIXLOGSTRING("ENTER CMMCMonitor::RunL");
    iProperty.Subscribe( iStatus );
    SetActive();
    User::LeaveIfError( iProperty.Get( KPSUidUikon, KUikMMCInserted, iMmcStatus ) );
    
    if ( iMmcStatus )
    	{
    	CPIXLOGSTRING("CMMCMonitor::MMC card is in");
    	}
    else 
    	{
		CPIXLOGSTRING("CMMCMonitor::no MMC card");
    	}

    for ( TInt driveNumber = EDriveA; driveNumber <= EDriveZ; driveNumber++ )
        {
        const TBool foundMmc = MmcStatus( driveNumber );
        if ( !foundMmc )
            {
            continue;
            }

        // This drive has been recognized as MMC. 
        TDriveNumber drv = TDriveNumber( driveNumber );

        TUint drvStatus( 0 );

        const TInt err = DriveInfo::GetDriveStatus( *iFsSession, driveNumber, drvStatus );
        if ( err ) 
            {
            continue;  // should not happen
            }

        if ( drvStatus & DriveInfo::EDrivePresent )
            {
            CPIXLOGSTRING("CMMCMonitor::RunL insert event");
            // Mount MMC and force reharvest
            iFilePlugin.MountL(drv, ETrue);
            }
        else
            {
            CPIXLOGSTRING("CMMCMonitor::RunL eject event");
            // If the MMC has been ejected, then need to dismount 
            // and undefine the volume
            iFilePlugin.UnMount(drv, ETrue);
            }
        }
    CPIXLOGSTRING("END CMMCMonitor::RunL");
    }

// End Of File
