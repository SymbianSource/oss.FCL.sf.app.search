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
#include <nmapiemailaddress.h>
#include <nmapimessagebody.h>
#include <nmapimailbox.h>
#include <QTimer>
#include <QDebug>
//#include <QThread> //If we happen to use QThread::yieldCurrentThread()

//Symbian specific details; picked up from cemailplugin.cpp. 
//Refactor it to cpixmaindefs.h
_LIT(KMsgBaseAppClassGeneric, "root msg email");

_LIT(KMsgSubject, "Subject");
_LIT(KMsgRecipients, "Recipients");
_LIT(KMsgBody, "Body");
_LIT(KMailBoxId, "MailBoxId");
_LIT(KFolderId, "FolderId");
_LIT(KMailBoxName, "MailBoxName");
_LIT(KFolderName, "FolderName");
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
    qDebug() << "QEmailFetcher::QEmailFetcher";
    }

//------------------------------------------------------------------------------
QEmailFetcher::~QEmailFetcher()
    {
    iEmailEventNotifier->cancel();
    delete iEmailEventNotifier;
    delete iEmailService;
    delete iMailBoxListings;
    delete iMailFolderList;
    }

//------------------------------------------------------------------------------
QEmailFetcher* QEmailFetcher::newInstance( MEmailItemObserver& aObserver ){
    QEmailFetcher* emailFetcher = NULL;
    qDebug() << "QEmailFetcher::newInstance :START";
    //Leak free init.
    try{
        emailFetcher = new QEmailFetcher( aObserver );
        emailFetcher->iEmailService = new NmApiEmailService( emailFetcher );
        emailFetcher->iEmailEventNotifier =  new NmApiEventNotifier( emailFetcher );
        emailFetcher->iMailBoxListings = new NmApiMailboxListing( emailFetcher );
        initialize( emailFetcher ); //Do the rest of the init.
    }catch(...){ //cleanup.
    qDebug() << "QEmailFetcher::newInstance ( Catch Block)";
        delete emailFetcher; 
        delete emailFetcher->iEmailService;
        delete emailFetcher->iEmailEventNotifier;
        delete emailFetcher->iMailBoxListings;
        emailFetcher->iEmailService = NULL;
        emailFetcher->iEmailEventNotifier = NULL;
        emailFetcher->iMailBoxListings = NULL;
        throw; //rethrow the exception to caller.
    }    
    qDebug() << "QEmailFetcher::newInstance :END";
    return emailFetcher; //returns only if not null.
}

//------------------------------------------------------------------------------
void QEmailFetcher::initialize( QEmailFetcher* aThis ){
    //The use of 'aThis' is because the current function is static.
    qDebug() << "QEmailFetcher::initialize :START";
    connect( aThis->iEmailService, SIGNAL(initialized(bool)), 
             aThis, SLOT(emailServiceIntialized(bool)) );
    aThis->iEmailService->initialise();
    //Monitor for Message changes
    aThis->connect( aThis->iEmailEventNotifier, 
                    SIGNAL(messageEvent(MessageEvent, quint64, quint64, QList<quint64>)),
                    aThis, 
                    SLOT(handleMessageEvent(MessageEvent, quint64, quint64, QList<quint64>)) );
    //Monitor for Mailbox changes
    aThis->connect( aThis->iEmailEventNotifier, 
                    SIGNAL(mailboxEvent(MailboxEvent, QList<quint64>)),
                    aThis, 
                    SLOT(handlemailboxEvent(MailboxEvent, QList<quint64>)));
    //Start the monitoring
    aThis->iEmailEventNotifier->start();
    qDebug() << "QEmailFetcher::initialize :END";
}

//------------------------------------------------------------------------------
void QEmailFetcher::emailServiceIntialized(bool aAllOk){
    qDebug() << "QEmailFetcher::emailServiceIntialized :START  aAllOk = " << aAllOk;
    if( aAllOk ){
        connect( iMailBoxListings, SIGNAL(mailboxesListed(qint32)), this, SLOT(handleMailboxesListed(qint32)) );
    }
    qDebug() << "QEmailFetcher::emailServiceIntialized :END";
}

