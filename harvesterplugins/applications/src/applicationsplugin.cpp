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

#include "applicationsplugin.h"
#include "harvesterserverlogger.h"
#include <common.h>

#include <ccpixindexer.h>
#include <csearchdocument.h>
#include <e32base.h>
//#include <menu2internalcrkeys.h> //for KCRUidMenu
#include <WidgetPropertyValue.h> // EBundleDisplayName 
#include <centralrepository.h>
#include <opensystemtrace.h> 
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "applicationspluginTraces.h"
#endif


//Hidden applications
//#define KHiddenAppRepositoryUid KCRUidMenu

_LIT( KMimeTypeField, CPIX_MIMETYPE_FIELD );
_LIT( KMimeTypeApplication, APPLICATION_MIMETYPE);

/** Field names */
_LIT(KApplicationFieldCaption, "Name");
_LIT(KApplicationFieldUid, "Uid");
_LIT(KApplicationFieldAbsolutePath, "Path");

// TAppInfo.Name() returns [121345678]. The below constants are used to extract '[' & ']'
const TInt KUidStartIndex = 1;
const TInt KUidEndIndex = 8;

/** The delay between harvesting chunks. */
const TInt KHarvestingDelay = 1000;

// -----------------------------------------------------------------------------
CApplicationsPlugin* CApplicationsPlugin::NewL()
	{
    OstTraceFunctionEntry0( CAPPLICATIONSPLUGIN_NEWL_ENTRY );
    CPIXLOGSTRING("CApplicationsPlugin::NewL()");
	CApplicationsPlugin* instance = CApplicationsPlugin::NewLC();
    CleanupStack::Pop(instance);
    OstTraceFunctionExit0( CAPPLICATIONSPLUGIN_NEWL_EXIT );
    return instance;
	}

// -----------------------------------------------------------------------------
CApplicationsPlugin* CApplicationsPlugin::NewLC()
	{
	CApplicationsPlugin* instance = new (ELeave) CApplicationsPlugin();
    CleanupStack::PushL(instance);
    instance->ConstructL();
    return instance;
	}

// -----------------------------------------------------------------------------
CApplicationsPlugin::CApplicationsPlugin()
	{
	}

// -----------------------------------------------------------------------------
CApplicationsPlugin::~CApplicationsPlugin()
	{
    if (iAsynchronizer)
        iAsynchronizer->CancelCallback();
    iApplicationServerSession.Close();
    iWidgetRegistry.Close();
    //delete iHiddenApplicationsRepository;
	delete iAsynchronizer;
	delete iNotifier;
	delete iIndexer;
	}

// -----------------------------------------------------------------------------
void CApplicationsPlugin::ConstructL()
	{
    iAsynchronizer = CDelayedCallback::NewL( CActive::EPriorityIdle );
    iNotifier = CApaAppListNotifier::NewL( this, CActive::EPriorityHigh );
    //iHiddenApplicationsRepository = CRepository::NewL( KHiddenAppRepositoryUid );
    User::LeaveIfError( iWidgetRegistry.Connect() );
    }

// -----------------------------------------------------------------------------
void CApplicationsPlugin::StartPluginL()
	{
    User::LeaveIfError( iApplicationServerSession.Connect() );
	User::LeaveIfError(iSearchSession.DefineVolume( _L(APPLICATIONS_QBASEAPPCLASS), KNullDesC ));
    
	// Open database
	iIndexer = CCPixIndexer::NewL(iSearchSession);
	iIndexer->OpenDatabaseL( _L(APPLICATIONS_QBASEAPPCLASS) );

	// Start harvester for this plugin
	iObserver->AddHarvestingQueue( this, iIndexer->GetBaseAppClass() );
	}

// -----------------------------------------------------------------------------
void CApplicationsPlugin::StartHarvestingL(const TDesC& /* aQualifiedBaseAppClass */)
    {
    // Harvest items on each call
    User::LeaveIfError( iApplicationServerSession.GetAllApps() );//if not KErrNone
    iIndexer->ResetL();
    //No need to check IsStatred() since this is the first start. 
#ifdef __PERFORMANCE_DATA
    iStartTime.UniversalTime();
#endif
   	iAsynchronizer->Start( 0, this, KHarvestingDelay );
    }

