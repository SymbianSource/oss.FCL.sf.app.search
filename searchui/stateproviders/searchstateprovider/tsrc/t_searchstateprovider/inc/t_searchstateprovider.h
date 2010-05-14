/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  Tests for SearchStateProvider class.
 *
 */

#include <QtTest/QtTest>
#include "searchstateprovider.h"
#include "hsistateprovider.h"

class HbMainWindow;
class HbAbstractViewItem;

/**
 * @test Test class for stateproviders modules.
 */
class SearchStateProviderTest : public QObject
    {
Q_OBJECT

public:
    SearchStateProviderTest();
    ~SearchStateProviderTest();

private slots:
    void init();
    void initTestCase();
    void cleanupTestCase();
    /**
     * t_searchstateprovider.cpp
     */
    void testCreateAndDestructSearchStateProvider();
    void testStates();
    void testCreateProgressiveState();
    void testCreateSettingsState();
    void testCreateinitState();
    /**
     * t_searchprogressivestate.cpp
     */

    void testProgressiveStateConstruction();
    void testProgressiveStateOnEntryAndExitSignalled();
    void testgetAppIconFromAppId();
    void testsetSelectedCategories();
    void testonAsyncSearchComplete();
    void testonGetDocumentComplete();
    void testopenResultitem();
    void testgetSettingCategory();
    void testsetSettings();
    void teststartNewSearch();
    void test_customizeGoButton();
    void testsettingsaction();
    void testcancelSearch();
    void testgetrowsInserted();
    void testgetrowsRemoved();
    void testhandleOkError();
    void test_viewingCompleted();
    void testsearchOnCategory();
    void testclear();
    void testnoResultsFound();
    void testcreateSuggestionLink();
    void testTDisplayMode2Format();
    void testfromSymbianCFbsBitmap();
    void testgetDrivefromMediaId();
    void testLaunchApplicationL();
    void testfilterDoc();

    /**
     * t_searchsettingsstate.cpp
     */
    void testsettingsStateConstruction();
    void testsettingsStateOnEntryAndExitSignalled();
    void testhandleBackEvent();
    void testgetItemCategory();
    void testisInternetOn();
    /**
     * t_searchinitstate.cpp
     */
    void testinitStateConstruction();
    void testinitStateOnEntryAndExitSignalled();

    /**
     * t_settingswidget.cpp
     */

    void testsettingswizardConstruction();
    void teststoreDefaultSettings();
    void teststoreSettingsToiniFile();
    void testloadDeviceSettings();
    void testloadBaseSettings();
    void testlaunchSettingWidget();
    void testisInternetSearchOptionSelected();
    void testunCheckSubCategories();
    void testcheckSubCategories();

    void testcheckBoxOkEvent();
    void testcheckBoxCancelEvent();
    void testitemChecked();
    void testcreateGui();
    void testpreparecategories();
    void testq_currentIndexChanged();

    void selectedItemCategory();

    /* void testOnWizardAddedSignalledNoWizardInCS();
     void testProgressUpdated();
     void testWizardSelected();    
     */
private:

    /**
     * Helper method testing construction of state
     *
     * @param library Name of the library/plugin that contains the state.
     * @param uri Unique state identificator.
     *
     * @return True if state was created successfully, false otherwise.
     * @since S60 ?S60_version.
     */
    bool verifyStateCreation(const QString& uri);

    /**
     * Helper method testing existence of state
     *
     * @param states List of states.
     * @param library Name of the library/plugin that contains the state.
     * @param uri Unique state identificator.
     *
     * @return True if state was found, false otherwise.
     * @since S60 ?S60_version.
     */
    bool findStateInStateList(const QList<HsStateToken>& states,
            const QString& library, const QString& uri);

    //void setContentServiceProperty(QState* state);

    HbMainWindow* mainWindow();

private:
    //    FtuContentService* mFtuContentService;
    HbMainWindow *mMainWindow;

    };

