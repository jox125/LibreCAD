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

#include <QApplication>
#include <QMouseEvent>

#include "lc_highlight.h"
#include "lc_linemath.h"
#include "lc_quickinfowidget.h"
#include "lc_refarc.h"
#include "lc_refcircle.h"
#include "lc_refconstructionline.h"
#include "lc_refellipse.h"
#include "lc_refline.h"
#include "lc_refpoint.h"
#include "lc_overlayentitiescontainer.h"
#include "rs_actioninterface.h"
#include "rs_arc.h"
#include "rs_circle.h"
#include "rs_commands.h"
#include "rs_constructionline.h"
#include "rs_coordinateevent.h"
#include "rs_creation.h"
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_line.h"
#include "rs_math.h"
#include "rs_point.h"
#include "rs_preview.h"
#include "rs_previewactioninterface.h"
#include "rs_settings.h"

// fixme - sand - consider more generic support of overlays and containers,
// with them working with preview etc might be more generic.. currently, preview handles both preview and reference points..
// such context-dependent things line absolute/relative position, angle, snap point info may be added to overlays,
// as wells as, say direction marks or something similar
/**
 * Constructor.
 *
 * Sets the entity container on which the action class inherited
 * from this interface operates.
 */
RS_PreviewActionInterface::RS_PreviewActionInterface(const char* name,
                                                     RS_EntityContainer& container,
                                                     RS_GraphicView& graphicView,
                                                     RS2::ActionType actionType) :
    RS_ActionInterface(name, container, graphicView, actionType)
  ,preview(std::make_unique<RS_Preview>(&container)), highlight(std::make_unique<LC_Highlight>())
{

    RS_DEBUG->print("RS_PreviewActionInterface::RS_PreviewActionInterface: Setting up action with preview: \"%s\"", name);

    // preview is linked to the container for getting access to
    // document settings / dictionary variables

    preview->setLayer(nullptr);
    initRefEntitiesMetrics();

    RS_DEBUG->print("RS_PreviewActionInterface::RS_PreviewActionInterface: Setting up action with preview: \"%s\": OK", name);
}

/** Destructor */
RS_PreviewActionInterface::~RS_PreviewActionInterface() {
    deletePreview();
    deleteInfoCursor();
    deleteHighlights();
}

void RS_PreviewActionInterface::init(int status) {
    deletePreview();
    deleteInfoCursor();
    deleteHighlights();
    RS_ActionInterface::init(status);
}

void RS_PreviewActionInterface::finish(bool updateTB) {
    deletePreview();
    deleteInfoCursor();
    deleteHighlights();
    RS_ActionInterface::finish(updateTB);
}

void RS_PreviewActionInterface::suspend() {
    RS_ActionInterface::suspend();
    deletePreview();
    deleteInfoCursor();
    deleteHighlights();
}

void RS_PreviewActionInterface::resume() {
    RS_ActionInterface::resume();
    initRefEntitiesMetrics();
    drawPreview();
    drawHighlights();
}

void RS_PreviewActionInterface::trigger() {
    RS_ActionInterface::trigger();
    deletePreview();
    deleteInfoCursor();
    deleteHighlights();
    deleteSnapper();

    doTrigger();

    drawSnapper();
    updateSelectionWidget();
    graphicView->redraw();
}

/**
 * Deletes the preview from the screen.
 */
void RS_PreviewActionInterface::deletePreview(){
    if (hasPreview){
        //avoid deleting NULL or empty preview
        preview->clear();
        hasPreview = false;
    }
    if (!graphicView->isCleanUp()){
        RS_EntityContainer *container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
        container->clear();

        LC_OverlayDrawablesContainer *drawablesContainer = viewport->getOverlaysDrawablesContainer(RS2::ActionPreviewEntity);
        drawablesContainer->clear();
    }
}

/**
 * Draws / deletes the current preview.
 */
void RS_PreviewActionInterface::drawPreview(){
    RS_EntityContainer *container = viewport->getOverlayEntitiesContainer(RS2::ActionPreviewEntity);
    container->clear();
    container->setOwner(false); // Little hack for now so we don't delete the preview twice
   // remove reference entities from preview container and put them into overlay container directly.
   // the reason for this - painter for them should use different pen than one for ordinary preview entities

   container->addEntity(preview.get());
   preview->addReferenceEntitiesToContainer(container);
   graphicView->redraw(RS2::RedrawOverlay);
   hasPreview = true;
}

