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

#ifndef QTMIR_SCREENADAPTOR_H
#define QTMIR_SCREENADAPTOR_H

#include "qtmir/screen.h"

#include <QObject>
#include <QPointer>
#include <QQmlListProperty>

class QScreen;
class ScreenConfig;
class ScreensController;

class ScreenAdaptor : public qtmir::Screen
{
    Q_OBJECT
public:
    ScreenAdaptor(QScreen* screen, QObject* parent = 0);
    ~ScreenAdaptor();

    miral::DisplayId displayId() const override;
    bool used() const override;
    QString name() const override;
    float scale() const override;
    QSizeF physicalSize() const override;
    qtmir::FormFactor formFactor() const override;
    qtmir::OutputTypes outputType() const override;
    MirPowerMode powerMode() const override;
    Qt::ScreenOrientation orientation() const override;
    QPoint position() const override;
    QQmlListProperty<qtmir::ScreenMode> availableModes() override;
    uint currentModeIndex() const override;
    bool isActive() const override;
    void setActive(bool active) override;

    QScreen *qscreen() const override;

    qtmir::ScreenConfiguration *beginConfiguration() const override;
    bool applyConfiguration(qtmir::ScreenConfiguration *configuration) override;

private Q_SLOTS:
    void updateScreenModes();

private:
    QList<qtmir::ScreenMode*> m_modes;
    QPointer<QScreen> m_screen;
    ScreensController *m_screensController;
};

#endif // QTMIR_SCREENADAPTOR_H
