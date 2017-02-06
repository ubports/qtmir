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

#include "qteventfeeder.h"
#include "cursor.h"
#include "eventbuilder.h"
#include "logging.h"
#include "timestamp.h"
#include "tracepoints.h" // generated from tracepoints.tp
#include "screensmodel.h"

#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qwindowsysteminterface_p.h>
#include <QGuiApplication>
#include <QTextCodec>
#include <QDebug>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

// common dir
#include <debughelpers.h>

using namespace qtmir;

// XKB Keysyms which do not map directly to Qt types (i.e. Unicode points)
static const uint32_t KeyTable[] = {
    // misc keys
    XKB_KEY_Escape,             Qt::Key_Escape,
    XKB_KEY_Tab,                Qt::Key_Tab,
    XKB_KEY_ISO_Left_Tab,       Qt::Key_Backtab,
    XKB_KEY_BackSpace,          Qt::Key_Backspace,
    XKB_KEY_Return,             Qt::Key_Return,
    XKB_KEY_Insert,             Qt::Key_Insert,
    XKB_KEY_Delete,             Qt::Key_Delete,
    XKB_KEY_Clear,              Qt::Key_Delete,
    XKB_KEY_Pause,              Qt::Key_Pause,
    XKB_KEY_Print,              Qt::Key_Print,
    0x1005FF60,                 Qt::Key_SysReq,         // hardcoded Sun SysReq
    0x1007ff00,                 Qt::Key_SysReq,         // hardcoded X386 SysReq

    // cursor movement

    XKB_KEY_Home,               Qt::Key_Home,
    XKB_KEY_End,                Qt::Key_End,
    XKB_KEY_Left,               Qt::Key_Left,
    XKB_KEY_Up,                 Qt::Key_Up,
    XKB_KEY_Right,              Qt::Key_Right,
    XKB_KEY_Down,               Qt::Key_Down,
    XKB_KEY_Prior,              Qt::Key_PageUp,
    XKB_KEY_Next,               Qt::Key_PageDown,

    // modifiers

    XKB_KEY_Shift_L,            Qt::Key_Shift,
    XKB_KEY_Shift_R,            Qt::Key_Shift,
    XKB_KEY_Shift_Lock,         Qt::Key_Shift,
    XKB_KEY_Control_L,          Qt::Key_Control,
    XKB_KEY_Control_R,          Qt::Key_Control,
    XKB_KEY_Meta_L,             Qt::Key_Meta,
    XKB_KEY_Meta_R,             Qt::Key_Meta,
    XKB_KEY_Alt_L,              Qt::Key_Alt,
    XKB_KEY_Alt_R,              Qt::Key_Alt,
    XKB_KEY_Caps_Lock,          Qt::Key_CapsLock,
    XKB_KEY_Num_Lock,           Qt::Key_NumLock,
    XKB_KEY_Scroll_Lock,        Qt::Key_ScrollLock,
    XKB_KEY_Super_L,            Qt::Key_Super_L,
    XKB_KEY_Super_R,            Qt::Key_Super_R,
    XKB_KEY_Menu,               Qt::Key_Menu,
    XKB_KEY_Hyper_L,            Qt::Key_Hyper_L,
    XKB_KEY_Hyper_R,            Qt::Key_Hyper_R,
    XKB_KEY_Help,               Qt::Key_Help,
    0x1000FF74,                 Qt::Key_Backtab,        // hardcoded HP backtab
    0x1005FF10,                 Qt::Key_F11,            // hardcoded Sun F36 (labeled F11)
    0x1005FF11,                 Qt::Key_F12,            // hardcoded Sun F37 (labeled F12)

    // numeric and function keypad keys

    XKB_KEY_KP_Space,           Qt::Key_Space,
    XKB_KEY_KP_Tab,             Qt::Key_Tab,
    XKB_KEY_KP_Enter,           Qt::Key_Enter,
    //XKB_KEY_KP_F1,            Qt::Key_F1,
    //XKB_KEY_KP_F2,            Qt::Key_F2,
    //XKB_KEY_KP_F3,            Qt::Key_F3,
    //XKB_KEY_KP_F4,            Qt::Key_F4,
    XKB_KEY_KP_Home,            Qt::Key_Home,
    XKB_KEY_KP_Left,            Qt::Key_Left,
    XKB_KEY_KP_Up,              Qt::Key_Up,
    XKB_KEY_KP_Right,           Qt::Key_Right,
    XKB_KEY_KP_Down,            Qt::Key_Down,
    XKB_KEY_KP_Prior,           Qt::Key_PageUp,
    XKB_KEY_KP_Next,            Qt::Key_PageDown,
    XKB_KEY_KP_End,             Qt::Key_End,
    XKB_KEY_KP_Begin,           Qt::Key_Clear,
    XKB_KEY_KP_Insert,          Qt::Key_Insert,
    XKB_KEY_KP_Delete,          Qt::Key_Delete,
    XKB_KEY_KP_Equal,           Qt::Key_Equal,
    XKB_KEY_KP_Multiply,        Qt::Key_Asterisk,
    XKB_KEY_KP_Add,             Qt::Key_Plus,
    XKB_KEY_KP_Separator,       Qt::Key_Comma,
    XKB_KEY_KP_Subtract,        Qt::Key_Minus,
    XKB_KEY_KP_Decimal,         Qt::Key_Period,
    XKB_KEY_KP_Divide,          Qt::Key_Slash,

    // International input method support keys

    // International & multi-key character composition
    XKB_KEY_ISO_Level3_Shift,   Qt::Key_AltGr,
    XKB_KEY_Multi_key,          Qt::Key_Multi_key,
    XKB_KEY_Codeinput,          Qt::Key_Codeinput,
    XKB_KEY_SingleCandidate,    Qt::Key_SingleCandidate,
    XKB_KEY_MultipleCandidate,  Qt::Key_MultipleCandidate,
    XKB_KEY_PreviousCandidate,  Qt::Key_PreviousCandidate,

    // Misc Functions
    XKB_KEY_Mode_switch,        Qt::Key_Mode_switch,
    XKB_KEY_script_switch,      Qt::Key_Mode_switch,

    // Japanese keyboard support
    XKB_KEY_Kanji,              Qt::Key_Kanji,
    XKB_KEY_Muhenkan,           Qt::Key_Muhenkan,
    //XKB_KEY_Henkan_Mode,      Qt::Key_Henkan_Mode,
    XKB_KEY_Henkan_Mode,        Qt::Key_Henkan,
    XKB_KEY_Henkan,             Qt::Key_Henkan,
    XKB_KEY_Romaji,             Qt::Key_Romaji,
    XKB_KEY_Hiragana,           Qt::Key_Hiragana,
    XKB_KEY_Katakana,           Qt::Key_Katakana,
    XKB_KEY_Hiragana_Katakana,  Qt::Key_Hiragana_Katakana,
    XKB_KEY_Zenkaku,            Qt::Key_Zenkaku,
    XKB_KEY_Hankaku,            Qt::Key_Hankaku,
    XKB_KEY_Zenkaku_Hankaku,    Qt::Key_Zenkaku_Hankaku,
    XKB_KEY_Touroku,            Qt::Key_Touroku,
    XKB_KEY_Massyo,             Qt::Key_Massyo,
    XKB_KEY_Kana_Lock,          Qt::Key_Kana_Lock,
    XKB_KEY_Kana_Shift,         Qt::Key_Kana_Shift,
    XKB_KEY_Eisu_Shift,         Qt::Key_Eisu_Shift,
    XKB_KEY_Eisu_toggle,        Qt::Key_Eisu_toggle,
    //XKB_KEY_Kanji_Bangou,     Qt::Key_Kanji_Bangou,
    //XKB_KEY_Zen_Koho,         Qt::Key_Zen_Koho,
    //XKB_KEY_Mae_Koho,         Qt::Key_Mae_Koho,
    XKB_KEY_Kanji_Bangou,       Qt::Key_Codeinput,
    XKB_KEY_Zen_Koho,           Qt::Key_MultipleCandidate,
    XKB_KEY_Mae_Koho,           Qt::Key_PreviousCandidate,

#ifdef XKB_KEY_KOREAN
    // Korean keyboard support
    XKB_KEY_Hangul,                  Qt::Key_Hangul,
    XKB_KEY_Hangul_Start,            Qt::Key_Hangul_Start,
    XKB_KEY_Hangul_End,              Qt::Key_Hangul_End,
    XKB_KEY_Hangul_Hanja,            Qt::Key_Hangul_Hanja,
    XKB_KEY_Hangul_Jamo,             Qt::Key_Hangul_Jamo,
    XKB_KEY_Hangul_Romaja,           Qt::Key_Hangul_Romaja,
    //XKB_KEY_Hangul_Codeinput,      Qt::Key_Hangul_Codeinput,
    XKB_KEY_Hangul_Codeinput,        Qt::Key_Codeinput,
    XKB_KEY_Hangul_Jeonja,           Qt::Key_Hangul_Jeonja,
    XKB_KEY_Hangul_Banja,            Qt::Key_Hangul_Banja,
    XKB_KEY_Hangul_PreHanja,         Qt::Key_Hangul_PreHanja,
    XKB_KEY_Hangul_PostHanja,        Qt::Key_Hangul_PostHanja,
    //XKB_KEY_Hangul_SingleCandidate,Qt::Key_Hangul_SingleCandidate,
    //XKB_KEY_Hangul_MultipleCandidate,Qt::Key_Hangul_MultipleCandidate,
    //XKB_KEY_Hangul_PreviousCandidate,Qt::Key_Hangul_PreviousCandidate,
    XKB_KEY_Hangul_SingleCandidate,  Qt::Key_SingleCandidate,
    XKB_KEY_Hangul_MultipleCandidate,Qt::Key_MultipleCandidate,
    XKB_KEY_Hangul_PreviousCandidate,Qt::Key_PreviousCandidate,
    XKB_KEY_Hangul_Special,          Qt::Key_Hangul_Special,
    //XKB_KEY_Hangul_switch,         Qt::Key_Hangul_switch,
    XKB_KEY_Hangul_switch,           Qt::Key_Mode_switch,
#endif  // XKB_KEY_KOREAN

    // dead keys
    XKB_KEY_dead_grave,              Qt::Key_Dead_Grave,
    XKB_KEY_dead_acute,              Qt::Key_Dead_Acute,
    XKB_KEY_dead_circumflex,         Qt::Key_Dead_Circumflex,
    XKB_KEY_dead_tilde,              Qt::Key_Dead_Tilde,
    XKB_KEY_dead_macron,             Qt::Key_Dead_Macron,
    XKB_KEY_dead_breve,              Qt::Key_Dead_Breve,
    XKB_KEY_dead_abovedot,           Qt::Key_Dead_Abovedot,
    XKB_KEY_dead_diaeresis,          Qt::Key_Dead_Diaeresis,
    XKB_KEY_dead_abovering,          Qt::Key_Dead_Abovering,
    XKB_KEY_dead_doubleacute,        Qt::Key_Dead_Doubleacute,
    XKB_KEY_dead_caron,              Qt::Key_Dead_Caron,
    XKB_KEY_dead_cedilla,            Qt::Key_Dead_Cedilla,
    XKB_KEY_dead_ogonek,             Qt::Key_Dead_Ogonek,
    XKB_KEY_dead_iota,               Qt::Key_Dead_Iota,
    XKB_KEY_dead_voiced_sound,       Qt::Key_Dead_Voiced_Sound,
    XKB_KEY_dead_semivoiced_sound,   Qt::Key_Dead_Semivoiced_Sound,
    XKB_KEY_dead_belowdot,           Qt::Key_Dead_Belowdot,
    XKB_KEY_dead_hook,               Qt::Key_Dead_Hook,
    XKB_KEY_dead_horn,               Qt::Key_Dead_Horn,

    // Special keys from X.org - This include multimedia keys,
    // wireless/bluetooth/uwb keys, special launcher keys, etc.
    XKB_KEY_XF86Back,                Qt::Key_Back,
    XKB_KEY_XF86Forward,             Qt::Key_Forward,
    XKB_KEY_XF86Stop,                Qt::Key_Stop,
    XKB_KEY_XF86Refresh,             Qt::Key_Refresh,
    XKB_KEY_XF86Favorites,           Qt::Key_Favorites,
    XKB_KEY_XF86AudioMedia,          Qt::Key_LaunchMedia,
    XKB_KEY_XF86OpenURL,             Qt::Key_OpenUrl,
    XKB_KEY_XF86HomePage,            Qt::Key_HomePage,
    XKB_KEY_XF86Search,              Qt::Key_Search,
    XKB_KEY_XF86AudioLowerVolume,    Qt::Key_VolumeDown,
    XKB_KEY_XF86AudioMute,           Qt::Key_VolumeMute,
    XKB_KEY_XF86AudioRaiseVolume,    Qt::Key_VolumeUp,
    XKB_KEY_XF86AudioPlay,           Qt::Key_MediaPlay,
    XKB_KEY_XF86AudioStop,           Qt::Key_MediaStop,
    XKB_KEY_XF86AudioPrev,           Qt::Key_MediaPrevious,
    XKB_KEY_XF86AudioNext,           Qt::Key_MediaNext,
    XKB_KEY_XF86AudioRecord,         Qt::Key_MediaRecord,
    XKB_KEY_XF86AudioPause,          Qt::Key_MediaPause,
    XKB_KEY_XF86Mail,                Qt::Key_LaunchMail,
    XKB_KEY_XF86MyComputer,          Qt::Key_Launch0,  // ### Qt 6: remap properly
    XKB_KEY_XF86Calculator,          Qt::Key_Launch1,
    XKB_KEY_XF86Memo,                Qt::Key_Memo,
    XKB_KEY_XF86ToDoList,            Qt::Key_ToDoList,
    XKB_KEY_XF86Calendar,            Qt::Key_Calendar,
    XKB_KEY_XF86PowerDown,           Qt::Key_PowerDown,
    XKB_KEY_XF86ContrastAdjust,      Qt::Key_ContrastAdjust,
    XKB_KEY_XF86Standby,             Qt::Key_Standby,
    XKB_KEY_XF86MonBrightnessUp,     Qt::Key_MonBrightnessUp,
    XKB_KEY_XF86MonBrightnessDown,   Qt::Key_MonBrightnessDown,
    XKB_KEY_XF86KbdLightOnOff,       Qt::Key_KeyboardLightOnOff,
    XKB_KEY_XF86KbdBrightnessUp,     Qt::Key_KeyboardBrightnessUp,
    XKB_KEY_XF86KbdBrightnessDown,   Qt::Key_KeyboardBrightnessDown,
    XKB_KEY_XF86PowerOff,            Qt::Key_PowerOff,
    XKB_KEY_XF86WakeUp,              Qt::Key_WakeUp,
    XKB_KEY_XF86Eject,               Qt::Key_Eject,
    XKB_KEY_XF86ScreenSaver,         Qt::Key_ScreenSaver,
    XKB_KEY_XF86WWW,                 Qt::Key_WWW,
    XKB_KEY_XF86Sleep,               Qt::Key_Sleep,
    XKB_KEY_XF86LightBulb,           Qt::Key_LightBulb,
    XKB_KEY_XF86Shop,                Qt::Key_Shop,
    XKB_KEY_XF86History,             Qt::Key_History,
    XKB_KEY_XF86AddFavorite,         Qt::Key_AddFavorite,
    XKB_KEY_XF86HotLinks,            Qt::Key_HotLinks,
    XKB_KEY_XF86BrightnessAdjust,    Qt::Key_BrightnessAdjust,
    XKB_KEY_XF86Finance,             Qt::Key_Finance,
    XKB_KEY_XF86Community,           Qt::Key_Community,
    XKB_KEY_XF86AudioRewind,         Qt::Key_AudioRewind,
    XKB_KEY_XF86BackForward,         Qt::Key_BackForward,
    XKB_KEY_XF86ApplicationLeft,     Qt::Key_ApplicationLeft,
    XKB_KEY_XF86ApplicationRight,    Qt::Key_ApplicationRight,
    XKB_KEY_XF86Book,                Qt::Key_Book,
    XKB_KEY_XF86CD,                  Qt::Key_CD,
    XKB_KEY_XF86Calculater,          Qt::Key_Calculator,
    XKB_KEY_XF86Clear,               Qt::Key_Clear,
    XKB_KEY_XF86ClearGrab,           Qt::Key_ClearGrab,
    XKB_KEY_XF86Close,               Qt::Key_Close,
    XKB_KEY_XF86Copy,                Qt::Key_Copy,
    XKB_KEY_XF86Cut,                 Qt::Key_Cut,
    XKB_KEY_XF86Display,             Qt::Key_Display,
    XKB_KEY_XF86DOS,                 Qt::Key_DOS,
    XKB_KEY_XF86Documents,           Qt::Key_Documents,
    XKB_KEY_XF86Excel,               Qt::Key_Excel,
    XKB_KEY_XF86Explorer,            Qt::Key_Explorer,
    XKB_KEY_XF86Game,                Qt::Key_Game,
    XKB_KEY_XF86Go,                  Qt::Key_Go,
    XKB_KEY_XF86iTouch,              Qt::Key_iTouch,
    XKB_KEY_XF86LogOff,              Qt::Key_LogOff,
    XKB_KEY_XF86Market,              Qt::Key_Market,
    XKB_KEY_XF86Meeting,             Qt::Key_Meeting,
    XKB_KEY_XF86MenuKB,              Qt::Key_MenuKB,
    XKB_KEY_XF86MenuPB,              Qt::Key_MenuPB,
    XKB_KEY_XF86MySites,             Qt::Key_MySites,
    XKB_KEY_XF86New,                 Qt::Key_New,
    XKB_KEY_XF86News,                Qt::Key_News,
    XKB_KEY_XF86OfficeHome,          Qt::Key_OfficeHome,
    XKB_KEY_XF86Open,                Qt::Key_Open,
    XKB_KEY_XF86Option,              Qt::Key_Option,
    XKB_KEY_XF86Paste,               Qt::Key_Paste,
    XKB_KEY_XF86Phone,               Qt::Key_Phone,
    XKB_KEY_XF86Reply,               Qt::Key_Reply,
    XKB_KEY_XF86Reload,              Qt::Key_Reload,
    XKB_KEY_XF86RotateWindows,       Qt::Key_RotateWindows,
    XKB_KEY_XF86RotationPB,          Qt::Key_RotationPB,
    XKB_KEY_XF86RotationKB,          Qt::Key_RotationKB,
    XKB_KEY_XF86Save,                Qt::Key_Save,
    XKB_KEY_XF86Send,                Qt::Key_Send,
    XKB_KEY_XF86Spell,               Qt::Key_Spell,
    XKB_KEY_XF86SplitScreen,         Qt::Key_SplitScreen,
    XKB_KEY_XF86Support,             Qt::Key_Support,
    XKB_KEY_XF86TaskPane,            Qt::Key_TaskPane,
    XKB_KEY_XF86Terminal,            Qt::Key_Terminal,
    XKB_KEY_XF86Tools,               Qt::Key_Tools,
    XKB_KEY_XF86Travel,              Qt::Key_Travel,
    XKB_KEY_XF86Video,               Qt::Key_Video,
    XKB_KEY_XF86Word,                Qt::Key_Word,
    XKB_KEY_XF86Xfer,                Qt::Key_Xfer,
    XKB_KEY_XF86ZoomIn,              Qt::Key_ZoomIn,
    XKB_KEY_XF86ZoomOut,             Qt::Key_ZoomOut,
    XKB_KEY_XF86Away,                Qt::Key_Away,
    XKB_KEY_XF86Messenger,           Qt::Key_Messenger,
    XKB_KEY_XF86WebCam,              Qt::Key_WebCam,
    XKB_KEY_XF86MailForward,         Qt::Key_MailForward,
    XKB_KEY_XF86Pictures,            Qt::Key_Pictures,
    XKB_KEY_XF86Music,               Qt::Key_Music,
    XKB_KEY_XF86Battery,             Qt::Key_Battery,
    XKB_KEY_XF86Bluetooth,           Qt::Key_Bluetooth,
    XKB_KEY_XF86WLAN,                Qt::Key_WLAN,
    XKB_KEY_XF86UWB,                 Qt::Key_UWB,
    XKB_KEY_XF86AudioForward,        Qt::Key_AudioForward,
    XKB_KEY_XF86AudioRepeat,         Qt::Key_AudioRepeat,
    XKB_KEY_XF86AudioRandomPlay,     Qt::Key_AudioRandomPlay,
    XKB_KEY_XF86Subtitle,            Qt::Key_Subtitle,
    XKB_KEY_XF86AudioCycleTrack,     Qt::Key_AudioCycleTrack,
    XKB_KEY_XF86Time,                Qt::Key_Time,
    XKB_KEY_XF86Select,              Qt::Key_Select,
    XKB_KEY_XF86View,                Qt::Key_View,
    XKB_KEY_XF86TopMenu,             Qt::Key_TopMenu,
    XKB_KEY_XF86Red,                 Qt::Key_Red,
    XKB_KEY_XF86Green,               Qt::Key_Green,
    XKB_KEY_XF86Yellow,              Qt::Key_Yellow,
    XKB_KEY_XF86Blue,                Qt::Key_Blue,
    XKB_KEY_XF86Bluetooth,           Qt::Key_Bluetooth,
    XKB_KEY_XF86Suspend,             Qt::Key_Suspend,
    XKB_KEY_XF86Hibernate,           Qt::Key_Hibernate,
    XKB_KEY_XF86TouchpadToggle,      Qt::Key_TouchpadToggle,
    XKB_KEY_XF86TouchpadOn,          Qt::Key_TouchpadOn,
    XKB_KEY_XF86TouchpadOff,         Qt::Key_TouchpadOff,
    XKB_KEY_XF86AudioMicMute,        Qt::Key_MicMute,
    XKB_KEY_XF86Launch0,             Qt::Key_Launch2, // ### Qt 6: remap properly
    XKB_KEY_XF86Launch1,             Qt::Key_Launch3,
    XKB_KEY_XF86Launch2,             Qt::Key_Launch4,
    XKB_KEY_XF86Launch3,             Qt::Key_Launch5,
    XKB_KEY_XF86Launch4,             Qt::Key_Launch6,
    XKB_KEY_XF86Launch5,             Qt::Key_Launch7,
    XKB_KEY_XF86Launch6,             Qt::Key_Launch8,
    XKB_KEY_XF86Launch7,             Qt::Key_Launch9,
    XKB_KEY_XF86Launch8,             Qt::Key_LaunchA,
    XKB_KEY_XF86Launch9,             Qt::Key_LaunchB,
    XKB_KEY_XF86LaunchA,             Qt::Key_LaunchC,
    XKB_KEY_XF86LaunchB,             Qt::Key_LaunchD,
    XKB_KEY_XF86LaunchC,             Qt::Key_LaunchE,
    XKB_KEY_XF86LaunchD,             Qt::Key_LaunchF,
    XKB_KEY_XF86LaunchE,             Qt::Key_LaunchG,
    XKB_KEY_XF86LaunchF,             Qt::Key_LaunchH,

    0,                          0
};

