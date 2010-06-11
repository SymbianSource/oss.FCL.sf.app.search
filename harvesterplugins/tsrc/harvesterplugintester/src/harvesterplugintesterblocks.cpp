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
* Description: This file contains testclass implementation.
*
*/

// change the level of RVCT compiler warning 'non-POD class passed thru ellipses' to a remark.
#ifdef __MARM__
#pragma diag_remark 1446
#endif

// [INCLUDE FILES] - do not remove
#include "harvesterplugintester.h"
#include <e32svr.h>
#include <f32file.h> 
#include <stifparser.h>
#include <stiftestinterface.h>
#include <favouritesdb.h>
#include <msvapi.h>
#include <bautils.h>
#include "harvesterobserver.h"
#include "cmessageplugin.h"
#include "cmessagesessionobserver.h" //For CMsvSession
#include "ccpixsearcher.h"
#include "bookmarksplugin.h"
#include "applicationsplugin.h"
#include "ccontactsplugin.h"
#include "ccalendarplugin.h"
#include "ccalendarobserver.h"
#include "mediaplugin.h"
#include <harvesterclient.h>
#include "mdsitementity.h"
#include "cpixmdedbmanager.h"
#include "cfolderrenamedharvester.h"
//#include "CBlacklistMgr.h"
#include "videoplugin.h"
#include "imageplugin.h"

_LIT(KAppBasePath,"@c:root file content");
_LIT(KAppBaseFolderFilePath,"@c:root file folder");
const TInt KMsgPluginBaseAppClassMaxLen = 64;

// For Notes --Start
#include <d32dbms.h>
#include <utf.h>
#include "notesplugin.h"

//Test Uid for testing Blacklist manager

//const TUid KTestUid = { 0x101D6348 };

#define MEDIA_QBASEAPPCLASS   "@c:root media audio"
#define LMEDIA_QBASEAPPCLASS  L"@c:root media audio"
#define VIDEO_QBASEAPPCLASS   "@c:root media video"
#define LVIDEO_QBASEAPPCLASS  L"@c:root media video"
#define IMAGE_QBASEAPPCLASS   "@c:root media image"
#define LIMAGE_QBASEAPPCLASS  L"@c:root media image"

// For Notes --End

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// Delete here all resources allocated and opened from test methods. 
// Called from destructor. 
void CHarvesterPluginTester::Delete() 
    {
    }

// -----------------------------------------------------------------------------
// Run specified method. Contains also table of test mothods and their names.
TInt CHarvesterPluginTester::RunMethodL( 
    CStifItemParser& aItem ) 
    {

    static TStifFunctionInfo const KFunctions[] =
        {  
        // Copy this line for every implemented function.
        // First string is the function name used in TestScripter script file.
        // Second is the actual implementation member function. 
        ENTRY( "TestStartHarvester", CHarvesterPluginTester::TestStartHarvesterL ),        
        ENTRY( "TestFormBaseAppClass", CHarvesterPluginTester::TestFormBaseAppClass ),
        ENTRY( "TestDatabasePath", CHarvesterPluginTester::TestDatabasePathL ),
        ENTRY( "TestCreateIndexItemL_Add", CHarvesterPluginTester::TestCreateIndexItemL ),
        ENTRY( "TestCreateIndexItemL_Update", CHarvesterPluginTester::TestCreateIndexItemL ),
        ENTRY( "TestCreateIndexItemL_Delete", CHarvesterPluginTester::TestCreateIndexItemL ),
        ENTRY( "TestFolderCreate", CHarvesterPluginTester::TestFolderCreateL ),
        ENTRY( "TestFolderRename", CHarvesterPluginTester::TestFolderRenameL ),
        ENTRY( "TestMessaging", CHarvesterPluginTester::TestMessageHarvesterL ),
        ENTRY( "TestMessageHarvesting", CHarvesterPluginTester::TestMessageHarvesterWithMessageL ),
        ENTRY( "TestMessageDriveChange", CHarvesterPluginTester::TestMessageHarvesterChangeDriveL ),
        ENTRY( "TestStartBookmarksHarvesterL", CHarvesterPluginTester::TestStartBookmarksHarvesterL ),
        ENTRY( "TestAddBookmarkL", CHarvesterPluginTester::TestAddBookmarkL ),
        ENTRY( "TestAddGetDomainL", CHarvesterPluginTester::TestAddGetDomainL ),
        ENTRY( "TestStartApplicationsHarvesterL", CHarvesterPluginTester::TestStartApplicationsHarvesterL ),        
        ENTRY( "TestCreateApplicationsIndexItemL", CHarvesterPluginTester::TestCreateApplicationsIndexItemL ),
		ENTRY( "TestStartNotesHarvesterL", CHarvesterPluginTester::TestStartNotesHarvesterL ),
        ENTRY( "TestAddNoteL", CHarvesterPluginTester::TestAddNoteL ),
        ENTRY( "TestAddLongNoteL", CHarvesterPluginTester::TestAddLongNoteL ),
        ENTRY( "TestAddAlphaNumericNoteL", CHarvesterPluginTester::TestAddAlphaNumericNoteL ),
        ENTRY( "TestDeleteNoteL", CHarvesterPluginTester::TestDeleteNoteL ),
        ENTRY( "TestUpdateNoteL", CHarvesterPluginTester::TestUpdateNoteL ),                
        ENTRY( "TestContactsHarvesting", CHarvesterPluginTester::TestStartContactsHarvesterL ),
        ENTRY( "TestCreateContactIndexItemL_Add", CHarvesterPluginTester::TestCreateContactIndexItemL ),
        ENTRY( "TestCreateContactIndexItemL_Edit", CHarvesterPluginTester::TestCreateContactIndexItemL ),
        ENTRY( "TestCreateContactIndexItemL_Delete", CHarvesterPluginTester::TestCreateContactIndexItemL ),
        ENTRY( "TestCreateContactGroup", CHarvesterPluginTester::TestCreateContactGroupL ),
        ENTRY( "TestCalenderHarvesting", CHarvesterPluginTester::TestStartCalenderHarvesterL ),
        ENTRY( "TestCalenderEntry",CHarvesterPluginTester::TestCalenderEntryL ),
        ENTRY( "TestCreateMMS",CHarvesterPluginTester::TestCreateMmsL ),
        ENTRY( "TestCreateEmail",CHarvesterPluginTester::TestCreateEmailL ),
		ENTRY( "TestAudioHarvesting",CHarvesterPluginTester::TestAudioHarvestingL ),
        ENTRY( "TestAudioHarvestingUpdateIndex",CHarvesterPluginTester::TestAudioHarvestingUpdateIndexL ),
        ENTRY( "TestAudioHarvestingDeleteIndex",CHarvesterPluginTester::TestAudioHarvestingDeleteIndexL ),
		ENTRY( "TestMdsSyncController",CHarvesterPluginTester::TestMdsSyncControllerL ),
		//ENTRY( "TestBlacklistPlugin",CHarvesterPluginTester::TestBlacklistPluginL ),
		//ENTRY( "TestBlacklistPluginVersion",CHarvesterPluginTester::TestBlacklistPluginVersionL ),
		ENTRY( "TestVideoHarvestingIndex",CHarvesterPluginTester::TestVideoHarvestingIndexL ),		
		ENTRY( "TestVideoHarvestingUpdateIndex",CHarvesterPluginTester::TestVideoHarvestingUpdateIndexL ),
		ENTRY( "TestVideoHarvestingDeleteIndex",CHarvesterPluginTester::TestVideoHarvestingDeleteIndexL ),
		ENTRY( "TestImageHarvestingAddIndex",CHarvesterPluginTester::TestImageHarvestingAddIndexL ),
		ENTRY( "TestImageHarvestingUpdateIndex",CHarvesterPluginTester::TestImageHarvestingUpdateIndexL ),
		ENTRY( "TestImageHarvestingDeleteIndex",CHarvesterPluginTester::TestImageHarvestingDeleteIndexL ),
		ENTRY( "TestAudioMMCEventL",CHarvesterPluginTester::TestAudioMMCEventL ),
		ENTRY( "TestVideoMMCEventL",CHarvesterPluginTester::TestVideoMMCEventL ),
		ENTRY( "TestImageMMCEventL",CHarvesterPluginTester::TestImageMMCEventL ),
        //ADD NEW ENTRY HERE
        // [test cases entries] - Do not remove
        };

    const TInt count = sizeof( KFunctions ) / 
                        sizeof( TStifFunctionInfo );

    return RunInternalL( KFunctions, count, aItem );

    }

