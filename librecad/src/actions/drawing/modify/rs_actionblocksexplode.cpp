/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include "rs_actionblocksexplode.h"
#include "rs_modification.h"

/**
 * Constructor.
 */
RS_ActionBlocksExplode::RS_ActionBlocksExplode(RS_EntityContainer& container,
        RS_GraphicView& graphicView)
        :LC_ActionPreSelectionAwareBase("Blocks Explode",
                           container, graphicView) {
    actionType=RS2::ActionBlocksExplode;
}

void RS_ActionBlocksExplode::doTrigger(bool keepSelected) {
    RS_Modification m(*container, viewport);
    // fixme - add options like "keep originals" and "current attributes"?
    m.explode(selectedEntities, true, false, keepSelected);
}

bool RS_ActionBlocksExplode::isEntityAllowedToSelect(RS_Entity *ent) const {
    return ent->isContainer();
}

void RS_ActionBlocksExplode::updateMouseButtonHintsForSelection() {
    updateMouseWidgetTRCancel(tr("Select to explode container (Enter to complete)"), MOD_CTRL(tr("Select and explode")));
}