static uint32_t translateKeysym(uint32_t sym, const QString &text) {
    int code = 0;

    QTextCodec *systemCodec = QTextCodec::codecForLocale();
    if (sym < 128 || (sym < 256 && systemCodec->mibEnum() == 4)) {
        // upper-case key, if known
        code = isprint((int)sym) ? toupper((int)sym) : 0;
    } else if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F35) {
        return Qt::Key_F1 + (int(sym) - XKB_KEY_F1);
    } else if (text.length() == 1 && text.unicode()->unicode() > 0x1f
               && text.unicode()->unicode() != 0x7f
               && !(sym >= XKB_KEY_dead_grave && sym <= XKB_KEY_dead_currency)) {
        code = text.unicode()->toUpper().unicode();
    } else {
        for (int i = 0; KeyTable[i]; i += 2)
            if (sym == KeyTable[i])
                code = KeyTable[i + 1];
    }

    return code;
}

namespace {

class QtWindowSystem : public QtEventFeeder::QtWindowSystemInterface
{
public:
    QtWindowSystem()
    {
        // because we're using QMetaObject::invoke with arguments of those types
        qRegisterMetaType<Qt::KeyboardModifiers>("Qt::KeyboardModifiers");
        qRegisterMetaType<Qt::MouseButtons>("Qt::MouseButtons");
    }

