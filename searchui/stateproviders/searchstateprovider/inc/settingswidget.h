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
 * Description:  Declaration of the settings widget
 *
 */
#ifndef SETTINGSWIDGET_H_
#define SETTINGSWIDGET_H_

#include <hbwidget.h>
#include <qlist.h>
#include <hbgridview.h>
#include <Search_global.h>
class HbCheckBox;
class HbDialog;
class QSignalMapper;

SEARCH_CLASS(SearchStateProviderTest)

class SettingsWidget : public HbWidget
    {
Q_OBJECT
public:
    /**
     * Construction
     */
    SettingsWidget();
    /**
     * destructor
     */
    ~SettingsWidget();
public:
    /**
     * Launching the settings widget
     */
    void launchSettingWidget();
    /**
     * Verifies internet selection
     * 
     */
    bool isInternetSearchOptionSelected();
    /**
     * unchecking subcategories under the main category once main category unchecked
     */
    void unCheckSubCategories(int aUnCheckSubCategory);
    /**
     * checking subcategories under the main category once main category checked
     */
    void checkSubCategories(int acheckSubCategory);
    /**
     * storing settings to application ini file
     * 
     */
    void storeSettingsToiniFile();
    /**
     * Load setting from application ini file
     * 
     */
    void loadSettingsFrominiFile();
    /**
     * make "OK" button visible 
     * 
     */
    void setActionVisibility();
    /**
     * enable default settings in the application ini file
     * 
     */
    void enableDefaultSettings();
public slots:
    /**
     * will be called when settings OK is clicked
     */
    void checkBoxOkEvent();
    /**
     * will be called when setting cancel is called
     */
    void checkBoxCancelEvent();
    /**
     * will be called whenn any check box is checked
     */
    void itemChecked(int);
    /**
     * will be called when change oin internet or device selectin
     * 
     */
    void changeDeviceInternetCheck();
signals:
    /**
     * Emitted when setting closed
     * 
     */
    void settingsEvent(bool);
    /**
     * Emitted if settings changed for "go" button
     * 
     */
    void settingsChanged();
    /**
     * Emitted when categories selected or deselected
     * 
     */
    void selectedItemCategory(int, bool);

private:
    /**
     * for Device category list
     */
    QList<HbCheckBox*> mDeviceCheckBoxList;
    /**
     * for Internet category list
     */
    QList<HbCheckBox*> mInternetCheckBoxList;
    /**
     * mapper to map checkboxes
     * 
     */
    QSignalMapper* signalMapper;
    /**
     * settings widget popup
     */
    HbDialog *popup;
    /**
     * to indicate internet is selected or not
     */
    bool isInternetSelected;

    /**
     * list of hardcoded device categories
     */
    QStringList deviceCategoryList;

    /**
     * list of hardcoded service providers
     */
    QStringList internetCategoryList;
    /**
     * for unit testing
     */
SEARCH_FRIEND_CLASS    (SearchStateProviderTest)
    };
#endif
