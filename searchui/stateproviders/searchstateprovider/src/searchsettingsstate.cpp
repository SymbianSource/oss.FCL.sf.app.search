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
 * Description:  Implements the wizard activation and interactions.
 *
 */

#include "searchsettingsstate.h"

#include <hbdocumentloader.h>
#include <hbview.h>
#include <hblabel.h>
#include <hbstackedwidget.h>
#include <hbicon.h>
#include <hbinstance.h>
#include <hbaction.h>
#include <hbgridview.h>
#include <qstandarditemmodel.h>
#include <qdebug.h>
#include <qgraphicswidget.h>
#include <qdir.h>
#include"settingswidget.h"

const char *WIZARD_VIEW = "tocView";

// ---------------------------------------------------------------------------
// SearchSettingsState::SearchSettingsState
// ---------------------------------------------------------------------------
//
SearchSettingsState::SearchSettingsState(QState *parent) :
    QState(parent), mDocumentLoader(NULL)
    {
    minitialCount = true;
    mWidget = new SettingsWidget();

    connect(mWidget, SIGNAL(settingsEvent(bool)), this, SLOT(handleBackEvent(bool)));

    connect(mWidget, SIGNAL(selectedItemCategory(int, bool)), this,
            SLOT(getItemCategory(int, bool)));

    }
// ---------------------------------------------------------------------------
// SearchSettingsState::_selectedcategory
// ---------------------------------------------------------------------------
//
void SearchSettingsState::getItemCategory(int str, bool avalue)
    {
    emit publishSelectedCategory(str, avalue);
    }

// ---------------------------------------------------------------------------
// SearchSettingsState::~SearchSettingsState
// ---------------------------------------------------------------------------
//
SearchSettingsState::~SearchSettingsState()
    {
    delete mDocumentLoader;
    //  delete mWidget;

    }

// ---------------------------------------------------------------------------
// SearchSettingsState::onEntry
// ---------------------------------------------------------------------------
//
void SearchSettingsState::onEntry(QEvent *event)
    {
    qDebug() << "search:SearchSettingsState::onEntry";
    QState::onEntry(event);
    if (minitialCount)
        {
        mWidget->loadSettingsFrominiFile();
        isInternetOn();
        minitialCount = false;
        emit backEventTriggered();
        }
    else
        {

        mWidget->launchSettingWidget();
        }
    }

// ---------------------------------------------------------------------------
// SearchSettingsState::onExit
// ---------------------------------------------------------------------------
//
void SearchSettingsState::onExit(QEvent *event)
    {
    QState::onExit(event);

    }

void SearchSettingsState::handleBackEvent(bool aStatus)
    {
    emit backEventTriggered();
    if (mWidget)
        {
        isInternetOn();
       emit clickstatus(aStatus);
        }   
    }
// ---------------------------------------------------------------------------
// SearchSettingsState::isInternetOn
// ---------------------------------------------------------------------------
//
void SearchSettingsState::isInternetOn()
    {
    if (mWidget->isInternetSearchOptionSelected())
        {
        emit customizeGoButton(true);
        }
    else
        {
        emit customizeGoButton(false);
        }
    }