void RS_PreviewActionInterface::deleteHighlights(){
    // fixme - optimize if empty
    highlight->clear();
    if (!graphicView->isCleanUp()){
        RS_EntityContainer *overlayContainer = viewport->getOverlayEntitiesContainer(RS2::OverlayEffects);
        overlayContainer->clear();

        LC_OverlayDrawablesContainer *drawablesContainer = viewport->getOverlaysDrawablesContainer(RS2::OverlayEffects);
        drawablesContainer->clear();
    }
}

void RS_PreviewActionInterface::drawHighlights(){
    RS_EntityContainer *overlayContainer=viewport->getOverlayEntitiesContainer(RS2::OverlayEffects);
    overlayContainer->clear();
    overlayContainer->setOwner(false);
    highlight->addEntitiesToContainer(overlayContainer);
    graphicView->redraw(RS2::RedrawOverlay);
}

void RS_PreviewActionInterface::highlightHoverWithRefPoints(RS_Entity* e, bool value){
    highlight->addEntity(e, value);
}

void RS_PreviewActionInterface::highlightHover(RS_Entity* e){
    highlight->addEntity(e, highlightEntitiesRefPointsOnHover);
}


void RS_PreviewActionInterface::highlightSelected(RS_Entity *e, bool enable){
    if (enable){
        highlight->addEntity(e, false);
    }
    else{
        highlight ->removeEntity(e);
    }
}

void RS_PreviewActionInterface::addToHighlights(RS_Entity *e, bool enable){
    if (enable){
        highlight->addEntity(e, false);
    }
    else{
        highlight ->removeEntity(e);
    }
}

bool RS_PreviewActionInterface::trySnapToRelZeroCoordinateEvent(const LC_MouseEvent *e){
    bool result = false;
    if (e->isShift){
        RS_Vector relZero = getRelativeZero();
        if (relZero.valid){
            fireCoordinateEvent(relZero);
            result = true;
        }
    }
    return result;
}

RS_Vector RS_PreviewActionInterface::getSnapAngleAwarePoint(const LC_MouseEvent *e, const RS_Vector& basepoint, const RS_Vector& pos, bool drawMark, bool force){
    RS_Vector result = pos;
    if (force){
        RS_Vector freePosition  = e->graphPoint; // fixme = test, review and decide whether free snap is actually needed there. May be use snapMode instead of free?
        // todo -  if there are restrictions or snap to grid, snap to angle will not work in snapper... yet this is double calc!
        if(!(snapMode.restriction != RS2::RestrictNothing || isSnapToGrid())){
            result = doSnapToAngle(freePosition, basepoint, snapToAngleStep);
            if (drawMark){
                previewSnapAngleMark(basepoint, result);
            }
        }
    }
    return result;
}

RS_Vector RS_PreviewActionInterface::getSnapAngleAwarePoint(const LC_MouseEvent *e, const RS_Vector& basepoint, const RS_Vector& pos, bool drawMark){
    return getSnapAngleAwarePoint(e, basepoint, pos, drawMark, e->isShift);
}

RS_Vector RS_PreviewActionInterface::getRelZeroAwarePoint(const LC_MouseEvent *e, const RS_Vector& pos){
    RS_Vector result = pos;
    if (e->isShift){
        RS_Vector relZero = getRelativeZero();
        if (relZero.valid){
            result = relZero;
        }
    }
    return result;
}

RS_Ellipse *RS_PreviewActionInterface::previewEllipse(const RS_EllipseData &ellipseData){
    auto *ellipse = new RS_Ellipse(preview.get(), ellipseData);
    preview->addEntity(ellipse);
    return ellipse;
}

RS_Circle* RS_PreviewActionInterface::previewCircle(const RS_CircleData &circleData){
    auto *circle = new RS_Circle(preview.get(), circleData);
    preview->addEntity(circle);
    return circle;
}

RS_Circle* RS_PreviewActionInterface::previewToCreateCircle(const RS_CircleData &circleData){
    auto *result = previewCircle(circleData);
    prepareEntityDescription(result, RS2::EntityDescriptionLevel::DescriptionCreating);
    return result;
}