_LIT( KNoErrorString, "No Error" );
//_LIT( KErrorString, " *** Error ***" );

void doLog( CStifLogger* logger, TInt error, const TDesC& errorString )
    {
    if( KErrNone == error ) logger->Log( KNoErrorString );
    else logger->Log( errorString );
    }

// Example test method function.
// -----------------------------------------------------------------------------
TInt CHarvesterPluginTester::TestStartHarvesterL( CStifItemParser& /*aItem*/ )
    {
    // Print to UI
    _LIT( KHarvesterPluginTester, "HarvesterPluginTester: %S" );
    _LIT( KExample, "In TestStartHarvesterL" );
    TestModuleIf().Printf( 0, KHarvesterPluginTester, KExample );

    // Print to log file
    iLog->Log( KExample );
    CFilePlugin* filePlugin = CFilePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( filePlugin );
    filePlugin->StartPluginL(); //Calls Add
    filePlugin->StartHarvestingL( KAppBasePath );
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    delete filePlugin;
    delete iPluginTester;
    doLog( iLog, KErrNone, KNoErrorString );
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestFormBaseAppClass( CStifItemParser& aItem )
    {
    // Print to UI
    _LIT( KFilePluginBaseAppClass, "FormBaseAppClass" );
    _LIT( KExample, "In TestFormBaseAppClass" );
    
    TestModuleIf().Printf( 0, KFilePluginBaseAppClass, KExample );
    
    TInt driveNumber;
    TBuf<100> formedBaseAppClass;
    TBuf<100> expectedBaseAppClass;
    TChar driveChar;
    CFilePlugin* filePlugin = CFilePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( filePlugin );
    filePlugin->StartPluginL();

    TInt error = KErrNone;
    TInt errorNumber = 0;
    _LIT( KFileBaseAppClassFolder, "root file folder");
    _LIT( KBaseAppClassFormatString, "@%c:root file folder");
    _LIT( KTestFormBaseAppClassNoError, "TestFormBaseAppClass: No Error" );
    
    if( aItem.GetNextInt ( driveNumber ) == KErrNone  && error == KErrNone )
        {
        filePlugin->FormBaseAppClass(  static_cast<TDriveNumber>( driveNumber ),KFileBaseAppClassFolder, formedBaseAppClass );
        if( aItem.GetNextChar ( driveChar ) == KErrNone )
            {
            expectedBaseAppClass.Format( KBaseAppClassFormatString, driveChar );
            error = expectedBaseAppClass.Compare( formedBaseAppClass );
            if( aItem.GetNextInt ( errorNumber ) == KErrNone )
                error = ( KErrNone == error || -1 == errorNumber ) ? KErrNone : KErrGeneral ;
            }
        }
    delete filePlugin;
    delete iPluginTester;
    
    doLog( iLog, error, KTestFormBaseAppClassNoError );
    return error;
    }

TInt CHarvesterPluginTester::TestDatabasePathL( CStifItemParser& aItem )
    {
    TInt driveNumber;
    CFilePlugin* filePlugin = CFilePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( filePlugin );
    filePlugin->StartPluginL();
    
    HBufC* buffer;
    TBuf<KMaxFileName> returnedDbPath;
    TBuf<KMaxFileName> expectedDbPath;
    TInt error = KErrNone;
    TInt errorNumber;
    TChar driveChar;
    _LIT( KTestDatabasePathNoError, "TestDatabasePathL: No Error" );
    _LIT(KPathFileContent, "\\root\\file\\content");
    _LIT( KDbFormatString, "%c:\\Private\\2001f6f7\\indexing\\indexdb\\root\\file\\content" );
    if( aItem.GetNextInt ( driveNumber ) == KErrNone  && error == KErrNone )
        {
        buffer = filePlugin->DatabasePathLC( static_cast<TDriveNumber>( driveNumber ), KPathFileContent );
        returnedDbPath.Copy( *buffer );
        if( aItem.GetNextChar ( driveChar ) == KErrNone )
            {
            expectedDbPath.Format( KDbFormatString, driveChar );
            error = expectedDbPath.Compare( returnedDbPath );
            if( aItem.GetNextInt ( errorNumber ) == KErrNone )
                error = ( KErrNone == error || -1 == errorNumber ) ? KErrNone : KErrGeneral ;
            }
        }
    CleanupStack::PopAndDestroy( buffer );
    delete filePlugin;
    delete iPluginTester;
    doLog( iLog, error, KTestDatabasePathNoError );
    return error;
    }

//Helper functions
enum TSearchType
    {
    ESearchTypeResultsExpected=0,
    ESearchTypeNoResultsExpected
    };

TInt doSearch( const TDesC& aSearchString, const TDesC& aBaseAppClass, TSearchType aSearchType )
    {
    RSearchServerSession session;
    User::LeaveIfError( session.Connect() );
    CCPixSearcher* searcher = CCPixSearcher::NewLC( session );
    searcher->OpenDatabaseL( aBaseAppClass );
    
    TInt docCount = searcher->SearchL( aSearchString, KNullDesC);
    
    CleanupStack::PopAndDestroy( searcher );
    session.Close();
    
    if( aSearchType == ESearchTypeResultsExpected )
        {
        return docCount > 0 ? KErrNone : KErrNotFound;
        }
    else
        {
        return 0 == docCount ? KErrNone : KErrNotFound;
        }
    }

TInt CHarvesterPluginTester::TestCreateIndexItemL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC fileName;
    TPtrC createIndexAction;
    CFilePlugin* filePlugin = CFilePlugin::NewL();
    CHarvesterObserver* observer = CHarvesterObserver::NewL( filePlugin );
    filePlugin->StartPluginL();
    _LIT( KFileNameFormat, "C:\\Data\\" );
    _LIT( KActionAdd, "add" );
    _LIT( KActionDelete, "delete" );
    _LIT( KActionUpdate, "update" );
    _LIT( KCreateIndexItemNoError, "CreateIndexItemL: Error" );

    TBuf<KMaxFileName> filePathName( KFileNameFormat );
    
    if( aItem.GetNextString ( createIndexAction ) == KErrNone )
        {
        if( createIndexAction.Compare( KActionAdd ) == 0 )
            {
            if( aItem.GetNextString ( fileName ) == KErrNone  && error == KErrNone )
                {
                filePathName.Append( fileName );
                filePlugin->CreateContentIndexItemL( filePathName, ECPixAddAction );
                User::After( (TTimeIntervalMicroSeconds32)35000000 );
                TPtrC searchString;
                while( aItem.GetNextString ( searchString ) == KErrNone  && error == KErrNone )
                    {
                    error = doSearch( searchString, KAppBasePath, ESearchTypeResultsExpected );
                    }
                filePlugin->CreateContentIndexItemL( filePathName, ECPixRemoveAction );
                }
            }
        if( createIndexAction.Compare( KActionDelete ) == 0 )
            {
            if( aItem.GetNextString ( fileName ) == KErrNone  && error == KErrNone )
                {
                filePathName.Append( fileName );
                filePlugin->CreateContentIndexItemL( filePathName, ECPixRemoveAction );
                TPtrC searchString;
                while( aItem.GetNextString ( searchString ) == KErrNone  && error == KErrNone )
                    {
                    error = doSearch( searchString, KAppBasePath, ESearchTypeNoResultsExpected );
                    }
                }
            }
        if( createIndexAction.Compare( KActionUpdate ) == 0 )
            {
            if( aItem.GetNextString ( fileName ) == KErrNone  && error == KErrNone )
                {
                filePathName.Append( fileName );
                filePlugin->CreateContentIndexItemL( filePathName, ECPixUpdateAction );
                User::After( (TTimeIntervalMicroSeconds32)35000000 );
                TPtrC searchString;
                while( aItem.GetNextString ( searchString ) == KErrNone  && error == KErrNone )
                    {
                    error = doSearch( searchString, KAppBasePath, ESearchTypeResultsExpected );
                    }
                }
            }
        }
    delete filePlugin;
    delete observer;
    doLog( iLog, error, KCreateIndexItemNoError );
    return error;
    }

