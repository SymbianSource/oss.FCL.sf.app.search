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
* Description: Utility class for fetching email.
*
*/

#include "qtemailfetcher.h"
#include <csearchdocument.h>
#include <cpixmaindefs.h>
//#include <QThread> //If we happen to use QThread::yieldCurrentThread()

//Symbian specific details; picked up from cemailplugin.cpp. 
//Refactor it to cpixmaindefs.h
_LIT(KMsgBaseAppClassGeneric, "root msg email");

_LIT(KMsgSubject, "Subject");
_LIT(KMsgRecipients, "Recipients");
_LIT(KMsgBody, "Body");
_LIT(KMimeTypeField, CPIX_MIMETYPE_FIELD);
_LIT(KMimeTypeMsg, "Messages");

//------------------------------------------------------------------------------
QEmailFetcher::QEmailFetcher( MEmailItemObserver& aObserver )
    :iEmailObserver( aObserver ), 
     iEmailEventNotifier( NULL ),
     iEmailService( NULL ),
     iMailBoxListings( NULL ), 
     iMailFolderList( NULL ),
     iEnvelopeListing( NULL ),
     iCurrentMailboxIndex( 0 ), 
     iCurrentFolderIndex( 0 )
    {
    }

//------------------------------------------------------------------------------
QEmailFetcher::~QEmailFetcher()
    {
    delete iEmailEventNotifier;
    delete iEmailService;
    delete iMailBoxListings;
    delete iMailFolderList;
    }

//------------------------------------------------------------------------------
QEmailFetcher* QEmailFetcher::newInstance( MEmailItemObserver& aObserver ){
    QEmailFetcher* emailFetcher = NULL;
    
    //Leak free init.
    try{
        QEmailFetcher* emailFetcher = new QEmailFetcher( aObserver );
        emailFetcher->iEmailService = new NmEmailService( emailFetcher );
        emailFetcher->iEmailEventNotifier =  new NmEventNotifier( emailFetcher );
        emailFetcher->iMailBoxListings = new NmMailboxListing( emailFetcher );
    }catch(...){ //cleanup.
        delete emailFetcher; 
        delete emailFetcher->iEmailService;
        delete emailFetcher->iEmailEventNotifier;
        delete emailFetcher->iMailBoxListings;
        emailFetcher->iEmailService = NULL;
        emailFetcher->iEmailEventNotifier = NULL;
        emailFetcher->iMailBoxListings = NULL;
        throw; //rethrow the exception to caller.
    }
    initialize( emailFetcher ); //Do the rest of the init.
    return emailFetcher; //returns only if not null.
}

//------------------------------------------------------------------------------
void QEmailFetcher::initialize( QEmailFetcher* aThis ){
    //The use of 'aThis' is because the current function is static.
    connect( aThis->iEmailService, SIGNAL(initialized(bool)), 
             aThis, SLOT(emailServiceIntialized(bool)) );
    aThis->iEmailService->initialise();
    aThis->connect( aThis->iEmailEventNotifier, 
                    SIGNAL(messageEvent(MessageEvent, quint64, quint64, QList<quint64>)),
                    aThis, 
                    SLOT(handleMessageEvent(MessageEvent, quint64, quint64, QList<quint64>)) );
}

//------------------------------------------------------------------------------
void QEmailFetcher::emailServiceIntialized(bool aAllOk){
    if( aAllOk ){
        connect( iMailBoxListings, SIGNAL(mailboxesListed(int)), this, SLOT(handleMailboxesListed(int)) );
    }
}

//------------------------------------------------------------------------------
void QEmailFetcher::StartHarvesting(){
    iMailBoxListings->start();
}