RS_Arc* RS_PreviewActionInterface::previewToCreateArc(const RS_ArcData &arcData){
    auto *result = previewArc(arcData);
    prepareEntityDescription(result, RS2::EntityDescriptionLevel::DescriptionCreating);
    return result;
}

RS_Line* RS_PreviewActionInterface::previewToCreateLine(const RS_LineData &lineData){
    auto *result = previewLine(lineData);
    prepareEntityDescription(result, RS2::EntityDescriptionLevel::DescriptionCreating);
    return result;
}

RS_Line* RS_PreviewActionInterface::previewToCreateLine(const RS_Vector &start, const RS_Vector &end){
    auto *result = previewLine(start, end);
    prepareEntityDescription(result, RS2::EntityDescriptionLevel::DescriptionCreating);
    return result;
}

RS_Ellipse* RS_PreviewActionInterface::previewToCreateEllipse(const RS_EllipseData &ellipseData){
    auto *result = previewEllipse(ellipseData);
    prepareEntityDescription(result, RS2::EntityDescriptionLevel::DescriptionCreating);
    return result;
}

RS_Point* RS_PreviewActionInterface::previewToCreatePoint(const RS_Vector &coord){
    auto *result = previewPoint(coord);
    prepareEntityDescription(result, RS2::EntityDescriptionLevel::DescriptionCreating);
    return result;
}

void RS_PreviewActionInterface::previewEntityToCreate(RS_Entity* en, bool addToPreview){
    if (addToPreview) {
        previewEntity(en);
    }
    prepareEntityDescription(en, RS2::EntityDescriptionLevel::DescriptionCreating);
}

RS_Arc* RS_PreviewActionInterface::previewArc(const RS_ArcData &arcData){
    auto *arc = new RS_Arc(preview.get(), arcData);
    preview->addEntity(arc);
    return arc;
}

RS_Arc* RS_PreviewActionInterface::previewRefArc(const RS_ArcData &arcData){
    auto *arc = new LC_RefArc(preview.get(), arcData);
    preview->addEntity(arc);
    return arc;
}

LC_RefEllipse* RS_PreviewActionInterface::previewRefEllipse(const RS_EllipseData &arcData){
    auto *arc = new LC_RefEllipse(preview.get(), arcData);
    preview->addEntity(arc);
    return arc;
}

RS_Line* RS_PreviewActionInterface::previewLine(const RS_LineData& data){
    auto *line = new RS_Line(this->preview.get(),data);
    preview->addEntity(line);
    return line;
}

RS_Line* RS_PreviewActionInterface::previewLine(const RS_Vector &start, const RS_Vector &end){
    auto *line = new RS_Line(this->preview.get(), start, end);
    preview->addEntity(line);
    return line;
}

void RS_PreviewActionInterface::previewEntity(RS_Entity* en){
    preview->addEntity(en);
}

void RS_PreviewActionInterface::addOverlay(LC_OverlayDrawable* drawable, RS2::OverlayGraphics position){
    LC_OverlayDrawablesContainer *drawablesContainer = viewport->getOverlaysDrawablesContainer(position);
    drawablesContainer->add(drawable);
}

void RS_PreviewActionInterface::previewRefPoint(const RS_Vector &coord){
    auto *point = new LC_RefPoint(this->preview.get(), coord, refPointSize, refPointMode);
    preview->addEntity(point);
}

void RS_PreviewActionInterface::previewRefSelectablePoint(const RS_Vector &coord){
    auto *point = new LC_RefPoint(this->preview.get(), coord, refPointSize, refPointMode);
    point->setHighlighted(true);
    preview->addEntity(point);
}

void RS_PreviewActionInterface::previewRefSelectableLine(const RS_Vector &start, const RS_Vector &end){
    auto *line = new LC_RefLine(this->preview.get(), start, end);
    line->setHighlighted(true);
    preview->addEntity(line);
}

RS_Point* RS_PreviewActionInterface::previewPoint(const RS_Vector &coord){
    auto *point = new RS_Point(this->preview.get(), coord);
    preview->addEntity(point);
    return point;
}

void RS_PreviewActionInterface::previewRefPoints(const std::vector<RS_Vector>& points){
    for (auto v: points) {
        previewRefPoint(v);
    }
}

void RS_PreviewActionInterface::previewRefLines(const std::vector<RS_LineData>& points){
    for (auto v: points) {
        previewRefLine(v.startpoint, v.endpoint);
    }
}