TInt CHarvesterPluginTester::TestFolderCreateL( CStifItemParser& /*aItem*/ )
    {
    TInt error = KErrNone;
    CFilePlugin* filePlugin = CFilePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( filePlugin );
    filePlugin->StartPluginL();
    filePlugin->StartHarvestingL( KAppBasePath );
    
    _LIT( KDirectoryToCreate, "C:\\data\\TestFolder\\" );    
    RFs fs;
    fs.Connect();
    TBool folderExists = BaflUtils::FolderExists(fs, KDirectoryToCreate);
    if(!folderExists)
        {
        error = fs.MkDir(KDirectoryToCreate);
        }    
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    
    if(error == KErrNone)
        {
        error = doSearch( _L("TestFolder"), KAppBaseFolderFilePath, ESearchTypeResultsExpected );
        fs.RmDir(KDirectoryToCreate);
        }
    fs.Close();
    delete filePlugin;
    delete iPluginTester;
    doLog( iLog, error, _L("Error: TestFolderCreateL") );
    
    return error;
    }

TInt CHarvesterPluginTester::TestFolderRenameL( CStifItemParser& /*aItem*/ )
    {
    TInt error = KErrNone;
    RFs fs;
    fs.Connect();
    
    CFilePlugin* filePlugin = CFilePlugin::NewL();
    CFolderRenamedHarvester* iFolderRenameHarvester = CFolderRenamedHarvester::NewL( *filePlugin, fs);
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( filePlugin );
    filePlugin->StartPluginL();
    filePlugin->StartHarvestingL( KAppBasePath );    
    
    _LIT( KDirectoryToCreate, "C:\\data\\TestRenameFolder\\" );
    _LIT( KDirectoryRenamed, "C:\\data\\TestFolderRenamed\\" );
    
    TFileName oldFolderName(KDirectoryToCreate);
    TFileName newFolderName(KDirectoryRenamed);    
    
    if(!BaflUtils::FolderExists(fs, KDirectoryToCreate))
        {
        User::LeaveIfError(fs.MkDir(KDirectoryToCreate));        
        }
    if(BaflUtils::FolderExists(fs, KDirectoryRenamed))
        {
        User::LeaveIfError(fs.RmDir(KDirectoryRenamed));
        }
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    
    error = doSearch( _L("TestRenameFolder"), KAppBaseFolderFilePath, ESearchTypeResultsExpected );        
  
    if(error == KErrNone)
        {
           fs.Rename(KDirectoryToCreate, KDirectoryRenamed);
           iFolderRenameHarvester->StartL( oldFolderName, newFolderName );           
        }    
    
    User::After( (TTimeIntervalMicroSeconds32)35000000 );
    
    //Search for the renamed directory
    error = doSearch( _L("TestFolderRenamed"), KAppBaseFolderFilePath, ESearchTypeNoResultsExpected );    
    
    fs.RmDir(KDirectoryRenamed);    
    
    delete filePlugin;
    delete iFolderRenameHarvester;
    delete iPluginTester;
    fs.Close();
    doLog( iLog, error, _L("Error: TestFolderRenameL") );
    
    return error;
    }
/**
* Message harvester test method.
* @since ?Series60_version
* @param aItem Script line containing parameters.
* @return Symbian OS error code.
*/
TInt CHarvesterPluginTester::TestMessageHarvesterL( CStifItemParser& /*aItem*/ )
    {
    // Print to UI
    _LIT( KHarvesterPluginTester, "HarvesterPluginTester" );
    _LIT( KExample, "In TestMessageHarvesterL" );
    TestModuleIf().Printf( 0, KHarvesterPluginTester, KExample );
    // Print to log file
    iLog->Log( KExample );
    
    CMessagePlugin* plugin = CMessagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    CMessageSessionObserver* sessionobserver = CMessageSessionObserver::NewL();
    CMsvSession* msgSession = CMsvSession::OpenSyncL(*sessionobserver);
    // Form the baseappclass for this media
    TBuf<KMsgPluginBaseAppClassMaxLen> baseAppClass;
    TInt drive = msgSession->CurrentDriveL();
    CMessagePlugin::FormBaseAppClass(TDriveNumber(drive),baseAppClass);
    //Get current base app class for drive
    //CMessagePlugin::FormBaseAppClass
    plugin->StartHarvestingL( baseAppClass );
    iPluginTester->iWaitForHarvester->Start();
    
    delete plugin; 
    delete iPluginTester;
    delete sessionobserver;
    delete msgSession;
    doLog(iLog,KErrNone,KNoErrorString);
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestMessageHarvesterWithMessageL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KHarvesterPluginTester, "HarvesterPluginTester" );
    _LIT( KExample, "In TestMessageHarvesterWithMessageL" );
    TInt error(KErrNone);
    
    TestModuleIf().Printf( 0, KHarvesterPluginTester, KExample );
    // Print to log file
    iLog->Log( KExample );
    CMessagePlugin* plugin = CMessagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    CMessageSessionObserver* sessionobserver = CMessageSessionObserver::NewL();
    CMsvSession* msgSession = CMsvSession::OpenSyncL(*sessionobserver);
    // Form the baseappclass for this media
    TBuf<KMsgPluginBaseAppClassMaxLen> baseAppClass;
    TInt drive = msgSession->CurrentDriveL();
    //Get current base app class for drive
    CMessagePlugin::FormBaseAppClass(TDriveNumber(drive),baseAppClass);
    plugin->StartHarvestingL( baseAppClass );
    //Create a new SMS message
    TMsvId msgid = MessagingUtils::CreateMessageL(msgSession,
                                                 _L("+3584400220055"),
                                                 _L("+919845062437"),
                                                 _L("This is test from bhuvnesh"));
    iPluginTester->iWaitForHarvester->Start();
    TInt count = SearchForTextL(_L("+919845062437"),baseAppClass,KNullDesC);
    MessagingUtils::RemoveEntryL(msgSession,msgid);
    if(count <= 0)
        {
        error = KErrNotFound;
        }
    
    delete plugin; 
    delete iPluginTester;
    delete sessionobserver;
    delete msgSession;
    doLog(iLog,error,_L("Error in TestMessageHarvesterWithMessageL"));
    return error;
    }

TInt CHarvesterPluginTester::TestMessageHarvesterChangeDriveL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KHarvesterPluginTester, "HarvesterPluginTester" );
    _LIT( KExample, "In TestMessageHarvesterChangeDriveL" );
    TInt error(KErrNone);
    
    TestModuleIf().Printf( 0, KHarvesterPluginTester, KExample );
    // Print to log file
    iLog->Log( KExample );
    CMessagePlugin* plugin = CMessagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    CMessageSessionObserver* sessionobserver = CMessageSessionObserver::NewL();
    CMsvSession* msgSession = CMsvSession::OpenSyncL(*sessionobserver);
    // Form the baseappclass for this media
    TBuf<KMsgPluginBaseAppClassMaxLen> baseAppClass;
    TInt drive = msgSession->CurrentDriveL();
    //Get current base app class for drive
    CMessagePlugin::FormBaseAppClass(TDriveNumber(drive),baseAppClass);
    plugin->StartHarvestingL( baseAppClass );
    iPluginTester->iWaitForHarvester->Start();
    //Harvesting completed change the drive and make sure new message harvested
    drive == EDriveC ? drive = EDriveE : drive = EDriveC;
    TRequestStatus status = KRequestPending;
    msgSession->ChangeDriveL(drive,status);
    User::WaitForAnyRequest();
    //User::WaitForRequest(status);//Wait to complete this request
    plugin->StartHarvestingL( baseAppClass );
    TMsvId msgid = MessagingUtils::CreateMessageL(msgSession,
                                                 _L("+3584400220055"),
                                                 _L("+919845062437"),
                                                 _L("This is test from bhuvnesh"));
    iPluginTester->iWaitForHarvester->Start();//Start again
    drive = msgSession->CurrentDriveL();
    CMessagePlugin::FormBaseAppClass(TDriveNumber(drive),baseAppClass);
    TInt count = SearchForTextL(_L("+919845062437"),baseAppClass,KNullDesC);
    if(count <= 0)
        {
        error = KErrNotFound;
        }
    MessagingUtils::RemoveEntryL(msgSession,msgid);
    
    delete plugin;
    delete iPluginTester;
    delete sessionobserver;
    delete msgSession;
    doLog(iLog,error,_L("Error in TestMessageHarvesterChangeDriveL"));
    return error;
    }

TInt CHarvesterPluginTester::SearchForTextL(const TDesC& aQueryString, const TDesC& aBaseAppclass,const TDesC& aDefaultField)
    {
    TInt DocumentCount(KErrNotFound);
    RSearchServerSession session;
    User::LeaveIfError(session.Connect());
    CCPixSearcher* searcher = CCPixSearcher::NewL(session);
    searcher->OpenDatabaseL(aBaseAppclass);
    // Send for indexing
    if ( searcher )
        {
        DocumentCount = searcher->SearchL(aQueryString, aDefaultField);
        }
    return DocumentCount;
    }

