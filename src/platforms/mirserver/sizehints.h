/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QTMIR_SIZEHINTS_H
#define QTMIR_SIZEHINTS_H

#include <QMetaType>
#include <QString>

namespace mir {
    namespace scene {
        class SurfaceCreationParameters;
    }
}

namespace qtmir {

class SizeHints {
public:
    SizeHints() {}
    SizeHints(const mir::scene::SurfaceCreationParameters&);

    QString toString() const;

    int minWidth{0};
    int maxWidth{0};

    int minHeight{0};
    int maxHeight{0};

    int widthIncrement{0};
    int heightIncrement{0};
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::SizeHints)

#endif // QTMIR_SIZEHINTS_H
