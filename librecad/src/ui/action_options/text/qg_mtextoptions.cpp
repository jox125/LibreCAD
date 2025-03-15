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
#include "qg_mtextoptions.h"

#include "rs_actiondrawmtext.h"
#include "rs_math.h"
#include "ui_qg_mtextoptions.h"
#include "rs_debug.h"

/*
 *  Constructs a QG_TextOptions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
QG_MTextOptions::QG_MTextOptions()
    : LC_ActionOptionsWidgetBase(RS2::ActionDrawMText, "", "")
	, ui(new Ui::Ui_MTextOptions{}){
	ui->setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
QG_MTextOptions::~QG_MTextOptions() = default;

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void QG_MTextOptions::languageChange(){
	ui->retranslateUi(this);
}

void QG_MTextOptions::doSaveSettings(){
}


void QG_MTextOptions::doSetAction(RS_ActionInterface *a, bool update){
    action = dynamic_cast<RS_ActionDrawMText *>(a);

    QString text;
    QString angle;
    if (update){
        text = action->getText();
        angle = fromDouble(action->getUcsAngleDegrees());
    } else {
        text = "";
        angle = "0.0";
    }
    ui->teText->setText(text);
    ui->leAngle->setText(angle);
}

void QG_MTextOptions::updateText() {
    if (action) {
/*#if defined(OOPL_VERSION) && defined(Q_WS_WIN)
        QCString iso = RS_System::localeToISO( QTextCodec::locale() );
        action->setText(
            RS_FilterDXF::toNativeString(
             QString::fromLocal8Bit( QTextCodec::codecForName( iso )->fromUnicode( teText->text() ) )
            )
        );
//#else*/
	   action->setText(ui->teText->toPlainText());
//#endif
    }
}

void QG_MTextOptions::updateAngle() {
    double angle;
    QString val = ui->leAngle->text();
    if (toDoubleAngleDegrees(val, angle, 0.0, false)) {
        action->setUcsAngleDegrees(angle);
    }
}
