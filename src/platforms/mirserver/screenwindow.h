/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#ifndef SCREENWINDOW_H
#define SCREENWINDOW_H

#include <qpa/qplatformwindow.h>

#include <QMap>
#include <QMutex>
#include <QRect>

// ScreenWindow implements the basics of a QPlatformWindow.
// QtMir enforces one Window per Screen, so Window and Screen are tightly coupled.
// All Mir specifics live in the associated Screen object.
class ScreenWindow : public QPlatformWindow
{
public:
    explicit ScreenWindow(QWindow *window);
    virtual ~ScreenWindow();

    static ScreenWindow *findWithWId(WId);

    bool isExposed() const override;
    void setExposed(const bool exposed);

    WId winId() const override { return m_winId; }

    void setScreen(QPlatformScreen *screen);

    void swapBuffers();
    void makeCurrent();
    void doneCurrent();

    QRect availableDesktopArea() const;
    void setAvailableDesktopArea(const QRect &rect);

    QRect normalWindowMargins() const;
    void setNormalWindowMargins(const QRect &rect);

    QRect dialogWindowMargins() const;
    void setDialogWindowMargins(const QRect &rect);

private:
    bool m_exposed;
    WId m_winId;
    QRect m_availableDesktopArea;
    QRect m_normalWindowMargins;
    QRect m_dialogWindowMargins;
    mutable QMutex m_mutex;

    static QMap<WId, ScreenWindow*> m_idToWindowMap;
};

#endif // SCREENWINDOW_H
