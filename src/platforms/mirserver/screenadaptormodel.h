/*
 * Copyright (C) 2017 Canonical, Ltd.
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

#ifndef SCREENADAPTORMODEL_H
#define SCREENADAPTORMODEL_H

#include "qtmir/screens.h"

namespace qtmir
{
class Screen;
}

class ScreenAdaptorModel : public qtmir::Screens
{
public:
    ScreenAdaptorModel(QObject* parent = 0);
    ~ScreenAdaptorModel();

    QVector<qtmir::Screen*> screens() const { return m_screenList; }

private Q_SLOTS:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);

private:
    QVector<qtmir::Screen*> m_screenList;
};

#endif // SCREENADAPTORMODEL_H
