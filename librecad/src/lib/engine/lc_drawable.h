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

#ifndef LIBRECAD_LC_DRAWABLE_H
#define LIBRECAD_LC_DRAWABLE_H

#include "rs.h"

class RS_Painter;

class LC_Drawable {
public:
    LC_Drawable();

    virtual void draw(RS_Painter *painter) = 0;

    // fixme - remove pattern offset - introduced in 6fe841c8fed4d16e68ac1512ebfa69473577c723, 11/10/2011

    /**
     * Must be overwritten to return the rtti of this entity
     * (e.g. RS2::EntityArc).
     */
    virtual RS2::EntityType rtti() const{
        return RS2::EntityUnknown;
    }

    bool is(RS2::EntityType rttiCandidate) const{
        return rtti() == rttiCandidate;
    }
};


#endif //LIBRECAD_LC_DRAWABLE_H
