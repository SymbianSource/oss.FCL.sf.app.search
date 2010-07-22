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
* Description: STIF testclass declaration
*
*/

#ifndef HARVESTERPLUGINTESTER_H
#define HARVESTERPLUGINTESTER_H

//  INCLUDES
#include <stiflogger.h>
#include <testscripterinternal.h>
#include <stiftestmodule.h>
#include <testclassassert.h>

#include <cntdb.h> //For contact test cases
#include <cntitem.h>
#include <cntfield.h>
#include <cntfldst.h>

// CONSTANTS
//const ?type ?constant_var = ?constant;
#include "cfileplugin.h"

// MACROS
//#define ?macro ?macro_def
#define TEST_CLASS_VERSION_MAJOR 0
#define TEST_CLASS_VERSION_MINOR 0
#define TEST_CLASS_VERSION_BUILD 0

// Logging path
_LIT( KHarvesterPluginTesterLogPath, "\\logs\\testframework\\HarvesterPluginTester\\" ); 
// Log file
_LIT( KHarvesterPluginTesterLogFile, "HarvesterPluginTester.txt" ); 
_LIT( KHarvesterPluginTesterLogFileWithTitle, "HarvesterPluginTester_[%S].txt" );

// FUNCTION PROTOTYPES
//?type ?function_name(?arg_list);

// FORWARD DECLARATIONS
//class ?FORWARD_CLASSNAME;
class CHarvesterPluginTester;
class CSearchDocument;
// DATA TYPES
//enum ?declaration
//typedef ?declaration
//extern ?data_type;

// CLASS DECLARATION

