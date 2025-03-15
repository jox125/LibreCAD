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

#ifndef LC_UCSLISTMODEL_H
#define LC_UCSLISTMODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include "lc_ucslistoptions.h"
#include "lc_ucslist.h"
#include "rs.h"

class LC_UCSListModel:public QAbstractTableModel{
    Q_OBJECT
public:
    explicit LC_UCSListModel(LC_UCSListOptions *modelOptions, QObject * parent = nullptr);

    ~LC_UCSListModel() override;
    void setUCSList(LC_UCSList *ucsList, RS2::LinearFormat format, RS2::AngleFormat angleFormat, int precision, int anglePrecision, RS2::Unit drawingUnit);

    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    LC_UCS *getItemForIndex(const QModelIndex &index) const;
    int translateColumn(int column) const;
    void fillUCSsList(QList<LC_UCS *> &list) const;
    QIcon getTypeIcon(LC_UCS *ucs) const;
    QIcon getOrthoTypeIcon(LC_UCS *ucs) const;
    QModelIndex getIndexForUCS(LC_UCS* ucs) const;
    void markActive(LC_UCS *ucs);
    LC_UCS* getActiveUCS(){
        LC_UCS* result = nullptr;
        if (ucsList != nullptr) {
            result = ucsList->getActive();
        }
        return result;
    }
    int count(){
        return ucss.count();
    }

    LC_UCS *getWCS();

    /**
   * Columns that are shown in the table
   */
    enum COLUMNS{
        ICON_TYPE,
        ICON_ORTHO_TYPE,
        NAME,
        UCS_DETAILS,
        LAST
    };

    QString getUCSInfo(LC_UCS *ucs);

    struct UCSItem{
        LC_UCS* ucs;
        QIcon iconType;
        QIcon iconGridType;
        QString ucsInfo;
        QString toolTip;
        QString displayName;
    };

protected:
    RS2::LinearFormat linearFormat;
    RS2::AngleFormat angleFormat;
    int anglePrec;
    int prec;
    RS2::Unit unit;
    LC_UCSList* ucsList {nullptr};
    QList<UCSItem*> ucss;
    QIcon iconWCS;
    QIcon iconUCS;
    QIcon iconGridOrtho;
    QIcon iconGridISOTop;
    QIcon iconGridISOLeft;
    QIcon iconGridISORight;
    QString getGridViewType(int orthoType);
    LC_UCSListOptions* options {nullptr};
    UCSItem* createUCSItem(LC_UCS *view);
};

#endif // LC_UCSLISTMODEL_H
