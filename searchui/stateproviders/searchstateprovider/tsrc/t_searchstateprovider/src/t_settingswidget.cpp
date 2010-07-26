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
#include <hbmainwindow.h>
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

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        (settingswizard->mDeviceCheckBoxList.at(i))->setCheckState(
                Qt::Unchecked);

    settingswizard->storeSettingsToiniFile();

    settingswizard->loadDeviceSettings();
    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Unchecked);

    delete settingswizard;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testloadDeviceSettings()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);
    settingswizard->initialize();

    settingswizard->q_currentIndexChanged(0);

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        (settingswizard->mDeviceCheckBoxList.at(i))->setCheckState(
                Qt::Checked);
    settingswizard->storeSettingsToiniFile();

    settingswizard->loadDeviceSettings();

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);

    delete settingswizard;
    }
void SearchStateProviderTest::testloadBaseSettings()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    QCOMPARE(settingswizard->mSelectedScope,0);
    delete settingswizard;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
//
void SearchStateProviderTest::testlaunchSettingWidget()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);
    settingswizard->launchSettingWidget();

    delete settingswizard;

    }
void SearchStateProviderTest::testisInternetSearchOptionSelected()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->initialize();
    settingswizard->isInternetSelected = false;
    QCOMPARE(settingswizard->isInternetSearchOptionSelected(),false);

    delete settingswizard;

    }
void SearchStateProviderTest::testunCheckSubCategories()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->initialize();
    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);
    settingswizard->mDeviceCheckBoxList.at(0)->setCheckState(Qt::Unchecked);

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Unchecked);

    delete settingswizard;

    }
void SearchStateProviderTest::testcheckSubCategories()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->initialize();

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);
    settingswizard->mDeviceCheckBoxList.at(0)->setCheckState(Qt::Checked);

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);

    delete settingswizard;

    }

void SearchStateProviderTest::teststoreDefaultSettings()
    {
    QDir dir;
    bool ret = dir.remove(SETTINGS_INI_PATH);

    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->initialize();
    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Checked);
    delete settingswizard;
    }
void SearchStateProviderTest::testcheckBoxOkEvent()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);

    QSignalSpy spy(settingswizard, SIGNAL(settingsEvent(bool)));
    settingswizard->checkBoxOkEvent();
    QCOMPARE(spy.count(), 1);
    delete settingswizard;

    }
void SearchStateProviderTest::testcheckBoxCancelEvent()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);
    QSignalSpy spy(settingswizard, SIGNAL(settingsEvent(bool)));
    settingswizard->checkBoxCancelEvent();
    QCOMPARE(spy.count(), 1);
    delete settingswizard;

    }
void SearchStateProviderTest::testitemChecked()
    {
    SettingsWidget* settingswizard = new SettingsWidget();
    settingswizard->initialize();
    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);

    settingswizard->q_currentIndexChanged(0);

    settingswizard->mDeviceCheckBoxList.at(0)->setCheckState(Qt::Unchecked);

    for (int i = 0; i < settingswizard->mDeviceCheckBoxList.count(); i++)
        QCOMPARE(settingswizard->mDeviceCheckBoxList.at(i)->checkState(),Qt::Unchecked);

    delete settingswizard;

    }

void SearchStateProviderTest::testpreparecategories()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);
    settingswizard->preparecategories();
    QVERIFY(settingswizard->mDeviceListDisplay.count());
    delete settingswizard;

    }
void SearchStateProviderTest::testq_currentIndexChanged()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

       settingswizard->mCategoryDbMapping.insert("Contacts", true);
       settingswizard->mCategoryDbMapping.insert("Messages", true);
       settingswizard->initialize();

       settingswizard->q_currentIndexChanged(0);
       settingswizard->q_currentIndexChanged(1);
       delete settingswizard;
    }
void SearchStateProviderTest::selectedItemCategory()
    {
    SettingsWidget* settingswizard = new SettingsWidget();

    settingswizard->mCategoryDbMapping.insert("Contacts", true);
    settingswizard->mCategoryDbMapping.insert("Messages", true);
    settingswizard->q_currentIndexChanged(0);
    QSignalSpy spy(settingswizard, SIGNAL(selectedItemCategory(int,bool)));
    settingswizard->loadDeviceSettings();
    QVERIFY(spy.count());
    delete settingswizard;
    }
void SearchStateProviderTest::testcreateGui()
    {
    SettingsWidget* settingswizard = new SettingsWidget();    
    delete settingswizard;
    }

