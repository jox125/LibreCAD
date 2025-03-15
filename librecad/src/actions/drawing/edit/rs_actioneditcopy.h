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

#ifndef RS_ACTIONEDITCOPY_H
#define RS_ACTIONEDITCOPY_H

#include "qg_actionhandler.h"
#include "rs_previewactioninterface.h"

// fixme - add disabling of paste actions if clipboard is empty!
/**
 * This action class can handle user events for copying,cutting and pasting entities
 * to the clipboard.
 *
 * @author Andrew Mustun
 */
class RS_ActionEditCopyPaste :public RS_PreviewActionInterface {
    Q_OBJECT
public:
    enum ActionMode{
        CUT,
        COPY,
        CUT_QUICK,
        COPY_QUICK,
        PASTE
    };

    RS_ActionEditCopyPaste(ActionMode mode,
                           RS_EntityContainer& container,
                           RS_GraphicView& graphicView);
    ~RS_ActionEditCopyPaste() override;

    void init(int status) override;
protected:
    /**
  * Action States.
  */
    enum Status {
        SetReferencePoint    /**< Setting the reference point. */
    };

    /** Copy (true) or cut (false) */
    ActionMode mode;
    bool invokedWithControl = false;
    std::unique_ptr<RS_Vector> referencePoint;
    RS2::CursorType doGetMouseCursor(int status) override;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    void onCoordinateEvent(int status, bool isZero, const RS_Vector &pos) override;
    void updateMouseButtonHints() override;
    void doTrigger() override;

    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
};
#endif
