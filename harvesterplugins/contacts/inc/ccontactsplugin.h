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

#ifndef CCONTACTSPLUGIN_H
#define CCONTACTSPLUGIN_H

#include <e32base.h>
#include <cntfldst.h>
#include <cntitem.h>
#include <cntdbobs.h>
#include <cindexingplugin.h>
#include <common.h>
#include "delayedcallback.h"

class CSearchDocument;
class CContactItemFieldSet;
class CCPixIndexer;

/** Field names */
_LIT(KContactsGivenNameField, "GivenName");
_LIT(KContactsFamilyNameField, "FamilyName");
_LIT(KContactsCompanyNameField, "CompanyName");
_LIT(KContactsPhoneNumberField, "PhoneNumber");
_LIT(KContactsAddressField, "Address");
_LIT(KContactsNoteField, "Note");
_LIT(KContactsJobTitleField, "JobTitle");
_LIT(KContactsSecondNameField, "SecondName");

_LIT(KContactsSuffixField, "Suffix");
_LIT(KContactsEMailField, "EMail");
_LIT(KContactsUrlField, "URL");

_LIT(KContactsPostOfficeField, "PostOffice");
_LIT(KContactsExtendedAddressField, "ExtendedAddress");
_LIT(KContactsLocalityField, "Locality");
_LIT(KContactsRegionField, "Region");
_LIT(KContactsPostcodeField, "PostCode");
_LIT(KContactsCountryField, "Country");
_LIT(KContactsSIPIDField, "SIPID");
_LIT(KContactsSpouseField, "Spouse");
_LIT(KContactsChildrenField, "Children");
_LIT(KContactsClassField, "Class");
_LIT(KContactsPrefixField, "Prefix");
_LIT(KContactsAdditionalNameField, "AdditionalName");
_LIT(KContactsFaxField, "Fax");
_LIT(KContactsGivenNamePronunciationField, "GivenNamePronunciation");
_LIT(KContactsFamilyNamePronunciationField, "FamilyNamePronunciation");
_LIT(KContactsCompanyNamePronunciationField, "CompanyNamePronunciation");


class CContactsPlugin : public CIndexingPlugin, public MContactDbObserver, public MDelayedCallbackObserver
{
public:
	static CContactsPlugin* NewL();
	static CContactsPlugin* NewLC();
	virtual ~CContactsPlugin();

	/**
	 * From CIndexingPlugin
	 */
	void StartPluginL();
	void StartHarvestingL(const TDesC& aQualifiedBaseAppClass);

	/**
	 * From MContactDbObserver, HandleDatabaseEventL.
	 */
	void HandleDatabaseEventL(TContactDbObserverEvent aEvent);

    /**
     * From MDelayedCallbackObserver, DelayedCallbackL
     */
    void DelayedCallbackL(TInt aCode);
    void DelayedError(TInt aError);

protected:
	CContactsPlugin();
	void ConstructL();

	/**
	 *  Adds information field (if available)
	 */
	void AddFieldL(CSearchDocument& aDocument, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& aFieldName );

	/**
	 *  Adds to excerpt
	 */
	void AddToExcerptL(CSearchDocument& aDocument, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& aFieldName );

	/**
	 *  Helper function: adds information field to the document and to the excerpt field(if available)
	 */
	void AddFieldToDocumentAndExcerptL(CSearchDocument& aDocument, CContactItemFieldSet& aFieldSet, TUid aFieldId, const TDesC& aFieldName );
	
	/**
	 * Creates the actual contact book index item
	 */
	void CreateContactIndexItemL(TInt aContentId, TCPixActionType aActionType);

private:
	
	/** Contact change notifier */
	CContactChangeNotifier* iChangeNotifier;
	/** Sorted contact id array */
	const CContactIdArray* iContacts;
	/** Contacts database. */
	CContactDatabase* iDatabase;
	/** The asynchronizer */
	CDelayedCallback* iAsynchronizer;
	/** Current harvested contact index */
	TInt iCurrentIndex;	
	/** placeholder for Excerpt text dynamic creation */
	HBufC* iExcerpt;
	
	// CPix database 
    CCPixIndexer* iIndexer;

#ifdef __PERFORMANCE_DATA
    TTime iStartTime;
    TTime iCompleteTime;
    void UpdatePerformaceDataL();
    void UpdatePerformaceDataL(TCPixActionType);
#endif	
};

#endif // CCONTACTSPLUGIN_H