    void setScreensModel(const std::shared_ptr<ScreensModel> &sc) override
    {
        m_screensModel = sc;
    }

    virtual QWindow* focusedWindow() override
    {
        return QGuiApplication::focusWindow();
    }

    QWindow* getWindowForTouchPoint(const QPoint &point) override //FIXME: not efficient, not updating focused window
    {
        return m_screensModel->getWindowForPoint(point);
    }

    void registerTouchDevice(QTouchDevice *device) override
    {
        QWindowSystemInterface::registerTouchDevice(device);
    }

    void handleExtendedKeyEvent(QWindow *window, ulong timestamp, QEvent::Type type, int key,
                Qt::KeyboardModifiers modifiers,
                quint32 nativeScanCode, quint32 nativeVirtualKey,
                quint32 nativeModifiers,
                const QString& text, bool autorep, ushort count) override
    {

#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0)) || (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
        QWindowSystemInterface::handleExtendedKeyEvent(window, timestamp, type, key, modifiers,
                nativeScanCode, nativeVirtualKey, nativeModifiers, text, autorep, count);
#else
        // The version above is the right one, but we have to workaround a FIXME hack in
        // QWindowSystemInterface::handleShortcutEvent which forcibly sets sync mode from the GUI thread.
        // Sending an event synchronously from the mir input thread risks a deadlock with the main/GUI thread
        // from a miral mutex locked by both thread (eg. holding Alt + dragging a window with the the mouse)
        // See: https://bugreports.qt.io/browse/QTBUG-56274
        // Bug was introduced by commit c7e5e1d9e01849347a9e59b8285477a20d82002b and fixed by commit
        // 33d748bb88676b69e596ae77badfeaf5a69a33d1
        QWindowSystemInterfacePrivate::KeyEvent *e =
                new QWindowSystemInterfacePrivate::KeyEvent(window, timestamp, type, key, modifiers,
                    nativeScanCode, nativeVirtualKey, nativeModifiers, text, autorep, count);
        QWindowSystemInterfacePrivate::postWindowSystemEvent(e);
#endif

    }

