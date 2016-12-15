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

#include <QSignalSpy>

#include "mirwindowmanager.h"
#include "stub_surface.h"
#include "stub_session.h"
#include "surfaceobserver.h"

#include <mir/events/event_builders.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/shell/display_layout.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace mf = mir::frontend;
namespace ms = mir::scene;
namespace msh = mir::shell;

using namespace mir::geometry;
using namespace testing;

using mir::events::make_event;

namespace
{
struct MockDisplayLayout : msh::DisplayLayout
{
    MOCK_METHOD1(clip_to_output, void (Rectangle& rect));
    MOCK_METHOD1(size_to_output, void (Rectangle& rect));
    MOCK_METHOD2(place_in_output, bool (mir::graphics::DisplayConfigurationOutputId id, Rectangle& rect));
};

struct MockSurface : StubSurface
{
    MOCK_METHOD2(configure, int (MirSurfaceAttrib attrib, int value));
    MOCK_METHOD1(rename, void (std::string const& title));
};

struct MockSession : StubSession
{
    MOCK_CONST_METHOD1(surface, std::shared_ptr<ms::Surface> (mir::frontend::SurfaceId surface));
    void send_error(mir::ClientVisibleError const&) {}
};

struct WindowManager : Test
{
    const std::shared_ptr<MockDisplayLayout> mock_display_layout =
        std::make_shared<NiceMock<MockDisplayLayout>>();

    std::shared_ptr<SessionListener> sessionListener = std::make_shared<SessionListener>();

    const std::shared_ptr<MirWindowManager> window_manager =
        MirWindowManager::create(mock_display_layout, sessionListener);

    const Rectangle arbitrary_display{{0, 0}, {97, 101}};
    const std::shared_ptr<MockSession> arbitrary_session = std::make_shared<NiceMock<MockSession>>();
    const std::shared_ptr<ms::Surface> arbitrary_surface = std::make_shared<StubSurface>();
    const ms::SurfaceCreationParameters arbitrary_params;
    const mf::SurfaceId arbitrary_surface_id{__LINE__};

    MOCK_METHOD2(build_surface, mf::SurfaceId(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params));

    void SetUp() override
    {
        ON_CALL(*this, build_surface(_, _)).WillByDefault(Return(arbitrary_surface_id));
        ON_CALL(*arbitrary_session, surface(_)).WillByDefault(Return(arbitrary_surface));

        window_manager->add_session(arbitrary_session);
    }

    void TearDown() override
    {
        window_manager->remove_session(arbitrary_session);
    }

    void add_surface()
    {
        EXPECT_CALL(*this, build_surface(_, _));

        window_manager->add_surface(
            arbitrary_session,
            arbitrary_params,
            [this](std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)
                {
                    return build_surface(session, params);
                });
    }

    std::vector<uint8_t> const arbitrary_cookie;
};
}

TEST_F(WindowManager, CreatesSurfaceUsingSuppliedBuilder)
{
    EXPECT_CALL(*this, build_surface(_, _));

    const auto surface = window_manager->add_surface(
        arbitrary_session,
        arbitrary_params,
        [this](std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)
            {
                return build_surface(session, params);
            });

    EXPECT_THAT(surface, Eq(arbitrary_surface_id));
}

TEST_F(WindowManager, SizesNewSurfaceToOutput)
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

    add_surface();
}

namespace
{
struct AttribValuePair
{
    MirSurfaceAttrib attribute;
    int value;
    friend std::ostream& operator<<(std::ostream& out, AttribValuePair const& pair)
    { return out << "attribute:" << pair.attribute << ", value:" << pair.value; }
};
struct SetAttribute : WindowManager, ::testing::WithParamInterface<AttribValuePair> {};
}

TEST_P(SetAttribute, ConfiguresSurface)
{
    const auto attribute = GetParam().attribute;
    const auto value = GetParam().value;

    const auto surface = std::make_shared<MockSurface>();

    EXPECT_CALL(*arbitrary_session, surface(_)).Times(AnyNumber()).WillRepeatedly(Return(surface));
    add_surface();

    EXPECT_CALL(*surface, configure(attribute, value)).WillOnce(Return(value));

    window_manager->set_surface_attribute(arbitrary_session, surface, attribute, value);
}

INSTANTIATE_TEST_CASE_P(WindowManager, SetAttribute,
    Values(
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_restored},
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_minimized},
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_maximized},
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_vertmaximized},
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_fullscreen},
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_horizmaximized},
        AttribValuePair{mir_surface_attrib_state, mir_surface_state_hidden},

        AttribValuePair{mir_surface_attrib_type, mir_surface_type_normal},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_utility},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_dialog},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_overlay},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_freestyle},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_popover},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_inputmethod},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_satellite},
        AttribValuePair{mir_surface_attrib_type, mir_surface_type_tip},

        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_portrait},
        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_landscape},
        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_portrait_inverted},
        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_landscape_inverted},
        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_portrait_any},
        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_landscape_any},
        AttribValuePair{mir_surface_attrib_preferred_orientation, mir_orientation_mode_any}
    ));

// The following calls are /currently/ ignored, but we can check they don't "blow up"
TEST_F(WindowManager, HandlesAddSession)
{
    EXPECT_NO_THROW(window_manager->add_session(arbitrary_session));
}

TEST_F(WindowManager, HandlesRemoveSession)
{
    EXPECT_NO_THROW(window_manager->remove_session(arbitrary_session));
}

