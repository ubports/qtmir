/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef STUB_DISPLAY_H
#define STUB_DISPLAY_H

#include "mock_display.h"
#include "mock_display_buffer.h"
#include "mock_display_configuration.h"

namespace mg = mir::graphics;
namespace geom = mir::geometry;

class StubDisplayConfiguration : public MockDisplayConfiguration
{
public:
    StubDisplayConfiguration(const std::vector<mg::DisplayConfigurationOutput> &config)
        : m_config(config)
    {}

    void for_each_output(std::function<void(mg::DisplayConfigurationOutput const&)> f) const override
    {
        for (auto config : m_config) {
            f(config);
        }
    }

private:
    const std::vector<mg::DisplayConfigurationOutput> m_config;
};


class StubDisplaySyncGroup : public MockDisplaySyncGroup
{
public:
    StubDisplaySyncGroup(MockDisplayBuffer *buffer) : buffer(buffer) {}

    void for_each_display_buffer(std::function<void(mg::DisplayBuffer&)> const& f) override
    {
        f(*buffer);
    }
private:
    MockDisplayBuffer *buffer;
};


class StubDisplay : public MockDisplay
{
public:
    // Note, GMock cannot mock functions which return non-copyable objects, so stubbing needed
    std::unique_ptr<mg::DisplayConfiguration> configuration() const override
    {
        return std::unique_ptr<mg::DisplayConfiguration>(
            new StubDisplayConfiguration(m_config)
        );
    }

    void for_each_display_sync_group(std::function<void(mg::DisplaySyncGroup&)> const& f) override
    {
        for (auto displayBuffer : m_displayBuffers) {
            StubDisplaySyncGroup b(displayBuffer);
            f(b);
        }
    }

    void setFakeConfiguration(std::vector<mg::DisplayConfigurationOutput> &config,
                              std::vector<MockDisplayBuffer*> &displayBuffers)
    {
        m_config = config;
        m_displayBuffers = displayBuffers;
    }

private:
    std::vector<mg::DisplayConfigurationOutput> m_config;
    std::vector<MockDisplayBuffer*> m_displayBuffers;
};

#endif // STUB_DISPLAY_H