    void handleTouchEvent(QWindow *window, ulong timestamp, QTouchDevice *device,
            const QList<struct QWindowSystemInterface::TouchPoint> &points, Qt::KeyboardModifiers mods) override
    {
        // See comment in handleExtendedKeyEvent
#if (QT_VERSION < QT_VERSION_CHECK(5, 6, 0)) || (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
        QWindowSystemInterface::handleTouchEvent(window, timestamp, device, points, mods);
#else
        {
            if (!points.size()) // Touch events must have at least one point
                return;
            QEvent::Type type;
            QList<QTouchEvent::TouchPoint> touchPoints = QWindowSystemInterfacePrivate::fromNativeTouchPoints(points, window, &type);

            QWindowSystemInterfacePrivate::TouchEvent *e =
                    new QWindowSystemInterfacePrivate::TouchEvent(window, timestamp, type, device, touchPoints, mods);
            QWindowSystemInterfacePrivate::postWindowSystemEvent(e);
        }
#endif
    }

private:
    std::shared_ptr<ScreensModel> m_screensModel;
};

} // anonymous namespace

QtEventFeeder::QtEventFeeder(const std::shared_ptr<ScreensModel> &screensModel)
    : QtEventFeeder(screensModel, new QtWindowSystem)
{
}

QtEventFeeder::QtEventFeeder(const std::shared_ptr<ScreensModel> &screensModel,
                             QtEventFeeder::QtWindowSystemInterface *windowSystem)
    : mQtWindowSystem(windowSystem)
{
    // Initialize touch device. Hardcoded just like in qtubuntu
    // TODO: Create them from info gathered from Mir and store things like device id and source
    //       in a QTouchDevice-derived class created by us. So that we can properly assemble back
    //       MirEvents our of QTouchEvents to give to mir::scene::Surface::consume.
    mTouchDevice = new QTouchDevice();  // Qt takes ownership of mTouchDevice with registerTouchDevice
    mTouchDevice->setType(QTouchDevice::TouchScreen);
    mTouchDevice->setCapabilities(
            QTouchDevice::Position | QTouchDevice::Area | QTouchDevice::Pressure |
            QTouchDevice::NormalizedPosition);
    mQtWindowSystem->setScreensModel(screensModel);
    mQtWindowSystem->registerTouchDevice(mTouchDevice);
}

