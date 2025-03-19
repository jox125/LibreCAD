/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2020 A. Stebich (librecad@mail.lordofbikes.de)
** Copyright (C) 2011 Rallaz (rallazz@gmail.com)
** Copyright (C) 2010-2011 R. van Twisk (librecad@rvt.dds.nl)
**
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
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

#include <algorithm>

#include <QScrollBar>
#include <QTableView>
#include <QHeaderView>
#include <QToolButton>
#include <QMenu>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QContextMenuEvent>

#include "lc_flexlayout.h"
#include "qg_actionhandler.h"
#include "qg_blockwidget.h"
#include "rs_blocklist.h"
#include "rs_debug.h"
#include "rs_settings.h"

QG_BlockModel::QG_BlockModel(QObject * parent) : QAbstractTableModel(parent) {
    blockVisible = QIcon(":/icons/visible.lci");
//    blockHidden = QIcon(":/icons/invisible.svg");
    blockHidden = QIcon(":/icons/not_visible.lci");
}

int QG_BlockModel::rowCount ( const QModelIndex & /*parent*/ ) const {
    return listBlock.size();
}

QModelIndex QG_BlockModel::parent ( const QModelIndex & /*index*/ ) const {
    return QModelIndex();
}

QModelIndex QG_BlockModel::index ( int row, int column, const QModelIndex & /*parent*/ ) const {
    if ( row >= listBlock.size() || row < 0)
        return QModelIndex();
    return createIndex ( row, column);
}

bool blockLessThan(const RS_Block *s1, const RS_Block *s2) {
     return s1->getName() < s2->getName();
}

void QG_BlockModel::setBlockList(RS_BlockList* bl) {
    /* since 4.6 the recommended way is to use begin/endResetModel()
     * TNick <nicu.tofan@gmail.com>
     */
    beginResetModel();
    listBlock.clear();
    if (bl == nullptr){
        endResetModel();
        return;
    }
    for (int i=0; i<bl->count(); ++i) {
        if ( !bl->at(i)->isUndone() )
            listBlock.append(bl->at(i));
    }
    setActiveBlock(bl->getActive());
    std::sort( listBlock.begin(), listBlock.end(), blockLessThan);

    //called to force redraw
    endResetModel();
}


RS_Block *QG_BlockModel::getBlock( int row) const{
    if ( row >= listBlock.size() || row < 0)
        return nullptr;
    return listBlock.at(row);
}

QModelIndex QG_BlockModel::getIndex (RS_Block * blk) const{
    int row = listBlock.indexOf(blk);
    if (row<0)
        return QModelIndex();
    return createIndex ( row, NAME);
}