RS_Line* RS_PreviewActionInterface::previewRefLine(const RS_Vector &start, const RS_Vector &end){
    auto *line = new LC_RefLine(this->preview.get(), start, end);
    preview->addEntity(line);
    return line;
}

RS_ConstructionLine* RS_PreviewActionInterface::previewRefConstructionLine(const RS_Vector &start, const RS_Vector &end){
    auto *line = new LC_RefConstructionLine(this->preview.get(), start, end);
    preview->addEntity(line);
    return line;
}

RS_Arc* RS_PreviewActionInterface::previewRefArc(const RS_Vector &center, const RS_Vector &startPoint, const RS_Vector &mouse, bool determineReversal){
        double radius = center.distanceTo(startPoint);
        double angle1 = center.angleTo(mouse);
        double angle2 = center.angleTo(startPoint);
        bool reversed = determineReversal ? RS_Math::getAngleDifference(angle2, angle1) < M_PI : true;
        auto arc = new LC_RefArc(preview.get(), RS_ArcData(center, radius, angle1, angle2, reversed));
        preview->addEntity(arc);
        return arc;
}

void RS_PreviewActionInterface::previewSnapAngleMark(const RS_Vector &center, const RS_Vector &refPoint/*, const RS_Vector &refPoint2*/){
    double angle = center.angleTo(refPoint);
    previewSnapAngleMark(center, angle);
}

// fixme - sand - move to overlay!
void RS_PreviewActionInterface::initFromSettings() {
    RS_Snapper::initFromSettings();
    m_angleSnapMarkerSize = LC_GET_ONE_INT("Appearance", "AngleSnapMarkerSize", 20);
    m_doNotAllowNonDecimalAnglesInput = LC_GET_ONE_BOOL("CADPreferences", "InputAnglesAsDecimalsOnly", false);
}

// fixme - sand - snap to relative angle support!!!
// fixme - rework to natural paint via overlay
void RS_PreviewActionInterface::previewSnapAngleMark(const RS_Vector &center, double angle) {
// todo - add separate option that will control visibility of mark?
    int radiusInPixels = m_angleSnapMarkerSize; // todo - move to settings
    int lineInPixels = radiusInPixels * 2; // todo - move to settings
    double radius = toGraphDX(radiusInPixels);
    double lineLength = toGraphDX(lineInPixels);

    angle = RS_Math::correctAnglePlusMinusPi(angle);
    double angleZero = toWorldAngle(m_anglesBase);
    if (LC_LineMath::isMeaningfulAngle(angle)){
        previewRefArc(RS_ArcData(center, radius, angleZero, angle, !m_anglesCounterClockWise));
        previewRefLine(center, center + RS_Vector::polar(lineLength, angle));
    }
    previewRefLine(center, center.relative(lineLength, angleZero));
}

RS_Circle* RS_PreviewActionInterface::previewRefCircle(const RS_Vector &center, const double radius){
    auto *circle = new LC_RefCircle(preview.get(), center, radius);
    preview->addEntity(circle);
    return circle;
};

RS_Vector RS_PreviewActionInterface::getFreeSnapAwarePoint(const LC_MouseEvent *e, const RS_Vector &pos) const{
    RS_Vector mouse;
    if (e->isShift){
        mouse = e->graphPoint;
    }
    else{
        mouse = pos;
    }
    return mouse;
}

void RS_PreviewActionInterface::initRefEntitiesMetrics(){
    LC_GROUP_GUARD("Appearance");
    {
        // Points drawing style:
        refPointMode = LC_GET_INT("RefPointType", DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot));
        QString pdsizeStr = LC_GET_STR("RefPointSize", "2.0");

        showRefEntitiesOnPreview = LC_GET_BOOL("VisualizePreviewRefPoints", true);
        highlightEntitiesOnHover = LC_GET_BOOL("VisualizeHovering", true);
        highlightEntitiesRefPointsOnHover = LC_GET_BOOL("VisualizeHoveringRefPoints", true);

        bool ok;
        refPointSize = RS_Math::eval(pdsizeStr, &ok);
        if (!ok) {
            refPointSize = LC_DEFAULTS_PDSize;
        }
    }
}

/**
 * Utility method for setting relative zero.
 * Further may be used for additional processing.
 * @param zero
 */