QtEventFeeder::~QtEventFeeder()
{
    delete mQtWindowSystem;
}

bool QtEventFeeder::dispatch(MirEvent const& event)
{
    auto type = mir_event_get_type(&event);
    if (type != mir_event_type_input)
        return false;

    auto iev = mir_event_get_input_event(&event);

    switch (mir_input_event_get_type(iev)) {
    case mir_input_event_type_key:
        dispatchKey(mir_input_event_get_keyboard_event(iev));
        break;
    case mir_input_event_type_touch:
        dispatchTouch(mir_input_event_get_touch_event(iev));
        break;
    case mir_input_event_type_pointer:
        dispatchPointer(mir_input_event_get_pointer_event(iev));
    default:
        break;
    }

    return true;
}

namespace
{

Qt::KeyboardModifiers getQtModifiersFromMir(MirInputEventModifiers modifiers)
{
    Qt::KeyboardModifiers qtModifiers = Qt::NoModifier;
    if (modifiers & mir_input_event_modifier_shift) {
        qtModifiers |= Qt::ShiftModifier;
    }
    if (modifiers & mir_input_event_modifier_ctrl) {
        qtModifiers |= Qt::ControlModifier;
    }
    if (modifiers & mir_input_event_modifier_alt) {
        qtModifiers |= Qt::AltModifier;
    }
    if (modifiers & mir_input_event_modifier_meta) {
        qtModifiers |= Qt::MetaModifier;
    }
    if (modifiers & mir_input_event_modifier_alt_right) {
        qtModifiers |= Qt::GroupSwitchModifier;
    }
    return qtModifiers;
}

Qt::MouseButtons getQtMouseButtonsfromMirPointerEvent(MirPointerEvent const* pev)
{
    Qt::MouseButtons buttons = Qt::NoButton;
    if (mir_pointer_event_button_state(pev, mir_pointer_button_primary))
        buttons |= Qt::LeftButton;
    if (mir_pointer_event_button_state(pev, mir_pointer_button_secondary))
        buttons |= Qt::RightButton;
    if (mir_pointer_event_button_state(pev, mir_pointer_button_tertiary))
        buttons |= Qt::MiddleButton;
    if (mir_pointer_event_button_state(pev, mir_pointer_button_back))
        buttons |= Qt::BackButton;
    if (mir_pointer_event_button_state(pev, mir_pointer_button_forward))
        buttons |= Qt::ForwardButton;

    return buttons;
}
} // namespace