TInt CHarvesterPluginTester::TestStartBookmarksHarvesterL( CStifItemParser& /*aItem */)
    {
    CBookmarksPlugin* plugin = CBookmarksPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    plugin->StartHarvestingL( _L(BOOKMARK_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    delete plugin;
    delete iPluginTester;
    doLog( iLog, KErrNone, KNoErrorString );
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestAddBookmarkL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    CBookmarksPlugin* plugin = CBookmarksPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //starts the bookmarks db monitor.

    //now add a bookmark.
    CFavouritesItem* item = CFavouritesItem::NewL();
    RFavouritesDb favoritesDb;
    RFavouritesSession favSession;
    error = favSession.Connect();
    if( KErrNone == error ) error = favoritesDb.Open( favSession, KBrowserBookmarks ) ;
    if( KErrNone != error ) return error;
    
    item->SetType( CFavouritesItem::EItem );
    item->SetParentFolder( KFavouritesRootUid );
    
    TPtrC string;
    error = aItem.GetNextString( string );
    if( KErrNone == error ) 
        item->SetNameL( string );
    
    error = aItem.GetNextString( string );
    if( KErrNone == error ) 
        item->SetUrlL( string );
    
    if( KErrNone == error ) 
        {
        error = favoritesDb.Begin( ETrue );
        if( KErrNone == error ) error = favoritesDb.Add( *item, ETrue ); //ETrue for Autorename. We are searching only by URL so this is fine.
        if( KErrNone == error ) error = favoritesDb.Commit();
        plugin->StartHarvestingL( _L(BOOKMARK_QBASEAPPCLASS) );
        if( KErrNone == error ) iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
        }
    
    //now, search for URL.
    if( KErrNone == error )
        {
        error = aItem.GetNextString( string );
        if( KErrNone == error ) 
            error = doSearch( string , _L( BOOKMARK_QBASEAPPCLASS ), ESearchTypeResultsExpected );
        }

    delete plugin;
    delete iPluginTester;
    
    favoritesDb.Close();
    favSession.Close();
    
    doLog( iLog, error, _L("Error in AddBookmarkL") );
    return error;
    }

//Forward declare this function.
//It is a local function in the .cpp file and hence not listed in .h file.
//Forward declare it so that the compilation can go thru - the implementation
//will be picked up by the linker.
void GetDomainNameL( const TDesC& aUrl, TPtrC& aDomain );

TInt CHarvesterPluginTester::TestAddGetDomainL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC inputString;
    TPtrC domain;
    while( aItem.GetNextString( inputString ) == KErrNone )
        {
        GetDomainNameL( inputString, domain );
        if( aItem.GetNextString( inputString ) == KErrNone )
            error = domain.Compare( inputString );
            if( error != KErrNone ) return error;
        }
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestStartApplicationsHarvesterL( CStifItemParser& /*aItem*/ )
    {
    CApplicationsPlugin* appsPlugin = CApplicationsPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( appsPlugin );
    appsPlugin->StartPluginL(); //Calls Add
    appsPlugin->StartHarvestingL( KNullDesC );
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    delete appsPlugin;
    delete iPluginTester;
    doLog( iLog, KErrNone, KNoErrorString );
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestCreateApplicationsIndexItemL( CStifItemParser& /*aItem*/ )
    {
    CApplicationsPlugin* appsPlugin = CApplicationsPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( appsPlugin );
    appsPlugin->StartPluginL();
    
    //Force reharvesting:
    appsPlugin->HandleAppListEvent( 1 ); //EListChanged = 1
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.

    delete appsPlugin;
    delete iPluginTester;
    doLog( iLog, KErrNone, KNoErrorString );
    return KErrNone;
    }
	
	TInt CHarvesterPluginTester::TestStartNotesHarvesterL( CStifItemParser& /*aItem*/ )
    {	
    CNotesPlugin* plugin = CNotesPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    plugin->StartHarvestingL( _L(NOTES_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    delete plugin;
    delete iPluginTester;    
    doLog( iLog, KErrNone, KNoErrorString );	
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestAddNoteL( CStifItemParser& aItem )
    {    
    TPtrC string;
    TPtrC searchstring;
    TInt error = aItem.GetNextString( string );
    error = aItem.GetNextString( searchstring );
    error = PerformNotesTestL(string , searchstring);
    return error;
    }

TInt CHarvesterPluginTester::TestAddLongNoteL( CStifItemParser& aItem )
    {
    TPtrC string;
    TPtrC searchstring;
    TInt error = aItem.GetNextString( string );
    error = aItem.GetNextString( searchstring );
    error = PerformNotesTestL(string , searchstring);
    return error;
    }
TInt CHarvesterPluginTester::TestAddAlphaNumericNoteL( CStifItemParser& aItem )
    {
    TPtrC string;
    TPtrC searchstring;
    TInt error = aItem.GetNextString( string );
    error = aItem.GetNextString( searchstring );
    error = PerformNotesTestL(string , searchstring);
    return error;
    }
TInt CHarvesterPluginTester::PerformNotesTestL( TPtrC aString1 , TPtrC aString2)
    {	
    TInt error = KErrNone;	
    HBufC8* memo = NULL;    
    CNotesPlugin* plugin = CNotesPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //starts the Notes database notifier    
    if(aString1.Length() > 0)
        {
        RFs fssession;
        User::LeaveIfError( fssession.Connect() );
        RFile testfile;        
        error = testfile.Open(fssession,aString1,EFileRead);
        if(error == KErrNone)
            {
            TInt size(0);
            testfile.Size(size);
            memo = HBufC8::NewL(size);
            TPtr8 memoptr = memo->Des();
            User::LeaveIfError( testfile.Read(memoptr, size ) );
            //testfile.Read(memoptr,size);
            testfile.Close();                
            fssession.Close();
            }
        } 
    if(error == KErrNone)
        {
        CCalendarObserver* session = CCalendarObserver::NewL();
        session->AddNoteL(memo->Des());
        plugin->StartHarvestingL( _L(NOTES_QBASEAPPCLASS) );
        iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
        error = doSearch( aString2, _L(NOTES_QBASEAPPCLASS), ESearchTypeResultsExpected );
        
        delete session;
        doLog( iLog, error, KNoErrorString );
        }
    delete memo;
    delete plugin;
    delete iPluginTester;
    return error;
    }

TInt CHarvesterPluginTester::TestDeleteNoteL( CStifItemParser& aItem )
    {	
    TInt error = KErrNone;
    _LIT( KSearchError, "Search Failed" );   
    CNotesPlugin* plugin = CNotesPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();   
    
    CCalendarObserver* session = CCalendarObserver::NewL();
    plugin->StartHarvestingL( _L(NOTES_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();
    // Add a Note entry
    TPtrC searchstring;
    error = aItem.GetNextString( searchstring );
    HBufC8* buf8 = HBufC8::NewL(2*searchstring.Length());
    buf8->Des().Copy(searchstring);
    session->AddNoteL( buf8->Des());
    User::After( (TTimeIntervalMicroSeconds32)35000000 );
    delete buf8;    
    error = doSearch( _L("TestNote"), _L( NOTES_QBASEAPPCLASS ), ESearchTypeResultsExpected );
    doLog( iLog, error, KSearchError );    
    if( error == KErrNone)
        {
        session->DeleteNoteEntryL();
        User::After( (TTimeIntervalMicroSeconds32)35000000 );
        //Delete the Note entry added previously and search for result    
        error = doSearch( _L("TestNote"), _L( NOTES_QBASEAPPCLASS ), ESearchTypeResultsExpected );             
        //If the entery is succesfully deleted, make error to KErrNone.To show testcase success
        if(error == KErrNotFound)
            error = KErrNone;    
        }
    delete session;
    delete plugin;
    delete iPluginTester;
    doLog( iLog, error, KSearchError );
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestUpdateNoteL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    _LIT( KSearchError, "Search Failed" );
    TPtrC oldString;
    error = aItem.GetNextString( oldString );
    TPtrC newString;
    error = aItem.GetNextString( newString );
    CNotesPlugin* plugin = CNotesPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();     
    CCalendarObserver* session = CCalendarObserver::NewL();
    plugin->StartHarvestingL( _L(NOTES_QBASEAPPCLASS) );
    // Add a Note entry
    HBufC8* oldbuf8 = HBufC8::NewL(2*oldString.Length());
    oldbuf8->Des().Copy(oldString);
    session->AddNoteL( oldbuf8->Des());
    TBuf<20> oldsearchstring;
    oldsearchstring.Copy( oldString );
    iPluginTester->iWaitForHarvester->Start();
    error = doSearch( oldsearchstring, _L( NOTES_QBASEAPPCLASS ), ESearchTypeResultsExpected );
    doLog( iLog, error, KSearchError );
    
    TBuf<20> newsearchstring;
    newsearchstring.Copy( newString );
    HBufC8* newbuf8 = HBufC8::NewL(2*newString.Length());
    newbuf8->Des().Copy(newString);
    //update the Note entry and search for result 
    session->UpdateNoteEntryL( oldbuf8->Des(), newbuf8->Des() );
    plugin->StartHarvestingL( _L(NOTES_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();
    error = doSearch( newsearchstring, _L( NOTES_QBASEAPPCLASS ), ESearchTypeResultsExpected );
    doLog( iLog, error, KSearchError );
    
    delete newbuf8;
    delete oldbuf8;
    delete session;
    delete plugin;
    delete iPluginTester;    
    return error;    
    }

TInt CHarvesterPluginTester::TestStartContactsHarvesterL( CStifItemParser& /*aItem*/ )
    {
    CContactsPlugin* plugin = CContactsPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //start to moniter contacts db
    plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    delete plugin;
    delete iPluginTester;
    doLog( iLog, KErrNone, KNoErrorString );
    return KErrNone;
    }

TContactItemId CHarvesterPluginTester::CreateNewContactL( CContactDatabase& database,const TDesC& aName,const TDesC& aPhoneNumber )
    {
    // Create a  contact card to contain the data
    CContactCard* newCard = CContactCard::NewLC();
    
    // Create the firstName field and add the data to it
    CContactItemField* firstName = CContactItemField::NewLC( KStorageTypeText, KUidContactFieldGivenName );
    firstName->TextStorage()->SetTextL(aName);    
    newCard->AddFieldL(*firstName);
    CleanupStack::Pop(firstName);
      
    // Create the phoneNo field and add the data to it
    CContactItemField* phoneNumber = CContactItemField::NewLC( KStorageTypeText, KUidContactFieldPhoneNumber );
    phoneNumber->SetMapping(KUidContactFieldVCardMapTEL);
    phoneNumber ->TextStorage()->SetTextL(aPhoneNumber);
    newCard->AddFieldL(*phoneNumber);
    CleanupStack::Pop(phoneNumber);
    
    // Add newCard to the database     
    const TContactItemId contactId = database.AddNewContactL(*newCard);    
    CleanupStack::PopAndDestroy(newCard);    
    return contactId;
    }

TInt CHarvesterPluginTester::TestCreateContactIndexItemL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC IndexAction;
    TPtrC ContactName;
    TPtrC NewContactName;
    _LIT( KActionAdd, "add" );
    _LIT( KSearchError, "Search Failed" );
    _LIT( KActionDelete, "delete" );
    _LIT( KActionUpdate, "update" );
    
    CContactsPlugin* plugin = CContactsPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //start to moniter contacts db
    CContactDatabase* db = CContactDatabase::OpenL();
    
    if( aItem.GetNextString ( IndexAction ) == KErrNone )
        {
         if( IndexAction.Compare( KActionAdd ) == 0 )
             {
             if( aItem.GetNextString ( ContactName ) == KErrNone )
                 {
                 TInt aContactId;
                 plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
                 aContactId = CreateNewContactL( *db, ContactName, _L("1234") );                 
                 iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.                 
                                                       
                 error = doSearch( ContactName, _L( CONTACT_QBASEAPPCLASS ), ESearchTypeResultsExpected ); 
                 db->DeleteContactL(aContactId);
                 }
             }
         if( IndexAction.Compare( KActionUpdate ) == 0 )
             {
             if( aItem.GetNextString ( ContactName ) == KErrNone )
                 {
                  TInt aContactId;
                  //Add a contact first and later edit the contact
                  plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );                                    
                  aContactId = CreateNewContactL( *db, ContactName, _L("123") );
                  iPluginTester->iWaitForHarvester->Start();
                  if( aItem.GetNextString ( NewContactName ) == KErrNone )
                      {
                      CContactCard* newCard = CContactCard::NewLC();                      
                      // Create the firstName field and add the data to it
                      CContactItemField* firstName = CContactItemField::NewLC( KStorageTypeText, KUidContactFieldGivenName );
                      firstName->TextStorage()->SetTextL(NewContactName);    
                      newCard->AddFieldL(*firstName);
                      CleanupStack::Pop(firstName);
                        
                      // Create the phoneNo field and add the data to it
                      CContactItemField* phoneNumber = CContactItemField::NewLC( KStorageTypeText, KUidContactFieldPhoneNumber );
                      phoneNumber->SetMapping(KUidContactFieldVCardMapTEL);
                      phoneNumber ->TextStorage()->SetTextL(_L("567"));
                      newCard->AddFieldL(*phoneNumber);
                      CleanupStack::Pop(phoneNumber);
                      
                      plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
                      CContactItem* contactItem = db->UpdateContactLC(aContactId, newCard);
                      iPluginTester->iWaitForHarvester->Start();
                      
                      CleanupStack::PopAndDestroy(contactItem);
                      CleanupStack::PopAndDestroy(newCard);
                      
                      error = doSearch( NewContactName, _L( CONTACT_QBASEAPPCLASS ), ESearchTypeResultsExpected );
                      }
                  /*CContactItem* contactItem = db->OpenContactL( aContactId );
                  CleanupStack::PushL( contactItem );                           
                  // First get the item's field set
                  CContactItemFieldSet& fieldSet=contactItem->CardFields();                              
                  // Search the field set for the given name field
                  TInt index = fieldSet.Find( KUidContactFieldGivenName );
                  CContactItemField &field= fieldSet[index];                          
                  if( aItem.GetNextString ( NewContactName ) == KErrNone )
                      {                      
                      field.TextStorage()->SetTextL( NewContactName );                          
                      // Commit the change back to database and clean up
                      db->CommitContactL( *contactItem );
                      }
                  CleanupStack::PopAndDestroy(contactItem)*/;                
                  
                  /*plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
                  iPluginTester->iWaitForHarvester->Start();
                  error = doSearch( NewContactName, _L( CONTACT_QBASEAPPCLASS ), ESearchTypeResultsExpected );
                  db->DeleteContactL( aContactId );*/
                  }                 
             }
         if( IndexAction.Compare( KActionDelete ) == 0 )
             {
             if( aItem.GetNextString ( ContactName ) == KErrNone )
                 {
                 TInt aContactId;
                 plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS));
                 //Add a contact to contactdatabase and later delete it
                 aContactId = CreateNewContactL( *db, ContactName, _L("123455"));                 
                 iPluginTester->iWaitForHarvester->Start();
                 error = doSearch( ContactName, _L( CONTACT_QBASEAPPCLASS ), ESearchTypeResultsExpected );
                 if(error == KErrNone)
                     {
                     plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS));
                     db->DeleteContactL(aContactId);
                     iPluginTester->iWaitForHarvester->Start();
                     error = doSearch( ContactName, _L( CONTACT_QBASEAPPCLASS ), ESearchTypeNoResultsExpected );
                     }                                 
                 }             
             }
        }
        
        delete db;
        delete plugin;
        delete iPluginTester;
        
        doLog( iLog, error, KSearchError );
        return KErrNone;    
    }
TInt CHarvesterPluginTester::TestCreateContactGroupL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    _LIT( KSearchError, "Search Failed" );
    TPtrC GroupName;
    CContactsPlugin* plugin = CContactsPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //start to moniter contacts db
    CContactDatabase* db = CContactDatabase::OpenL();    
    
    if( aItem.GetNextString ( GroupName ) == KErrNone )
        {
        plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
        TInt aContactId = CreateNewContactL( *db, _L("Group Contact"), _L("123455") );
        CContactItem* newitem = db->CreateContactGroupL( _L("TestGroup") );
        db->AddContactToGroupL( aContactId, newitem->Id() );       
        iPluginTester->iWaitForHarvester->Start();    
        error = doSearch( GroupName, _L( CONTACT_QBASEAPPCLASS ), ESearchTypeResultsExpected );
        
        //update the group by adding a new contact, to get update event
        CContactGroup* group = static_cast<CContactGroup*>(db->OpenContactLX(newitem->Id()));        
        CleanupStack::PushL(group);
        plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
        TInt aId = CreateNewContactL( *db, _L("Contact1"), _L("455") );        
        db->AddContactToGroupL( aId, group->Id());
        db->CommitContactL(*group);
        iPluginTester->iWaitForHarvester->Start();    
        CleanupStack::PopAndDestroy(2);
        
        //Delete the group and its contacts
        plugin->StartHarvestingL( _L(CONTACT_QBASEAPPCLASS) );
        db->DeleteContactL(aContactId);
        db->RemoveContactFromGroupL(aId, newitem->Id());
        db->DeleteContactL( newitem->Id() );
        iPluginTester->iWaitForHarvester->Start();
        }
    
    delete plugin;
    delete iPluginTester;
    delete db;
    doLog( iLog, error, KSearchError );
    return KErrNone;    
    }

TInt CHarvesterPluginTester::TestStartCalenderHarvesterL( CStifItemParser& /*aItem*/ )
    {
    CCalendarPlugin* plugin = CCalendarPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //start to moniter contacts db
    plugin->StartHarvestingL( _L(CALENDAR_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();//Wait here till Harvesting is complete.
    delete plugin;
    delete iPluginTester;
    doLog( iLog, KErrNone, KNoErrorString );
    return KErrNone;    
    }

TInt CHarvesterPluginTester::TestCalenderEntryL( CStifItemParser& /*aItem*/ )
    {
    TInt error = KErrNone;
    _LIT( KSearchError, "Search Failed" );
    CCalendarPlugin* plugin = CCalendarPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();   
    
    CCalendarObserver* session = CCalendarObserver::NewL();
    plugin->StartHarvestingL( _L(CALENDAR_QBASEAPPCLASS) );
    // Add a calender entry
    session->AddEntryL();
    iPluginTester->iWaitForHarvester->Start();
    User::After((TTimeIntervalMicroSeconds32)30000000);
    error = doSearch( _L("Meeting"), _L( CALENDAR_QBASEAPPCLASS ), ESearchTypeResultsExpected );
    doLog( iLog, error, KSearchError );
    
    // For update event
    session->UpdateCalenderEntryL();
    plugin->StartHarvestingL( _L(CALENDAR_QBASEAPPCLASS) );
    iPluginTester->iWaitForHarvester->Start();
    
    plugin->StartHarvestingL( _L(CALENDAR_QBASEAPPCLASS) );
    session->DeleteEntryL();
    iPluginTester->iWaitForHarvester->Start();
    //Delete the calender entry added previously and search for result    
    error = doSearch( _L("scheduled"), _L( CALENDAR_QBASEAPPCLASS ), ESearchTypeResultsExpected );
        
    //If the entery is succesfully deleted, make error to KErrNone.To show testcase success
    if(error == KErrNotFound)
       error = KErrNone;    
    
    delete session;
    delete plugin;
    delete iPluginTester;
    doLog( iLog, error, KSearchError );     
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestCreateMmsL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    _LIT( KSearchError, "Search Failed" );
    TPtrC filepath;
    CMessagePlugin* plugin = CMessagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    CMessageSessionObserver* sessionobserver = CMessageSessionObserver::NewL();
    CMsvSession* msgSession = CMsvSession::OpenSyncL(*sessionobserver);    
    TInt found = aItem.GetNextString( filepath );
    TBuf<KMsgPluginBaseAppClassMaxLen> baseAppClass;
    TInt drive = msgSession->CurrentDriveL();
    CMessagePlugin::FormBaseAppClass(TDriveNumber(drive),baseAppClass);
    
    plugin->StartHarvestingL( baseAppClass );
    
    //Create MMS with text attachment for UTF encoded data
    TMsvId msgid = MessagingUtils::CreateMmsMessageL(msgSession,
                                                 _L("+358440067886"),
                                                 _L("+919845062437"),
                                                 filepath);
    
    iPluginTester->iWaitForHarvester->Start();
    
    TInt count = SearchForTextL(_L("UTF"),baseAppClass,KNullDesC);
    if(count <= 0)
       {
       error = KErrNotFound;
       }
    MessagingUtils::RemoveMmsEntryL( msgSession, msgid );
    delete plugin;
    delete iPluginTester;
    delete sessionobserver;
    delete msgSession;    
    doLog(iLog,error,KSearchError);
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestCreateEmailL( CStifItemParser& /*aItem */)
    {
    TInt error = KErrNone;
    _LIT( KSearchError, "Search Failed" );
    CMessagePlugin* plugin = CMessagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    CMessageSessionObserver* sessionobserver = CMessageSessionObserver::NewL();
    CMsvSession* msgSession = CMsvSession::OpenSyncL( *sessionobserver );
    
    TBuf<KMsgPluginBaseAppClassMaxLen> baseAppClass;
    TInt drive = msgSession->CurrentDriveL();
    CMessagePlugin::FormBaseAppClass( TDriveNumber(drive), baseAppClass );        
    plugin->StartHarvestingL( baseAppClass );
    //Create Email entry using SMTP client 
    TMsvId msgid = MessagingUtils::CreateEmailEntryL( msgSession );
    iPluginTester->iWaitForHarvester->Start();
    TInt count = SearchForTextL( _L("SampleEmail"), baseAppClass, KNullDesC );
    if(count <= 0)
       {
       error = KErrNotFound;
       }
    MessagingUtils::RemoveMmsEntryL( msgSession, msgid );
    delete plugin;
    delete iPluginTester;
    delete sessionobserver;
    delete msgSession;    
    doLog(iLog,error,KSearchError);
    return KErrNone;
    }

TInt CHarvesterPluginTester::TestAudioHarvestingL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KHarvesterPluginTester, "HarvesterPluginTester" );
    _LIT( KExample, "In TestAudioHarvestingL" );
    TInt error(KErrNone);
    TestModuleIf().Printf( 0, KHarvesterPluginTester, KExample );
    // Print to log file
    iLog->Log( KExample );
    CAudioPlugin* plugin = CAudioPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //Initialize the Plugin
    //Copy file path
    RFs fileSession;
    fileSession.Connect();
    _LIT(KPathToMusic,"c:\\data\\music\\");
    fileSession.Delete(_L("c:\\data\\music\\Eagle_Landed.mp3")); //Delete if already exist
    //pause harvester while copy
    RHarvesterClient   harvester;
    User::LeaveIfError(harvester.Connect() );
    harvester.Pause();
    TRAP_IGNORE( BaflUtils::EnsurePathExistsL(fileSession,KPathToMusic) );//Create folder
    BaflUtils::CopyFile(fileSession,_L("c:\\data\\Eagle_Landed.mp3"),KPathToMusic);
    //Resume harvester for this harvesting
    harvester.Resume();
    harvester.Close();
    //Do harvesting and search for Eagle it should be available
    plugin->StartHarvestingL(_L(MEDIA_QBASEAPPCLASS));
    //Wait for one minutes after doc processing to Index and Flush to happen
    iPluginTester->SetWaitTime((TTimeIntervalMicroSeconds32)60000000);
    iPluginTester->iWaitForHarvester->Start(); //Start Wait AO and let it complete
    TInt count = SearchForTextL(_L("Eagle"),_L(MEDIA_QBASEAPPCLASS),KNullDesC);
    if(count <= 0)
        {
        error = KErrNotFound;
        }
    doLog(iLog,error,_L("Error in TestAudioHarvestingL"));
    delete plugin;
    delete iPluginTester;
    fileSession.Close();    
    //End search
    return error;
    }
TInt CHarvesterPluginTester::TestAudioHarvestingUpdateIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;
    TPtrC newFile;
    TBuf<KMaxFileName> srcPath(_L("c:\\data\\Sounds\\"));
    TBuf<KMaxFileName> desPath;
    desPath.Copy( srcPath );
    CAudioPlugin* plugin = CAudioPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {
        srcPath.Append( filename );
        if( aItem.GetNextString(newFile) == KErrNone )
            {
            desPath.Append( newFile );
            RHarvesterClient harvester;
            User::LeaveIfError(harvester.Connect());
            harvester.Pause();
            TBool fileExist = BaflUtils::FileExists( fSession, srcPath );        
            if(!fileExist)
            {
            BaflUtils::EnsurePathExistsL( fSession, srcPath );//Create folder
            BaflUtils::CopyFile( fSession, filepath, srcPath );                    
            }            
            BaflUtils::RenameFile( fSession, srcPath, desPath );
            harvester.Resume();
            harvester.Close();
            plugin->StartHarvestingL( _L(MEDIA_QBASEAPPCLASS) );
            //wait for index to flush
            iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
            //wait till video harvesting completes
            iPluginTester->iWaitForHarvester->Start();
            TInt count = SearchForTextL(_L("testaudio"), _L(MEDIA_QBASEAPPCLASS), KNullDesC );
            if(count <= 0)
               {
               error = KErrNotFound;
               }
            doLog( iLog, error, _L("Error in TestAudioHarvestingUpdateIndexL") );
            }        
        }
        else
            doLog( iLog, KErrNotFound, _L("Error in TestAudioHarvestingUpdateIndexL") );           
        CleanupStack::PopAndDestroy();
        delete plugin;
        delete iPluginTester;
        return error;
        }

TInt CHarvesterPluginTester::TestAudioHarvestingDeleteIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;    
    TBuf<KMaxFileName> srcPath(_L("c:\\data\\Sounds\\"));
    CAudioPlugin* plugin = CAudioPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {
        srcPath.Append( filename );        
        RHarvesterClient harvester;
        User::LeaveIfError(harvester.Connect());
        harvester.Pause();
        TBool fileExist = BaflUtils::FileExists( fSession, srcPath );        
        if(!fileExist)
        {
        BaflUtils::EnsurePathExistsL( fSession, srcPath );//Create folder
        BaflUtils::CopyFile( fSession, filepath, srcPath );                    
        }            
        BaflUtils::DeleteFile( fSession, srcPath );        
        harvester.Resume();
        harvester.Close();
        plugin->StartHarvestingL( _L(MEDIA_QBASEAPPCLASS) );
        //wait for index to flush
        iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
        //wait till video harvesting completes
        iPluginTester->iWaitForHarvester->Start();
        TInt count = SearchForTextL(_L("eagle"), _L(MEDIA_QBASEAPPCLASS), KNullDesC );
        if(count <= 0)
           {
           // If the search is not found,then testcase is success
           doLog( iLog, error, _L("Error in TestAudioHarvestingDeleteIndexL") );
           }
        }
    else
        doLog( iLog, KErrNotFound, _L("Error in TestAudioHarvestingDeleteIndexL") );           
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    return error;
    }
        
TInt CHarvesterPluginTester::TestMdsSyncControllerL( CStifItemParser& /*aItem*/ )
    {
    _LIT( KHarvesterPluginTester, "HarvesterPluginTester" );
    _LIT( KExample, "In TestMdsSyncControllerL" );
    TInt error(KErrNone);
    TestModuleIf().Printf( 0, KHarvesterPluginTester, KExample );
    // Print to log file
    iLog->Log( KExample );
    CMDSEntity* entity;
    CCPIXMDEDbManager* dbcontroller;
    entity = CMDSEntity::NewL();
    TUid uid = {0x101FB3E3};
    dbcontroller = CCPIXMDEDbManager::NewL(uid);
    entity->Setkey(1);
    entity->SetUri(_L("\\music\\bhuvnesh.mp3"));
    TDriveNumber drive = TDriveNumber(EDriveC);
    entity->SetDrive(drive);
    dbcontroller->AddL(entity->Key(),*entity);
    error = dbcontroller->IsAlreadyExistL(entity->Key());
    if( error )
        {
        iLog->Log( _L("Already exist") );
        }
    entity->Reset();
    dbcontroller->GetItemL(1,*entity);
    dbcontroller->RemoveL(entity->Key());
    error = dbcontroller->IsAlreadyExistL( entity->Key() );
    if( error )
        {
        iLog->Log( _L("Already exist") );
        }
    dbcontroller->ResetL();
    delete entity;
    delete dbcontroller;
    return error;
    }
	
TInt CHarvesterPluginTester::TestBlacklistPluginL( CStifItemParser& /*aItem*/ )
    {
    //@todo: This test case shoud be in IDS middleware harvester STIF cases
    TInt err = KErrNone;
    /*
    CBlacklistMgr* blacklistmanager = CBlacklistMgr::NewL();
    CleanupStack::PushL( blacklistmanager );
    TInt version = 0;
    aItem.GetNextInt(version);
    //Add an Uid to Blacklist DB
    blacklistmanager->AddL( KTestUid , version );
    //Check if the Uid is added to database or not
    TBool found = blacklistmanager->FindL(KTestUid , version );
    
    if(!found) err = KErrNotFound;
    //clear the UID from the database
    blacklistmanager->Remove(KTestUid);
    CleanupStack::PopAndDestroy( blacklistmanager ); 
    doLog( iLog, err, KNoErrorString );
    */
    return err;
    }

TInt CHarvesterPluginTester::TestBlacklistPluginVersionL( CStifItemParser& /*aItem*/ )
    {
    //@todo: This test case shoud be in IDS middleware harvester STIF cases
    TInt err = KErrNone;
    /*	
    CBlacklistMgr* blacklistmanager = CBlacklistMgr::NewL();
    CleanupStack::PushL( blacklistmanager );
    TInt oldversion = 0;
    TInt newversion = 0;
    aItem.GetNextInt(oldversion);
    aItem.GetNextInt(newversion);
    //Add an Uid to Blacklist DB with old version
    blacklistmanager->AddL( KTestUid , oldversion );
    //Add an Uid to Blacklist DB with new version
    blacklistmanager->AddL( KTestUid , newversion );
    //Check if the Uid with old version exists
    TBool found = blacklistmanager->FindL(KTestUid , oldversion );
    if( found )
        {
        err = KErrNotFound;
        }
    else
        {
        //check with new version
        found = blacklistmanager->FindL(KTestUid , newversion );
        if(!found) err = KErrNotFound;
        }    
    //clear the UID from the database
    blacklistmanager->Remove(KTestUid);
    CleanupStack::PopAndDestroy( blacklistmanager );    
    doLog( iLog, err, KNoErrorString );*/
    return err;
    }
TInt CHarvesterPluginTester::TestVideoHarvestingIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;
    TPtrC newFile;
    CVideoPlugin* plugin = CVideoPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {        
        TBuf<KMaxFileName> desPath(_L("c:\\data\\Videos\\"));
        desPath.Append( filename );
        //Delete the file if it already exists
        fSession.Delete( desPath ); 
        //To ensure video plugin is loaded, pause the harvester
        RHarvesterClient harvester;
        User::LeaveIfError(harvester.Connect());
        harvester.Pause();
        BaflUtils::EnsurePathExistsL( fSession, desPath );//Create folder
        BaflUtils::CopyFile( fSession, filepath, desPath );
        //Resume harvester for this harvesting
        harvester.Resume();
        harvester.Close();
        //Now start video plugin harvesting
        plugin->StartHarvestingL( _L(VIDEO_QBASEAPPCLASS) );
        //wait for index to flush
        iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
        //wait till video harvesting completes
        iPluginTester->iWaitForHarvester->Start();
        TInt count = SearchForTextL( filename, _L(VIDEO_QBASEAPPCLASS), KNullDesC );
        if(count <= 0)
           {
           error = KErrNotFound;
           }
        }
        else
           error = KErrNotFound;
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    doLog( iLog, error, _L("Error in TestVideoHarvestingIndexL") );
    return error;
    }

TInt CHarvesterPluginTester::TestVideoHarvestingUpdateIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;
    TPtrC newFile;
    TBuf<KMaxFileName> srcPath(_L("c:\\data\\Videos\\"));
    TBuf<KMaxFileName> desPath;
    desPath.Copy( srcPath );
    CVideoPlugin* plugin = CVideoPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {
        srcPath.Append( filename );
        if( aItem.GetNextString(newFile) == KErrNone )
            {
            desPath.Append( newFile );
            RHarvesterClient harvester;
            User::LeaveIfError(harvester.Connect());
            harvester.Pause();
            TBool fileExist = BaflUtils::FileExists( fSession, srcPath );        
            if(!fileExist)
            {
            BaflUtils::EnsurePathExistsL( fSession, srcPath );//Create folder
            BaflUtils::CopyFile( fSession, filepath, srcPath );                    
            }            
            BaflUtils::RenameFile( fSession, srcPath, desPath );
            harvester.Resume();
            harvester.Close();
            plugin->StartHarvestingL( _L(VIDEO_QBASEAPPCLASS) );
            //wait for index to flush
            iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
            //wait till video harvesting completes
            iPluginTester->iWaitForHarvester->Start();
            TInt count = SearchForTextL(_L("Falls"), _L(VIDEO_QBASEAPPCLASS), KNullDesC );
            if(count <= 0)
               {
               error = KErrNotFound;
               }
            doLog( iLog, error, _L("Error in TestVideoHarvestingUpdateIndexL") );
            }        
        }
    else
        doLog( iLog, KErrNotFound, _L("Error in TestVideoHarvestingUpdateIndexL") );           
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    return error;
    }

