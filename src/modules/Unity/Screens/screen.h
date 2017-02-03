#ifndef SCREEN_H
#define SCREEN_H

#include <QObject>
#include <QPointer>
#include <QQmlListProperty>

#include <customscreenconfiguration.h>
#include <screentypes.h>

class QScreen;
class ScreenConfig;
class ScreensController;

class Screen : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

    Q_PROPERTY(bool used READ used NOTIFY usedChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(qtmir::OutputTypes outputType READ outputType NOTIFY outputTypeChanged)
    Q_PROPERTY(float scale READ scale NOTIFY scaleChanged)
    Q_PROPERTY(qtmir::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(uint currentModeIndex READ currentModeIndex NOTIFY currentModeIndexChanged)
    Q_PROPERTY(QQmlListProperty<qtmir::ScreenMode> availableModes READ availableModes NOTIFY availableModesChanged)
    Q_PROPERTY(QSizeF physicalSize READ physicalSize NOTIFY physicalSizeChanged)

public:
    explicit Screen(QScreen* screen, QObject* parent = 0);
    ~Screen();

    qtmir::OutputId outputId() const;
    bool used() const;
    QString name() const;
    float scale() const;
    QSizeF physicalSize() const;
    qtmir::FormFactor formFactor() const;
    qtmir::OutputTypes outputType() const;
    QPoint position() const;
    QQmlListProperty<qtmir::ScreenMode> availableModes();
    uint currentModeIndex() const;
    bool isActive() const;

    QScreen *screen() const;

    Q_INVOKABLE ScreenConfig *beginConfiguration() const;
    Q_INVOKABLE bool applyConfiguration(ScreenConfig *configuration);

    void setActive(bool active);

Q_SIGNALS:
    void usedChanged();
    void nameChanged();
    void outputTypeChanged();
    void scaleChanged();
    void formFactorChanged();
    void positionChanged();
    void currentModeIndexChanged();
    void physicalSizeChanged();
    void availableModesChanged();
    void activeChanged(bool active);

private Q_SLOTS:
    void updateScreenModes();

private:
    QList<qtmir::ScreenMode*> m_modes;
    QPointer<QScreen> m_screen;
    ScreensController *m_screensController;
};

class ScreenConfig: public QObject,
                    public CustomScreenConfiguration
{
    Q_OBJECT
    Q_PROPERTY(bool used MEMBER used)
    Q_PROPERTY(float scale MEMBER scale)
    Q_PROPERTY(qtmir::FormFactor formFactor MEMBER formFactor)
    Q_PROPERTY(uint currentModeIndex MEMBER currentModeIndex)
    Q_PROPERTY(QPoint position MEMBER topLeft)

public:
    ScreenConfig(QObject* parent = 0);

    ScreenConfig &operator=(const CustomScreenConfiguration& other);

    friend class Screen;
};

#endif // SCREEN_H