void QtEventFeeder::dispatchPointer(const MirPointerEvent *pev)
{
    auto timestamp = qtmir::compressTimestamp<qtmir::Timestamp>(
                std::chrono::nanoseconds(mir_input_event_get_event_time(
                                             mir_pointer_event_input_event(pev))));

    auto action = mir_pointer_event_action(pev);
    qCDebug(QTMIR_MIR_INPUT) << "Received" << qPrintable(mirPointerEventToString(pev));

    auto modifiers = getQtModifiersFromMir(mir_pointer_event_modifiers(pev));

    auto relative = QPointF(mir_pointer_event_axis_value(pev, mir_pointer_axis_relative_x),
                            mir_pointer_event_axis_value(pev, mir_pointer_axis_relative_y));

    auto absolute = QPointF(mir_pointer_event_axis_value(pev, mir_pointer_axis_x),
                            mir_pointer_event_axis_value(pev, mir_pointer_axis_y));

    switch (action) {
    case mir_pointer_action_button_up:
    case mir_pointer_action_button_down:
    case mir_pointer_action_motion:
    {
        const float hDelta = mir_pointer_event_axis_value(pev, mir_pointer_axis_hscroll);
        const float vDelta = mir_pointer_event_axis_value(pev, mir_pointer_axis_vscroll);

        auto buttons = getQtMouseButtonsfromMirPointerEvent(pev);
        if (hDelta != 0 || vDelta != 0) {
            const QPoint angleDelta = QPoint(hDelta * 15, vDelta * 15);

            auto wheel = new QWheelEvent(relative, absolute, QPoint(), angleDelta, 0, Qt::Vertical, buttons, modifiers);
            wheel->setTimestamp(timestamp.count());
            QGuiApplication::postEvent(this, wheel);
        }

        auto type = action == mir_pointer_action_motion ? QEvent::MouseMove
                                                        : action == mir_pointer_action_button_up ? QEvent::MouseButtonRelease
                                                                                                 : QEvent::MouseButtonRelease;

        Qt::MouseButtons stateChange = m_buttons ^ buttons;
        Qt::MouseButton button = Qt::NoButton;
        for (int check = Qt::LeftButton;
            check <= int(Qt::MaxMouseButton);
             check = check << 1) {
            if (check & stateChange) {
                button = Qt::MouseButton(check);
                break;
            }
        }

        m_buttons = buttons;
        auto mouseEvent = new QMouseEvent(type, relative, absolute, button, buttons, modifiers);
        mouseEvent->setTimestamp(timestamp.count());
        QGuiApplication::postEvent(this, mouseEvent);
        break;
    }
    default:
        qCDebug(QTMIR_MIR_INPUT) << "Unrecognized pointer event";
    }
}