TInt CHarvesterPluginTester::TestVideoHarvestingDeleteIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;    
    TBuf<KMaxFileName> srcPath(_L("c:\\data\\Videos\\"));
    CVideoPlugin* plugin = CVideoPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {
        srcPath.Append( filename );        
        RHarvesterClient harvester;
        User::LeaveIfError(harvester.Connect());
        harvester.Pause();
        TBool fileExist = BaflUtils::FileExists( fSession, srcPath );        
        if(!fileExist)
        {
        BaflUtils::EnsurePathExistsL( fSession, srcPath );//Create folder
        BaflUtils::CopyFile( fSession, filepath, srcPath );                    
        }            
        BaflUtils::DeleteFile( fSession, srcPath );        
        harvester.Resume();
        harvester.Close();
        plugin->StartHarvestingL( _L(VIDEO_QBASEAPPCLASS) );
        //wait for index to flush
        iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
        //wait till video harvesting completes
        iPluginTester->iWaitForHarvester->Start();
        TInt count = SearchForTextL(_L("Niagara"), _L(VIDEO_QBASEAPPCLASS), KNullDesC );
        if(count <= 0)
           {
           // If the search is not found,then testcase is success
           doLog( iLog, error, _L("Error in TestVideoHarvestingDeleteIndexL") );
           }
        }
    else
        doLog( iLog, KErrNotFound, _L("Error in TestVideoHarvestingDeleteIndexL") );           
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    return error;
    }

