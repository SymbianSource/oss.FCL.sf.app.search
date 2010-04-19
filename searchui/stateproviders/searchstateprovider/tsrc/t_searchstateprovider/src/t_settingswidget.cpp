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
* Description: 
*
*/
#include "t_searchstateprovider.h"
#include "settingswidget.h"
#include <qlist.h>
#include <hbcheckbox.h>
#include <hbdialog.h>
#include <qnamespace.h>
#include <qglobal.h>
#include <QtGui>
#include <hbaction.h>
#include <qdir.h>

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testsettingswizardConstruction()
    {
    // HbMainWindow* wind = mainWindow();
    SettingsWidget* settingswizard = new SettingsWidget();

    QVERIFY(settingswizard != NULL);

    delete settingswizard;
    }

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::teststoreSettingsToiniFile()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        (settingswizard->mDeviceCheckBoxList.at(i))->setCheckState(
                Qt::Unchecked);

    settingswizard->storeSettingsToiniFile();

    settingswizard->loadSettingsFrominiFile();
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Unchecked);

    delete settingswizard;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testloadSettingsFrominiFile()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        (settingswizard->mDeviceCheckBoxList.at(i))->setCheckState(
                Qt::Checked);
    settingswizard->storeSettingsToiniFile();

    settingswizard->loadSettingsFrominiFile();
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);

    delete settingswizard;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testlaunchSettingWidget()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->launchSettingWidget();
    //  QVERIFY(settingswizard->popup->isActive());

    delete settingswizard;

    }
void SearchStateProviderTest::testisInternetSearchOptionSelected()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->isInternetSelected = false;
    QCOMPARE(settingswizard->isInternetSearchOptionSelected(),false);

    delete settingswizard;

    }
void SearchStateProviderTest::testunCheckSubCategories()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->checkSubCategories(0);
    settingswizard->unCheckSubCategories(0);
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);

    delete settingswizard;

    }
void SearchStateProviderTest::testcheckSubCategories()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    settingswizard->unCheckSubCategories(0);
    settingswizard->checkSubCategories(0);
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);

    delete settingswizard;

    }
void SearchStateProviderTest::testsetActionVisibility()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->mDeviceCheckBoxList.at(2)->setChecked(true);
    settingswizard->setActionVisibility();
    QVERIFY(settingswizard->popup->primaryAction()->isVisible());
    delete settingswizard;
    }

void SearchStateProviderTest::testenableDefaultSettings()
    {
    QDir dir;
    bool ret = dir.remove("Search.ini");
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->enableDefaultSettings();
    settingswizard->loadSettingsFrominiFile();
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);
    delete settingswizard;
    }
void SearchStateProviderTest::testcheckBoxOkEvent()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    QSignalSpy spy(settingswizard, SIGNAL(settingsEvent()));
    settingswizard->checkBoxOkEvent();
    QCOMPARE(spy.count(), 1);
    delete settingswizard;

    }
void SearchStateProviderTest::testcheckBoxCancelEvent()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    QSignalSpy spy(settingswizard, SIGNAL(settingsEvent()));
    settingswizard->checkBoxCancelEvent();
    QCOMPARE(spy.count(), 1);
    delete settingswizard;

    }
void SearchStateProviderTest::testitemChecked()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->mDeviceCheckBoxList.at(0)->setChecked(2);
    settingswizard->itemChecked(0);
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);

    delete settingswizard;

    }
void SearchStateProviderTest::testchangeDeviceInternetCheck()
    {
    delete mMainWindow;
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->isInternetSelected = false;

    QSignalSpy spy(settingswizard, SIGNAL(settingsChanged()));
    settingswizard->changeDeviceInternetCheck();
    QCOMPARE(settingswizard->mInternetCheckBoxList.at(0)->checkState(), Qt::Checked);

    QCOMPARE(spy.count(),1);
    delete settingswizard;

    }

