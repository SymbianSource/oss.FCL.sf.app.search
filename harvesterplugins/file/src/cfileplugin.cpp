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


#include <e32base.h>
#include <pathinfo.h> // PathInfo
#include <driveinfo.h> // TDriveInfo
#include "cfileplugin.h"
#include "harvesterserverlogger.h"
#include "cfileharvester.h"
#include "cfileplugin.h"
#include "cfilemonitor.h"
#include "cmmcmonitor.h"
#include <rsearchserversession.h>
#include <csearchdocument.h>
#include <common.h>

#include <ccpixindexer.h>

// local declarations and functions
namespace {

_LIT(KCPixSearchServerPrivateDirectory, "\\Private\\2001f6f7\\");
_LIT(KPathIndexDbPath, CPIX_INDEVICE_INDEXDB);

_LIT(KPathTrailer, "\\root\\file");
_LIT(KFileBaseAppClassGeneric, ":root file");
_LIT(KFilePluginAtSign, "@");
_LIT(KFilePluginColon, ":");

/**
* MapFileToDrive - returns the TDriveNumber that the file is located on.
* @aFilename filename
* @aDrive returns the TDriveNumber
* returns KErrNone on success or a standard error code
*/
TInt MapFileToDrive(const TDesC& aFilename, TDriveNumber& aDrive)
    {
    TParse file;
    file.Set(aFilename, NULL, NULL);
    TPtrC drvChar = file.Drive().Left(1);
    TChar d(drvChar[0]);

    TInt drive;
    const TInt ret = RFs::CharToDrive(d, drive);
    if (!ret)
        {
        aDrive = TDriveNumber(drive);
        }
    return ret;
    }

/**
* MapBaseAppClassToDrive - gets the TDriveNumber associated with the aBaseAppClass.
* @aBaseAppClass e.g. "@c:root file"
* @aDrive returns the TDriveNumber for the aBaseAppClass
* returns KErrNone on success or a standard error code
*/
TInt MapBaseAppClassToDrive(const TDesC& aBaseAppClass, TDriveNumber& aDrive)
    {
    if (KErrNone != aBaseAppClass.Left(1).Compare(KFilePluginAtSign))
        {
        return KErrGeneral;
        }

    TPtrC drvChar = aBaseAppClass.Left(2).Right(1);
    TChar d(drvChar[0]);
    TInt drive;
    const TInt ret = RFs::CharToDrive(d, drive);
    if (!ret)
        {
        aDrive = TDriveNumber(drive);
        }

    return ret;
    }

} // anonymous namespace

CFilePlugin* CFilePlugin::NewL()
    {
    CFilePlugin* instance = CFilePlugin::NewLC();
    CleanupStack::Pop(instance);
    return instance;
    }

CFilePlugin* CFilePlugin::NewLC()
    {
    CFilePlugin* instance = new (ELeave)CFilePlugin();
    CleanupStack::PushL(instance);
    instance->ConstructL();
    return instance;
    }

CFilePlugin::CFilePlugin()
    {
    for (TInt i=EDriveA; i<=EDriveZ; i++)
        {
        iIndexer[i] = NULL; //Initialize to NULL
        }
    }

CFilePlugin::~CFilePlugin()
	{
	CPIXLOGSTRING("CFilePlugin::~CFilePlugin()");

	// remove notification paths before destroying iMonitor
	for (TInt i=EDriveA; i<=EDriveZ; i++)
		{
		UnMount(TDriveNumber(i), EFalse);
		}
	delete iHarvester;
	delete iMonitor;
	delete iMmcMonitor;

	TInt err = iFs.DismountPlugin(KFastFindFSPluginName);
	CPIXLOGSTRING2("CFilePlugin::~CFilePlugin(), iFs.DismountPlugin: %i", err);
	err = iFs.RemovePlugin(KFastFindFSPluginName);
	CPIXLOGSTRING2("CFilePlugin::~CFilePlugin(), iFs.RemovePlugin: %i", err);
	iFs.Close();
	}