TInt CHarvesterPluginTester::TestImageHarvestingAddIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;
    TPtrC newFile;
    CImagePlugin* plugin = CImagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {        
        TBuf<KMaxFileName> desPath(_L("c:\\data\\Images\\"));
        desPath.Append( filename );
        //Delete the file if it already exists
        fSession.Delete( desPath ); 
        //To ensure video plugin is loaded, pause the harvester
        RHarvesterClient harvester;
        User::LeaveIfError(harvester.Connect());
        harvester.Pause();
        BaflUtils::EnsurePathExistsL( fSession, desPath );//Create folder
        BaflUtils::CopyFile( fSession, filepath, desPath );
        //Resume harvester for this harvesting
        harvester.Resume();
        harvester.Close();
        //Now start video plugin harvesting
        plugin->StartHarvestingL( _L(IMAGE_QBASEAPPCLASS) );
        //wait for index to flush
        iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
        iPluginTester->iWaitForHarvester->Start();
        TInt count = SearchForTextL( filename, _L(IMAGE_QBASEAPPCLASS), KNullDesC );
        if(count <= 0)
           {
           error = KErrNotFound;
           }
        }
        else
           error = KErrNotFound;
    doLog( iLog, error, _L("Error in TestImageHarvestingAddIndexL") );  
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    return error;
    }