void RS_PreviewActionInterface::moveRelativeZero(const RS_Vector& zero){
    viewport->moveRelativeZero(zero);
}

void RS_PreviewActionInterface::markRelativeZero() {
    viewport->markRelativeZero();
}

bool RS_PreviewActionInterface::is(RS_Entity *e, RS2::EntityType type) const{
    return  e != nullptr && e->is(type);
}

RS_Entity *RS_PreviewActionInterface::catchModifiableEntity(LC_MouseEvent *e, const RS2::EntityType &enType) {
    RS_Entity *en = catchEntityByEvent(e, enType, RS2::ResolveAll);
    if (en != nullptr && !en->isParentIgnoredOnModifications()){
        return en;
    }
    else{
        return nullptr;
    }
}


RS_Entity* RS_PreviewActionInterface::catchModifiableEntity(LC_MouseEvent *e, const EntityTypeList &enTypeList){
    RS_Entity *en = RS_Snapper::catchEntity(e->graphPoint, enTypeList, RS2::ResolveAll);
    if (en != nullptr && !en->isParentIgnoredOnModifications()){
        return en;
    }
    else{
        return nullptr;
    }
}

RS_Entity* RS_PreviewActionInterface::catchModifiableEntity(RS_Vector& coord, const RS2::EntityType &enType){
    RS_Entity *en = catchEntity(coord, enType, RS2::ResolveAll);
    if (en != nullptr && !en->isParentIgnoredOnModifications()){
        return en;
    }
    else{
        return nullptr;
    }
}

RS_Entity* RS_PreviewActionInterface::catchModifiableAndDescribe(LC_MouseEvent *e, const RS2::EntityType &enType){
    RS_Entity *en = catchModifiableEntity(e, enType);
    if (en != nullptr) {
        prepareEntityDescription(en, RS2::EntityDescriptionLevel::DescriptionCatched);
    }
    return en;
}

RS_Entity* RS_PreviewActionInterface::catchModifiableAndDescribe(LC_MouseEvent *e, const EntityTypeList &enTypeList){
    RS_Entity *en = catchModifiableEntity(e, enTypeList);
    if (en != nullptr) {
        prepareEntityDescription(en, RS2::EntityDescriptionLevel::DescriptionCatched);
    }
    return en;
}

RS_Entity* RS_PreviewActionInterface::catchAndDescribe( const RS_Vector &pos,RS2::ResolveLevel level){
    auto entity = catchEntity(pos, level);
    if (entity != nullptr) {
        prepareEntityDescription(entity, RS2::EntityDescriptionLevel::DescriptionCatched);
    }
    return entity;
}

RS_Entity* RS_PreviewActionInterface::catchAndDescribe(LC_MouseEvent *e, RS2::EntityType enType, RS2::ResolveLevel level) {
    auto entity = catchEntityByEvent(e, enType, level);
    if (entity != nullptr) {
        prepareEntityDescription(entity,RS2::EntityDescriptionLevel::DescriptionCatched);
    }
    return entity;
}


RS_Entity* RS_PreviewActionInterface::catchAndDescribe(LC_MouseEvent *e, const EntityTypeList &enTypeList, RS2::ResolveLevel level) {
    auto entity = catchEntityByEvent(e, enTypeList, level);
    if (entity != nullptr) {
        prepareEntityDescription(entity,RS2::EntityDescriptionLevel::DescriptionCatched);
    }
    return entity;
}

RS_Entity* RS_PreviewActionInterface::catchAndDescribe(LC_MouseEvent* e, RS2::ResolveLevel level) {
    auto entity = catchEntityByEvent(e, level);
    if (entity != nullptr) {
        prepareEntityDescription(entity,RS2::EntityDescriptionLevel::DescriptionCatched);
    }
    return entity;
}

void RS_PreviewActionInterface::prepareEntityDescription(RS_Entity *entity, RS2::EntityDescriptionLevel level) {
    if (infoCursorOverlayPrefs->enabled){
        if ((infoCursorOverlayPrefs->showEntityInfoOnCatch && level == RS2::EntityDescriptionLevel::DescriptionCatched)
           || (infoCursorOverlayPrefs->showEntityInfoOnCreation && level == RS2::EntityDescriptionLevel::DescriptionCreating)
           || (infoCursorOverlayPrefs->showEntityInfoOnModification && level == RS2::EntityDescriptionLevel::DescriptionModifying)){
            QString entityInfoStr = obtainEntityDescriptionForInfoCursor(entity,level);
            if (!entityInfoStr.isEmpty()) {
                QString snapString = infoCursorOverlayData.getZone2();
                QString updatedZone2;
                if (!snapString.isEmpty()){
                    updatedZone2 = snapString + "\n"  + entityInfoStr;
                }
                else{
                    updatedZone2 = entityInfoStr;
                }
                infoCursorOverlayData.setZone2(updatedZone2);
            }
        }
    }
}

