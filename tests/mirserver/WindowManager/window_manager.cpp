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
 *
 */

#include "mirwindowmanager.h"
#include <mir/shell/display_layout.h>
#include <mir/scene/surface_creation_parameters.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../../../../../../../usr/include/mircommon/mir/geometry/rectangle.h"

namespace mf = mir::frontend;
namespace ms = mir::scene;
namespace msh = mir::shell;

using namespace mir::geometry;
using namespace testing;

namespace
{
struct MockDisplayLayout : msh::DisplayLayout
{
    MOCK_METHOD1(clip_to_output, void (Rectangle& rect));
    MOCK_METHOD1(size_to_output, void (Rectangle& rect));
    MOCK_METHOD2(place_in_output, void (mir::graphics::DisplayConfigurationOutputId id, Rectangle& rect));
};

struct WindowManager : Test
{
    const std::shared_ptr<MockDisplayLayout> mock_display_layout =
        std::make_shared<NiceMock<MockDisplayLayout>>();

    std::unique_ptr<MirWindowManager> window_manager =
        MirWindowManager::create(nullptr, mock_display_layout);

    std::shared_ptr<ms::Session> const arbitrary_session;
    ms::SurfaceCreationParameters const arbitrary_params;
    mf::SurfaceId const arbitrary_surface_id{__LINE__};

    MOCK_METHOD2(build_surface, mf::SurfaceId(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params));

    void SetUp() override
    {
        ON_CALL(*this, build_surface(_, _)).WillByDefault(Return(arbitrary_surface_id));
    }
};
}

TEST_F(WindowManager, creates_surface_using_supplied_builder)
{
    EXPECT_CALL(*this, build_surface(_, _));

    auto const surface = window_manager->add_surface(
        arbitrary_session,
        arbitrary_params,
        [this](std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)
            {
                return build_surface(session, params);
            });

    EXPECT_THAT(surface, Eq(arbitrary_surface_id));
}

TEST_F(WindowManager, sizes_new_surface_to_output)
{
    EXPECT_CALL(*this, build_surface(_, _)).Times(AnyNumber());

    const Size request_size{0, 0};
    const Size expect_size{57, 91};

    ms::SurfaceCreationParameters params;
    params.size = request_size;

    EXPECT_CALL(*mock_display_layout, size_to_output(_)).
        WillOnce(Invoke([&](Rectangle& rect)
            {
                EXPECT_THAT(rect.size, Eq(request_size));
                rect.size = expect_size;
            }));

    window_manager->add_surface(
        arbitrary_session,
        arbitrary_params,
        [this](std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)
            {
                return build_surface(session, params);
            });
}