QVariant QG_BlockModel::data ( const QModelIndex & index, int role ) const {
    if (!index.isValid() || index.row() >= listBlock.size())
        return QVariant();

    RS_Block* blk = listBlock.at(index.row());

    if (role ==Qt::DecorationRole && index.column() == VISIBLE) {
        if (!blk->isFrozen()) {
            return blockVisible;
        } else {
            return blockHidden;
        }
    }
    if (role ==Qt::DisplayRole && index.column() == NAME) {
        return blk->getName();
    }
    if (role == Qt::FontRole && index.column() == NAME) {
        if (activeBlock && activeBlock == blk) {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
//Other roles:
    return QVariant();
}

 /**
 * Constructor.
 */
QG_BlockWidget::QG_BlockWidget(QG_ActionHandler* ah, QWidget* parent,
                               const char* name, Qt::WindowFlags f)
        : QWidget(parent, f) {

    setObjectName(name);
    actionHandler = ah;
    blockList = nullptr;
    lastBlock = nullptr;

    blockModel = new QG_BlockModel(this);
    blockView = new QTableView(this);
    blockView->setModel (blockModel);
    blockView->setShowGrid (false);
    blockView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    blockView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    blockView->setFocusPolicy(Qt::NoFocus);
    blockView->setColumnWidth(QG_BlockModel::VISIBLE, 20);
    blockView->verticalHeader()->hide();
    blockView->horizontalHeader()->setStretchLastSection(true);
    blockView->horizontalHeader()->hide();

    blockView->setStyleSheet("QWidget {background-color: white;}  QScrollBar{ background-color: none }");

    auto* lay = new QVBoxLayout(this);
    lay->setSpacing ( 2 );
    lay->setContentsMargins(2, 2, 2, 2);

    auto *layButtons = new LC_FlexLayout(0,5,5);

    // QHBoxLayout* layButtons = new QHBoxLayout();
    // QHBoxLayout* layButtons2 = new QHBoxLayout();
    QToolButton* but;
    // show all blocks:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/visible.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Show all blocks"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksDefreezeAll);
    layButtons->addWidget(but);
    // hide all blocks:
    but = new QToolButton(this);
//    but->setIcon( QIcon(":/icons/invisible.svg") );
    but->setIcon( QIcon(":/icons/not_visible.lci") );
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Hide all blocks"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksFreezeAll);
    layButtons->addWidget(but);
    // create block:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/create_block.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Create Block"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksCreate);
    layButtons->addWidget(but);
    // add block:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/add.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Add an empty block"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksAdd);
    layButtons->addWidget(but);
    // remove block:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/remove.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Remove block"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksRemove);
    layButtons->addWidget(but);
    // edit attributes:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/rename_active_block.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Rename the active block"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksAttributes);
      // layButtons2->addWidget(but);
    layButtons->addWidget(but);
    // edit block:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/properties.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Edit the active block\n"
                          "in a separate window"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksEdit);
    layButtons->addWidget(but);
    // save block:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/save.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("save the active block to a file"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksSave);
    layButtons->addWidget(but);
    // insert block:
    but = new QToolButton(this);
    but->setIcon(QIcon(":/icons/insert_active_block.lci"));
    // but->setMinimumSize(button_size);
    but->setToolTip(tr("Insert the active block"));
    connect(but, &QToolButton::clicked, actionHandler, &QG_ActionHandler::slotBlocksInsert);
    layButtons->addWidget(but);

    // lineEdit to filter block list with RegEx
    matchBlockName = new QLineEdit(this);
    matchBlockName->setReadOnly(false);
    matchBlockName->setPlaceholderText(tr("Filter"));
    matchBlockName->setClearButtonEnabled(true);
    matchBlockName->setToolTip(tr("Looking for matching block names"));
    connect(matchBlockName, &QLineEdit::textChanged, this, &QG_BlockWidget::slotUpdateBlockList);

    lay->addWidget(matchBlockName);
    lay->addLayout(layButtons);
    // lay->addLayout(layButtons2);
    lay->addWidget(blockView);

    connect(blockView, &QTableView::clicked, this, &QG_BlockWidget::slotActivated);
    connect(blockView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &QG_BlockWidget::slotSelectionChanged);

    updateWidgetSettings();
}


/**
 * Updates the block box from the blocks in the graphic.
 */
void QG_BlockWidget::update() {
    RS_DEBUG->print("QG_BlockWidget::update()");

    if (blockList==nullptr) {
        RS_DEBUG->print("QG_BlockWidget::update(): blockList is nullptr");
        blockModel->setActiveBlock(nullptr);
        return;
    }

    RS_Block* activeBlock = (blockList != nullptr) ? blockList->getActive() : nullptr;

    blockModel->setBlockList(blockList);

    RS_Block* b = lastBlock;
    activateBlock(activeBlock);
    lastBlock = b;
    blockView->resizeRowsToContents();

    restoreSelections();

    RS_DEBUG->print("QG_BlockWidget::update() done");
}


void QG_BlockWidget::restoreSelections() {

    QItemSelectionModel* selectionModel = blockView->selectionModel();

    for (auto block: *blockList) {
        if (block == nullptr || !block->isVisibleInBlockList() || !block->isSelectedInBlockList())
            continue;

        QModelIndex idx = blockModel->getIndex(block);
        QItemSelection selection(idx, idx);
        selectionModel->select(selection, QItemSelectionModel::Select);
    }
}


/**
 * Activates the given block and makes it the active
 * block in the blocklist.
 */
void QG_BlockWidget::activateBlock(RS_Block* block) {
    RS_DEBUG->print("QG_BlockWidget::activateBlock()");

    if (block==nullptr || blockList==nullptr) {
        return;
    }

    lastBlock = blockList->getActive();
    blockList->activate(block);
    int yPos = blockView->verticalScrollBar()->value();
    QModelIndex idx = blockModel->getIndex (block);

    // remember selected status of the block
    bool selected = block->isSelectedInBlockList();

    blockView->setCurrentIndex ( idx );
    blockModel->setActiveBlock(block);
    blockView->viewport()->update();

    // restore selected status of the block
    QItemSelectionModel::SelectionFlag selFlag = selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect;
    block->selectedInBlockList(selected);
    blockView->selectionModel()->select(QItemSelection(idx, idx), selFlag);
    blockView->verticalScrollBar()->setValue(yPos);
}

/**
 * Called when the user activates (highlights) a block.
 */
void QG_BlockWidget::slotActivated(QModelIndex blockIdx) {
    if (!blockIdx.isValid() || blockList==nullptr)
        return;

    RS_Block * block = blockModel->getBlock( blockIdx.row() );
    if (block == nullptr)
        return;

    if (blockIdx.column() == QG_BlockModel::VISIBLE) {
        RS_Block* b = blockList->getActive();
        blockList->activate(block);
        actionHandler->slotBlocksToggleView();
        activateBlock(b);
        return;
    }

    if (blockIdx.column() == QG_BlockModel::NAME) {
        lastBlock = blockList->getActive();
        activateBlock(block);
    }
}


/**
 * Called on blocks selection/deselection
 */
void QG_BlockWidget::slotSelectionChanged(
    const QItemSelection &selected,
    const QItemSelection &deselected)
{
    QItemSelectionModel *selectionModel {blockView->selectionModel()};

    foreach (QModelIndex index, selected.indexes()) {
        auto block = blockModel->getBlock(index.row());
        if (block) {
            block->selectedInBlockList(true);
            selectionModel->select(QItemSelection(index, index), QItemSelectionModel::Select);
        }
    }

    foreach (QModelIndex index, deselected.indexes()) {
        auto block = blockModel->getBlock(index.row());
        if (block && block->isVisibleInBlockList()) {
            block->selectedInBlockList(false);
            selectionModel->select(QItemSelection(index, index), QItemSelectionModel::Deselect);
        }
    }
}


/**
 * Shows a context menu for the block widget. Launched with a right click.
 */
void QG_BlockWidget::contextMenuEvent(QContextMenuEvent *e) {

    // select item (block) in Block List widget first because left-mouse-click event are not to be triggered
    // slotActivated(blockView->currentIndex());

    auto contextMenu = std::make_unique<QMenu>(this);

    using ActionMemberFunc = void (QG_ActionHandler::*)();
    const auto addActionFunc = [this, &contextMenu](const QString& name, ActionMemberFunc func) {
        contextMenu->addAction(name, actionHandler, func);
    };
    // Actions for all blocks:
    addActionFunc(tr("&Defreeze all Blocks"), &QG_ActionHandler::slotBlocksDefreezeAll);
    addActionFunc(tr("&Freeze all Blocks"), &QG_ActionHandler::slotBlocksFreezeAll);
    contextMenu->addSeparator();
    // Actions for selected blocks or,
    // if nothing is selected, for active block:
    addActionFunc(tr("&Toggle Visibility"), &QG_ActionHandler::slotBlocksToggleView);
    addActionFunc(tr("&Remove Block"), &QG_ActionHandler::slotBlocksRemove);
    contextMenu->addSeparator();
    // Single block actions:
    addActionFunc(tr("&Add Block"), &QG_ActionHandler::slotBlocksAdd);
    addActionFunc(tr("&Rename Block"), &QG_ActionHandler::slotBlocksAttributes);
    addActionFunc(tr("&Edit Block"), &QG_ActionHandler::slotBlocksEdit);
    addActionFunc(tr("&Insert Block"), &QG_ActionHandler::slotBlocksInsert);
    addActionFunc(tr("&Create New Block"), &QG_ActionHandler::slotBlocksCreate);
    contextMenu->exec(QCursor::pos());

    e->accept();
}



/**
 * Escape releases focus.
 */
void QG_BlockWidget::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
    case Qt::Key_Escape:
        emit escape();
        break;

    default:
        QWidget::keyPressEvent(e);
        break;
    }
}


void QG_BlockWidget::blockAdded(RS_Block*) {
    update();
    if (! matchBlockName->text().isEmpty()) {
        slotUpdateBlockList();
    }
}


/**
 * Called when reg-expression matchBlockName->text changed
 */
void QG_BlockWidget::slotUpdateBlockList() {

    if (!blockList) {
        return;
    }

    QRegularExpression rx{ QRegularExpression::wildcardToRegularExpression(matchBlockName->text())};

    for (int i = 0; i < blockList->count(); i++) {
        RS_Block* block = blockModel->getBlock(i);
        if (!block) continue;
        if (block->getName().indexOf(rx) == 0) {
            blockView->showRow(i);
            blockModel->getBlock(i)->visibleInBlockList(true);
        } else {
            blockView->hideRow(i);
            blockModel->getBlock(i)->visibleInBlockList(false);
        }
    }

    restoreSelections();
}

void QG_BlockWidget::updateWidgetSettings(){
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
