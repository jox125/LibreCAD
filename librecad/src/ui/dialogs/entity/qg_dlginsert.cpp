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
#include "qg_dlginsert.h"

#include "rs_insert.h"
#include "rs_graphic.h"
#include "rs_math.h"

/*
 *  Constructs a QG_DlgInsert as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
QG_DlgInsert::QG_DlgInsert(QWidget *parent, LC_GraphicViewport *pViewport, RS_Insert* insert)
    :LC_EntityPropertiesDlg(parent, "InsertProperties",pViewport){
    setupUi(this);
    setEntity(insert);
}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_DlgInsert::~QG_DlgInsert(){
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_DlgInsert::languageChange(){
    retranslateUi(this);
}

void QG_DlgInsert::setEntity(RS_Insert* i) {
    entity = i;
    RS_Graphic* graphic = entity->getGraphic();
    if (graphic) {
        cbLayer->init(*(graphic->getLayerList()), false, false);
    }
    RS_Layer* lay = entity->getLayer(false);
    if (lay) {
        cbLayer->setLayer(*lay);
    }

    wPen->setPen(entity, lay, "Pen");

    toUI(entity->getInsertionPoint(), leInsertionPointX, leInsertionPointY);
    toUIRaw(entity->getScale(), leScaleX, leScaleY);
    toUIAngleDeg(entity->getAngle(), leAngle);
    QString s;

    s.setNum(entity->getRows());
    leRows->setText(s);
    s.setNum(entity->getCols());
    leCols->setText(s);
    s.setNum(entity->getSpacing().y);
    leRowSpacing->setText(s);
    s.setNum(entity->getSpacing().x);
    leColSpacing->setText(s);
}

void QG_DlgInsert::updateEntity() {
    entity->setInsertionPoint(toWCS(leInsertionPointX, leInsertionPointY, entity->getInsertionPoint()));
    entity->setScale(toWCSRaw(leScaleX,leScaleY, entity->getScale()));
    entity->setAngle(toWCSAngle(leAngle, entity->getAngle()));

    // fixme - complete
    entity->setRows(RS_Math::round(RS_Math::eval(leRows->text())));
    entity->setCols(RS_Math::round(RS_Math::eval(leCols->text())));
    entity->setSpacing(RS_Vector(RS_Math::eval(leColSpacing->text()),
                                 RS_Math::eval(leRowSpacing->text())));

    entity->setPen(wPen->getPen());
    entity->setLayer(cbLayer->getLayer());

    entity->update();
}
