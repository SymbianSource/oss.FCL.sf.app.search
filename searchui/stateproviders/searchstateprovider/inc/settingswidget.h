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
#include <search_global.h>
#include <hbicon.h>
class OnlineHandler;
class HbCheckBox;
class HbDialog;
class QSignalMapper;
class HbDocumentLoader;
class HbDataFormModel;
class HbDataForm;
class HbRadioButtonList;
class HbDataFormModelItem;
class HbComboBox;

SEARCH_CLASS(SearchStateProviderTest)

class SettingsWidget : public QObject
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
    void unCheckSubCategories();
    /**
     * checking subcategories under the main category once main category checked
     */
    void checkSubCategories();
    /**
     * storing settings to application ini file
     * 
     */
    void storeSettingsToiniFile();
    /**
     * Load setting from application ini file
     * 
     */
    void loadDeviceSettings();

    /**
     * Load default settings for search categories to ini file
     * 
     */

    void loadBaseSettings();

    /**
     * make "OK" button visible 
     * 
     */
    void setActionVisibility();
    /**
     * enable default settings in the application ini file
     * 
     */
    void storeDefaultSettings();

    /**
     * setting up gui for the settings widget
     */

    void createGui();

    /**
     * Filter out categories from the sql database provided by the engine
     * for only those categories that have been successfully harvested
     */

    void preparecategories();

    /**
     * initilize the settings wizard while entering to the setting state
     */

    void initialize();

    void loadIS();

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
     * slot called while clicking items added to  the combobox
     */

    void q_currentIndexChanged(int);

    void slotproviderDetails(QString, HbIcon, int);

    void slotdefaultProvider(const int);

    void slotitemSelected(int);
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

    void ISProvidersIcon(HbIcon, int);

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
    QMap<QString, bool> mCategoryDbMapping;

    QStringList mDeviceListDisplay;
    /**
     * list of hardcoded device categories:cretaed to
     * set up default category values to the ini file
     */

    QStringList mDeviceCategoryRefList;
    /**
     * intermediate variable to store selected category values
     */

    QList<bool> mDeviceMapping;
    /**
     * list of hardcoded service providers
     */
    //QStringList internetCategoryList;
    /**
     * for unit testing
     */
    QStringList mCategoryList;
    /**
     * DocumentLoader variable for the setting widget
     */

    HbDocumentLoader* mDocumentLoader;

    /**
     * Dataform for the settingwidget
     */

    HbDataForm* dataform;

    /**
     * Dataform model
     */
    HbDataFormModel* mModel;
    /**
     * variable for customizing each item the in the dataform
     */

    QList<HbDataFormModelItem*> mModelItemList;

    QList<QAction*> mActions;

    int mSelectedScope;

    int mSelectedProvider;
    /**
     * Radio buttion list for internet search categories
     */

    HbRadioButtonList* mradiolist;

    /** 
     * combobox items
     */
    HbComboBox *comboBox;

    bool mInstialize;
    bool mchangestate;

    OnlineHandler *mInternetHandler;

    QMap<int, QString> mServiceProviders;

    SEARCH_FRIEND_CLASS (SearchStateProviderTest)
    };
#endif
