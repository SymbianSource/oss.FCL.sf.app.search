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
* Description:  Harvester message plugin
*
*/


//  INCLUDES 

#include <e32base.h>

#include <mtclreg.h>
#include <smsclnt.h> // SMS Headers
#include <mmsclient.h> // MMS Headers
#include <mmsvattachmentmanager.h>
#include <impcmtm.h> // IMAP Headers
#include <smtcmtm.h> // SMTP
#include <popcmtm.h> // POP

#include "harvesterserverlogger.h"
#include "cmessagedatahandler.h"
#include "cmessageplugin.h"
#include <csearchdocument.h>
#include <common.h>

#include <ccpixindexer.h>
#include "OstTraceDefinitions.h"
#ifdef OST_TRACE_COMPILER_IN_USE
#include "cmessagedatahandlerTraces.h"
#endif


/** Number of symbols from MsgBody taken to excerpt */
const TInt KMsgBodyExcerptSymbols = 90;

// For Ucs2 detection
const TInt KUtf8BomLength = 3;
const TUint8 KUtf8Bom[KUtf8BomLength] = {0xEF, 0xBB, 0xBF};

// CSearchDocument ID max length
const TUint KMaxDocId = 20;
// Hardcoded limit for the body size to index (and store)
const TInt KMaxDocumentSize = 1024;

_LIT8(KMimeTypeText, "text/plain" );

_LIT(KMimeTypeField, CPIX_MIMETYPE_FIELD);
_LIT(KMimeTypeMessaging, MESSAGING_MIMETYPE);

/** Field names */
_LIT(KToField, TO_FIELD);
_LIT(KCcField, CC_FIELD);
_LIT(KBccField, BCC_FIELD);
_LIT(KFromField, FROM_FIELD);
_LIT(KFolderField, FOLDER_FIELD);
_LIT(KBodyField, BODY_FIELD);
_LIT(KSubjectField, SUBJECT_FIELD);

// ============================ MEMBER FUNCTIONS ===============================

