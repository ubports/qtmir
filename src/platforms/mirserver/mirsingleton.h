/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#ifndef QTMIR_MIRSINGLETON_H
#define QTMIR_MIRSINGLETON_H

// unity-api
#include <unity/shell/application/Mir.h>

namespace qtmir {

class Mir : public ::Mir
{
    Q_OBJECT
    Q_PROPERTY(QString currentKeymap READ currentKeymap WRITE setCurrentKeymap NOTIFY currentKeymapChanged)
public:
    virtual ~Mir();

    static Mir *instance();

    void setCursorName(const QString &cursorName) override;
    QString cursorName() const override;

    QString currentKeymap() const;
    void setCurrentKeymap(const QString &currentKeymap);

Q_SIGNALS:
    void currentKeymapChanged(const QString &currentKeymap);

private:
    Mir();
    Q_DISABLE_COPY(Mir)

    QString m_cursorName;
    QString m_currentKeymap;
    static qtmir::Mir *m_instance;
};

} // namespace qtmir

#endif // QTMIR_MIRSINGLETON_H
