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
 * Description:  Implementation of the settings widget
 *
 */
#include "settingswidget.h"
#include <hbdialog.h>
#include <hbcheckbox.h>
#include <qgraphicslinearlayout.h>
#include <hbaction.h>
#include <hblabel.h>
#include <qsignalmapper.h>
#include <hbtooltip.h>
#include <qsettings.h>
#include <hbinputdialog.h>
#include<hbscrollarea.h>
#include <qgraphicswidget.h>
#include <hbglobal.h>

//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::SettingsWidget()
//
//----------------------------------------------------------------------------------------------------------------------------
SettingsWidget::SettingsWidget() :
    HbWidget()
    {
    deviceCategoryList = (QStringList() << hbTrId("txt_search_list_device")
            << hbTrId("txt_search_list_contatcs") << hbTrId(
            "txt_search_list_media") << hbTrId("txt_search_list_messagemail")
            << hbTrId("txt_search_list_calendarnotes") << hbTrId(
            "txt_search_list_applications") << hbTrId(
            "txt_search_list_all_other_files"));
    internetCategoryList = (QStringList() << hbTrId(
            "txt_search_list_internet") << "Google" << "Bing");

    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout(
            Qt::Vertical);

    setLayout(linearLayout);

    popup = new HbDialog();
    popup->setDismissPolicy(HbDialog::NoDismiss);

    popup->setTimeout(HbDialog::NoTimeout);

    HbAction * action = new HbAction(hbTrId("txt_common_button_ok"), popup);
    connect(action, SIGNAL(triggered()), this, SLOT(checkBoxOkEvent()));
    popup->setPrimaryAction(action);

    action = new HbAction(hbTrId("txt_common_button_cancel"), popup);
    connect(action, SIGNAL(triggered()), this, SLOT(checkBoxCancelEvent()));
    popup->setSecondaryAction(action);

    //device category creation

    HbCheckBox *checkboxitem = NULL;

    signalMapper = new QSignalMapper(this);

    for (int i = 0; i < deviceCategoryList.count(); i++)
        {
        checkboxitem = new HbCheckBox(deviceCategoryList.at(i));

        mDeviceCheckBoxList.append(checkboxitem);

        signalMapper->setMapping(mDeviceCheckBoxList.at(i), i);

        connect(mDeviceCheckBoxList.at(i), SIGNAL(stateChanged(int)),
                signalMapper, SLOT(map()));

        if (i == 0)
            {
            //arrange device search &internet search
            linearLayout->addItem(mDeviceCheckBoxList.at(i));
            }
        else
            {
            //arrange other subcategoriess
            mDeviceCheckBoxList.at(i)->setContentsMargins(20, 0, 0, 0);
            linearLayout->addItem(mDeviceCheckBoxList.at(i));
            }
        }
    int j = 0;
    for (int i = deviceCategoryList.count(); i < internetCategoryList.count()
            + deviceCategoryList.count(); i++)
        {

        checkboxitem = new HbCheckBox(internetCategoryList.at(j));

        mInternetCheckBoxList.append(checkboxitem);

        signalMapper->setMapping(mInternetCheckBoxList.at(j), i);

        connect(mInternetCheckBoxList.at(j), SIGNAL(stateChanged(int)),
                signalMapper, SLOT(map()));

        if (i == deviceCategoryList.count())
            {
            //arrange device search &internet search
            linearLayout->addItem(mInternetCheckBoxList.at(j));
            }
        else
            {
            //arrange other subcategoriess
            mInternetCheckBoxList.at(j)->setContentsMargins(20, 0, 0, 0);
            linearLayout->addItem(mInternetCheckBoxList.at(j));

            }
        j++;
        }

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(itemChecked(int)));

    //Heading label

    HbLabel *label = new HbLabel(hbTrId(
            "txt_search_qtl_dialog_pri_heading_delimiter"));
    popup->setHeadingWidget(label);

    HbScrollArea* scrollArea = new HbScrollArea(popup);
    scrollArea->setScrollDirections(Qt::Vertical);
    scrollArea->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    scrollArea->setContentWidget(this);
    popup->setContentWidget(scrollArea);

    // loadSettingsFrominiFile();
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::~SettingsWidget()
//
//----------------------------------------------------------------------------------------------------------------------------
SettingsWidget::~SettingsWidget()
    {
    delete signalMapper;
    //delete popup;
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::launchSettingWidget()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::launchSettingWidget()
    {
    loadSettingsFrominiFile();
    setActionVisibility();
    popup->show();
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::setActionVisibility()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::setActionVisibility()
    {
    bool noItemSelected = true;
    for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
        {
        if (mDeviceCheckBoxList.at(i)->checkState() == Qt::Checked)
            {
            noItemSelected = false;
            }
        }
    for (int i = 0; i < mInternetCheckBoxList.count(); i++)
        {
        if (mInternetCheckBoxList.at(i)->checkState() == Qt::Checked)
            {
            noItemSelected = false;
            }
        }
    if (!noItemSelected)
        {
        popup->primaryAction()->setVisible(true);
        }
    else
        {
        popup->primaryAction()->setVisible(false);
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::itemChecked( int selectedIndex)
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::itemChecked(int selectedIndex)
    {
    int totalcount = deviceCategoryList.count()
            + internetCategoryList.count();

    if (selectedIndex == 0) // validating the parent category "devices"
        {
        if ((mDeviceCheckBoxList.at(selectedIndex)->checkState()
                == Qt::Checked))
            {
            isInternetSelected = false;
            checkSubCategories(0);
            unCheckSubCategories(0);
            }
        else
            {
            unCheckSubCategories(1);
            }
        setActionVisibility();
        return;
        }
    else if (selectedIndex == deviceCategoryList.count())// validating the parent category "internet"
        {
        if (mInternetCheckBoxList.at(selectedIndex
                - deviceCategoryList.count())->checkState() == Qt::Checked)
            {
            isInternetSelected = true;
            checkSubCategories(1);
            unCheckSubCategories(1);
            }
        else
            {
            unCheckSubCategories(0);
            }
        setActionVisibility();
        return;
        }
    if (selectedIndex < deviceCategoryList.count())// validating the sub categories under  "devices"
        {
        if (mDeviceCheckBoxList.at(selectedIndex)->checkState()
                == Qt::Checked)
            {
            isInternetSelected = false;
            unCheckSubCategories(0);

            }
        }
    else // validating the sub categories under  "internet"
        {
        int curentindex = (selectedIndex + internetCategoryList.count())
                - (deviceCategoryList.count() + internetCategoryList.count());

        if (mInternetCheckBoxList.at(curentindex)->checkState()
                == Qt::Checked)
            {
            isInternetSelected = true;
            unCheckSubCategories(1);
            //return;
            }
        }
    setActionVisibility();
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::check(int checkSubCategory)
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::checkSubCategories(int checkSubCategory)
    {
    if (checkSubCategory == 0)
        {
        for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
            {
            mDeviceCheckBoxList.at(i)->setCheckState(Qt::Checked);
            }
        }
    else
        {
        for (int i = 0; i < mInternetCheckBoxList.count(); i++)
            {
            mInternetCheckBoxList.at(i)->setCheckState(Qt::Checked);
            }
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::unCheckSubCategories(int unCheckSubCategory)
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::unCheckSubCategories(int unCheckSubCategory)
    {
    if (unCheckSubCategory == 0)
        {
        for (int i = 0; i < mInternetCheckBoxList.count(); i++)
            {
            mInternetCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
            }
        }
    else
        {
        for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
            {
            mDeviceCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
            }
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::checkBoxCancelEvent()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::checkBoxCancelEvent()
    {
    loadSettingsFrominiFile();//bug fix for cancel event selection should not reflect "go" button
    emit settingsEvent(false);
    }
//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::checkBoxOkEvent()
//
//----------------------------------------------------------------------------------------------------------------------------

void SettingsWidget::checkBoxOkEvent()
    {
    storeSettingsToiniFile();
    emit settingsEvent(true);
    }
//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::storeSettingsToiniFile()
//
//----------------------------------------------------------------------------------------------------------------------------

void SettingsWidget::storeSettingsToiniFile()
    {
    QSettings appSettings("Search.ini", QSettings::IniFormat);

    appSettings.setValue("DeviceCount", mDeviceCheckBoxList.count());
    for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
        {
        if (mDeviceCheckBoxList.at(i)->checkState() == Qt::Checked)
            {
            appSettings.setValue(mDeviceCheckBoxList.at(i)->text(), 1);
            emit selectedItemCategory(i, true);
            }
        else
            {
            appSettings.setValue(mDeviceCheckBoxList.at(i)->text(), 0);
            emit selectedItemCategory(i, false);
            }
        }
    // int totalcount = i;
    appSettings.setValue("InternetCount", mInternetCheckBoxList.count());
    for (int i = 0; i < mInternetCheckBoxList.count(); i++)
        {
        if (mInternetCheckBoxList.at(i)->checkState() == Qt::Checked)
            {
            appSettings.setValue(mInternetCheckBoxList.at(i)->text(), 1);
            //  emit selectedcategory(totalcount,true);
            }
        else
            {
            appSettings.setValue(mInternetCheckBoxList.at(i)->text(), 0);
            //emit selectedcategory(totalcount,false);
            }
        //    totalcount++;
        }
    }

//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::loadSettingsFrominiFile()
//
//----------------------------------------------------------------------------------------------------------------------------

void SettingsWidget::loadSettingsFrominiFile()
    {
    enableDefaultSettings();
    isInternetSelected = false;
    QSettings appSettings("Search.ini", QSettings::IniFormat);
    int ret = 0;
    for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
        {
        ret = appSettings.value(mDeviceCheckBoxList.at(i)->text()).toInt();
        if (ret)
            {
            mDeviceCheckBoxList.at(i)->setCheckState(Qt::Checked);
            emit selectedItemCategory(i, true);
            }
        else
            {
            mDeviceCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
            emit selectedItemCategory(i, false);
            }
        }
    for (int i = 0; i < mInternetCheckBoxList.count(); i++)
        {

        ret = appSettings.value(mInternetCheckBoxList.at(i)->text()).toInt();
        if (ret)
            {
            isInternetSelected = true;
            mInternetCheckBoxList.at(i)->setCheckState(Qt::Checked);
            // emit selectedcategory(mInternetCheckBoxList.at(i)->text(),true);
            }
        else
            {
            mInternetCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
            // emit selectedcategory(mInternetCheckBoxList.at(i)->text(),false);
            }
        }

    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::isInternetSearchOptionSelected()
//
//----------------------------------------------------------------------------------------------------------------------------
bool SettingsWidget::isInternetSearchOptionSelected()
    {
    return isInternetSelected;
    }
//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::changeDeviceInternetCheck()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::changeDeviceInternetCheck()
    {
    if (isInternetSearchOptionSelected() == true)
        {
        mDeviceCheckBoxList.at(0)->setCheckState(Qt::Checked);

        }
    else

        {

        mInternetCheckBoxList.at(0)->setCheckState(Qt::Checked);

        }

    emit settingsChanged();
    }
//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::enableDefaultSettings()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::enableDefaultSettings()
    {
    QSettings appSettings("Search.ini", QSettings::IniFormat);
    if (!appSettings.contains("DeviceCount")) // change the settings for the first time only
        {
        appSettings.setValue("DeviceCount", mDeviceCheckBoxList.count());
        for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
            {
            //   mDeviceCheckBoxList.at(i)->setCheckState(Qt::Checked);
            appSettings.setValue(mDeviceCheckBoxList.at(i)->text(), 1);
            }
        for (int i = 0; i < mInternetCheckBoxList.count(); i++)
            {
            //mInternetCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
            appSettings.setValue(mInternetCheckBoxList.at(i)->text(), 0);
            }
        //  storeSettingsToiniFile();
        }

    }
