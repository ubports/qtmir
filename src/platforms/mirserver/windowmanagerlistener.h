#ifndef WINDOWMANAGERLISTENER_H
#define WINDOWMANAGERLISTENER_H

#include <QObject>

#include <mir/scene/surface.h>
#include <memory>

class WindowManagerListener : public QObject
{
    Q_OBJECT
public:
    explicit WindowManagerListener(QObject *parent = 0);

    enum SurfaceProperty {
        Name,
        ShellChrome
    };

Q_SIGNALS:
    void surfaceMofidied(const std::shared_ptr<mir::scene::Surface>& surface,
                         WindowManagerListener::SurfaceProperty property,
                         const QVariant& value);
};

#endif // WINDOWMANAGERLISTENER_H