void CFilePlugin::ConstructL()
	{
    User::LeaveIfError( iFs.Connect() );
    TInt err = iFs.AddPlugin(KFastFindFSPluginFile);
    CPIXLOGSTRING2("CFilePlugin::ConstructL, iFs.AddPlugin: %i", err);
    if ( err != KErrAlreadyExists )
    	{
    	err = iFs.MountPlugin(KFastFindFSPluginName);
    	CPIXLOGSTRING2("CFilePlugin::ConstructL, iFs.MountPlugin: %i", err);
    	}
    // check if already up, unload and reload
    else if ( err == KErrAlreadyExists )
    	{
    	// dismount
    	TInt err = iFs.DismountPlugin(KFastFindFSPluginName);
	    CPIXLOGSTRING2("CFilePlugin::ConstructL(), iFs.DismountPlugin: %i", err);
		err = iFs.RemovePlugin(KFastFindFSPluginName);
		CPIXLOGSTRING2("CFilePlugin::ConstructL(), iFs.RemovePlugin: %i", err);
		// if no error reload
		if ( err == KErrNone )
			{
			err = iFs.AddPlugin(KFastFindFSPluginFile);
			CPIXLOGSTRING2("CFilePlugin::ConstructL, iFs.AddPlugin: %i", err);
			err = iFs.MountPlugin(KFastFindFSPluginName);
			CPIXLOGSTRING2("CFilePlugin::ConstructL, iFs.MountPlugin: %i", err);
			}
    	}

    iHarvester = CFileHarvester::NewL(*this, iFs);
    iMonitor = CFileMonitor::NewL(*this, &iFs);
    iMmcMonitor = CMMCMonitor::NewL(*this, &iFs);
    }

void CFilePlugin::StartPluginL()
	{
    TInt error = KErrNone;

    if (!iIsMonitorInit)
        {
        error = iMonitor->Initialize();
        CPIXLOGSTRING2("CFilePlugin::StartMonitoring, error: %i", error );
        iIsMonitorInit = ETrue;
        }

    // Start the monitoring
    if (error == KErrNone && iIsMonitorInit)
        {
        iMonitor->StartMonitoring();
        CPIXLOGSTRING("CFilePlugin::StartMonitoring - iFileMonitor->StartMonitoring ");
        }

    iMmcMonitor->StartMonitoring();

    CPIXLOGSTRING("END CFilePlugin::StartMonitoring");

    // Add harvesters for each non removable drive
    for ( TInt driveNumber = EDriveA; driveNumber <= EDriveZ; driveNumber++ )
        {
        if ( iFs.IsValidDrive( driveNumber ) )
            {
    		TUint drvStatus( 0 );
    		TInt err = DriveInfo::GetDriveStatus(iFs, TDriveNumber(driveNumber), drvStatus);
    		if ( err != KErrNone )
    			{
    			continue;
    			}

    		// Harvest drive that are internal, non-removable, user visible, present
    		if ( ( drvStatus & DriveInfo::EDriveInternal ) &&
    			!( drvStatus & DriveInfo::EDriveRemovable ) && // NOT removable
    			 ( drvStatus & DriveInfo::EDriveUserVisible ) &&
    			 ( drvStatus & DriveInfo::EDrivePresent ) )
    			{
    		    MountL(TDriveNumber(driveNumber));
    			}
            }
        }
}

void CFilePlugin::MountL(TDriveNumber aMedia, TBool aForceReharvest)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::MountL");
    // Check if already exists
    if (iIndexer[aMedia])
        return;

    // Add Notifications paths prior to opening IndexDB.
    AddNotificationPathsL(aMedia);

    // Form the baseappclass for this media
    TBuf<KFilePluginBaseAppClassMaxLen> baseAppClass;
    FormBaseAppClass(aMedia, baseAppClass);

    // Define this volume
    HBufC* path = DatabasePathLC(aMedia);
    User::LeaveIfError(iSearchSession.DefineVolume(baseAppClass, *path));
    CleanupStack::PopAndDestroy(path);
    
    // construct and open the database
    iIndexer[aMedia] = CCPixIndexer::NewL(iSearchSession);
    iIndexer[aMedia]->OpenDatabaseL(baseAppClass);

    // Add to harvesting queue
    iObserver->AddHarvestingQueue(this, baseAppClass, aForceReharvest);
    CPIXLOGSTRING("END CFilePlugin::MountL");
    }