// -----------------------------------------------------------------------------
void CApplicationsPlugin::AddWidgetInfoL( CSearchDocument* aDocument, TUid aUid )
    {
    TBuf<KMaxFileName> temp;//we can reuse this.
    
    iWidgetRegistry.GetWidgetPath( aUid, temp );
    aDocument->AddFieldL(KApplicationFieldAbsolutePath, temp,  CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
    CPIXLOGSTRING2("AddApplicationInfo(): PATH = %S ", &temp);
    OstTraceExt1( TRACE_NORMAL, CAPPLICATIONSPLUGIN_ADDWIDGETINFOL, "CApplicationsPlugin::AddWidgetInfoL;PATH=%S", &temp );

    //GetWidgetPropertyValueL returns CWidgetPropertyValue* which in turn has an operator to convert to TDesC
    aDocument->AddFieldL(KApplicationFieldCaption, *(iWidgetRegistry.GetWidgetPropertyValueL( aUid, EBundleDisplayName )),  CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );

    iWidgetRegistry.GetWidgetBundleName( aUid, temp );
    aDocument->AddExcerptL( temp );
    OstTraceExt1( TRACE_NORMAL, DUP1_CAPPLICATIONSPLUGIN_ADDWIDGETINFOL, "CApplicationsPlugin::AddWidgetInfoL;DisplayName=%S", &temp );
    CPIXLOGSTRING2("AddApplicationInfo(): DisplayName = %S ", &temp );
    }

// -----------------------------------------------------------------------------
//This need not be a member function.
void AddApplicationInfoL( CSearchDocument* aDocument, TApaAppInfo& aAppInfo )
    {
    TBuf<KMaxFileName> docidString = aAppInfo.iUid.Name(); //This returns stuff in the form "[UID]". So remove the brackets.
    docidString = docidString.Mid( KUidStartIndex, KUidEndIndex );
    
    aDocument->AddFieldL(KApplicationFieldCaption, aAppInfo.iShortCaption, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
    aDocument->AddFieldL(KApplicationFieldAbsolutePath, aAppInfo.iFullName, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
    aDocument->AddExcerptL( aAppInfo.iCaption );
    OstTraceExt2( TRACE_NORMAL, _ADDAPPLICATIONINFOL, "::AddApplicationInfoL;UID=%S;PATH=%S", &docidString, &aAppInfo.iFullName );
    OstTraceExt2( TRACE_NORMAL, DUP1__ADDAPPLICATIONINFOL, "::AddApplicationInfoL;Excerpt=%S;Caption=%S", &aAppInfo.iCaption, &aAppInfo.iShortCaption );
    
    CPIXLOGSTRING3("AddApplicationInfo(): UID = %S, PATH = %S ", &docidString, &aAppInfo.iFullName );
    CPIXLOGSTRING3("AddApplicationInfo():  Excerpt = %S, Caption = %S ", &aAppInfo.iCaption, &aAppInfo.iShortCaption );
    }

// -----------------------------------------------------------------------------
TBool CApplicationsPlugin::IsAppHiddenL(TUid aUid)
    {
    //Application should not have 'hidden' capability.
    TBool ret( EFalse );
    TApaAppCapabilityBuf cap;
    OstTrace1( TRACE_NORMAL, CAPPLICATIONSPLUGIN_ISAPPHIDDENL, "CApplicationsPlugin::IsAppHiddenL;UID=%d", aUid );
    CPIXLOGSTRING2("CApplicationsPlugin::IsAppHidden(): UID = %d", aUid );
    if ( iApplicationServerSession.GetAppCapability(cap, aUid) == KErrNone )
        {
        OstTraceState0( STATE_DUP1_CAPPLICATIONSPLUGIN_ISAPPHIDDENL, "GetCapability returned KErrNone", "" );
    
        CPIXLOGSTRING("CApplicationsPlugin::IsAppHidden(): GetCapability returned KErrNone");
        ret = cap().iAppIsHidden;
        }

    //Application should not be listed hidden in application shell.
//    TBuf<NCentralRepositoryConstants::KMaxUnicodeStringLength> uidResult;
//    if( iHiddenApplicationsRepository->Get( KMenuHideApplication, uidResult ) == KErrNone )
//        {
//        CPIXLOGSTRING2("CApplicationsPlugin::CreateApplicationsIndexItemL(): Hidden UIDs = %S", &uidResult );
//        TBufC16<NCentralRepositoryConstants::KMaxUnicodeStringLength> buf(uidResult);
//        HBufC* uidString = buf.AllocLC();
//        //If not in the list, it means it is hidden; so dont harvest
//        if( uidString->FindF( aUid.Name().Mid( KUidStartIndex, KUidEndIndex ) ) != KErrNotFound ) 
//            {
//            CleanupStack::PopAndDestroy( uidString );
//            CPIXLOGSTRING("CApplicationsPlugin::IsAppHidden(): UID in hidden app repository");
//            return EFalse;
//            }
//        CleanupStack::PopAndDestroy( uidString );
//        }
    OstTrace1( TRACE_NORMAL, DUP1_CAPPLICATIONSPLUGIN_ISAPPHIDDENL, "CApplicationsPlugin::IsAppHiddenL;Return Value=%d", &ret );

    CPIXLOGSTRING2("CApplicationsPlugin::IsAppHidden(): %d", &ret);
    return ret;
    }

// -----------------------------------------------------------------------------
void CApplicationsPlugin::CreateApplicationsIndexItemL( TApaAppInfo& aAppInfo, TCPixActionType /*aActionType*/ )
    {
    //If application has 'hidden' capability, don't index.
    if( IsAppHiddenL( aAppInfo.iUid ) ) return;
    
    TBuf<KMaxFileName> docidString;
    docidString.Append( aAppInfo.iUid.Name() ); //This returns descriptor in the form "[UID]". So remove the brackets.
    docidString = docidString.Mid( KUidStartIndex, KUidEndIndex  );
    
    CSearchDocument* document = CSearchDocument::NewLC( docidString, _L(APPLICATIONS_APPCLASS) );
    //The UID field should not be aggregated for now as we dont want it to be searchable by default.
    //By default, all tokenized fields are aggregated and therefore searchable.
    //If we dont tokenize, then the field will not be searchable at all.
    //As a middle path, we tokenize this field, but explicitly chose NOT to aggregate it.
    //That way, if a client is interested in the UID field, he can choose to query it explicitly.
    document->AddFieldL(KMimeTypeField, KMimeTypeApplication, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
    document->AddFieldL(KApplicationFieldUid, docidString, CDocumentField::EStoreYes | CDocumentField::EIndexTokenized | CDocumentField::EAggregateNo );

    if( iWidgetRegistry.IsWidget( aAppInfo.iUid  ) )
        AddWidgetInfoL( document, aAppInfo.iUid );
    else
        AddApplicationInfoL( document, aAppInfo );

    TRAPD( error, iIndexer->AddL( *document ) );
    if( KErrNone == error )
        {
        OstTrace0( TRACE_NORMAL, CAPPLICATIONSPLUGIN_CREATEAPPLICATIONSINDEXITEML, "CApplicationsPlugin::CreateApplicationsIndexItemL : No Error" );
        CPIXLOGSTRING("CApplicationsPlugin::CreateApplicationsIndexItemL(): No Error" );
        }
    else 
        {
        OstTrace1( TRACE_NORMAL, DUP1_CAPPLICATIONSPLUGIN_CREATEAPPLICATIONSINDEXITEML, "CApplicationsPlugin::CreateApplicationsIndexItemL;Error=%d", error );
        CPIXLOGSTRING2("CApplicationsPlugin::CreateApplicationsIndexItemL(): Error = %d", error );
        }
    CleanupStack::PopAndDestroy( document );
    }

// -----------------------------------------------------------------------------
void CApplicationsPlugin::DelayedCallbackL( TInt /*aCode*/ )
    {
    TApaAppInfo appInfo;
    const TInt error = iApplicationServerSession.GetNextApp( appInfo );
    if(  error == KErrNone )
        {
        CreateApplicationsIndexItemL( appInfo, ECPixAddAction );
        }

    if ( error != RApaLsSession::ENoMoreAppsInList )
        {
        //No need to check IsStatred() since control reaches 
        //here only on asynchornize complete.
        iAsynchronizer->Start( 0, this, KHarvestingDelay );
        }
    else
        {
        Flush( *iIndexer );
#ifdef __PERFORMANCE_DATA
    UpdatePerformaceDataL();
#endif
        iObserver->HarvestingCompleted( this, iIndexer->GetBaseAppClass(), KErrNone );
        }
	}

// -----------------------------------------------------------------------------
void CApplicationsPlugin::DelayedError( TInt aCode )
    {
    Flush(*iIndexer);
    iObserver->HarvestingCompleted(this, iIndexer->GetBaseAppClass(), aCode);
    }
   
// -----------------------------------------------------------------------------
void CApplicationsPlugin::HandleAppListEvent( TInt aEvent )
    {
    OstTraceFunctionEntry0( CAPPLICATIONSPLUGIN_HANDLEAPPLISTEVENT_ENTRY );
    OstTrace1( TRACE_NORMAL, CAPPLICATIONSPLUGIN_HANDLEAPPLISTEVENT, "CApplicationsPlugin::HandleAppListEvent;Event=%d", aEvent );
    CPIXLOGSTRING2("CApplicationsPlugin::HandleAppListEvent: Start with Event = %d", aEvent );
    if( aEvent == EAppListChanged )
        {
        if( iAsynchronizer->CallbackPending() )
            {
            iAsynchronizer->CancelCallback(); //first cancel any ongoing harvesting.
            }
        TRAP_IGNORE( StartHarvestingL( KNullDesC ) ); //simply reharvest
        }
    CPIXLOGSTRING("CApplicationsPlugin::HandleAppListEvent: Exit" );
    OstTraceFunctionExit0( CAPPLICATIONSPLUGIN_HANDLEAPPLISTEVENT_EXIT );
    }

#ifdef __PERFORMANCE_DATA
void CApplicationsPlugin::UpdatePerformaceDataL()
    {
    TTime now;
   
    
    iCompleteTime.UniversalTime();
    TTimeIntervalMicroSeconds timeDiff = iCompleteTime.MicroSecondsFrom(iStartTime);
    
    RFs fileSession;
    RFile perfFile;
    User::LeaveIfError( fileSession.Connect () );
    
    
    /* Open file if it exists, otherwise create it and write content in it */
    
        if(perfFile.Open(fileSession, _L("c:\\data\\ApplicationsPerf.txt"), EFileWrite))
                   User::LeaveIfError(perfFile.Create (fileSession, _L("c:\\data\\ApplicationsPerf.txt"), EFileWrite));
    
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
    ptr.Append( _L(": Ani: Time took for Harvesting Applications is : "));
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
