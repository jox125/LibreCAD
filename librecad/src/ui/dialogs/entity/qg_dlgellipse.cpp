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
#include "qg_dlgellipse.h"

#include "rs_debug.h"
#include "rs_ellipse.h"
#include "rs_graphic.h"
#include "rs_math.h"
#include "rs_settings.h"

/*
 *  Constructs a QG_DlgEllipse as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
QG_DlgEllipse::QG_DlgEllipse(QWidget *parent, LC_GraphicViewport *pViewport, RS_Ellipse* ellipse)
    :LC_EntityPropertiesDlg(parent, "EllipseProperties", pViewport){
    setupUi(this);
    setEntity(ellipse);
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_DlgEllipse::languageChange(){
    retranslateUi(this);
}

void QG_DlgEllipse::setEntity(RS_Ellipse* e) {
    entity = e;
    RS_Graphic* graphic = entity->getGraphic();
    if (graphic) {
        cbLayer->init(*(graphic->getLayerList()), false, false);
    }
    RS_Layer* lay = entity->getLayer(false);
    if (lay) {
        cbLayer->setLayer(*lay);
    }

    wPen->setPen(entity, lay, "Pen");

    toUI(entity->getCenter(), leCenterX, leCenterY);

    double majorAxisLen = entity->getMajorRadius();
    double minorAxisLen = entity->getMinorRadius();

    toUIValue(majorAxisLen, leMajor);
    toUIValue(minorAxisLen, leMinor);

    double wcsMajorAngle = entity->getMajorP().angle();

    toUIAngleDeg(wcsMajorAngle, leRotation);

    // fixme - sand - for ellipse arc, internal angles are used (assuming that major angle = 0). Rework this for the consistency over the entire UI - use ucs angle like RS_ARC!!
    toUIAngleDegRaw(entity->getAngle1(), leAngle1);
    toUIAngleDegRaw(entity->getAngle2(), leAngle2);

    toUIBool(entity->isReversed(), cbReversed);

    // fixme - sand - refactor to common function
    if (LC_GET_ONE_BOOL("Appearance","ShowEntityIDs", false)){
        lId->setText(QString("ID: %1").arg(entity->getId()));
    }
    else{
        lId->setVisible(false);
    }
}

void QG_DlgEllipse::updateEntity() {
    if (entity == nullptr)
        return;

    double major = toWCSValue(leMajor, entity->getMajorRadius());
    if (major < RS_TOLERANCE) {
        LC_ERR << __func__<<"(): invalid ellipse major radius: "<< major<<", ellipse not modified";
        return;
    }
    double minor = toWCSValue(leMinor, entity->getMinorRadius());

    double rotation = toWCSAngle(leRotation, entity->getMajorP().angle());

    entity->setMajorP(RS_Vector::polar(major, rotation));
    entity->setRatio(minor/major);

    entity->setCenter(toWCS(leCenterX, leCenterY, entity->getCenter()));

    entity->setAngle1(toRawAngleValue(leAngle1, entity->getAngle1()));
    entity->setAngle2(toRawAngleValue(leAngle2, entity->getAngle2()));

    entity->setReversed(cbReversed->isChecked());
    // todo - sand - should we call revertDirection() as for arc?

    entity->setPen(wPen->getPen());
    entity->setLayer(cbLayer->getLayer());
}