/**
*  CHarvesterPluginTester test class for STIF Test Framework TestScripter.
*  ?other_description_lines
*
*  @lib ?library
*  @since ?Series60_version
*/
NONSHARABLE_CLASS(CHarvesterPluginTester) : public CScriptBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CHarvesterPluginTester* NewL( CTestModuleIf& aTestModuleIf );

        /**
        * Destructor.
        */
        virtual ~CHarvesterPluginTester();

    public: // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    public: // Functions from base classes

        /**
        * From CScriptBase Runs a script line.
        * @since ?Series60_version
        * @param aItem Script line containing method name and parameters
        * @return Symbian OS error code
        */
        virtual TInt RunMethodL( CStifItemParser& aItem );

    protected:  // New functions

        /**
        * ?member_description.
        * @since ?Series60_version
        * @param ?arg1 ?description
        * @return ?description
        */
        //?type ?member_function( ?type ?arg1 );

    protected:  // Functions from base classes

        /**
        * From ?base_class ?member_description
        */
        //?type ?member_function();

    private:

        /**
        * C++ default constructor.
        */
        CHarvesterPluginTester( CTestModuleIf& aTestModuleIf );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        // Prohibit copy constructor if not deriving from CBase.
        // ?classname( const ?classname& );
        // Prohibit assigment operator if not deriving from CBase.
        // ?classname& operator=( const ?classname& );

        /**
        * Frees all resources allocated from test methods.
        * @since ?Series60_version
        */
        void Delete();

        /**
        * Test methods are listed below. 
        */

        /**
        * Example test method.
        * @since ?Series60_version
        * @param aItem Script line containing parameters.
        * @return Symbian OS error code.
        */
        virtual TInt TestStartHarvesterL( CStifItemParser& aItem );
		
        virtual TInt TestFormBaseAppClassL( CStifItemParser& aItem );
		
		virtual TInt TestDatabasePathL( CStifItemParser& aItem );
		
		virtual TInt TestCreateIndexItemL( CStifItemParser& aItem );		

        virtual TInt TestFolderCreateL( CStifItemParser& aItem );        

        virtual TInt TestFolderRenameL( CStifItemParser& aItem );
        
        /**
         * Message harvester test method.
         * @since ?Series60_version
         * @param aItem Script line containing parameters.
         * @return Symbian OS error code.
         */
        virtual TInt TestMessageHarvesterL( CStifItemParser& aItem );
        
        /**
         * Message harvester test method.
         * @since ?Series60_version
         * @param aItem Script line containing parameters.
         * @return Symbian OS error code.
         */
        virtual TInt TestMessageHarvesterWithMessageL( CStifItemParser& aItem );
        
        /**
         * Message harvester test method.
         * @since ?Series60_version
         * @param aItem Script line containing parameters.
         * @return Symbian OS error code.
         */
        virtual TInt TestMessageHarvesterChangeDriveL( CStifItemParser& aItem );
        
        virtual TInt TestMessageRunErrorL( CStifItemParser& aItem );
        
        virtual TInt TestMessageIsMediaRemovableL( CStifItemParser& aItem );
        
        virtual TInt TestMessageMountNoIndexerL( CStifItemParser& aItem );
        
        virtual TInt TestStartBookmarksHarvesterL( CStifItemParser& aItem );
        
        virtual TInt TestAddBookmarkL( CStifItemParser& aItem );
        
        virtual TInt TestAddGetDomainL( CStifItemParser& aItem );
        
        TInt SearchForTextL(const TDesC& aText, const TDesC& aBaseAppclass,const TDesC& aDefaultField);
        
        /*** Start of Applications plugin tests ***/
        virtual TInt TestStartApplicationsHarvesterL( CStifItemParser& aItem );
        virtual TInt TestCreateApplicationsIndexItemL( CStifItemParser& aItem );
        /*** End of Applications plugin tests   ***/
		
        /*** Start of Notes plugin tests ***/
        virtual TInt TestStartNotesHarvesterL( CStifItemParser& aItem );
        virtual TInt TestAddNoteL( CStifItemParser& aItem );
        virtual TInt TestAddLongNoteL( CStifItemParser& aItem );
        virtual TInt TestAddAlphaNumericNoteL( CStifItemParser& aItem );
        TInt PerformNotesTestL( TPtrC aString1 , TPtrC aString2 );
        virtual TInt TestDeleteNoteL( CStifItemParser& aItem);
        virtual TInt TestUpdateNoteL( CStifItemParser& aItem );
        /*** End of Notes plugin tests   ***/
        
        virtual TInt TestStartContactsHarvesterL( CStifItemParser& aItem );
                        
        TContactItemId CreateNewContactL(CContactDatabase& database,const TDesC& aName,const TDesC& aPhoneNumber);
                        
        virtual TInt TestCreateContactIndexItemL( CStifItemParser& aItem );
        
        virtual TInt TestCreateContactIndexItemNoIndexerL( CStifItemParser& aItem );
        
        virtual TInt TestContactGetDateL( CStifItemParser& aItem );
        
        virtual TInt TestContactDelayedErrorL( CStifItemParser& aItem );
        
        virtual TInt TestContactDelayedCallbackNoIContactsL( CStifItemParser& aItem );
        
        virtual TInt TestContactDelayedCallbackNoIObserverL( CStifItemParser& aItem );
        
        virtual TInt TestContactDelayedCallbackWrongCountL( CStifItemParser& aItem );
        
		virtual TInt TestCreateAllContactFieldsL( CStifItemParser& aItem );
		
        virtual TInt TestCreateContactGroupL( CStifItemParser& aItem );
        
        virtual TInt TestStartCalenderHarvesterL( CStifItemParser& aItem );
                
        virtual TInt TestCalenderEntryL( CStifItemParser& aItem );        
        
        virtual TInt TestCreateMmsL( CStifItemParser& aItem );
        
        virtual TInt TestCreateEmailL( CStifItemParser& aItem );
		
		virtual TInt TestAudioHarvestingL( CStifItemParser& aItem );
		
		virtual TInt TestAudioHarvestingUpdateIndexL( CStifItemParser& aItem );
		        
        virtual TInt TestAudioHarvestingDeleteIndexL( CStifItemParser& aItem );
		
		virtual TInt TestMdsSyncControllerL( CStifItemParser& aItem );
		
		virtual TInt TestBlacklistPluginL( CStifItemParser& aItem );
		
		virtual TInt TestBlacklistPluginVersionL( CStifItemParser& aItem );
		
		virtual TInt TestVideoHarvestingIndexL( CStifItemParser& aItem );

        virtual TInt TestVideoHarvestingUpdateIndexL( CStifItemParser& aItem );
		
		virtual TInt TestVideoHarvestingDeleteIndexL( CStifItemParser& aItem );
		
		virtual TInt TestImageHarvestingAddIndexL( CStifItemParser& aItem );
		
		virtual TInt TestImageHarvestingUpdateIndexL( CStifItemParser& aItem );
		
		virtual TInt TestImageHarvestingDeleteIndexL( CStifItemParser& aItem );
		
		virtual TInt TestAudioMMCEventL( CStifItemParser& aItem );
		
		virtual TInt TestVideoMMCEventL( CStifItemParser& aItem );
		
		virtual TInt TestImageMMCEventL( CStifItemParser& aItem );
		
		virtual TInt TestStartEmailPluginL( CStifItemParser& aItem );
		
		CSearchDocument* prepareemaildocument();
		
		virtual TInt TestHandleEmailDocL( CStifItemParser& aItem );
		
		virtual TInt TestQualifiedBaseAppClassL( CStifItemParser& );
		
		virtual TAny TestHandleMsgMovedL( CStifItemParser& );
		
		virtual TAny TestMessageDataHandlerL( CStifItemParser& );
		
		virtual TInt TestMsgUnMountL( CStifItemParser& );
		
		virtual TInt TestFormFormBaseAppClassL( CStifItemParser& );
		
		virtual TInt TestMsgDeleteL( CStifItemParser& );
		
		virtual TInt TestMsgHandlesessionL( CStifItemParser& );
		
		virtual TInt TestMsgHandlesessionPanicL( CStifItemParser& aItem );
		
		virtual TInt TestMsgOverwriteOrAddToQuereL( CStifItemParser& aItem );
		
		virtual TInt TestMsgMessageHarvesterL( CStifItemParser& aItem );
        /**
         * Method used to log version of test class
         */
        void SendTestClassVersion();

        //ADD NEW METHOD DEC HERE
        //[TestMethods] - Do not remove

    public:     // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    protected:  // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

    private:    // Data
        // ?one_line_short_description_of_data
        //?data_declaration;

        // Reserved pointer for future extension
        //TAny* iReserved;

    public:     // Friend classes
        //?friend_class_declaration;
    protected:  // Friend classes
        //?friend_class_declaration;
    private:    // Friend classes
        //?friend_class_declaration;

    };

#endif      // HARVESTERPLUGINTESTER_H

// End of File