//------------------------------------------------------------------------------
void QEmailFetcher::StartHarvesting(){
    qDebug() << "QEmailFetcher::StartHarvesting :START";
    bool ret = iMailBoxListings->start();
    qDebug() << "QEmailFetcher::StartHarvesting :END return = " << ret;
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

CSearchDocument* getPartialSearchDocument( const NmApiMessageEnvelope& aEnvelope ) {
    qDebug() << "getPartialSearchDocument :START";
    CSearchDocument* doc = 0;
    QT_TRAP_THROWING(
        //Use qt_Qstring2TPtrC since we are working with <b>const</b> EmailMessageEnvelope.
        doc = CSearchDocument::NewL( qt_QString2TPtrC( QString().setNum( aEnvelope.id() ) ), 
                                     KMsgBaseAppClassGeneric );        
        );
    qDebug() << "getPartialSearchDocument :END";
    return doc;
    }
} //anonymous namespace
//------------------------------------------------------------------------------

CSearchDocument* QEmailFetcher::getSearchDocument( const NmApiMessageEnvelope& aEnvelope ,quint64 aMailboxId, quint64 aFolderId ){
    QList<NmApiEmailAddress> toList;
    qDebug() << "QEmailFetcher::getSearchDocument :START";
    //Need to cast away const-ness since the get method is unfortunately not const.
    const_cast<NmApiMessageEnvelope&>(aEnvelope).getToRecipients( toList );

    //We need ALL the recipients in a SINGLE field.
    QString recipients = "";
    for( int i=0; i<toList.length(); i++ )
        recipients += toList.at( i ).displayName() + " "; //or should we get address?

    NmApiMessageBody body;
    //Cast away const-ness since the get method is unfortunately not const.
    //Returns void. Cannot check for success/failure.
    const_cast<NmApiMessageEnvelope&>(aEnvelope).getPlainTextBody( body ); 
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
    doc->AddFieldL( KMailBoxId, qt_QString2TPtrC( QString().setNum( aMailboxId ) ), CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
    doc->AddFieldL( KFolderId, qt_QString2TPtrC( QString().setNum( aFolderId ) ), CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
    
//    NmApiMailbox aMailBox;
//    iEmailService->getMailbox( aMailboxId, aMailBox );
//    doc->AddFieldL( KMailBoxName, qt_QString2TPtrC(aMailBox.name()) , CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
    //TODO : Add folder name field
    //_LIT(KFolderName, "FolderName");
    //Adding subject and body to the excerpt. 
    QString excerpt ;
    excerpt = aEnvelope.subject();
    excerpt += msgBody ;
    doc->AddExcerptL( qt_QString2TPtrC(excerpt) );
    );
    qDebug() << "QEmailFetcher::getSearchDocument :END";
    return doc;
}


//------------------------------------------------------------------------------
//Just to avoid duplication of the following two lines.
void QEmailFetcher::NotifyHarvestingComplete(){
    qDebug() << "QEmailFetcher::NotifyHarvestingComplete :START";
    iCurrentMailboxIndex = iCurrentFolderIndex = 0;
    QT_TRAP_THROWING( iEmailObserver.HarvestingCompleted() );
    qDebug() << "QEmailFetcher::NotifyHarvestingComplete :END";
    return;
}

//------------------------------------------------------------------------------
void QEmailFetcher::handleMailboxesListed(qint32 aCount){
    qDebug() << "QEmailFetcher::handleMailboxesListed :START";
    iCurrentMailboxIndex = 0;
    if( aCount == NmApiMailboxListing::MailboxListingFailed ) {
        NotifyHarvestingComplete();
        qDebug() << "QEmailFetcher::handleMailboxesListed :MailboxListingFailed";
        return;
    }
    if( aCount>0 && iMailBoxListings->getMailboxes( iMailBoxes ) ){
        //Already set to NULL in constructor, so safe to call delete first time.
        qDebug() << "QEmailFetcher::handleMailboxesListed :processNextMailbox";
        processNextMailbox();
    }else{
        qDebug() << "QEmailFetcher::handleMailboxesListed :Harvesting Completed";
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
    qDebug() << "QEmailFetcher::processNextMailbox :START";
    //No more mailboxes, notify completion.
    if( iCurrentMailboxIndex >= iMailBoxes.count() ) {
        NotifyHarvestingComplete();
        qDebug() << "QEmailFetcher::processNextMailbox :END (harvesting completed)";
        return;
    }
    
    //More mailboxes available.
    delete iMailFolderList; iMailFolderList = NULL;
    iMailFolderList = new NmApiFolderListing( this, iMailBoxes.at( iCurrentMailboxIndex++ ).id() );
    connect( iMailFolderList, SIGNAL(foldersListed( qint32 )), this, SLOT(handleMailFoldersListed( qint32)) );
    const int waitForSeconds = 30; //TODO Move this constant out of here if needed elsewhere
    QTimer::singleShot( waitForSeconds, iMailFolderList, SLOT( start()) );
    qDebug() << "QEmailFetcher::processNextMailbox :END (goto next mailbox)";
}

//------------------------------------------------------------------------------
void QEmailFetcher::handleMailFoldersListed(int aCount){
    qDebug() << "QEmailFetcher::handleMailFoldersListed :START";
    iCurrentFolderIndex = 0;    
    if( aCount == NmApiFolderListing::FolderListingFailed ){
        qDebug() << "QEmailFetcher::handleMailFoldersListed :FolderListingFailed";
        processNextMailbox();
        return;//Don't proceed futher.
    }
    if( aCount && iMailFolderList->getFolders( iFolders ) ){ 
       qDebug() << "QEmailFetcher::handleMailFoldersListed :processNextFolder";
        processNextFolder();
    }else{
        qDebug() << "QEmailFetcher::handleMailFoldersListed :processNextMailbox";
        processNextMailbox();
        return;
    }
}

//------------------------------------------------------------------------------
void QEmailFetcher::processNextFolder(){
    qDebug() << "QEmailFetcher::processNextFolder :START";
    //No more folders in current mailbox.
    if( iCurrentFolderIndex >= iFolders.count() ) {
        qDebug() << "QEmailFetcher::processNextFolder :processNextMailbox";
        processNextMailbox();
        return;//Don't proceed futher.
    }
    
    //More folders to process.
    //Already set to NULL in constructor, so safe to call delete first time.
    delete iEnvelopeListing; iEnvelopeListing = NULL; 
    iEnvelopeListing= new NmApiEnvelopeListing( 
            this, 
            iFolders.at( iCurrentFolderIndex++ ).id(),
            iMailBoxes.at( iCurrentMailboxIndex-1 ).id() ); //we have already incremented iMailboxIndex.

    connect(iEnvelopeListing, SIGNAL(envelopesListed(qint32)),this,SLOT(processMessages(qint32)));
    iEnvelopeListing->start();
    qDebug() << "QEmailFetcher::processNextFolder :processNextFolder";
}

//------------------------------------------------------------------------------
void QEmailFetcher::processMessages(qint32 aCount){
    qDebug() << "QEmailFetcher::processMessages :START";
    if( aCount == NmApiEnvelopeListing::EnvelopeListingFailed ) {
        qDebug() << "QEmailFetcher::processMessages :EnvelopeListingFailed";
        processNextFolder();
        return;//Don't proceed futher.
    }
    QList<NmApiMessageEnvelope> envelopes;
    if ( aCount > 0 && iEnvelopeListing->getEnvelopes(envelopes) ) {
        for( int i=0; i<envelopes.count(); i++ ) {
            const NmApiMessageEnvelope &envelope = envelopes.at( i );
            //Create document and call back observer.
            QT_TRAP_THROWING( iEmailObserver.HandleDocumentL( getSearchDocument( envelope, iFolders.at( iCurrentFolderIndex -1 ).id(), iMailBoxes.at( iCurrentMailboxIndex-1 ).id() ), 
                                                              ECPixAddAction ) );
        }
    }
    qDebug() << "QEmailFetcher::processMessages :END";
}

//------------------------------------------------------------------------------
void QEmailFetcher::handleMessageEvent( NmApiMessageEvent aEvent, quint64 aMailboxId, quint64 aFolderId, QList<quint64> aMessageList){
    NmApiMessageEnvelope envelope;
    qDebug() << "QEmailFetcher::handleMessageEvent :START";
    const int messageCount = aMessageList.count();
    if( messageCount>0 ){
    if( aEvent == MessageCreated || aEvent == MessageChanged ){
        qDebug() << "QEmailFetcher::handleMessageEvent :MessageCreated || MessageChanged";
        for( int i=0; i<messageCount; i++ ){
            if( iEmailService->getEnvelope( aMailboxId, aFolderId, aMessageList.at( i ), envelope ) ){
               qDebug() << "QEmailFetcher::handleMessageEvent :HandleDocumentL";
                QT_TRAP_THROWING( 
                   iEmailObserver.HandleDocumentL( getSearchDocument( envelope, aMailboxId, aFolderId ), 
                           //Doing this simply avoids *duplicate* code for update action.
                           aEvent == MessageCreated ? ECPixAddAction : ECPixUpdateAction ) );
            }
        }
    }
    else if( aEvent == MessageDeleted ) {
        qDebug() << "QEmailFetcher::handleMessageEvent :MessageDeleted";
        //TODO We can do better. For delete, we dont have to create full document. Just the ID should be enough.
        //We can have another function called getPartialSearchDocument so deletes will be faster.
        for( int i=0; i<messageCount; i++ ){
            if( iEmailService->getEnvelope( aMailboxId, aFolderId, aMessageList.at( i ), envelope ) ){
                qDebug() << "QEmailFetcher::handleMessageEvent :MessageDeleted : HandleDocumentL";
                QT_TRAP_THROWING( 
                iEmailObserver.HandleDocumentL( getPartialSearchDocument( envelope ), ECPixRemoveAction ) );
            }
        }
    }
    }
}

//--------------------------------------------------------------------------------------
void QEmailFetcher::handlemailboxEvent( EmailClientApi::NmApiMailboxEvent event, QList<quint64> idlist ){
    const int mailboxCount = idlist.count();    
    if( event == MailboxCreated )
        {
        //New mailbox is created.Harvest the Mailbox name and all the folder names
        }
    else
        {
        //MailBox is deleted so delete the document related to all the mailbox and folders in the mailbox
        }
}

CSearchDocument* QEmailFetcher::getMailboxorfolderSearchDocument( quint64 aMailboxId, quint64 aFolderId, TEmailDocType aDocType, QString aFoldername ){
    
    CSearchDocument* doc = 0;
    QT_TRAP_THROWING(
    //Use qt_Qstring2TPtrC since we are working with <b>const</b> EmailMessageEnvelope.
            if ( aDocType == EEmailTypeFolder)
                {
                doc = CSearchDocument::NewL( qt_QString2TPtrC( QString().setNum( aFolderId ) ), 
                                 KMsgBaseAppClassGeneric );
                doc->AddFieldL( KFolderName, qt_QString2TPtrC(aFoldername) , CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
                doc->AddFieldL( KFolderId, qt_QString2TPtrC( QString().setNum( aFolderId ) ), CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
                }
            else if ( aDocType == EEmailTypeMailBox )
                {
                doc = CSearchDocument::NewL( qt_QString2TPtrC( QString().setNum( aMailboxId ) ), 
                                             KMsgBaseAppClassGeneric );
                NmApiMailbox aMailBox;
                iEmailService->getMailbox( aMailboxId, aMailBox );
                doc->AddFieldL( KMailBoxName, qt_QString2TPtrC(aMailBox.name()) , CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );
                }
            doc->AddFieldL( KMimeTypeField, KMimeTypeMsg, CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized);
            doc->AddFieldL( KMailBoxId, qt_QString2TPtrC( QString().setNum( aMailboxId ) ), CDocumentField::EStoreYes | CDocumentField::EIndexUnTokenized );

    );
    qDebug() << "QEmailFetcher::getSearchDocument :END";
    return doc;
}