void QtEventFeeder::dispatchKey(const MirKeyboardEvent *kev)
{
    auto timestamp = qtmir::compressTimestamp<qtmir::Timestamp>(
                std::chrono::nanoseconds(mir_input_event_get_event_time(
                                             mir_keyboard_event_input_event(kev))));

    xkb_keysym_t xk_sym = mir_keyboard_event_key_code(kev);

    // Key modifier and unicode index mapping.
    auto modifiers = getQtModifiersFromMir(mir_keyboard_event_modifiers(kev));

    // Key action
    QEvent::Type keyType = QEvent::KeyRelease;
    bool is_auto_rep = false;

    switch (mir_keyboard_event_action(kev))
    {
    case mir_keyboard_action_repeat:
        is_auto_rep = true; // fall-through
    case mir_keyboard_action_down:
        keyType = QEvent::KeyPress;
        break;
    case mir_keyboard_action_up:
        keyType = QEvent::KeyRelease;
        break;
    default:
        break;
    }

    // Key event propagation.
    QString text;
    QVarLengthArray<char, 32> chars(32);
    {
        int result = xkb_keysym_to_utf8(xk_sym, chars.data(), chars.size());

        if (result > 0) {
            text = QString::fromUtf8(chars.constData());
        }
    }
    int keyCode = translateKeysym(xk_sym, text);

    qCDebug(QTMIR_MIR_INPUT).nospace() << "Received " << qPrintable(mirKeyboardEventToString(kev))
        << ". Dispatching to " << mQtWindowSystem->focusedWindow();

    mQtWindowSystem->handleExtendedKeyEvent(mQtWindowSystem->focusedWindow(),
        timestamp.count(), keyType, keyCode, modifiers,
        mir_keyboard_event_scan_code(kev), xk_sym,
        mir_keyboard_event_modifiers(kev), text, is_auto_rep);
}

void QtEventFeeder::dispatchTouch(const MirTouchEvent *tev)
{
    auto timestamp = qtmir::compressTimestamp<qtmir::Timestamp>(
                std::chrono::nanoseconds(mir_input_event_get_event_time(
                                             mir_touch_event_input_event(tev))));

    tracepoint(qtmirserver, touchEventDispatch_start, std::chrono::nanoseconds(timestamp).count());

    qCDebug(QTMIR_MIR_INPUT) << "Received" << qPrintable(mirTouchEventToString(tev));

    // FIXME(loicm) Max pressure is device specific. That one is for the Samsung Galaxy Nexus. That
    //     needs to be fixed as soon as the compat input lib adds query support.
    const float kMaxPressure = 1.28;
    const int kPointerCount = mir_touch_event_point_count(tev);
    QList<QWindowSystemInterface::TouchPoint> touchPoints;
    QWindow *window = nullptr;

    if (kPointerCount > 0) {
        window = mQtWindowSystem->getWindowForTouchPoint(
                    QPoint(mir_touch_event_axis_value(tev, 0, mir_touch_axis_x),
                           mir_touch_event_axis_value(tev, 0, mir_touch_axis_y)));

        if (!window) {
            qCDebug(QTMIR_MIR_INPUT) << "REJECTING INPUT EVENT, no matching window";
            return;
        }

        const QRect kWindowGeometry = window->geometry();

        // TODO: Is it worth setting the Qt::TouchPointStationary ones? Currently they are left
        //       as Qt::TouchPointMoved
        for (int i = 0; i < kPointerCount; ++i) {
            QWindowSystemInterface::TouchPoint touchPoint;

            const float kX = mir_touch_event_axis_value(tev, i, mir_touch_axis_x);
            const float kY = mir_touch_event_axis_value(tev, i, mir_touch_axis_y);
            const float kW = mir_touch_event_axis_value(tev, i, mir_touch_axis_touch_major);
            const float kH = mir_touch_event_axis_value(tev, i, mir_touch_axis_touch_minor);
            const float kP = mir_touch_event_axis_value(tev, i, mir_touch_axis_pressure);
            touchPoint.id = mir_touch_event_id(tev, i);

            touchPoint.normalPosition = QPointF(kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
            touchPoint.area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
            touchPoint.pressure = kP / kMaxPressure;
            switch (mir_touch_event_action(tev, i))
            {
            case mir_touch_action_up:
                touchPoint.state = Qt::TouchPointReleased;
                break;
            case mir_touch_action_down:
                touchPoint.state = Qt::TouchPointPressed;
                break;
            case mir_touch_action_change:
                touchPoint.state = Qt::TouchPointMoved;
                break;
            default:
                break;
            }

            touchPoints.append(touchPoint);
        }
    }

    // Qt needs a happy, sane stream of touch events. So let's make sure we're not forwarding
    // any insanity.
    validateTouches(window, timestamp.count(), touchPoints);

    // Touch event propagation.
    qCDebug(QTMIR_MIR_INPUT) << "Sending to Qt" << qPrintable(touchesToString(touchPoints));
    mQtWindowSystem->handleTouchEvent(window,
        //scales down the nsec_t (int64) to fit a ulong, precision lost but time difference suitable
        timestamp.count(),
        mTouchDevice,
        touchPoints);

    tracepoint(qtmirserver, touchEventDispatch_end, std::chrono::nanoseconds(timestamp).count());
}

bool QtEventFeeder::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Wheel:
    {
        QWindowSystemInterface::setSynchronousWindowSystemEvents(true);

        QWheelEvent* we = static_cast<QWheelEvent*>(e);
        QWindowSystemInterface::handleWheelEvent(nullptr, we->timestamp(), we->pos(), we->globalPos(),
                we->pixelDelta(), we->angleDelta(), we->modifiers(), Qt::ScrollUpdate);

        QWindowSystemInterface::setSynchronousWindowSystemEvents(false);
        return true;
    } break;
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    {
        QWindowSystemInterface::setSynchronousWindowSystemEvents(true);

        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        QWindowSystemInterface::handleMouseEvent(nullptr, me->timestamp(), me->localPos(), me->globalPos(), me->buttons(), me->modifiers());

        QWindowSystemInterface::setSynchronousWindowSystemEvents(false);
        return true;
    } break;
    default:
        break;
    }

    return QObject::event(e);
}