// ---------------------------------------------------------------------------
// CMessageDataHandler::NewL
// ---------------------------------------------------------------------------
//
CMessageDataHandler* CMessageDataHandler::NewL(CMessagePlugin& aMessagePlugin,
		CMsvSession& aSession)
	{
		OstTraceFunctionEntry0( CMESSAGEDATAHANDLER_NEWL_ENTRY );
		CPIXLOGSTRING( "CMessageDataHandler::NewL() - enter" );

	CMessageDataHandler* self = new (ELeave) CMessageDataHandler(
			aMessagePlugin, aSession);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);

		CPIXLOGSTRING( "CMessageDataHandler::NewL() - return" );
	OstTraceFunctionExit0( CMESSAGEDATAHANDLER_NEWL_EXIT );
	return self;
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::~CMessageDataHandler
// ---------------------------------------------------------------------------
//
CMessageDataHandler::~CMessageDataHandler()
	{
	Cancel();
	iFs.Close();
	delete iMmsMtm;
	delete iSmsMtm;
	delete iSmtpMtm;
	delete iPop3Mtm;
	delete iImap4Mtm;
	delete iMtmReg;
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::CMessageDataHandler
// ---------------------------------------------------------------------------
//
CMessageDataHandler::CMessageDataHandler(CMessagePlugin& aMessagePlugin, CMsvSession& aSession)
	: CActive(EPriorityLow), iMessagePlugin(aMessagePlugin), iMsvSession(aSession)
	{
	CActiveScheduler::Add(this);
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::ConstructL
// ---------------------------------------------------------------------------
//
void CMessageDataHandler::ConstructL()
	{
	// TODO iFs only used in MMS attachment logic, 
	// from a performance perspective, is it worthwhile connecting here?
	User::LeaveIfError(iFs.Connect());
	iMtmReg = CClientMtmRegistry::NewL(iMsvSession);
	TRAPD(err,iSmsMtm = static_cast<CSmsClientMtm*> (iMtmReg->NewMtmL(KUidMsgTypeSMS)));
	OstTrace1( TRACE_NORMAL, CMESSAGEDATAHANDLER_CONSTRUCTL, "CMessageDataHandler::ConstructL;iSmsMtm err=%d",err );
	CPIXLOGSTRING2( "CMessageDataHandler::ConstructL() iSmsMtm err = %d",err);
	TRAP(err,iMmsMtm = static_cast<CMmsClientMtm*> (iMtmReg->NewMtmL(KUidMsgTypeMultimedia)));
	OstTrace1( TRACE_NORMAL, DUP1_CMESSAGEDATAHANDLER_CONSTRUCTL, "CMessageDataHandler::ConstructL;iMmsMtm err=%d", err );
	CPIXLOGSTRING2( "CMessageDataHandler::ConstructL() iMmsMtm err = %d",err);
	/*
	TRAP(err,iSmtpMtm = static_cast<CSmtpClientMtm*> (iMtmReg->NewMtmL(KUidMsgTypeSMTP)));
	CPIXLOGSTRING2( "CMessageDataHandler::ConstructL() iSmtpMtm err = %d",err);
	TRAP(err, iPop3Mtm = static_cast<CPop3ClientMtm*> (iMtmReg->NewMtmL(KUidMsgTypePOP3)));
	CPIXLOGSTRING2( "CMessageDataHandler::ConstructL() iPop3Mtm err = %d",err);
	TRAP(err,iImap4Mtm = static_cast<CImap4ClientMtm*> (iMtmReg->NewMtmL(KUidMsgTypeIMAP4)));
	CPIXLOGSTRING2( "CMessageDataHandler::ConstructL() iImap4Mtm err = %d",err);
	*/
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::GatherData
// ---------------------------------------------------------------------------
//
void CMessageDataHandler::GatherDataL(TMsvId& aMessageId,
		TCPixActionType aActionType, TMsvId& aFolderId)
	{
	TMessageItem item(aMessageId, aActionType, aFolderId);
	iMessageArray.AppendL(item);
	HandleNextRequest();
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::IsValidMessageTypeL
// ---------------------------------------------------------------------------
//	    
TInt CMessageDataHandler::IsValidMessageType(TMsvId aIdToCheck,
		TMsgType& aMessageType)
	{
		OstTraceFunctionEntry0( CMESSAGEDATAHANDLER_ISVALIDMESSAGETYPE_ENTRY );
		CPIXLOGSTRING("ENTER CMessageDataHandler::IsValidMessageType");
	TMsvEntry entry;
	TMsvId service = 0;
	TInt err = iMsvSession.GetEntry(aIdToCheck, service, entry);
		OstTrace1( TRACE_NORMAL, CMESSAGEDATAHANDLER_ISVALIDMESSAGETYPE, "CMessageDataHandler::IsValidMessageType;GetEntry Error=%d", err );
		CPIXLOGSTRING2("CMessageDataHandler::IsValidMessageType GetEntry: %d", err);
	if (!err)
		{
		aMessageType = iMessagePlugin.CalculateMessageType(entry);
			OstTrace1( TRACE_NORMAL, DUP1_CMESSAGEDATAHANDLER_ISVALIDMESSAGETYPE, "CMessageDataHandler::IsValidMessageType;Message Type=%d", aMessageType );
			CPIXLOGSTRING2("CMessageDataHandler::IsValidMessageType type: %d", aMessageType );
		}
		CPIXLOGSTRING("END CMessageDataHandler::IsValidMessageType");
	OstTraceFunctionExit0( CMESSAGEDATAHANDLER_ISVALIDMESSAGETYPE_EXIT );
	return err;
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::CreateSmsIndexItemLC
// ---------------------------------------------------------------------------
//	    
void CMessageDataHandler::CreateMessageIndexItemL(const TMsvId& aMsvId,
		TCPixActionType aActionType, const TMsvId& aFolderId)
	{
	OstTrace1( TRACE_NORMAL, CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;aMsvId=%d", aMsvId );
	CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): aMsvId = %d ", aMsvId );

	// Index an empty item if removal action
	if (aActionType == ECPixRemoveAction)
		{
		TBuf<20> docid_str;
		docid_str.AppendNum(aMsvId);
		if (iMessagePlugin.GetIndexer())
			{
			TRAPD(err, iMessagePlugin.GetIndexer()->DeleteL(docid_str));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP1_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): Document deleted." );
				CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): Document deleted.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP2_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;Error in deleting the doc=%d", err );
				CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): Error %d in deleting the document.", err);				
				}
			}
		return;
		}

	// Check message type
	TMsgType messageType = EMsgTypeInvalid; // initialisation
	TInt error = IsValidMessageType(aMsvId, messageType);

	// creating CSearchDocument object with unique ID for this application
	CSearchDocument* index_item = NULL;

	switch (messageType)
		{
		case EMsgTypeSms:
			{
			TRAPD(err, index_item = CreateSmsDocumentL(aMsvId, aFolderId));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP3_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): SMS document created." );
				CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): SMS document created.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP4_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;Error in creating the sms doc=%d", err );
				CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): Error %d in creating SMS document.", err);
				}
			break;
			}
		case EMsgTypeMms:
			{
			TRAPD(err, index_item = CreateMmsDocumentL(aMsvId, aFolderId));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP5_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): MMS document created." );
				CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): MMS document created.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP6_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;Error in creating the mms doc=%d", err );
				CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): Error %d in creating MMS document.", err);
				}
			break;
			}
		case EMsgTypeEmailPop3:
		case EMsgTypeEmailImap4:
		case EMsgTypeEmailSmtp:
			{
			TRAPD(err, index_item = CreateEmailDocumentL(aMsvId, aFolderId));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP7_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): E-mail document created." );
				CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): E-mail document created.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP8_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;Error in ccreating the email doc=%d", err );
				CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): Error %d in creating e-mail document.", err);
				}
			break;
			}
		case EMsgTypeInvalid:
		// For EMsgTypeDraft there is no way to tell if it was a SMS, MMS or email,
		// so don't create index_item.
		case EMsgTypeDraft:
		default:
			// Error occurred
			break;
		}

	// Exit if wrong message type
	if (index_item == NULL)
		{
		OstTrace0( TRACE_NORMAL, DUP9_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): Document was not created." );
		CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): Document was not created.");
		return;
		}
	CleanupStack::PushL(index_item);
		
	// Folder field
	TMsvEntry entry;
	TMsvId service = 0;
	iMsvSession.GetEntry(aFolderId, service, entry);
	HBufC *folder_str = entry.iDetails.AllocLC();
	index_item->AddFieldL(KFolderField, *folder_str, CDocumentField::EStoreYes | CDocumentField::EIndexNo);

	// Mime type field
	index_item->AddFieldL(KMimeTypeField, KMimeTypeMessaging, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);		
	
		// Send for indexing
	if (iMessagePlugin.GetIndexer())
		{
		if (aActionType == ECPixAddAction)
			{
			TRAPD(err, iMessagePlugin.GetIndexer()->AddL(*index_item));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP10_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): Added." );
				CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): Added.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP11_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;Error in Adding=%d", err );
				CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): Error %d in adding.", err);
				}
			}
		else if (aActionType == ECPixUpdateAction)
			{
			TRAPD(err, iMessagePlugin.GetIndexer()->UpdateL(*index_item));
			if (err == KErrNone)
				{
				OstTrace0( TRACE_NORMAL, DUP12_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL(): Updated." );
				CPIXLOGSTRING("CMessageDataHandler::CreateMessageIndexItemL(): Updated.");
				}
			else
				{
				OstTrace1( TRACE_NORMAL, DUP13_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "CMessageDataHandler::CreateMessageIndexItemL;Error in Updating=%d", err );
				CPIXLOGSTRING2("CMessageDataHandler::CreateMessageIndexItemL(): Error %d in updating.", err);
				}
			}
		}
	else
		{
		OstTrace0( TRACE_NORMAL, DUP14_CMESSAGEDATAHANDLER_CREATEMESSAGEINDEXITEML, "END CMessageDataHandler::CreateMessageIndexItemL(): No indexer present." );
		CPIXLOGSTRING("END CMessageDataHandler::CreateMessageIndexItemL(): No indexer present.");
		}
		
	CleanupStack::PopAndDestroy(folder_str);
	CleanupStack::PopAndDestroy(index_item);
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::CreateSmsDocumentL
// ---------------------------------------------------------------------------
//
CSearchDocument* CMessageDataHandler::CreateSmsDocumentL(const TMsvId& aMsvId, const TMsvId& aFolderId)
	{
	// Select mtm for this type
	iSmsMtm->SwitchCurrentEntryL(aMsvId);
	iSmsMtm->LoadMessageL();

	// creating CSearchDocument object with unique ID for this application
	TBuf<KMaxDocId> docid_str;
	docid_str.AppendNum(aMsvId);
	CSearchDocument* index_item = CSearchDocument::NewLC(docid_str, _L(SMSAPPCLASS));

	// Add from field use the name instead of number if in the contacts db.
	// (iSmsMtm->SmsHeader().FromAddress() - only returns the number)
	TMsvEntry entry;
	TMsvId service = 0;
	iMsvSession.GetEntry(aMsvId, service, entry);
	HBufC *fromNameOrNumberBuf = entry.iDetails.AllocLC();
	index_item->AddFieldL(KFromField, *fromNameOrNumberBuf);

	// Add the recipients as content items
	TBuf<64> to_field;
	const CDesCArray
			& recipientArray =
					static_cast<const CDesCArray&> (iSmsMtm->AddresseeList().RecipientList());
	for (TInt i = 0; i < recipientArray.MdcaCount(); i++)
		{
		to_field = KToField;
		if (i>0)
			to_field.AppendNum(i);
		index_item->AddFieldL(to_field, recipientArray.MdcaPoint(i));
		}

	// Add the body text as a content item
	TInt msgLength = iSmsMtm->Body().DocumentLength();
	HBufC* bodyText = HBufC::NewLC(msgLength);
	TPtr body_ptr = bodyText->Des();
	iSmsMtm->Body().Extract(body_ptr);
	index_item->AddFieldL(KBodyField, *bodyText);
	
	// Add excerpt
	// Note SMS does not have a subject field.
	HBufC* excerpt = CreateExcerptLC(*fromNameOrNumberBuf, recipientArray, KNullDesC(), body_ptr, aFolderId);
	index_item->AddExcerptL(*excerpt);
	CleanupStack::PopAndDestroy(excerpt);	
	CleanupStack::PopAndDestroy(bodyText);

	CleanupStack::PopAndDestroy(fromNameOrNumberBuf);

	// Pop the item
	CleanupStack::Pop(index_item);
	return index_item;
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::CreateMmsDocumentL
// ---------------------------------------------------------------------------
//
CSearchDocument* CMessageDataHandler::CreateMmsDocumentL(const TMsvId& aMsvId, const TMsvId& aFolderId)
	{
	// Select mtm for this type
 	iMmsMtm->SwitchCurrentEntryL(aMsvId);
	iMmsMtm->LoadMessageL();

	// creating CSearchDocument object with unique ID for this application
	TBuf<KMaxDocId> docid_str;
	docid_str.AppendNum(aMsvId);
	CSearchDocument* index_item = CSearchDocument::NewLC(docid_str, _L(MMSAPPCLASS));
	
	// Add from field
	index_item->AddFieldL(KFromField, iMmsMtm->Sender());

	// Add the recipients as content items
	TBuf<64> to_field;
	const CDesCArray
			& recipientArray =
					static_cast<const CDesCArray&> (iMmsMtm->AddresseeList().RecipientList());
	for (TInt i = 0; i < recipientArray.MdcaCount(); i++)
		{
		to_field = KToField;
		if (i>0)
			to_field.AppendNum(i);
		index_item->AddFieldL(to_field, recipientArray.MdcaPoint(i));
		}

	// Add subject
	TPtrC subject(iMmsMtm->SubjectL());
	index_item->AddFieldL(KSubjectField, subject);

	// Add the body parts as a content items
    // See if the MMS contains a text file attachment
    TBool excerptFieldAdded(EFalse);
    TInt bodycount = 0;
    CMsvStore* msvStore = iMmsMtm->Entry().ReadStoreL();
	CleanupStack::PushL( msvStore );
	MMsvAttachmentManager& attManager = msvStore->AttachmentManagerL();
	for ( TInt i = 0; i < attManager.AttachmentCount(); i++ )
	    {
		CMsvAttachment* attInfo = attManager.GetAttachmentInfoL(i);
		CleanupStack::PushL( attInfo );
		
        if (attInfo->MimeType() == KMimeTypeText())
            {
            // Mime type
            RFile attFile = attManager.GetAttachmentFileL(i);
            CleanupClosePushL( attFile );
            TInt charsetMIB = TextFileCharsetL(iFs, attFile);
            
            // Get the file size
            TInt fileSize = 0;
            User::LeaveIfError(attFile.Size(fileSize));
            
            // Read the file
            HBufC8* textSource = HBufC8::NewLC(fileSize);
            TPtr8 ptr8 = textSource->Des();
            attFile.Read(ptr8);
  
  			// Convert character set
            HBufC* text = HBufC::NewLC( fileSize );
            TPtr buf = text->Des();
            if ( KMmsIso10646Ucs2 != charsetMIB )
                {
                if ( KMmsUsAscii == charsetMIB )
                    buf.Copy(*textSource);
                else
                    ConvertToUcs2FromUtfL(iFs, buf, *textSource, charsetMIB);
                }
            else
                {
                TPtrC ptr(reinterpret_cast<const TUint16*>(textSource->Des().Ptr()), 
                                  textSource->Length() / sizeof(TText));
        
                // Ignore the first unicode charcter FFFE
                ptr.Set( ptr.Mid( 1 ) );
                buf = ptr;
                }

        
            // Add the text of the file as a content item
            // MMS message body is in attachment
			TBuf<64> body_field;
			body_field = KBodyField;
			if (bodycount>0)
			{
				// Add additional body fields
				body_field.AppendNum(bodycount);
			}
			else
			{
				// Add excerpt for the first body field
				HBufC* excerpt = CreateExcerptLC(iMmsMtm->Sender(), recipientArray, subject, *text, aFolderId);
				index_item->AddExcerptL(*excerpt);
				CleanupStack::PopAndDestroy(excerpt);
				excerptFieldAdded  = ETrue;
			}

            // TODO: add only few kilobytes of the body?
            index_item->AddFieldL(body_field, *text);
            ++bodycount;

            CleanupStack::PopAndDestroy( 3, &attFile ); // text, textSource, attFile
            }		
        else 
            {
            // add attachment name when binary file.
            if ( !excerptFieldAdded )
                {
                // attachment name might not be set
                HBufC* excerpt = CreateExcerptLC(iMmsMtm->Sender(), recipientArray, subject, attInfo->AttachmentName(), aFolderId);
                index_item->AddExcerptL(*excerpt);
                CleanupStack::PopAndDestroy(excerpt);
                excerptFieldAdded  = ETrue;
                }
            }
	    CleanupStack::PopAndDestroy( attInfo );		
	    }

    // If no excerpt has been added yet, then add information have
    if ( !excerptFieldAdded )
        {
        HBufC* excerpt = CreateExcerptLC(iMmsMtm->Sender(), recipientArray, subject, KNullDesC(), aFolderId);
        index_item->AddExcerptL(*excerpt);
        CleanupStack::PopAndDestroy(excerpt);
        }

    // Cleanup
    CleanupStack::PopAndDestroy( msvStore );

    // Pop the item
    CleanupStack::Pop(index_item);
    return index_item;
    }

// ---------------------------------------------------------------------------
// CMessageDataHandler::CreateEmailDocumentL
// ---------------------------------------------------------------------------
//
CSearchDocument* CMessageDataHandler::CreateEmailDocumentL(const TMsvId& aMsvId, const TMsvId& aFolderId)
	{
	// creating CSearchDocument object with unique ID for this application
	TBuf<KMaxDocId> docid_str;
	docid_str.AppendNum(aMsvId);
	CSearchDocument* index_item = CSearchDocument::NewLC(docid_str, _L(EMAILAPPCLASS));

	// Open the message entry
	CMsvEntry* message_entry = CMsvEntry::NewL(iMsvSession, aMsvId, TMsvSelectionOrdering());
	CleanupStack::PushL(message_entry);
	
	// Get body
	CParaFormatLayer* paraFormatLayer = CParaFormatLayer::NewL();
	CleanupStack::PushL(paraFormatLayer);
	CCharFormatLayer* charFormatLayer = CCharFormatLayer::NewL();
	CleanupStack::PushL(charFormatLayer);
	CRichText* richtext = CRichText::NewL(paraFormatLayer, charFormatLayer);
	CleanupStack::PushL(richtext);
	CImEmailMessage* message_body = CImEmailMessage::NewLC(*message_entry);
	message_body->GetBodyTextL(aMsvId, CImEmailMessage::EThisMessageOnly, *richtext, *paraFormatLayer, *charFormatLayer);
	
	// Read the message header
	CMsvStore* message_store = message_entry->ReadStoreL();
	CleanupStack::PushL(message_store);
	CImHeader* header = CImHeader::NewLC();
	header->RestoreL(*message_store);
	
	// Add from field
	index_item->AddFieldL(KFromField, header->From());

	// Add the ToRecipients as content items
	TBuf<64> to_field;
	for (TInt i = 0; i < header->ToRecipients().MdcaCount(); i++)
		{
		to_field = KToField;
		if (i>0)
			to_field.AppendNum(i);
		index_item->AddFieldL(to_field, header->ToRecipients().MdcaPoint(i));
		}

	// Add the CcRecipients as content items
	TBuf<64> cc_field;
	for (TInt i = 0; i < header->CcRecipients().MdcaCount(); i++)
		{
		cc_field = KCcField;
		if (i>0)
			cc_field.AppendNum(i);
		index_item->AddFieldL(cc_field, header->CcRecipients().MdcaPoint(i));
		}

	// Add the BccRecipients as content items
	TBuf<64> bcc_field;
	for (TInt i = 0; i < header->BccRecipients().MdcaCount(); i++)
		{
		bcc_field = KBccField;
		if (i>0)
			bcc_field.AppendNum(i);
		index_item->AddFieldL(bcc_field, header->BccRecipients().MdcaPoint(i));
		}

	const TInt richTxtLen = (richtext->DocumentLength() < KMaxDocumentSize) ? richtext->DocumentLength() : KMaxDocumentSize;
	HBufC* body_text = richtext->Read(0).Left(richTxtLen).AllocLC();

	// Add subject
	TPtrC subject(header->Subject());
	index_item->AddFieldL(KSubjectField, subject);
	
	// Add message body
	index_item->AddFieldL(KBodyField, *body_text);
	
	// Add excerpt
    HBufC* excerpt = CreateExcerptLC(header->From(), header->ToRecipients(), subject, *body_text, aFolderId);
    index_item->AddExcerptL(*excerpt);
	CleanupStack::PopAndDestroy(excerpt);

	// Cleanup everything last item message entry
	CleanupStack::PopAndDestroy(8, message_entry);

	// Pop the item
	CleanupStack::Pop(index_item);
	return index_item;	
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::CreateExcerptLC
// ---------------------------------------------------------------------------
//
HBufC* CMessageDataHandler::CreateExcerptLC(const TDesC& aFromAddress,
											const CDesCArray& aRecipientArray,
											const TDesC& aSubject,
											const TDesC& aBodyText,
											const TMsvId& aFolderId)
	{
    //Note:Only From/To (as appropriate): and Body: fields to be added to excerpt for now. 
    //See appclass-hierarchy.txt for details.  
    
	_LIT(KEllipsis, "...");
	_LIT(KSpace, " ");
	TInt excerptLength = KMsgBodyExcerptSymbols + KEllipsis().Length(); 

	//Not removing commented out code as this may come back into use in near future.
//	TMsvEntry entry;
//	TMsvId service = 0;
//	iMsvSession.GetEntry(aFolderId, service, entry);
//	HBufC *folder_str = entry.iDetails.AllocLC();
//
//	excerptLength += folder_str->Length();
//	excerptLength += KSpace().Length();

	if( aFolderId == KMsvGlobalInBoxIndexEntryIdValue && aFromAddress.Length() > 0 )
	    {
        excerptLength += aFromAddress.Length();
        excerptLength += KSpace().Length();
	    }
	else if( aFolderId != KMsvGlobalInBoxIndexEntryIdValue && aRecipientArray.MdcaCount() > 0 )
	    {
        excerptLength += aRecipientArray.MdcaPoint(0).Length();
        excerptLength += KSpace().Length(); 
	    }

	HBufC* excerpt = HBufC::NewL(excerptLength);
	TPtr excerptPtr = excerpt->Des();

	//Not removing commented out code as this may come back into use in near future.
//	excerptPtr.Copy(*folder_str);
//	excerptPtr.Append(KSpace);
	
	//For inbox items, From: is present while for sent items To: is present.
    if ((aFromAddress.Length() > 0) && (aFolderId == KMsvGlobalInBoxIndexEntryIdValue))
        {
        excerptPtr.Append(aFromAddress);
        excerptPtr.Append(KSpace);
        }
    else if ((aRecipientArray.MdcaCount() > 0) && (aFolderId != KMsvGlobalInBoxIndexEntryIdValue))
        {
        excerptPtr.Append(aRecipientArray.MdcaPoint(0));
        excerptPtr.Append(KSpace);
        }

	//Not deleting this code as it might have to be brought back into use
	//in (possibly near) future.
//	if (aSubject.Length() > 0)
//		{
//		excerptPtr.Append(aSubject);
//		excerptPtr.Append(KSpace);
//		}

	excerptPtr.Append(aBodyText.Left(KMsgBodyExcerptSymbols));
	if (aBodyText.Length() > KMsgBodyExcerptSymbols)
		excerptPtr.Append(KEllipsis);

//	CleanupStack::PopAndDestroy(folder_str);
	CleanupStack::PushL(excerpt);
	return excerpt;
	}

// ---------------------------------------------------------------------------
// CMessageDataHandler::IsTextUcs2
// ---------------------------------------------------------------------------
//
TBool CMessageDataHandler::IsTextUcs2(const TDesC8& aText)
    {
    TBool ret = EFalse;
    // Check UCS-2 Byte order mark is in the first two bytes of aText
    if (aText.Length() >= sizeof(KMmsByteOrderMark))
        {
        TPtrC ptr(reinterpret_cast<const TUint16*>(aText.Ptr()), 
                        aText.Length() / sizeof(TText));

        if (KMmsByteOrderMark == ptr[0] ||
            KMmsReversedByteOrderMark == ptr[0])
            {
            ret = ETrue;
            }
        }
    return ret;
    }   	

// ---------------------------------------------------------------------------
// CMessageDataHandler::TextContainsUtf8Bom
// ---------------------------------------------------------------------------
//
TBool CMessageDataHandler::TextContainsUtf8Bom(const TDesC8& aText)
    {    
    TBool ret = EFalse;
    // Check to see if there is the UTF8 BOM at the begining
    TPtrC8 utf8Id(KUtf8Bom, KUtf8BomLength);
    if (aText.Length() >= KUtf8BomLength &&
        aText.Left(KUtf8BomLength) == utf8Id)
        {
        ret = ETrue;
        }
    return ret;
    }

// ---------------------------------------------------------------------------
// CMessageDataHandler::TextFileCharsetL
// ---------------------------------------------------------------------------
//
TUint CMessageDataHandler::TextFileCharsetL(RFs aFs, RFile aFile)
    {
    TUint ret = KMmsUsAscii;
    TBuf8<sizeof(KMmsByteOrderMark)> ucs2Buf;

    TInt size;
    User::LeaveIfError(aFile.Size(size));
    if (size >= sizeof(KMmsByteOrderMark))
        {
        // Check for the UCS-2 Byte Order Mark (BOM) at the beginning 
        // beginning of the file
        User::LeaveIfError(aFile.Read(ucs2Buf));
        if(IsTextUcs2(ucs2Buf))
            {
            ret = KMmsIso10646Ucs2;
            }
        }

    if (KMmsIso10646Ucs2 != ret)
        {
        // If the text is not UCS-2 it will probably be Utf8 or Ascii

        TInt seekPos = 0;
        User::LeaveIfError(aFile.Seek(ESeekStart, seekPos));
        
        HBufC8* buf = HBufC8::NewLC(size);
        TPtr8 ptr = buf->Des();
        User::LeaveIfError(aFile.Read(ptr));

        // Check to see if there is the UTF8 BOM at the begining
        if (TextContainsUtf8Bom(ptr))
            {
            ret = KMmsUtf8;
            }
        else
            {
            CCnvCharacterSetConverter* converter = 
                              CCnvCharacterSetConverter::NewLC();
            CArrayFix<CCnvCharacterSetConverter::SCharacterSet>* availCharSets =
                CCnvCharacterSetConverter::CreateArrayOfCharacterSetsAvailableLC(aFs);

            CArrayFix<CCnvCharacterSetConverter::SCharacterSet>* utf8CharSet = 
                new (ELeave) CArrayFixFlat<CCnvCharacterSetConverter::SCharacterSet>(1); 
            CleanupStack::PushL(utf8CharSet);

            TInt count = availCharSets->Count();
            for (TInt i = 0; i < count; i++)
                {
                CCnvCharacterSetConverter::SCharacterSet& charSet = (*availCharSets)[i];
                if (KCharacterSetIdentifierUtf8 == charSet.Identifier())
                    {
                    utf8CharSet->AppendL(charSet);
                    break;
                    }
                }
            TInt confidence;
            TUint charSetId;
            converter->AutoDetectCharacterSetL(confidence, 
                                               charSetId, 
                                               *utf8CharSet,
                                               *buf);
            const TInt KMaxConfidence = 100;
            if (KMaxConfidence == confidence)
                {
                ret = KMmsUtf8;
                }
            CleanupStack::PopAndDestroy(3, converter);

            }

        CleanupStack::PopAndDestroy(buf);
        }

    TInt zero = 0;
    User::LeaveIfError(aFile.Seek(ESeekStart, zero));

    return ret;
    }

// ---------------------------------------------------------------------------
// CMessageDataHandler::ConvertToUcs2FromUtfL
// ---------------------------------------------------------------------------
//
void CMessageDataHandler::ConvertToUcs2FromUtfL(RFs& aFs,
                                                 TDes& aUcs2Text,
                                                 const TDesC8& aUtfText,
                                                 TUint aMibCharset)
    {
    TPtrC8 toConvert(aUtfText);
    if (KMmsUtf8 == aMibCharset)
        {
        if (TextContainsUtf8Bom(toConvert))
            {
            toConvert.Set(toConvert.Mid(KUtf8BomLength));
            }
        }
    // prepare to convert character set to unicode
    CCnvCharacterSetConverter* converter = 
                          CCnvCharacterSetConverter::NewLC();
    // change character set ID from MIB to Symbian
    TUint charsetID = converter->ConvertMibEnumOfCharacterSetToIdentifierL(
                                     aMibCharset, aFs);
    CCnvCharacterSetConverter::TAvailability availability =
            converter->PrepareToConvertToOrFromL(charsetID, aFs);

    if (availability == CCnvCharacterSetConverter::ENotAvailable)
        {
        User::Leave(KErrNotSupported);
        }
    TInt state = CCnvCharacterSetConverter::KStateDefault;

    User::LeaveIfError(converter->ConvertToUnicode(aUcs2Text, toConvert, state));

    CleanupStack::PopAndDestroy(converter);
    }
    
	
// -----------------------------------------------------------------------------
// CMessageDataHandler::DoCancel
// -----------------------------------------------------------------------------
//   
void CMessageDataHandler::DoCancel()
	{
	}

// -----------------------------------------------------------------------------
// CMessageDataHandler::RunL
// -----------------------------------------------------------------------------
//   
void CMessageDataHandler::RunL()
	{
		OstTrace1( TRACE_NORMAL, CMESSAGEDATAHANDLER_RUNL, "CMessageDataHandler::RunL;status=%d", iStatus.Int() );
		CPIXLOGSTRING2( "CMessageDataHandler::RunL status %d",iStatus.Int() );

	// Implement round robin for the CActives
	Deque();
	CActiveScheduler::Add(this);

	// Handle next message
	if (iMessageArray.Count() > 0)
		{
		// Take first id from array
		CreateMessageIndexItemL(iMessageArray[0].iMessageId,
				iMessageArray[0].iActionType, iMessageArray[0].iFolderId);
		
		// Remove item that is handled
		iMessageArray.Remove(0);

		// Handle next
		HandleNextRequest();
		}
	}

// -----------------------------------------------------------------------------
// CMessageDataHandler::RunError
// -----------------------------------------------------------------------------
//   
TInt CMessageDataHandler::RunError(TInt aError)
	{
		OstTrace1( TRACE_NORMAL, CMESSAGEDATAHANDLER_RUNERROR, "CMessageDataHandler::RunError;Error=%d", aError );
		CPIXLOGSTRING2( "CMessageDataHandler::RunError err %d", aError );
	if (iMessageArray.Count() > 0)
		{
		// Remove item that is handled
		iMessageArray.Remove(0);

		// Handle next after the error
		HandleNextRequest();
		}
	return KErrNone;
	}

// ---------------------------------------------------------------------------
// HandleNextRequest
// ---------------------------------------------------------------------------
//
void CMessageDataHandler::HandleNextRequest()
	{
	if (!IsActive())
		{
		SetActive();
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, KErrNone);
		}
	}

// End of File

