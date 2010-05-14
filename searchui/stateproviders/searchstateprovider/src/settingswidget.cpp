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
#include <hbscrollarea.h>
#include <qgraphicswidget.h>
#include <hbglobal.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdocumentloader.h>
#include <hbcombobox.h>
#include <hbdataformviewitem.h>
#include <hbradiobuttonlist.h>
#include <qdir.h>
#include <qsql.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <cpixcontentinfocommon.h>
const char *DELIMETER_DOCML = ":/xml/delimeterscreen.docml";
const char *DIALOG = "dialog";
const char *DATAFORM = "dataForm";

//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::SettingsWidget()
//
//----------------------------------------------------------------------------------------------------------------------------
SettingsWidget::SettingsWidget() :
    signalMapper(NULL), popup(NULL), mDocumentLoader(NULL), dataform(NULL),
            mModel(NULL),mSelectedScope(0),mSelectedProvider(0),comboBox(NULL)
    {
    for (int i = 0; i < 8; i++)
        {
        mDeviceMapping.append(false);
        }

    mDeviceCategoryRefList = (QStringList() << "first" << "second" << "third"
            << "fourth" << "fifth" << "sixth" << "seventh" << "eigth");

    mchangestate = true;
    mInstialize = true;
    isInternetSelected = false;
    storeDefaultSettings();
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::intialize()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::initialize()
    {
    bool ok = false;
    if (!mDocumentLoader)
        {
        mDocumentLoader = new HbDocumentLoader();        
        mDocumentLoader->load(DELIMETER_DOCML, &ok);
        }
    QGraphicsWidget *widget = NULL;
    if (!popup)
        {
        widget = mDocumentLoader->findWidget(DIALOG);
        Q_ASSERT_X(ok && (widget != 0), "DIALOG", "invalid view");

        popup = qobject_cast<HbDialog*> (widget);
        }
    if (!dataform)
        {
        widget = mDocumentLoader->findWidget(DATAFORM);
        Q_ASSERT_X(ok && (widget != 0), "DATAFORM", "invalid view");
        dataform = qobject_cast<HbDataForm*> (widget);
        }
    if (!mModel)
        {
        mModel = new HbDataFormModel();
        }
    if (!signalMapper)
        {
        signalMapper = new QSignalMapper(this);
        }

    createGui();
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::createGui()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::createGui()
    {
    popup->setDismissPolicy(HbDialog::NoDismiss);
    popup->setTimeout(HbDialog::NoTimeout);

    HbAction * action = new HbAction(hbTrId("txt_common_button_ok"), popup);
    connect(action, SIGNAL(triggered()), this, SLOT(checkBoxOkEvent()));
    mActions.append(action);

    action = new HbAction(hbTrId("txt_common_button_cancel"), popup);
    connect(action, SIGNAL(triggered()), this, SLOT(checkBoxCancelEvent()));
    mActions.append(action);

    popup->addActions(mActions);

    dataform->setModel(mModel);

    HbLabel *label = new HbLabel(hbTrId("txt_search_title_search_scope"));
    popup->setHeadingWidget(label);

    preparecategories();

    loadBaseSettings();

    HbDataFormModelItem* themeComboGeneral = mModel->appendDataFormItem(
            HbDataFormModelItem::ComboBoxItem, hbTrId(
                    "txt_search_info_select_search_scope"));

    themeComboGeneral->setContentWidgetData(QString("items"), mCategoryList);

    themeComboGeneral->setContentWidgetData(QString("currentIndex"),
            mSelectedScope);

    QModelIndex index = mModel->indexFromItem(themeComboGeneral);

    HbDataFormViewItem *formItem = static_cast<HbDataFormViewItem *> (dataform->itemByIndex(index));

      comboBox =
            static_cast<HbComboBox*> (formItem->dataItemContentWidget());

    q_currentIndexChanged(mSelectedScope);

    connect(comboBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(q_currentIndexChanged(int)));

    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::q_currentIndexChanged()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::q_currentIndexChanged(int avalue)
    {
    for (int i = 0; i < mModelItemList.count(); i++)
        {
        mModel->removeItem(mModelItemList.at(i));
        }
    mModelItemList.clear();
   // int ret = mModel->rowCount();
    if (!avalue) //device category creation
        {
        isInternetSelected = false;
        HbCheckBox *checkboxitem = NULL;
        mDeviceCheckBoxList.clear();
        for (int i = 0; i < mDeviceListDisplay.count(); i++)
            {

            HbDataFormModelItem* mModelItem = mModel->appendDataFormItem(
                    HbDataFormModelItem::CheckBoxItem);
            mModelItemList.append(mModelItem);
            QModelIndex index = mModel->indexFromItem(mModelItem);

            HbDataFormViewItem *formItem = static_cast<HbDataFormViewItem *> (dataform->itemByIndex(index));

            checkboxitem
                    = static_cast<HbCheckBox*> (formItem->dataItemContentWidget());

            checkboxitem->setText(mDeviceListDisplay.at(i));

            mDeviceCheckBoxList.append(checkboxitem);

            signalMapper->setMapping(mDeviceCheckBoxList.at(i), i);

            connect(mDeviceCheckBoxList.at(i), SIGNAL(stateChanged(int)),
                    signalMapper, SLOT(map()));

            connect(signalMapper, SIGNAL(mapped(int)), this,
                    SLOT(itemChecked(int)));
            }
        loadDeviceSettings();
        }
    else
        {
        isInternetSelected = true;
        mDeviceCheckBoxList.clear();
        HbDataFormModelItem* mModelItem = mModel->appendDataFormItem(
                HbDataFormModelItem::RadioButtonListItem);
        mModelItemList.append(mModelItem);
        mModelItem->setContentWidgetData("items", internetCategoryList);

        QModelIndex index = mModel->indexFromItem(mModelItem);

        HbDataFormViewItem *formItem = static_cast<HbDataFormViewItem *> (dataform->itemByIndex(index));

        mradiolist
                = static_cast<HbRadioButtonList*> (formItem->dataItemContentWidget());

        mradiolist->setPreviewMode(HbRadioButtonList::NoPreview);

        mradiolist->setSelected(mSelectedProvider);
        connect(mradiolist, SIGNAL(itemSelected(int)), this,
                SLOT(q_itemSelected(int)));

        for (int i = 0; i < mActions.count(); i++)
            popup->removeAction(mActions.at(i));
        popup->addActions(mActions);
        popup->removeAction(mActions.at(0));
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::preparecategories()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::preparecategories()
    {
    // read form database


    QString mConnectionName("cpixcontentinfo.sq");
    QString mDatabaseName("c:\\Private\\2001f6fb\\cpixcontentinfo.sq");

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", mConnectionName);
    db.setDatabaseName(mDatabaseName);

    QSqlQuery query(QSqlDatabase::database(mConnectionName));
    QString statement = "SELECT NAME FROM table1";

    query.prepare(statement);
    query.exec();
    while (query.next())
        {
        QString category_name = query.value(0).toString();
        int value = query.value(2).toInt();
        if (value)
            {
            mCategoryDbMapping.insert(category_name, false);
            }
        else
            {
            mCategoryDbMapping.insert(category_name, true);
            }
        qDebug() <<"string db= "<<category_name;
        }
    db.close();         
    
  /*  mCategoryDbMapping.insert("Contacts", true); 
    mCategoryDbMapping.insert("Audios", true); 
    mCategoryDbMapping.insert("Images", true); 
    mCategoryDbMapping.insert("Videos", true); 
    mCategoryDbMapping.insert("Messages", true); 
    mCategoryDbMapping.insert("email", true); 
    mCategoryDbMapping.insert("Calendar", true); 
    mCategoryDbMapping.insert("Notes", true); 
    mCategoryDbMapping.insert("Applications", true); 
    mCategoryDbMapping.insert("Bookmarks", true); 
    mCategoryDbMapping.insert("Files", true); */
    
   // mCategoryDbMapping.insert("email", true); // to remove once email starts working
    
    mDeviceListDisplay.append(hbTrId("txt_search_list_select_all"));
    mDeviceMapping.insert(0, true);

    if (mCategoryDbMapping.value("Contacts"))
        {
        mDeviceMapping.insert(1, true);
        mDeviceListDisplay.append(hbTrId("txt_search_list_contatcs"));
        }

    if (mCategoryDbMapping.value("Audios")
            || mCategoryDbMapping.value("Images") || mCategoryDbMapping.value(
            "Videos"))
        {
        mDeviceListDisplay.append(hbTrId("txt_search_list_media"));
        mDeviceMapping.insert(2, true);
        }

    if (mCategoryDbMapping.value("Messages") || mCategoryDbMapping.value(
            "email"))
        {
        mDeviceListDisplay.append(hbTrId("txt_search_list_messagemail"));
        mDeviceMapping.insert(3, true);
        }

    if (mCategoryDbMapping.value("Calendar") || mCategoryDbMapping.value(
            "Notes"))
        {
        mDeviceListDisplay.append(hbTrId("txt_search_list_calendarnotes"));
        mDeviceMapping.insert(4, true);
        }

    if (mCategoryDbMapping.value("Applications"))
        {
        mDeviceListDisplay.append(hbTrId("txt_search_list_applications"));
        mDeviceMapping.insert(5, true);
        }

    if (mCategoryDbMapping.value("Bookmarks"))
        {
        mDeviceListDisplay.append(hbTrId("txt_search_list_bookmarks"));
        mDeviceMapping.insert(6, true);
        }

    if (mCategoryDbMapping.value("Files"))
        {
        mDeviceListDisplay.append(hbTrId("txt_search_list_all_other_files"));
        mDeviceMapping.insert(7, true);
        }
    internetCategoryList = (QStringList());

    mCategoryList = (QStringList() << hbTrId("txt_search_list_device")
            << hbTrId("txt_search_list_internet"));
    }

//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::~SettingsWidget()
//
//----------------------------------------------------------------------------------------------------------------------------
SettingsWidget::~SettingsWidget()
    {
    delete signalMapper;
    //delete popup;
    //delete dataform;
    delete mModel;
    //delete mradiolist;
    mDeviceCheckBoxList.clear();
    mInternetCheckBoxList.clear();
    mModelItemList.clear();
    mActions.clear();
    delete mDocumentLoader;
    //delete popup;
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::launchSettingWidget()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::launchSettingWidget()
    {
    if (mInstialize)
        {
        initialize();
        mInstialize = false;
        }
    comboBox->setCurrentIndex(mSelectedScope);
    loadDeviceSettings();
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
    for (int i = 0; i < mActions.count(); i++)
        popup->removeAction(mActions.at(i));
    popup->addActions(mActions);
    if (noItemSelected)
        {
        popup->removeAction(mActions.at(0));
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::itemChecked( int selectedIndex)
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::itemChecked(int selectedIndex)
    {
    if (mDeviceCheckBoxList.count() != 0)
        {
        if (selectedIndex == 0 && mchangestate) // validating the select all
            {
            if ((mDeviceCheckBoxList.at(selectedIndex)->checkState()
                    == Qt::Checked))
                {
                checkSubCategories();
                }
            else
                {
                unCheckSubCategories();
                }
            }
        else
            {
            if ((mDeviceCheckBoxList.at(selectedIndex)->checkState()
                    == Qt::Unchecked))
                {
                mchangestate = false;
                mDeviceCheckBoxList.at(0)->setCheckState(Qt::Unchecked);
                setActionVisibility();
                return;
                }
            }
        }
    mchangestate = true;
    setActionVisibility();
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::checkSubCategories
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::checkSubCategories()
    {
    for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
        {
        mDeviceCheckBoxList.at(i)->setCheckState(Qt::Checked);
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::unCheckSubCategories
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::unCheckSubCategories()
    {
    for (int i = 0; i < mDeviceCheckBoxList.count(); i++)
        {
        mDeviceCheckBoxList.at(i)->setCheckState(Qt::Unchecked);
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::checkBoxCancelEvent()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::checkBoxCancelEvent()
    {
    //   loadDeviceSettings();//bug fix for cancel event selection should not reflect "go" button
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
    QSettings appSettings("search.ini", QSettings::IniFormat);
    appSettings.setValue("selectedcategory", mSelectedScope);
    appSettings.setValue("devicecount", mDeviceCategoryRefList.count());
    if (!isInternetSelected)
        {
        int j = 0;
        for (int i = 0; i < mDeviceCategoryRefList.count(); i++)
            {
            if (mDeviceMapping.at(i))
                {
                if (mDeviceCheckBoxList.at(j)->checkState() == Qt::Checked)
                    {
                    appSettings.setValue(mDeviceCategoryRefList.at(i), 1);
                    emit selectedItemCategory(i, true);
                    }
                else
                    {
                    appSettings.setValue(mDeviceCategoryRefList.at(i), 0);
                    emit selectedItemCategory(i, false);
                    }
                j++;
                }
            else
                {
                appSettings.setValue(mDeviceCategoryRefList.at(i), 0);
                emit selectedItemCategory(i, false);
                }

            }
        }
    appSettings.setValue("internetcount", internetCategoryList.count());
    appSettings.setValue("selectedprovider", mSelectedProvider);
    }
//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::loadBaseSettings()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::loadBaseSettings()
    {
    QSettings appSettings("search.ini", QSettings::IniFormat);
    int value;
    if (!(appSettings.contains("selectedcategory")))
        {
        value = 0;
        }
    else
        {
        value = appSettings.value("selectedcategory").toInt();
        }
    mSelectedScope = value;
    if (mSelectedScope)
        {
        isInternetSelected = true;
        }
    else
        {
        isInternetSelected = false;
        }
    mSelectedProvider = appSettings.value("selectedprovider").toInt();
    }

//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::loadDeviceSettings()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::loadDeviceSettings()
    {
    QSettings appSettings("search.ini", QSettings::IniFormat);
    int value;
    if (!isInternetSelected)
        {
        int j = 0;
        for (int i = 0; i < mDeviceCategoryRefList.count(); i++)
            {
            if (!(appSettings.contains(mDeviceCategoryRefList.at(i))))
                {
                value = 1;
                }
            else
                {
                value
                        = appSettings.value(mDeviceCategoryRefList.at(i)).toInt();
                }
            if (mDeviceMapping.count() && mDeviceCheckBoxList.count()
                    && mDeviceMapping.at(i))
                {
                if (value)
                    {
                    mDeviceCheckBoxList.at(j)->setCheckState(Qt::Checked);
                    }
                else
                    {
                    mDeviceCheckBoxList.at(j)->setCheckState(Qt::Unchecked);
                    }
                j++;
                }
            if (value) // get the setting before ui preparation
                {
                emit selectedItemCategory(i, true);
                }
            else
                {
                emit selectedItemCategory(i, false);
                }
            }
        }
    }
//----------------------------------------------------------------------------------------------------------------------------
//SettingsWidget::isInternetSearchOptionSelected()
//
//----------------------------------------------------------------------------------------------------------------------------
bool SettingsWidget::isInternetSearchOptionSelected()
    {
    loadBaseSettings();
    return isInternetSelected;
    }

//----------------------------------------------------------------------------------------------------------------------------
//void SettingsWidget::storeDefaultSettings()
//
//----------------------------------------------------------------------------------------------------------------------------
void SettingsWidget::storeDefaultSettings()
    {
    QSettings appSettings("search.ini", QSettings::IniFormat);
    if (!appSettings.contains("selectedcategory")) // change the settings for the first time only
        {
        mSelectedScope = 0;
        mSelectedProvider = 0;
        appSettings.setValue("selectedcategory", mSelectedScope);
        appSettings.setValue("devicecount", mDeviceCategoryRefList.count());
        for (int i = 0; i < mDeviceCategoryRefList.count(); i++)
            appSettings.setValue(mDeviceCategoryRefList.at(i), 1);
        appSettings.setValue("internetcount", internetCategoryList.count());
        appSettings.setValue("selectedprovider", mSelectedProvider);
        }
    }