TEST_F(WindowManager, HandlesAddDisplay)
{
    EXPECT_NO_THROW(window_manager->add_display(arbitrary_display));
}

TEST_F(WindowManager, HandlesRemoveDisplay)
{
    EXPECT_NO_THROW(window_manager->remove_display(arbitrary_display));
}

TEST_F(WindowManager, HandlesModifySurface)
{
    const auto surface = std::make_shared<MockSurface>();
    EXPECT_CALL(*arbitrary_session, surface(_)).Times(AnyNumber()).WillRepeatedly(Return(surface));
    add_surface();
    std::string title = "TestSurface";

    SurfaceObserver surfaceObserver;
    SurfaceObserver::registerObserverForSurface(&surfaceObserver, surface.get());

    mir::shell::SurfaceSpecification modifications;
    modifications.min_width = mir::geometry::Width(100);
    modifications.min_height = mir::geometry::Height(101);
    modifications.max_width = mir::geometry::Width(102);
    modifications.max_height = mir::geometry::Height(103);
    modifications.shell_chrome = mir_shell_chrome_low;
    modifications.name = title;

    QSignalSpy spyMinimumWidthChanged(&surfaceObserver, SIGNAL(minimumWidthChanged(int)));
    QSignalSpy spyMinimumHeightChanged(&surfaceObserver, SIGNAL(minimumHeightChanged(int)));
    QSignalSpy spyMaximumWidthChanged(&surfaceObserver, SIGNAL(maximumWidthChanged(int)));
    QSignalSpy spyMaximumHeightChanged(&surfaceObserver, SIGNAL(maximumHeightChanged(int)));
    QSignalSpy spyShellChromeChanged(&surfaceObserver, SIGNAL(shellChromeChanged(MirShellChrome)));
    EXPECT_CALL(*surface, rename(title));

    window_manager->modify_surface(arbitrary_session, surface, modifications);

    EXPECT_EQ(100, spyMinimumWidthChanged.takeFirst().at(0).toInt());
    EXPECT_EQ(101, spyMinimumHeightChanged.takeFirst().at(0).toInt());
    EXPECT_EQ(102, spyMaximumWidthChanged.takeFirst().at(0).toInt());
    EXPECT_EQ(103, spyMaximumHeightChanged.takeFirst().at(0).toInt());
    EXPECT_EQ(mir_shell_chrome_low, spyShellChromeChanged.takeFirst().at(0).toInt());

    window_manager->remove_surface(arbitrary_session, surface);
}

TEST_F(WindowManager, HandlesKeyboardEvent)
{
    const MirInputDeviceId arbitrary_device{0};
    const auto arbitrary_timestamp = std::chrono::steady_clock().now().time_since_epoch();
    const auto arbitrary_action = mir_keyboard_action_down;
    const xkb_keysym_t arbitrary_key_code{0};
    const int arbitrary_scan_code = 0;
    const MirInputEventModifiers arbitrary_event_modifiers{0};

    const auto generic_event = make_event(
        arbitrary_device,
        arbitrary_timestamp,
        arbitrary_cookie,
        arbitrary_action,
        arbitrary_key_code,
        arbitrary_scan_code,
        arbitrary_event_modifiers);

    const auto input_event = mir_event_get_input_event(generic_event.get());
    const auto event = mir_input_event_get_keyboard_event(input_event);

    EXPECT_NO_THROW(window_manager->handle_keyboard_event(event));
}

TEST_F(WindowManager, HandlesTouchEvent)
{
    const MirInputDeviceId arbitrary_device{0};
    const auto arbitrary_timestamp = std::chrono::steady_clock().now().time_since_epoch();
    const MirInputEventModifiers arbitrary_event_modifiers{0};

    const auto generic_event = make_event(
        arbitrary_device,
        arbitrary_timestamp,
        arbitrary_cookie,
        arbitrary_event_modifiers);

    const auto input_event = mir_event_get_input_event(generic_event.get());
    const auto event = mir_input_event_get_touch_event(input_event);

    EXPECT_NO_THROW(window_manager->handle_touch_event(event));
}

TEST_F(WindowManager, HandlesPointerEvent)
{
    const MirInputDeviceId arbitrary_device{0};
    const auto arbitrary_timestamp = std::chrono::steady_clock().now().time_since_epoch();
    const MirInputEventModifiers arbitrary_event_modifiers{0};
    const auto arbitrary_pointer_action = mir_pointer_action_button_down;
    const auto arbitrary_pointer_buttons = mir_pointer_button_primary;
    const float arbitrary_x_axis_value{0};
    const float arbitrary_y_axis_value{0};
    const float arbitrary_hscroll_value{0};
    const float arbitrary_vscroll_value{0};
    const float arbitrary_relative_x_value{0};
    const float arbitrary_relative_y_value{0};

    const auto generic_event = make_event(
        arbitrary_device,
        arbitrary_timestamp,
        arbitrary_cookie,
        arbitrary_event_modifiers,
        arbitrary_pointer_action,
        arbitrary_pointer_buttons,
        arbitrary_x_axis_value,
        arbitrary_y_axis_value,
        arbitrary_hscroll_value,
        arbitrary_vscroll_value,
        arbitrary_relative_x_value,
        arbitrary_relative_y_value);

    const auto input_event = mir_event_get_input_event(generic_event.get());
    const auto event = mir_input_event_get_pointer_event(input_event);

    EXPECT_NO_THROW(window_manager->handle_pointer_event(event));
}
