/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2025 LibreCAD.org
 Copyright (C) 2025 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#include "lc_ucslistbutton.h"
#include "lc_ucs.h"
#include "lc_ucslistwidget.h"

LC_UCSListButton::LC_UCSListButton(LC_UCSListWidget *w):QToolButton(nullptr), widget{w} {
    setPopupMode(QToolButton::MenuButtonPopup);
    menu = new QMenu();
    connect(menu, &QMenu::aboutToShow, this, &LC_UCSListButton::fillMenu);
    setMenu(menu);
}

void LC_UCSListButton::enableSubActions(bool value){
    if (value){
        setMenu(menu);
    }
    else{
        setMenu(nullptr);
    }
}

// a bit weird logic, yet adding actions to menu and removing them on close does not work - menu is shown correctly, yet signal for trigger is not called.
// therefore, the list of actions is reused, and not-needed actions are simply invisible.
void LC_UCSListButton::fillMenu() {
    QList<LC_UCS*> ucsList;
    widget->fillUCSList(ucsList);

    auto activeUCS = widget->getActiveUCS();

    int ucsCount = ucsList.count();
    int actionsCount = createdActions.count();
    if (ucsCount <= actionsCount){
        int i;
        for (i = 0; i < ucsCount; i++){
            auto a = createdActions.at(i);
            auto u = ucsList.at(i);
            auto typeIcon = widget->getUCSTypeIcon(u);
            a->setText(u->getName());
            a->setIcon(typeIcon);
            a->setVisible(true);
            a->setChecked(u == activeUCS);
            auto modelIndex = widget->getIndexForUCS(u);
            a->setProperty("_UCS_IDX", QVariant(modelIndex));
        }
        for (;i < actionsCount; i++){
            QAction* a = createdActions.at(i);
            a->setVisible(false);
        }
    }
    else{
        int i;
        for (i = 0;  i < actionsCount; i++){
            auto a = createdActions.at(i);
            auto u = ucsList.at(i);
            auto typeIcon = widget->getUCSTypeIcon(u);
            a->setText(u->getName());
            a->setIcon(typeIcon);
            a->setVisible(true);
            a->setChecked(u == activeUCS);
            auto modelIndex = widget->getIndexForUCS(u);
            a->setProperty("_UCS_IDX", QVariant(modelIndex));
        }
        for (; i < ucsCount; i++){
            auto u = ucsList.at(i);
            auto name = u->getName();
            auto typeIcon = widget->getUCSTypeIcon(u);
            auto* a = menu->addAction(typeIcon, name);
            connect(a, &QAction::triggered, this, &LC_UCSListButton::menuTriggered);
            createdActions << a;
            a->setEnabled(true);
            a->setChecked(u == activeUCS);
            a->setCheckable(true);
            a->setVisible(true);
            auto modelIndex = widget->getIndexForUCS(u);
            a->setProperty("_UCS_IDX", QVariant(modelIndex));
        }
    }
}

void LC_UCSListButton::menuTriggered([[maybe_unused]]bool checked){
    auto *action = qobject_cast<QAction*>(sender());
    if (action != nullptr) {
        QVariant variant = action->property("_UCS_IDX");
        if (variant.isValid()){
            QModelIndex index = variant.toModelIndex();
            widget->applyUCSByIndex(index);
        }
    }
}