TInt CHarvesterPluginTester::TestImageHarvestingUpdateIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;
    TPtrC newFile;
    TBuf<KMaxFileName> srcPath(_L("c:\\data\\Images\\"));
    TBuf<KMaxFileName> desPath;
    desPath.Copy( srcPath );
    CImagePlugin* plugin = CImagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {
        srcPath.Append( filename );
        if( aItem.GetNextString(newFile) == KErrNone )
            {
            desPath.Append( newFile );
            RHarvesterClient harvester;
            User::LeaveIfError(harvester.Connect());
            harvester.Pause();
            TBool fileExist = BaflUtils::FileExists( fSession, srcPath );        
            if(!fileExist)
            {
            BaflUtils::EnsurePathExistsL( fSession, srcPath );//Create folder
            BaflUtils::CopyFile( fSession, filepath, srcPath );                    
            }            
            BaflUtils::RenameFile( fSession, srcPath, desPath );
            harvester.Resume();
            harvester.Close();
            plugin->StartHarvestingL( _L(IMAGE_QBASEAPPCLASS) );
            //wait for index to flush
            iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );            
            iPluginTester->iWaitForHarvester->Start();
            TInt count = SearchForTextL(_L("Portrait"), _L(IMAGE_QBASEAPPCLASS), KNullDesC );
            if(count <= 0)
               {
               error = KErrNotFound;
               }
            doLog( iLog, error, _L("Error in TestImageHarvestingUpdateIndexL") );
            }        
        }
    else
        doLog( iLog, KErrNotFound, _L("Error in TestImageHarvestingUpdateIndexL") );           
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    return error;
    }