//------------------------------------------------------------------------------
namespace {

// Taken from qt/src/corelib/kernel/qcore_symbian_p.cpp, as recomended in
// http://qt.nokia.com/files/pdf/whitepaper-using-qt-and-symbian-c-together, page 34.
// URL last accessed on April 6th, 2010.

// Returned TPtrC is valid as long as the given parameter is valid and unmodified
static inline TPtrC qt_QString2TPtrC( const QString& string )
{
    return TPtrC16(static_cast<const TUint16*>(string.utf16()), string.length());
}

//------------------------------------------------------------------------------
// TODO Remove this code if qt_QString2TPtrC works.
// TODO If this function is used, remember to release memory.
// Ownership with caller.
//HBufC* qt_QString2HBufC(const QString& aString)
//{
//    HBufC *buffer;
//#ifdef QT_NO_UNICODE
//    TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString.toLocal8Bit().constData()));
//#else
//    TPtrC16 ptr(qt_QString2TPtrC(aString));
//#endif
//    buffer = q_check_ptr(HBufC::New(ptr.Length()));
//    buffer->Des().Copy(ptr);
//    return buffer;
//}

//------------------------------------------------------------------------------
//Private free function creates CSearchDocument from EMailMessageEnvelope.
CSearchDocument* getSearchDocument( const NmMessageEnvelope& aEnvelope ){
    QList<NmEmailAddress> toList;
    //Need to cast away const-ness since the get method is unfortunately not const.
    const_cast<NmMessageEnvelope&>(aEnvelope).getToRecipients( toList );

    //We need ALL the recipients in a SINGLE field.
    QString recipients = "";
    for( int i=0; i<toList.length(); i++ )
        recipients += toList.at( i ).displayName() + " "; //or should we get address?

    NmMessageBody body;
    //Cast away const-ness since the get method is unfortunately not const.
    //Returns void. Cannot check for success/failure.
    const_cast<NmMessageEnvelope&>(aEnvelope).getPlainTextBody( body ); 
    QString msgBody = body.content();

    CSearchDocument* doc = 0;
    QT_TRAP_THROWING(
    //Use qt_Qstring2TPtrC since we are working with <b>const</b> EmailMessageEnvelope.
    doc = CSearchDocument::NewL( qt_QString2TPtrC( QString().setNum( aEnvelope.id() ) ), 
                                 KMsgBaseAppClassGeneric );
    doc->AddFieldL( KMimeTypeField, KMimeTypeMsg, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
    doc->AddFieldL( KMsgSubject, qt_QString2TPtrC( aEnvelope.subject() ), CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
    doc->AddFieldL( KMsgRecipients, qt_QString2TPtrC( recipients ), CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
    doc->AddFieldL( KMsgBody, qt_QString2TPtrC( msgBody ), CDocumentField::EStoreYes | CDocumentField::EIndexTokenized );
    //TODO: What should go in here?
    doc->AddExcerptL( KNullDesC );
    );
    return doc;
}
} //anonymous namespace

//------------------------------------------------------------------------------
//Just to avoid duplication of the following two lines.
void QEmailFetcher::NotifyHarvestingComplete(){
    iCurrentMailboxIndex = iCurrentFolderIndex = 0;
    QT_TRAP_THROWING( iEmailObserver.HarvestingCompleted() );
    return;
}

//------------------------------------------------------------------------------
void QEmailFetcher::handleMailboxesListed(int aCount){
    iCurrentMailboxIndex = 0;
    if( aCount == NmMailboxListing::MailboxListingFailed ) {
        NotifyHarvestingComplete();
        return;
    }
    if( aCount>0 && iMailBoxListings->getMailboxes( iMailBoxes ) ){
        //Already set to NULL in constructor, so safe to call delete first time.
        processNextMailbox();
    }else{
        NotifyHarvestingComplete();
        return;
    }
}

//------------------------------------------------------------------------------
//Options to make async (like other plugins' Asynchronizer):
//1. Use http://doc.trolltech.com/4.6/qtimer.html and connect timeout() signal to something?
//Downside: 
//Have to save the state of the function and resume. Achievable via static members. 
//Remeber to reset counters.
//2. Use timer; unlike above, have handleMailboxesListed() simply trigger a  
//Timer controlled function.
//3. Use QThread::currentThread()->yieldCurrentThread();
//Downside: Not tested.
//4. As recommended by the email API documentation, use SingleShotTimer:
//QTimer::singleShot(nsecs,nmFolderListing,SLOT(start());
//
//Recommendation: Use option 4.

void QEmailFetcher::processNextMailbox(){
    //No more mailboxes, notify completion.
    if( iCurrentMailboxIndex >= iMailBoxes.count() ) {
        NotifyHarvestingComplete();
        return;
    }
    
    //More mailboxes available.
    delete iMailFolderList; iMailFolderList = NULL;
    iMailFolderList = new NmFolderListing( this, iMailBoxes.at( iCurrentMailboxIndex++ ).id() );
    connect( iMailFolderList, SIGNAL(foldersListed()), this, SLOT(handleMailFoldersListed()) );
    const int waitForSeconds = 30; //TODO Move this constant out of here if needed elsewhere
    QTimer::singleShot( waitForSeconds, iMailFolderList, SLOT( start()) );
}

//------------------------------------------------------------------------------
void QEmailFetcher::handleMailFoldersListed(int aCount){
    iCurrentFolderIndex = 0;
    if( aCount == NmFolderListing::FolderListingFailed ){
        processNextMailbox();
        return;//Don't proceed futher.
    }
    if( aCount && iMailFolderList->getFolders( iFolders ) ){ 
        processNextFolder();
    }else{
        processNextMailbox();
        return;
    }
}

//------------------------------------------------------------------------------
void QEmailFetcher::processNextFolder(){
    //No more folders in current mailbox.
    if( iCurrentFolderIndex >= iFolders.count() ) {
        processNextMailbox();
        return;//Don't proceed futher.
    }
    
    //More folders to process.
    //Already set to NULL in constructor, so safe to call delete first time.
    delete iEnvelopeListing; iEnvelopeListing = NULL; 
    iEnvelopeListing= new NmEnvelopeListing( 
            this, 
            iFolders.at( iCurrentFolderIndex++ ).id(),
            iMailBoxes.at( iCurrentMailboxIndex-1 ).id() ); //we have already incremented iMailboxIndex.

    connect(iEnvelopeListing, SIGNAL(envelopesListed(int)),this,SLOT(processMessages(int)));
    iEnvelopeListing->start();
}

//------------------------------------------------------------------------------
void QEmailFetcher::processMessages(int aCount){
    if( aCount == NmEnvelopeListing::EnvelopeListingFailed ) {
        processNextFolder();
        return;//Don't proceed futher.
    }
    QList<NmMessageEnvelope> envelopes;
    if ( aCount > 0 && iEnvelopeListing->getEnvelopes(envelopes) ) {
        for( int i=0; i<envelopes.count(); i++ ) {
            const NmMessageEnvelope &envelope = envelopes.at( i );
            //Create document and call back observer.
            QT_TRAP_THROWING( iEmailObserver.HandleDocumentL( getSearchDocument( envelope ), ECPixAddAction ) );
        }
    }
}

//------------------------------------------------------------------------------
void QEmailFetcher::handleMessageEvent( const MessageEvent aEvent, quint64 aMailboxId, quint64 aFolderId, QList<quint64> aMessageList){
    NmMessageEnvelope envelope;
    const int messageCount = aMessageList.count();
    if( messageCount>0 ){
    if( aEvent == MessageCreated || aEvent == MessageChanged ){
        for( int i=0; i<messageCount; i++ ){
            if( iEmailService->getEnvelope( aMailboxId, aFolderId, aMessageList.at( i ), envelope ) ){
                QT_TRAP_THROWING( 
                   iEmailObserver.HandleDocumentL( getSearchDocument( envelope ), 
                           //Doing this simply avoids *duplicate* code for update action.
                           aEvent == MessageCreated ? ECPixAddAction : ECPixUpdateAction ) );
            }
        }
    }
    else if( aEvent == MessageDeleted ) {
        //TODO We can do better. For delete, we dont have to create full document. Just the ID should be enough.
        //We can have another function called getPartialSearchDocument so deletes will be faster.
        for( int i=0; i<messageCount; i++ ){
            if( iEmailService->getEnvelope( aMailboxId, aFolderId, aMessageList.at( i ), envelope ) ){
                QT_TRAP_THROWING( 
                iEmailObserver.HandleDocumentL( getSearchDocument( envelope ), ECPixRemoveAction ) );
            }
        }
    }
    }
}
