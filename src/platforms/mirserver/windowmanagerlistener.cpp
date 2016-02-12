#include "windowmanagerlistener.h"

WindowManagerListener::WindowManagerListener(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<WindowManagerListener::SurfaceProperty>("WindowManagerListener::SurfaceProperty");
}

