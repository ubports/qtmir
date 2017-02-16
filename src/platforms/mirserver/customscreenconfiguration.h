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

#ifndef CUSTOMSCREENCONFIGURATION_H
#define CUSTOMSCREENCONFIGURATION_H

#include "qtmir/screen.h"

#include <QVector>

struct CustomScreenConfiguration
{
    bool valid{false};
    qtmir::OutputId id;

    bool used;
    QPoint topLeft;
    uint32_t currentModeIndex;
    MirPowerMode powerMode;
    MirOrientation orientation;
    float scale;
    qtmir::FormFactor formFactor;
};

class ScreenConfiguration : public qtmir::ScreenConfig,
                            public CustomScreenConfiguration
{
public:
    ScreenConfiguration() = default;
    ScreenConfiguration(const CustomScreenConfiguration& other)
    {
        (CustomScreenConfiguration&)*this = other;
    }
    ScreenConfiguration(const qtmir::ScreenConfig& other)
    {
        CustomScreenConfiguration::valid = other.valid();
        CustomScreenConfiguration::id = other.outputId();
        CustomScreenConfiguration::used = other.used();
        CustomScreenConfiguration::topLeft = other.topLeft();
        CustomScreenConfiguration::currentModeIndex = other.currentModeIndex();
        CustomScreenConfiguration::powerMode = other.powerMode();
        CustomScreenConfiguration::orientation = other.orientation();
        CustomScreenConfiguration::scale = other.scale();
        CustomScreenConfiguration::formFactor = other.formFactor();
    }

    bool valid() const override { return CustomScreenConfiguration::valid; }
    qtmir::OutputId outputId() const override { return CustomScreenConfiguration::id; }

    bool used() const override { return CustomScreenConfiguration::used; }
    void setUsed(bool _used) override { CustomScreenConfiguration::used = _used; }

    QPoint topLeft() const override { return CustomScreenConfiguration::topLeft; }
    void setTopLeft(const QPoint& _topLeft) override { CustomScreenConfiguration::topLeft = _topLeft; }

    uint32_t currentModeIndex() const override { return CustomScreenConfiguration::currentModeIndex; }
    void setCurrentModeIndex(uint32_t _index) override { CustomScreenConfiguration::currentModeIndex = _index; }

    float scale() const override { return CustomScreenConfiguration::scale; }
    void setScale(float _scale) override { CustomScreenConfiguration::scale = _scale; }

    qtmir::FormFactor formFactor() const override { return CustomScreenConfiguration::formFactor; }
    void setFormFactor(qtmir::FormFactor _formFactor) override { CustomScreenConfiguration::formFactor = _formFactor; }

    MirPowerMode powerMode() const override { return CustomScreenConfiguration::powerMode; }
    MirOrientation orientation() const override  { return CustomScreenConfiguration::orientation; }
};

typedef QVector<CustomScreenConfiguration> CustomScreenConfigurationList;

#endif // CUSTOMSCREENCONFIGURATION_H
