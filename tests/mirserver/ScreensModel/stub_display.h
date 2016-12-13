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

#ifndef STUB_DISPLAY_H
#define STUB_DISPLAY_H

#include <mir/test/doubles/null_display.h>
#include <mir/test/doubles/null_display_sync_group.h>
namespace mg = mir::graphics; // Bug lp:1614983
#include <mir/test/doubles/mock_display_configuration.h>

#include <mir/compositor/display_listener.h>

namespace geom = mir::geometry;

using NullDisplay = mir::test::doubles::NullDisplay;
using MockDisplayConfiguration = mir::test::doubles::MockDisplayConfiguration;

class MockGLDisplayBuffer;

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


class StubDisplaySyncGroup : public mir::test::doubles::NullDisplaySyncGroup
{
public:
    StubDisplaySyncGroup(mg::DisplayBuffer *buffer) : m_buffer(buffer) {}

    void for_each_display_buffer(std::function<void(mg::DisplayBuffer&)> const& f) override
    {
        f(*m_buffer);
    }

private:
    mg::DisplayBuffer *m_buffer;
};


class StubDisplay : public NullDisplay
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
            StubDisplaySyncGroup b(reinterpret_cast<mg::DisplayBuffer *>(displayBuffer));
            f(b);
        }
    }

    void setFakeConfiguration(std::vector<mg::DisplayConfigurationOutput> &config,
                              std::vector<MockGLDisplayBuffer*> displayBuffers)
    {
        m_config = config;
        m_displayBuffers = displayBuffers;
    }

private:
    std::vector<mg::DisplayConfigurationOutput> m_config;
    std::vector<MockGLDisplayBuffer*> m_displayBuffers;
};

class StubDisplayListener : public mir::compositor::DisplayListener
{
    void add_display(mir::geometry::Rectangle const& /*area*/) override {};

    void remove_display(mir::geometry::Rectangle const& /*area*/) override {};
};

#endif // STUB_DISPLAY_H