TInt CHarvesterPluginTester::TestImageHarvestingDeleteIndexL( CStifItemParser& aItem )
    {
    TInt error = KErrNone;
    TPtrC filepath;
    TPtrC filename;    
    TBuf<KMaxFileName> srcPath(_L("c:\\data\\Images\\"));
    CImagePlugin* plugin = CImagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL();
    RFs fSession;
    fSession.Connect();
    CleanupClosePushL( fSession );
    if((aItem.GetNextString(filepath)==KErrNone) && (aItem.GetNextString(filename) == KErrNone))
        {
        srcPath.Append( filename );        
        RHarvesterClient harvester;
        User::LeaveIfError(harvester.Connect());
        harvester.Pause();
        TBool fileExist = BaflUtils::FileExists( fSession, srcPath );        
        if(!fileExist)
        {
        BaflUtils::EnsurePathExistsL( fSession, srcPath );//Create folder
        BaflUtils::CopyFile( fSession, filepath, srcPath );                    
        }            
        BaflUtils::DeleteFile( fSession, srcPath );        
        harvester.Resume();
        harvester.Close();
        plugin->StartHarvestingL( _L(IMAGE_QBASEAPPCLASS) );
        //wait for index to flush
        iPluginTester->SetWaitTime( (TTimeIntervalMicroSeconds32)60000000 );
        //wait till image harvesting completes
        iPluginTester->iWaitForHarvester->Start();
        TInt count = SearchForTextL(_L("Square"), _L(IMAGE_QBASEAPPCLASS), KNullDesC );
        if(count <= 0)
           {
           // If the search is not found,then testcase is success
           doLog( iLog, error, _L("Error in TestImageHarvestingDeleteIndexL") );
           }
        }
    else
        doLog( iLog, KErrNotFound, _L("Error in TestImageHarvestingDeleteIndexL") );           
    CleanupStack::PopAndDestroy();
    delete plugin;
    delete iPluginTester;
    return error;
    }

TInt CHarvesterPluginTester::TestAudioMMCEventL( CStifItemParser& aItem )
    {    
    TInt error(KErrNone);
    TInt drive;    
    TInt mmcstatus;
    aItem.GetNextInt ( drive );
    aItem.GetNextInt ( mmcstatus );    
    CAudioPlugin* plugin = CAudioPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //Initialize the Plugin
    TRAPD( err , plugin->HandleMMCEventL( (TDriveNumber)drive , mmcstatus) );
    //iPluginTester->iWaitForHarvester->Start(); //Start Wait AO and let it complete
    doLog(iLog,error,_L("Error in TestAudioMMCEventL"));
    delete plugin;
    delete iPluginTester;    
    //End search
    return err;
    }
        
TInt CHarvesterPluginTester::TestVideoMMCEventL( CStifItemParser& aItem )
    {
    TInt error(KErrNone);
    TInt drive;    
    TInt mmcstatus;
    aItem.GetNextInt ( drive );
    aItem.GetNextInt ( mmcstatus );    
    CVideoPlugin* plugin = CVideoPlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //Initialize the Plugin
    TRAPD( err , plugin->HandleMMCEventL( (TDriveNumber)drive , mmcstatus) );
    //iPluginTester->iWaitForHarvester->Start(); //Start Wait AO and let it complete
    doLog(iLog,error,_L("Error in TestVideoMMCEventL"));
    delete plugin;
    delete iPluginTester;    
    //End search
    return err;
    }
        
TInt CHarvesterPluginTester::TestImageMMCEventL( CStifItemParser& aItem )
    {
    TInt error(KErrNone);
    TInt drive;    
    TInt mmcstatus;
    aItem.GetNextInt ( drive );
    aItem.GetNextInt ( mmcstatus );    
    CImagePlugin* plugin = CImagePlugin::NewL();
    CHarvesterObserver* iPluginTester = CHarvesterObserver::NewL( plugin );
    plugin->StartPluginL(); //Initialize the Plugin
    TRAPD( err , plugin->HandleMMCEventL( (TDriveNumber)drive , mmcstatus) );
    //iPluginTester->iWaitForHarvester->Start(); //Start Wait AO and let it complete
    doLog(iLog,error,_L("Error in TestImageMMCEventL"));
    delete plugin;
    delete iPluginTester;    
    //End search
    return err;
    }

//  [End of File] - Do not remove
