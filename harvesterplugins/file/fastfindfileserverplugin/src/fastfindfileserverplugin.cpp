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
* Description:  Monitors file creations, modifications and deletions.
*
*/

#include <f32pluginutils.h>
#include "fastfindfileserverplugin.h"
#include "harvesterlog.h"

_LIT( KFastFindFileServerPlugin, "CPixFileServerPlugin" );

//-----------------------------------------------------------------------------
// CFastFindFileServerPlugin implementation
//-----------------------------------------------------------------------------
//
CFastFindFileServerPlugin::CFastFindFileServerPlugin()
: iFormatDriveNumber( -1 )
	{
	WRITELOG( "CFastFindFileServerPlugin::CFastFindFileServerPlugin()" );
	}
	
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
CFastFindFileServerPlugin::~CFastFindFileServerPlugin()
    {
    WRITELOG( "CFastFindFileServerPlugin::~CFastFindFileServerPlugin()" );
    
    TRAP_IGNORE( DisableL() );
    iFsSession.Close();
    
    iCreatedFiles.ResetAndDestroy();
    iCreatedFiles.Close();
    
    iPaths.ResetAndDestroy();
    iPaths.Close();
    
    iIgnorePaths.ResetAndDestroy();
    iIgnorePaths.Close();
        
    iQueue.ResetAndDestroy();
    iQueue.Close();
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
CFastFindFileServerPlugin* CFastFindFileServerPlugin::NewL()
    {
    WRITELOG( "CFastFindFileServerPlugin::NewL()" );
    return new (ELeave) CFastFindFileServerPlugin;
    }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::InitialiseL()
    {
    WRITELOG( "CFastFindFileServerPlugin::InitializeL()" );
    User::LeaveIfError( iFsSession.Connect() );
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::EnableL()
    {
    WRITELOG( "CFastFindFileServerPlugin::EnableL()" );
    User::LeaveIfError( RegisterIntercept(EFsFileCreate, EPostIntercept) );
    User::LeaveIfError( RegisterIntercept(EFsFileRename, EPostIntercept) );
    User::LeaveIfError( RegisterIntercept(EFsRename, EPostIntercept) );
    User::LeaveIfError( RegisterIntercept(EFsDelete, EPostIntercept) );
//    User::LeaveIfError( RegisterIntercept(EFsFileReplace, EPostIntercept) );
    User::LeaveIfError( RegisterIntercept(EFsReplace, EPostIntercept) );
//    User::LeaveIfError( RegisterIntercept(EFsFileModified, EPostIntercept) );
    User::LeaveIfError( RegisterIntercept(EFsFileSetModified, EPostIntercept) );
    User::LeaveIfError( RegisterIntercept(EFsFileSubClose, EPostIntercept) );
	// format events
	User::LeaveIfError( RegisterIntercept(EFsFormatSubClose, EPostIntercept) );
	User::LeaveIfError( RegisterIntercept(EFsFormatOpen, EPostIntercept) );

#ifdef _DEBUG_EVENTS
    RegisterDebugEventsL();
#endif
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::DisableL()
    {
    WRITELOG( "CFastFindFileServerPlugin::DisableL()" );
    User::LeaveIfError( UnregisterIntercept(EFsFileCreate, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept(EFsFileRename, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept(EFsRename, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept(EFsDelete, EPrePostIntercept) );
//    User::LeaveIfError( UnregisterIntercept(EFsFileReplace, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept(EFsReplace, EPrePostIntercept) );
//    User::LeaveIfError( UnregisterIntercept(EFsFileModified, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept(EFsFileSetModified, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept(EFsFileSubClose, EPrePostIntercept) );
	// format events
	User::LeaveIfError( UnregisterIntercept(EFsFormatSubClose, EPostIntercept) );
	User::LeaveIfError( UnregisterIntercept(EFsFormatOpen, EPostIntercept) );

#ifdef _DEBUG_EVENTS
    UnregisterDebugEventsL();
#endif
    }
    

//-----------------------------------------------------------------------------
// AddConnection
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::AddConnection()
    {
    WRITELOG( "CFastFindFileServerPlugin::AddConnection()" );
    
    ++iConnectionCount;
    }

//-----------------------------------------------------------------------------
// RemoveConnection
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::RemoveConnection()
    {
    WRITELOG( "CFastFindFileServerPlugin::RemoveConnection()" );
    
    --iConnectionCount;

    // remove notification request if this was last connection
    if ( iConnectionCount <= 0 )
        {
        WRITELOG( "CFastFindFileServerPlugin::RemoveConnection() last connection" );

        iNotification = NULL;
        }
    }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPlugin::DoRequestL( TFsPluginRequest& aRequest )
	{
	TInt err = KErrNone;
	TInt function = aRequest.Function();
	const TBool formatFunction = function == EFsFormatOpen || function == EFsFormatSubClose;

#ifdef _DEBUG_EVENTS
    PrintDebugEvents( function );
#endif

    if ( function == EFsFileSubClose && iCreatedFiles.Count() == 0 )
        {
        return KErrNone;
        }

    WRITELOG1( "----- CFastFindFileServerPlugin::DoRequestL() - plugin function: %d -----", function );

    TFileName fileName;
    fileName.Zero();
    TFileName newFileName;
    newFileName.Zero();

	if ( !formatFunction )
		{
		GetName( &aRequest, fileName );
		WRITELOG1( "CFastFindFileServerPlugin::DoRequestL() - fileName: %S", &fileName );
		}

    // get process id
	TUid processId = { 0 };

    /*RThread clientThread;
    RProcess clientProcess;
    err = aRequest.Message().Client( clientThread, EOwnerThread );
    if ( err == KErrNone )
        {
        err = clientThread.Process( clientProcess );
        if ( err == KErrNone )
            {
            processId = clientProcess.Identity();
            }
        }
    clientProcess.Close();
    clientThread.Close();*/

	processId = aRequest.Message().Identity();

    // if rename, check destination path
    if ( function == EFsRename || function == EFsFileRename || function == EFsReplace )
        {
        GetNewName( &aRequest, newFileName );
        WRITELOG1( "CFastFindFileServerPlugin::DoRequestL() - newFileName: %S", &newFileName );
        if ( !CheckPath(newFileName) )
            {
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - path not supported" );
            return KErrNone;
            }
        if ( !CheckAttribs(newFileName) )
            {
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - attribute check failed" );
            return KErrNone;
            }
        }
    else if ( !formatFunction )
        {
        if ( !CheckPath(fileName) )
            {
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - path not supported" );
            return KErrNone;
            }

        if ( !CheckAttribs(fileName) )
            {
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - attribute check failed" );
            return KErrNone;
            }
        }

    TInt fileEventType = EFastFindFileUnknown;
    TInt drvNumber = aRequest.DriveNumber();

    WRITELOG1( "CFastFindFileServerPlugin::DoRequestL() - drive number: %d", drvNumber );

    switch( function )
        {
        case EFsFileCreate:
            {
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - EFsFileCreate" );

            TFileName* fn = new (ELeave) TFileName;
            fn->Zero();
            fn->Copy( fileName );
            iCreatedFiles.Append( fn );
            User::LeaveIfError( UnregisterIntercept(EFsFileSetModified, EPrePostIntercept) );
            return KErrNone;
            }

        case EFsFileSubClose:
            {
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - EFsFileSubClose" );

            TBool found = EFalse;
            for ( TInt i = 0; i < iCreatedFiles.Count(); i++ )
                {
                TFileName* fn = iCreatedFiles[i];
                if ( fn->CompareC(fileName,1,NULL) == 0 )
                    {
                    fileEventType = EFastFindFileCreated;
                    delete fn;
                    fn = NULL;
                    iCreatedFiles.Remove( i );
                    found = ETrue;
                    User::LeaveIfError( RegisterIntercept(EFsFileSetModified, EPostIntercept) );
                    }
                }

            if ( !found )
                {
                return KErrNone;
                }
            }
            break;

        case EFsRename:
            {
            WRITELOG1( "CFastFindFileServerPlugin::DoRequestL() - EFsRenamed, new file: %S", &newFileName );
            fileEventType = EFastFindFileRenamed;
            }
            break;

        case EFsFileRename:
            WRITELOG1( "CFastFindFileServerPlugin::DoRequestL() - EFsFileRenamed, new file: %S", &newFileName );
            fileEventType = EFastFindFileRenamed;
            break;

        case EFsFileSetModified:
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - EFsFileModified" );
            fileEventType = EFastFindFileModified;
            break;

        case EFsReplace:
            WRITELOG1( "CFastFindFileServerPlugin::DoRequestL() - EFsReplace, new file: %S", &newFileName );
            fileEventType = EFastFindFileReplaced;
            break;

        case EFsDelete:
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - EFsDelete" );
            fileEventType = EFastFindFileDeleted;
            break;

		case EFsFormatOpen:
			WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - EFsFormatOpen" );
			// get the drive letter
			iFormatDriveNumber = drvNumber;
			fileEventType = EFastFindDriveFormatted;
			processId.iUid = 0;
			//return KErrNone;
			break;

		case EFsFormatSubClose:
			WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - EFsFormatSubClose" );
			if ( iFormatDriveNumber < 0 )
				{
				return KErrNone;
				}
			drvNumber = iFormatDriveNumber;
			iFormatDriveNumber = -1;
			fileEventType = EFastFindDriveFormatted;
			if ( processId.iUid == 0 )
				{
				processId.iUid = 1;
				}
			break;

        default:
            WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - Unknown function" );
            return KErrNone;
        }

    if ( iNotification )
        {
        WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - iNotification found" );
        TFastFindFSPStatusPckg clientStatusBuf;
        TFastFindFSPStatus& clientStatus = clientStatusBuf();
        clientStatus.iDriveNumber = drvNumber;
        clientStatus.iFileName.Copy( fileName );
        if ( newFileName.Length() > 0 )
            {
            clientStatus.iNewFileName.Copy( newFileName );
            }
        clientStatus.iFileEventType = fileEventType;
        clientStatus.iProcessId = processId;

        TRAP( err, iNotification->WriteParam1L(clientStatusBuf) );
        iNotification->Complete( err );
        iNotification = NULL;
        WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - iNotification complete" );
        }
    else // no notification ready, put in the queue
        {
        WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - iNotification not found. Put in the queue" );

        TFastFindFSPStatus* clientStatus = new (ELeave) TFastFindFSPStatus;
        clientStatus->iDriveNumber = drvNumber;
        clientStatus->iFileName.Copy( fileName );
        if ( newFileName.Length() > 0 )
            {
            clientStatus->iNewFileName.Copy( newFileName );
            }
        clientStatus->iFileEventType = fileEventType;
        clientStatus->iProcessId = processId;

        iQueue.Append( clientStatus ); // owenership is transferred
        err = KErrNone;
        WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - added to queue" );
        }

    WRITELOG( "CFastFindFileServerPlugin::DoRequestL() - return" );
    return err;
    }

//-----------------------------------------------------------------------------
// CFastFindFileServerPluginConn implementation
//-----------------------------------------------------------------------------
class CFastFindFileServerPluginConn : public CFsPluginConn
    {
    public:
        static CFastFindFileServerPluginConn* NewL( CFastFindFileServerPlugin& aPlugin );
        virtual ~CFastFindFileServerPluginConn();

        virtual TInt DoControl( CFsPluginConnRequest& aRequest );
        virtual void DoRequest( CFsPluginConnRequest& aRequest );
        virtual void DoCancel( TInt aReqMask );

    private:
        CFastFindFileServerPluginConn( CFastFindFileServerPlugin& aPlugin );

        CFastFindFileServerPlugin& iPlugin;

        RMessage2* iMessage;
    };
    
/**
* Leaving New function for the plugin
* @internalComponent
*/
CFastFindFileServerPluginConn* CFastFindFileServerPluginConn::NewL( 
		CFastFindFileServerPlugin& aPlugin )
    {
    WRITELOG( "CFastFindFileServerPluginConn::NewL()" );
    return new (ELeave) CFastFindFileServerPluginConn( aPlugin );
    }


/**
* Constructor for the plugin
* @internalComponent
*/
CFastFindFileServerPluginConn::CFastFindFileServerPluginConn(
               CFastFindFileServerPlugin& aPlugin )
  : iPlugin( aPlugin )
    {
    WRITELOG( "CFastFindFileServerPluginConn::CFastFindFileServerPluginConn()" );

    iPlugin.AddConnection();
    }


/**
* The destructor for the test virus scanner hook.
* @internalComponent
*/
CFastFindFileServerPluginConn::~CFastFindFileServerPluginConn()
    {
    WRITELOG( "CFastFindFileServerPluginConn::~CFastFindFileServerPluginConn()" );

    iPlugin.RemoveConnection();
    }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPluginConn::DoControl( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPluginConn::DoControl()" );
    TInt err = KErrNotSupported;
    
    CFastFindFileServerPlugin& myPlugin = *(CFastFindFileServerPlugin*)Plugin();
    
    switch( aRequest.Function() )
        {
        case EFastFindFSPOpEnable:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpEnable" );
            TRAP( err, myPlugin.EnableL() );
            break;
            }
        
        case EFastFindFSPOpDisable:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpDisable" );
            TRAP( err, myPlugin.DisableL() );
            break;
            }
        
        default:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - Unknown Control" );
            break;            
            }
        }

    return err;
    }
    

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPluginConn::DoRequest( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPluginConn::DoRequest()" );
    CFastFindFileServerPlugin& myPlugin = *(CFastFindFileServerPlugin*)Plugin();
    
    switch( aRequest.Function() )
        {
        case EFastFindFSPOpRegisterNotification:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpRegisterNotification" );
            TInt err = myPlugin.RegisterNotification( aRequest );
            
            if ( err != KErrNone )
                {
                aRequest.Complete( err );
                }
            break;
            }
            
        case EFastFindFSPOpAddNotificationPath:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpAddNotificationPath" );
            TInt err = myPlugin.AddNotificationPath( aRequest );
            aRequest.Complete( err );
            break;
            }
            
        case EFastFindFSPOpRemoveNotificationPath:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpRemoveNotificationPath" );
            TInt err = myPlugin.RemoveNotificationPath( aRequest );
            aRequest.Complete( err );
            break;
            }
            
        case EFastFindFSPOpAddIgnorePath:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpAddIgnorePath" );
            TInt err = myPlugin.AddIgnorePath( aRequest );
            aRequest.Complete( err );
            break;
            }
            
        case EFastFindFSPOpRemoveIgnorePath:
            {
            WRITELOG( "CFastFindFileServerPluginConn::DoControl() - EFastFindFSPOpRemoveIgnorePath" );
            TInt err = myPlugin.RemoveIgnorePath( aRequest );
            aRequest.Complete( err );
            break;
            }
        }
    }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPluginConn::DoCancel( TInt /*aReqMask*/ )
    {
    WRITELOG( "CFastFindFileServerPluginConn::DoCancel()" );
    iRequestQue.DoCancelAll( KErrCancel );
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
CFsPluginConn* CFastFindFileServerPlugin::NewPluginConnL()
    {
    WRITELOG( "CFastFindFileServerPluginConn::NewPluginConnL()" );
    return CFastFindFileServerPluginConn::NewL( *this );
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPlugin::RegisterNotification( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPlugin::RegisterNotification()" );
    
    if ( iNotification )
        {
        return KErrInUse;
        }
    
    iNotification = &aRequest;
    
    if ( iQueue.Count() > 0 )
        {
        WRITELOG( "CFastFindFileServerPlugin::RegisterNotification() - item in queue" );
        
        TFastFindFSPStatus* queueStatus = iQueue[0];
        
        TFastFindFSPStatusPckg pckg;
        TFastFindFSPStatus& status = pckg();
        
        status.iDriveNumber = queueStatus->iDriveNumber;
        status.iFileEventType = queueStatus->iFileEventType;
        status.iFileName.Copy( queueStatus->iFileName );
        status.iNewFileName.Copy( queueStatus->iNewFileName );
        status.iProcessId = queueStatus->iProcessId;
        
        TRAPD( err, iNotification->WriteParam1L(pckg) );
        iNotification->Complete( err );
        iNotification = NULL;
        
        delete queueStatus;
        queueStatus = NULL;
        iQueue.Remove( 0 );
        }
    
    return KErrNone;
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPlugin::AddNotificationPath( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPlugin::AddNotificationPath()" );
    TInt err = KErrNone;
    
    TFastFindFSPStatusPckg pckg;
    TRAP( err, aRequest.ReadParam1L(pckg) );
    
    if ( err != KErrNone )
        {
        return err;
        }
    
    TFastFindFSPStatus& status = pckg();
    
    if ( status.iFileName.Length() > 0 )
        {
        // check if already exists
        for ( TInt i = 0; i < iPaths.Count(); i++ )
            {
            TFileName* tf = iPaths[i];
            if ( tf->Compare(status.iFileName) == 0 )
                {
                return KErrNone;
                }
            }

        WRITELOG1( "CFastFindFileServerPlugin::AddNotificationPath() - add path: %S", &status.iFileName );
        TFileName* fn = new  TFileName;
        
        if ( fn )
            {
            fn->Copy( status.iFileName );
            iPaths.Append( fn );
            }
        else
        	{
        	err = KErrNoMemory;
        	}
        }
    else
        {
        err = KErrNotFound;
        }
        
    return err;
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPlugin::RemoveNotificationPath( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPlugin::RemoveNotificationPath()" );
    TInt err = KErrNone;
    
    TFastFindFSPStatusPckg pckg;
    TRAP( err, aRequest.ReadParam1L(pckg) );
    
    if ( err != KErrNone )
        {
        return err;
        }
    
    TFastFindFSPStatus& status = pckg();
    
    if ( status.iFileName.Length() > 0 )
        {
        // check if already exist
        if ( iPaths.Count() > 0 )
            {
            for ( TInt i = 0; i < iPaths.Count(); i++ )
                {
                TFileName* tf = iPaths[i];
                if ( tf->Compare(status.iFileName) == 0 )
                    {
                    WRITELOG1( "CFastFindFileServerPlugin::RemoveNotificationPath() - remove path: %S", &status.iFileName );
                    delete tf;
                    tf = NULL;
                    iPaths.Remove( i );
                    }
                }
            }
        }
    else
        {
        err = KErrNotFound;
        }
        
    return err;
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPlugin::AddIgnorePath( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPlugin::AddIgnorePath()" );
    TInt err = KErrNone;
    
    TFastFindFSPStatusPckg pckg;
    TRAP( err, aRequest.ReadParam1L(pckg) );
    
    if ( err != KErrNone )
        {
        return err;
        }
    
    TFastFindFSPStatus& status = pckg();
    
    if ( status.iFileName.Length() > 0 )
        {
        // check if already exist
        if ( iIgnorePaths.Count() > 0 )
            {
            for ( TInt i = 0; i < iIgnorePaths.Count(); i++ )
                {
                TFileName* tf = iIgnorePaths[i];
                if( tf->Compare(status.iFileName) == 0 )
                    {
                    return KErrNone;
                    }
                }
            }
            
        WRITELOG1( "CFastFindFileServerPlugin::AddIgnorePath() - add path: %S", &status.iFileName );
        TFileName* fn = new TFileName;
        
        if ( fn )
            {
            fn->Copy( status.iFileName );
            iIgnorePaths.Append( fn ); // ownership is transferred
            }
        else
        	{
        	err = KErrNoMemory;
        	}
        }
    else
        {
        err = KErrNotFound;
        }
        
    return err;
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TInt CFastFindFileServerPlugin::RemoveIgnorePath( CFsPluginConnRequest& aRequest )
    {
    WRITELOG( "CFastFindFileServerPlugin::RemoveIgnorePath()" );
    TInt err = KErrNone;
    
    TFastFindFSPStatusPckg pckg;
    TRAP( err, aRequest.ReadParam1L(pckg) );
    
    if ( err != KErrNone )
        {
        return err;
        }
    
    TFastFindFSPStatus& status = pckg();
    
    if ( status.iFileName.Length() > 0 )
        {
        // check if already exist
        if ( iIgnorePaths.Count() > 0 )
            {
            for ( TInt i = 0; i < iIgnorePaths.Count(); i++ )
                {
                TFileName* tf = iIgnorePaths[i];
                if ( tf->Compare(status.iFileName) == 0 )
                    {
                    WRITELOG1( "CFastFindFileServerPlugin::RemoveIgnorePath() - remove path: %S", &status.iFileName );
                    delete tf;
                    tf = NULL;
                    iIgnorePaths.Remove( i );
                    }
                }
            }
        }
    else
        {
        err = KErrNotFound;
        }
        
    return err;
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
TBool CFastFindFileServerPlugin::CheckPath( TFileName& aFileName )
    {
    // check if ignored pathlist
    if ( iIgnorePaths.Count() > 0 )
        {
        for ( TInt i = 0; i < iIgnorePaths.Count(); i++ )
            {    
            TFileName* pathName = iIgnorePaths[i];
            pathName->LowerCase();
            TFileName tempFileName;
            tempFileName.Copy( aFileName );
            tempFileName.LowerCase();
            // WRITELOG1("CFastFindFileServerPlugin::CheckPath() - search ignore path: %S", pathName);
            if ( tempFileName.Find(*pathName) != KErrNotFound )
                {
                // WRITELOG( "CFastFindFileServerPlugin::CheckPath() - is ignore path" );
                return EFalse;
                }
            }
        }
    
    // check if notification path
    if ( iPaths.Count() > 0 )
        {
        for ( TInt i = 0; i < iPaths.Count(); i++ )
            {    
            TFileName* pathName = iPaths[i];
            pathName->LowerCase();
            TFileName tempFileName;
            tempFileName.Copy( aFileName );
            tempFileName.LowerCase();
            //WRITELOG1("CFastFindFileServerPlugin::CheckPath() - search path: %S", &pathName);
            if ( tempFileName.Find(*pathName) != KErrNotFound )
                {
                //WRITELOG( "CFastFindFileServerPlugin::CheckPath() - path found" );
                return ETrue;
                }
            }
        }
    else
        {
        // WRITELOG( "CFastFindFileServerPlugin::CheckPath() - no notification paths" );
        return ETrue;
        }
            
    return EFalse;
    }
    
//-----------------------------------------------------------------------------
// CFastFindFileServerPlugin::CheckAttribs()
//-----------------------------------------------------------------------------
//
TBool CFastFindFileServerPlugin::CheckAttribs( TFileName& aFileName )
    {
    TParse parse;
    parse.Set( aFileName, NULL, NULL );
    TPath path( parse.DriveAndPath() );
    
    TUint att = 0;
    
    // check if path is hidden or system path
    TInt err = iFsSession.Att( path, att );
    if ( err == KErrNone )
        {
        if ( att & KEntryAttHidden || att & KEntryAttSystem )
            {
            return EFalse;
            }
        }
        
    // or is the file hidden or system file
    att = 0;
    err = iFsSession.Att( aFileName, att );
    if ( err == KErrNone )
        {
        if ( att & KEntryAttHidden || att & KEntryAttSystem )
            {
            return EFalse;
            }
        }
        
    return ETrue;
    }
    
//-----------------------------------------------------------------------------
// CFastFindFileServerPluginFactory implementation
//-----------------------------------------------------------------------------
//
class CFastFindFileServerPluginFactory : public CFsPluginFactory
    {
    public:
        CFastFindFileServerPluginFactory();
        virtual TInt Install();            
        virtual CFsPlugin* NewPluginL();
        virtual TInt UniquePosition();
    };
    
// Constructor for the plugin factory
// @internalComponent
CFastFindFileServerPluginFactory::CFastFindFileServerPluginFactory()
    {
    WRITELOG( "CFastFindFileServerPluginFactory::CFastFindFileServerPluginFactory()" );
    }

// Install function for the plugin factory
// @internalComponent
TInt CFastFindFileServerPluginFactory::Install()
    {
    WRITELOG( "CFastFindFileServerPluginFactory::Install()" );
    iSupportedDrives = KPluginAutoAttach;
    
    return( SetName(&KFastFindFileServerPlugin) );
    }

// @internalComponent
TInt CFastFindFileServerPluginFactory::UniquePosition()
    {
    WRITELOG( "CFastFindFileServerPluginFactory::UniquePosition()" );
    return( KFastFindFSPluginPosition );
    }

// Plugin factory function
// @internalComponent
CFsPlugin* CFastFindFileServerPluginFactory::NewPluginL()
    {
    WRITELOG( "CFastFindFileServerPluginFactory::NewPluginL()" );
    return CFastFindFileServerPlugin::NewL();
    }

// Create a new Plugin
// @internalComponent
extern "C"
    {
    EXPORT_C CFsPluginFactory* CreateFileSystem()
        {
        WRITELOG( "CFastFindFileServerPluginFactory::CreateFileSystem" );
        return( new CFastFindFileServerPluginFactory() );
        }
    }

#ifdef _DEBUG_EVENTS

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::RegisterDebugEventsL()
    {
    User::LeaveIfError( RegisterIntercept( EFsFileDuplicate, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileCreate, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileWrite, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileFlush, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileSetAtt, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileChangeMode, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsDelete, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileAdopt, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileReplace, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileLock, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileSize, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileRename, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsRename, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileOpen, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileTemp,    EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileUnLock, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileSetSize, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileDrive, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsReplace, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileSubClose, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileRead, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileSeek, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileAtt, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileSet, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsFileName, EPreIntercept) );
    User::LeaveIfError( RegisterIntercept( EFsDirOpen, EPreIntercept) );
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::UnregisterDebugEventsL()
    {
    User::LeaveIfError( UnregisterIntercept( EFsFileDuplicate, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileCreate, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileWrite, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileFlush, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileSetAtt, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileChangeMode, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsDelete, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileAdopt, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileReplace, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileLock, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileSize, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileRename, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsRename, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileOpen, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileTemp, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileUnLock, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileSetSize, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileDrive, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsReplace, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileSubClose, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileRead, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileSeek, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileAtt, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileSet, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsFileName, EPrePostIntercept) );
    User::LeaveIfError( UnregisterIntercept( EFsDirOpen, EPrePostIntercept) );
    }
    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
//
void CFastFindFileServerPlugin::PrintDebugEvents( TInt aFunction )
    {
    switch ( aFunction )
        {
        case EFsFileDuplicate:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileDuplicate" );
            break;
            
        case EFsFileCreate:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileCreate" );
            break;
        case EFsFileWrite:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileWrite" );
            break;
        case EFsFileFlush:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileFlush" );
            break;
        case EFsFileSetAtt:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSetAtt" );
            break;
        case EFsFileChangeMode:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileChangeMode" );
            break;    
        case EFsDelete:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsDelete" );
            break;    
        case EFsFileAdopt:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileAdopt" );
            break;    
        case EFsFileReplace:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileReplace" );
            break;    
        case EFsFileLock:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileLock" );
            break;    
        case EFsFileSize:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSize" );
            break;    
        case EFsFileModified:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileModified" );
            break;    
        case EFsFileRename:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileRename" );
            break;    
        case EFsRename:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsRename" );
            break;    
        case EFsFileOpen:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileOpen" );
            break;    
        case EFsFileTemp:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileTemp" );
            break;    
        case EFsFileUnLock:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileUnLock" );
            break;    
        case EFsFileSetSize:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSetSize" );
            break;    
        case EFsFileSetModified:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSetModified" );
            break;
        case EFsFileDrive:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileDrive" );
            break;    
        case EFsReplace:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsReplace" );
            break;    
        case EFsFileSubClose:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSubClose" );
            break;    
        case EFsFileRead:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileRead" );
            break;
        case EFsFileSeek:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSeek" );
            break;    
        case EFsFileAtt:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileAtt" );
            break;    
        case EFsFileSet:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileSet" );
            break;    
        case EFsFileName:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsFileName" );
            break;    
        case EFsDirOpen:
            WRITELOG( "CFastFindFileServerPlugin::PrintDebugEvents() - EFsDirOpen" );
            break;    
        default:
        break;
        }
    }
#endif

//#ifdef _FORCEDEBUG
//#undef _DEBUG
//#undef _FORCEDEBUG
//#endif