void CFilePlugin::UnMount(TDriveNumber aMedia, TBool aUndefineAsWell)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::UnMount ");
    // Check if already exists
    if (!iIndexer[aMedia])
        {
        return;
        }

    // Form the baseappclass for this media
    TBuf<KFilePluginBaseAppClassMaxLen> baseAppClass;
    FormBaseAppClass(aMedia, baseAppClass);

    // Remove from harvesting queue
    iObserver->RemoveHarvestingQueue(this, baseAppClass);
    
    // Delete the index object
    if (iIndexer[aMedia])
        {
        delete iIndexer[aMedia];
        iIndexer[aMedia] = NULL;
        }

    RemoveNotificationPaths(aMedia);

    if (aUndefineAsWell)
        {
        iSearchSession.UnDefineVolume(baseAppClass);
        }
    CPIXLOGSTRING("END CFilePlugin::UnMount ");
    }

void CFilePlugin::StartHarvestingL(const TDesC& aQualifiedBaseAppClass)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::StartHarvestingL ");
    // Map base app class to a drive number
    TDriveNumber drive(EDriveA); //Initialize to get rid of compiler warning.
    if (KErrNone != MapBaseAppClassToDrive(aQualifiedBaseAppClass, drive))
        {
        User::Leave(KErrGeneral);
        }

    // Leave if no indexer for this drive
    if (!iIndexer[drive])
        {
        User::Leave(KErrGeneral);
        }

#ifdef __PERFORMANCE_DATA
    	iStartTime[drive].UniversalTime();
#endif
    // Reset the database
    iIndexer[drive]->ResetL();

    // Start the actual harvest
    iHarvester->StartL(drive);
    CPIXLOGSTRING("END CFilePlugin::StartHarvestingL ");
    }

void CFilePlugin::CreateFileIndexItemL(const TDesC& aFilename, TCPixActionType aActionType)
    {
    TFileName lowerCaseFilename(aFilename);
    lowerCaseFilename.LowerCase();

    CPIXLOGSTRING3("CFilePlugin::CreateFileIndexItemL lowerCaseFilename = %S aActionType = %d ", 
					&lowerCaseFilename, aActionType);

	TDriveNumber drive(EDriveA);
	User::LeaveIfError( MapFileToDrive( lowerCaseFilename, drive ) );

	CCPixIndexer* indexer = iIndexer[drive];
	if (!indexer)
		{
		CPIXLOGSTRING("CFilePlugin::CreateFileIndexItemL(): Could not map file to drive.");
		return;
		}

	// Index an empty item if removal action
	if (aActionType == ECPixRemoveAction)
		{
#ifdef _DEBUG
		TRAPD(err, indexer->DeleteL(lowerCaseFilename));
		CPIXLOGSTRING2("CFilePlugin::CreateFileIndexItemL(): DeleteL returned %d.", err);
#else   
		TRAP_IGNORE( indexer->DeleteL(lowerCaseFilename) );
#endif
		}
	else
		{
		// creating CSearchDocument object with unique ID for this application
		CSearchDocument* index_item = CSearchDocument::NewLC(lowerCaseFilename, KNullDesC, KNullDesC, CSearchDocument::EFileParser);

		// Send for indexing
		if (aActionType == ECPixAddAction)
			{
#ifdef _DEBUG
			TRAPD(err, indexer->AddL(*index_item));
	        CPIXLOGSTRING2("CFilePlugin::CreateFileIndexItemL(): AddL returned %d.", err);
#else
			TRAP_IGNORE( indexer->AddL(*index_item) );
#endif
			}
		else if (aActionType == ECPixUpdateAction)
			{
#ifdef _DEBUG		
			TRAPD(err, indexer->UpdateL(*index_item));
	        CPIXLOGSTRING2("CFilePlugin::CreateFileIndexItemL(): UpdateL returned %d.", err);
#else
			TRAP_IGNORE( indexer->UpdateL(*index_item) );
#endif
			}
		CleanupStack::PopAndDestroy(index_item);
		}
    }

void CFilePlugin::HarvestingCompleted(TDriveNumber aDriveNumber, TInt aError)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::HarvestingCompleted ");

    if (iIndexer[aDriveNumber])
        {
        Flush(*iIndexer[aDriveNumber]);
        }
    TBuf<KFilePluginBaseAppClassMaxLen> baseAppClass;
    FormBaseAppClass(TDriveNumber(aDriveNumber), baseAppClass);