void QtEventFeeder::validateTouches(QWindow *window, ulong timestamp,
        QList<QWindowSystemInterface::TouchPoint> &touchPoints)
{
    QSet<int> updatedTouches;

    {
        int i = 0;
        while (i < touchPoints.count()) {
            bool mustDiscardTouch = !validateTouch(touchPoints[i]);
            if (mustDiscardTouch) {
                touchPoints.removeAt(i);
            } else {
                updatedTouches.insert(touchPoints.at(i).id);
                ++i;
            }
        }
    }

    // Release all unmentioned touches, one by one.
    QHash<int, QWindowSystemInterface::TouchPoint>::iterator it = mActiveTouches.begin();
    while (it != mActiveTouches.end()) {
        if (!updatedTouches.contains(it.key())) {
            qCWarning(QTMIR_MIR_INPUT)
                << "There's a touch (id =" << it.key() << ") missing. Releasing it.";
            sendActiveTouchRelease(window, timestamp, it.key());
            it = mActiveTouches.erase(it);
        } else {
            ++it;
        }
    }

    // update mActiveTouches
    for (int i = 0; i < touchPoints.count(); ++i) {
        auto &touchPoint = touchPoints.at(i);
        if (touchPoint.state == Qt::TouchPointReleased) {
            mActiveTouches.remove(touchPoint.id);
        } else {
            mActiveTouches[touchPoint.id] = touchPoint;
        }
    }
}

void QtEventFeeder::sendActiveTouchRelease(QWindow *window, ulong timestamp, int id)
{
    QList<QWindowSystemInterface::TouchPoint> touchPoints = mActiveTouches.values();

    for (int i = 0; i < touchPoints.count(); ++i) {
        QWindowSystemInterface::TouchPoint &touchPoint = touchPoints[i];
        if (touchPoint.id == id) {
            touchPoint.state = Qt::TouchPointReleased;
        } else {
            touchPoint.state = Qt::TouchPointStationary;
        }
    }

    qCDebug(QTMIR_MIR_INPUT) << "Sending to Qt" << qPrintable(touchesToString(touchPoints));
    mQtWindowSystem->handleTouchEvent(window, timestamp, mTouchDevice, touchPoints);
}

bool QtEventFeeder::validateTouch(QWindowSystemInterface::TouchPoint &touchPoint)
{
    bool ok = true;

    switch (touchPoint.state) {
    case Qt::TouchPointPressed:
        if (mActiveTouches.contains(touchPoint.id)) {
            qCWarning(QTMIR_MIR_INPUT)
                << "Would press an already existing touch (id =" << touchPoint.id
                << "). Making it move instead.";
            touchPoint.state = Qt::TouchPointMoved;
        }
        break;
    case Qt::TouchPointMoved:
        if (!mActiveTouches.contains(touchPoint.id)) {
            qCWarning(QTMIR_MIR_INPUT)
                << "Would move a touch that wasn't pressed before (id =" << touchPoint.id
                << "). Making it press instead.";
            touchPoint.state = Qt::TouchPointPressed;
        }
        break;
    case Qt::TouchPointStationary:
        if (!mActiveTouches.contains(touchPoint.id)) {
            qCWarning(QTMIR_MIR_INPUT)
                << "There's an stationary touch that wasn't pressed before (id =" << touchPoint.id
                << "). Making it press instead.";
            touchPoint.state = Qt::TouchPointPressed;
        }
        break;
    case Qt::TouchPointReleased:
        if (!mActiveTouches.contains(touchPoint.id)) {
            qCWarning(QTMIR_MIR_INPUT)
                << "Would release a touch that wasn't pressed before (id =" << touchPoint.id
                << "). Ignoring it.";
            ok = false;
        }
        break;
    default:
        qFatal("QtEventFeeder: invalid touch state");
    }

    return ok;
}

QString QtEventFeeder::touchesToString(const QList<struct QWindowSystemInterface::TouchPoint> &points)
{
    QString result;
    for (int i = 0; i < points.count(); ++i) {
        if (i > 0) {
            result.append(",");
        }
        const struct QWindowSystemInterface::TouchPoint &point = points.at(i);
        result.append(QStringLiteral("(id=%1,state=%2,normalPosition=(%3,%4))")
            .arg(point.id)
            .arg(touchPointStateToString(point.state))
            .arg(point.normalPosition.x())
            .arg(point.normalPosition.y())
            );
    }
    return result;
}
