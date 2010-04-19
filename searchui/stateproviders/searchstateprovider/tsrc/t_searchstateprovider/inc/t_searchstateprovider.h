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
    void testCreateInvalidState();

    /**
     * t_searchprogressivestate.cpp
     */

    void testProgressiveStateConstruction();
    void testProgressiveStateOnEntryAndExitSignalled();
    void testonSearchComplete();
    void testonAsyncSearchComplete();
    void testonGetDocumentComplete();
    void testgetSettingCategory();
    void testopenResultitem();
    void testcreateSuggestionLink();
    void testnoResultsFound();
    void testclear();
    void testsearchOnCategory();
    void teststartNewSearch();
    void testsetHistory();
    void testsetSettings();
    void testaboutToCloseView();
    void test_CustomizeGoButton();

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
    void testenableDefaultSettings();
    void teststoreSettingsToiniFile();
    void testloadSettingsFrominiFile();

    void testlaunchSettingWidget();
    void testisInternetSearchOptionSelected();
    void testunCheckSubCategories();
    void testcheckSubCategories();
    void testsetActionVisibility();

    void testcheckBoxOkEvent();
    void testcheckBoxCancelEvent();
    void testitemChecked();
    void testchangeDeviceInternetCheck();

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
    bool verifyStateCreation(const QString& library, const QString& uri);

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