void RS_PreviewActionInterface::appendInfoCursorEntityCreationMessage(QString message){
    if (infoCursorOverlayPrefs->enabled && infoCursorOverlayPrefs->showEntityInfoOnCreation) {
        appendInfoCursorZoneMessage(message, 2, false);
    }
}

void RS_PreviewActionInterface::appendInfoCursorZoneMessage(QString message, int zoneNumber, bool replaceContent){
    if (!message.isEmpty()) {
        bool enable = infoCursorOverlayPrefs->enabled;
        if (enable) {
            switch (zoneNumber) {
                case 1: {
                    QString msgToSet;
                    if (replaceContent){
                        msgToSet = message;
                    }
                    else{
                        QString existingInfo = infoCursorOverlayData.getZone1();
                        if (!existingInfo.isEmpty()) {
                            msgToSet = existingInfo + "\n" + message;
                        } else {
                            msgToSet = message;
                        }
                    }
                    infoCursorOverlayData.setZone1(msgToSet);
                    break;
                }
                case 2: {
                    QString msgToSet;
                    if (replaceContent){
                        msgToSet = message;
                    }
                    else{
                        QString existingInfo = infoCursorOverlayData.getZone2();
                        if (!existingInfo.isEmpty()) {
                            msgToSet = existingInfo + "\n" + message;
                        } else {
                            msgToSet = message;
                        }
                    }
                    infoCursorOverlayData.setZone2(msgToSet);
                    break;
                }
                case 3: {
                    QString msgToSet;
                    if (replaceContent){
                        msgToSet = message;
                    }
                    else{
                        QString existingInfo = infoCursorOverlayData.getZone3();
                        if (!existingInfo.isEmpty()) {
                            msgToSet = existingInfo + "\n" + message;
                        } else {
                            msgToSet = message;
                        }
                    }
                    infoCursorOverlayData.setZone3(msgToSet);
                    break;
                }
                case 4: {
                    QString msgToSet;
                    if (replaceContent){
                        msgToSet = message;
                    }
                    else{
                        QString existingInfo = infoCursorOverlayData.getZone4();
                        if (!existingInfo.isEmpty()) {
                            msgToSet = existingInfo + "\n" + message;
                        } else {
                            msgToSet = message;
                        }
                    }
                    infoCursorOverlayData.setZone4(msgToSet);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

QString RS_PreviewActionInterface::obtainEntityDescriptionForInfoCursor(RS_Entity *e, RS2::EntityDescriptionLevel level) {
   return graphicView->obtainEntityDescription(e, level);
}

void RS_PreviewActionInterface::deletePreviewAndHighlights() {
    deletePreview();
    deleteHighlights();
}

void RS_PreviewActionInterface::drawPreviewAndHighlights() {
    drawHighlights();
    drawPreview();
}

void RS_PreviewActionInterface::mouseMoveEvent(QMouseEvent *event) {
    int status = getStatus();
    LC_MouseEvent* lcEvent = toLCMouseMoveEvent(event);
    deletePreviewAndHighlights();
    onMouseMoveEvent(status, lcEvent);
    delete lcEvent;
    drawPreviewAndHighlights();
}

void RS_PreviewActionInterface::onMouseLeftButtonRelease(int status, QMouseEvent *e) {
    LC_MouseEvent* lcEvent = toLCMouseMoveEvent(e);
    onMouseLeftButtonRelease(status, lcEvent);
}

void RS_PreviewActionInterface::onMouseRightButtonRelease(int status, QMouseEvent *e) {
    LC_MouseEvent* lcEvent = toLCMouseMoveEvent(e);
    onMouseRightButtonRelease(status, lcEvent);
}

void RS_PreviewActionInterface::onMouseLeftButtonPress(int status, QMouseEvent *e) {
    LC_MouseEvent* lcEvent = toLCMouseMoveEvent(e);
    onMouseLeftButtonPress(status, lcEvent);
}

void RS_PreviewActionInterface::onMouseRightButtonPress(int status, QMouseEvent *e) {
    LC_MouseEvent* lcEvent = toLCMouseMoveEvent(e);
    onMouseRightButtonPress(status, lcEvent);
}

void RS_PreviewActionInterface::onMouseLeftButtonRelease([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *e) {}
void RS_PreviewActionInterface::onMouseRightButtonRelease([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *e) {}
void RS_PreviewActionInterface::onMouseLeftButtonPress([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *e) {}
void RS_PreviewActionInterface::onMouseRightButtonPress([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent *e) {}

void RS_PreviewActionInterface::onMouseMoveEvent([[maybe_unused]]int status, [[maybe_unused]]LC_MouseEvent* event) {}

LC_MouseEvent *RS_PreviewActionInterface::toLCMouseMoveEvent(QMouseEvent *e) {
    auto* result = new LC_MouseEvent();
    result->isShift = isShift(e);
    result->isControl = isControl(e);
    result->snapPoint = snapPoint(e);
    result->graphPoint = toGraph(e);
    result->uiPosition = e->pos();
    result->originalEvent = e;
    return result;
}

void RS_PreviewActionInterface::fireCoordinateEventForSnap(LC_MouseEvent *e){
    fireCoordinateEvent(e->snapPoint);
}

RS_Entity *RS_PreviewActionInterface::catchEntityByEvent(LC_MouseEvent *e, RS2::ResolveLevel level) {
    return catchEntity(e->graphPoint, level);
}

RS_Entity *RS_PreviewActionInterface::catchEntityByEvent(LC_MouseEvent *e, RS2::EntityType enType, RS2::ResolveLevel level) {
    return catchEntity(e->graphPoint, enType, level);
}

RS_Entity *RS_PreviewActionInterface::catchEntityByEvent(LC_MouseEvent *e, const EntityTypeList &enTypeList, RS2::ResolveLevel level) {
    return catchEntity(e->graphPoint, enTypeList, level);
}

bool RS_PreviewActionInterface::parseToWCSAngle(const QString &c, double& wcsAngleRad){
    bool ok = false;
    double ucsBasisAngleDeg = evalAngleValue(c, &ok);
    if (ok){
        ucsBasisAngleDeg = LC_LineMath::getMeaningfulAngle(ucsBasisAngleDeg);
        double ucsBasisAngleRad = RS_Math::deg2rad(ucsBasisAngleDeg);
        double ucsAbsValueRad = viewport->toUCSAbsAngle(ucsBasisAngleRad, m_anglesBase, m_anglesCounterClockWise);
        wcsAngleRad = viewport->toWorldAngle(ucsAbsValueRad);
    }
    return ok;
}

bool RS_PreviewActionInterface::parseToUCSBasisAngle(const QString &c, double& ucsBasisAngleRad){
    bool ok = false;
    double ucsBasisAngleDeg = evalAngleValue(c, &ok);
    if (ok){
        ucsBasisAngleDeg = LC_LineMath::getMeaningfulAngle(ucsBasisAngleDeg);
        ucsBasisAngleRad = RS_Math::deg2rad(ucsBasisAngleDeg);
    }
    return ok;
}

bool RS_PreviewActionInterface::parseToRelativeAngle(const QString &c, double& ucsBasisAngleRad){
    bool ok = false;
    double ucsBasisAngleDeg = evalAngleValue(c, &ok);
    if (ok){
        ucsBasisAngleDeg = LC_LineMath::getMeaningfulAngle(ucsBasisAngleDeg);
        ucsBasisAngleRad = adjustRelativeAngleSignByBasis(RS_Math::deg2rad(ucsBasisAngleDeg));
    }
    return ok;
}


double RS_PreviewActionInterface::evalAngleValue(const QString &c, bool *ok) const{
    QString stringToEval;
    if (m_doNotAllowNonDecimalAnglesInput) {
        stringToEval = c;
    }
    else{
        stringToEval = RS_Units::replaceAllPotentialAnglesByDecimalDegrees(c, ok);
    }
    double result = 0.0;
    if (ok){
        result = RS_Math::eval(stringToEval, ok);
    }
    return result;
}
