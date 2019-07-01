/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#ifndef SCREENPLATFORMWINDOW_H
#define SCREENPLATFORMWINDOW_H

#include <qpa/qplatformwindow.h>

// ScreenPlatformWindow implements the basics of a QPlatformWindow.
// QtMir enforces one Window per Screen, so Window and Screen are tightly coupled.
// All Mir specifics live in the associated Screen object.

class ScreenPlatformWindow : public QObject,
                             public QPlatformWindow
{
    Q_OBJECT
public:
    explicit ScreenPlatformWindow(QWindow *window, bool exposed);
    virtual ~ScreenPlatformWindow();

    void setVisible(bool visible) override;
    void setGeometry(const QRect &rect) override;
    void requestActivateWindow() override;

    bool isExposed() const override;
    bool isActive() const override;
    void setExposed(const bool exposed);

    WId winId() const override { return m_winId; }

    void setScreen(QPlatformScreen *screen);

    void swapBuffers();
    void makeCurrent();
    void doneCurrent();

public Q_SLOTS:
    void setActive(bool active);

private Q_SLOTS:
    void updateExpose();

private:
    void setPrimary(const bool primary);

    bool m_exposed;
    bool m_primary;
    bool m_active;
    WId m_winId;
};

#endif // SCREENPLATFORMWINDOW_H