#ifdef __PERFORMANCE_DATA
    TRAP_IGNORE( UpdatePerformaceDataL(aDriveNumber) );
#endif
    iObserver->HarvestingCompleted(this, baseAppClass, aError);

    CPIXLOGSTRING("END CFilePlugin::HarvestingCompleted ");
    }

void CFilePlugin::AddNotificationPathsL(const TDriveNumber aDriveNumber)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::AddNotificationPathsL ");
    iMonitor->AddNotificationPathsL(aDriveNumber);
    iHarvester->AddIgnorePathsL(aDriveNumber);
    CPIXLOGSTRING("END CFilePlugin::AddNotificationPathsL ");
    }

void CFilePlugin::RemoveNotificationPaths(const TDriveNumber aDriveNumber)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::RemoveNotificationPaths");
    iMonitor->RemoveNotificationPaths(aDriveNumber);
    iHarvester->RemoveIgnorePaths(aDriveNumber);
    CPIXLOGSTRING("END CFilePlugin::RemoveNotificationPaths");
    }

TInt CFilePlugin::FormBaseAppClass(TDriveNumber aMedia, TDes& aBaseAppClass)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::FormBaseAppClass");
    TChar chr;
    const TInt ret = RFs::DriveToChar(aMedia, chr);
    if (KErrNone == ret)
        {
        aBaseAppClass.Copy(KFilePluginAtSign);
        aBaseAppClass.Append(chr);
        aBaseAppClass.LowerCase();
        aBaseAppClass.Append(KFileBaseAppClassGeneric);
        }

    CPIXLOGSTRING("END CFilePlugin::FormBaseAppClass");
    return ret;
    }

HBufC* CFilePlugin::DatabasePathLC(TDriveNumber aMedia)
    {
    CPIXLOGSTRING("ENTER CFilePlugin::DatabasePathLC");
    // Allocate extra space for root path e.g. "C:\\Private\\2001f6f7\\"
    const TInt KRootPathMaxLength = 30;
    HBufC* indexDbPath = HBufC::NewLC(KRootPathMaxLength + KPathIndexDbPath().Length() + KPathTrailer().Length());
    TPtr indexDbPathPtr = indexDbPath->Des();

    // Data caging implementation
    iFs.CreatePrivatePath(aMedia);

    TChar chr;
    RFs::DriveToChar(aMedia, chr);
    indexDbPathPtr.Append(chr);
    indexDbPathPtr.Append(KFilePluginColon);

    TFileName pathWithoutDrive;
    iFs.PrivatePath(pathWithoutDrive);
    indexDbPathPtr.Append(KCPixSearchServerPrivateDirectory);

    indexDbPathPtr.Append(KPathIndexDbPath);
    indexDbPathPtr.Append(KPathTrailer);

    CPIXLOGSTRING("END CFilePlugin::DatabasePathLC");
    return indexDbPath;
    }

#ifdef __PERFORMANCE_DATA
void CFilePlugin::UpdatePerformaceDataL(TDriveNumber aDriveNumber)
    {
    TTime now;
  	TChar aChar;
    
    iCompleteTime.UniversalTime();
    TTimeIntervalMicroSeconds timeDiff = iCompleteTime.MicroSecondsFrom(iStartTime[aDriveNumber]);
    
    RFs fileSession;
    RFile perfFile;
    User::LeaveIfError( fileSession.Connect () );
	RFs::DriveToChar((TInt)aDriveNumber, aChar);	
    
    
    /* Open file if it exists, otherwise create it and write content in it */
    
        if(perfFile.Open(fileSession, _L("c:\\data\\FilePerf.txt"), EFileWrite))
                   User::LeaveIfError(perfFile.Create (fileSession, _L("c:\\data\\FilePerf.txt"), EFileWrite));
    
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
    ptr.Append(_L("Drive "));
    ptr.Append(aChar);
    ptr.Append( _L(" Ani: Time took for Harvesting File is : "));
    ptr.AppendNum(timeDiff.Int64()/1000) ;
    ptr.Append(_L(" MilliSeonds \n"));
    TInt myInt = 0;
    perfFile.Seek(ESeekEnd,myInt);
    perfFile.Write (ptr);
    perfFile.Close ();
    fileSession.Close ();
    delete heap;
    }
#endif

// End of file

