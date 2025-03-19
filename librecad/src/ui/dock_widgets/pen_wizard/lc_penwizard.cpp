/*
**********************************************************************************
**
** This file was created for LibreCAD (https://github.com/LibreCAD/LibreCAD).
**
** Copyright (C) 2016 ravas (github.com/r-a-v-a-s)
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
** http://www.gnu.org/licenses/gpl-2.0.html
**
**********************************************************************************
*/

#include <QVBoxLayout>


#include "colorwizard.h"
#include "lc_penwizard.h"

#include <QToolButton>

#include "qc_mdiwindow.h"
#include "qg_graphicview.h"
#include "rs_entity.h"
#include "rs_graphic.h"
#include "rs_settings.h"

LC_PenWizard::LC_PenWizard(const QString& title, QWidget* parent)
    : QDockWidget(title, parent)
    , color_wiz(new ColorWizard(this))
{
    auto frame = new QFrame(this);
    auto layout = new QVBoxLayout;
    frame->setLayout(layout);

    layout->setContentsMargins(QMargins{});
    layout->addWidget(color_wiz);

    setWidget(frame);

    connect(color_wiz, &ColorWizard::requestingColorChange,
            this, &LC_PenWizard::setColorForSelected);
    connect(color_wiz, &ColorWizard::requestingSelection,
            this, &LC_PenWizard::selectByColor);
    connect(color_wiz, &ColorWizard::colorDoubleClicked,
            this, &LC_PenWizard::setActivePenColor);

    updateWidgetSettings();
}

void LC_PenWizard::setColorForSelected(QColor color)
{
    auto graphic = mdi_win->getGraphic();
    auto pen = graphic->getActivePen();
    pen.setColor(RS_Color(color));

    foreach (auto e, graphic->getEntityList())
    {
        if (e->isSelected())
        {
            e->setPen(pen);
            e->setSelected(false);
        }
    }
    mdi_win->getGraphicView()->redraw(RS2::RedrawDrawing);
}

void LC_PenWizard::selectByColor(QColor color){
    auto graphic = mdi_win->getGraphic();
    foreach (auto e, graphic->getEntityList())
    {
        if (e->getPen().getColor().name() == color.name())
        {
            e->setSelected(true);
        }
    }
    mdi_win->getGraphicView()->redraw(RS2::RedrawDrawing);
}

void LC_PenWizard::setActivePenColor(QColor color){
    auto graphic = mdi_win->getGraphic();
    auto pen = graphic->getActivePen();
    pen.setColor(RS_Color(color));
    graphic->setActivePen(pen);
}

void LC_PenWizard::setMdiWindow(QC_MDIWindow* mdiWindow){
    mdi_win = mdiWindow;
}

void LC_PenWizard::updateWidgetSettings(){
    LC_GROUP("Widgets"); {
        bool flatIcons = LC_GET_BOOL("DockWidgetsFlatIcons", true);
        int iconSize = LC_GET_INT("DockWidgetsIconSize", 16);

        QSize size(iconSize, iconSize);

        QList<QToolButton *> widgets = this->findChildren<QToolButton *>();
        foreach(QToolButton *w, widgets) {
            w->setAutoRaise(flatIcons);
            w->setIconSize(size);
        }
    }
    LC_GROUP_END();
}
