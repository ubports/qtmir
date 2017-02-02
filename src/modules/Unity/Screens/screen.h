#ifndef SCREEN_H
#define SCREEN_H

#include <QObject>
#include <QPointer>
#include <QQmlListProperty>

#include <screentypes.h>

class QScreen;

class Screen : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool used READ used WRITE setUsed NOTIFY usedChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(qtmir::OutputTypes outputType READ outputType NOTIFY outputTypeChanged)
    Q_PROPERTY(float scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(qtmir::FormFactor formFactor READ formFactor WRITE setFormFactor NOTIFY formFactorChanged)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(qtmir::ScreenMode* mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QQmlListProperty<qtmir::ScreenMode> availableModes READ availableModes NOTIFY availableModesChanged)
    Q_PROPERTY(QSizeF physicalSize READ physicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    explicit Screen(QScreen* screen, QObject* parent = 0);
    ~Screen();

    bool used() const;
    QString name() const;
    float scale() const;
    QSizeF physicalSize() const;
    qtmir::FormFactor formFactor() const;
    qtmir::OutputTypes outputType() const;
    QPoint position() const;
    QQmlListProperty<qtmir::ScreenMode> availableModes();
    qtmir::ScreenMode *mode() const;
    bool isActive() const;

    QScreen *screen() const;

Q_SIGNALS:
    void usedChanged();
    void nameChanged();
    void outputTypeChanged();
    void scaleChanged();
    void formFactorChanged();
    void positionChanged();
    void modeChanged();
    void physicalSizeChanged();
    void availableModesChanged();
    void activeChanged(bool active);

public Q_SLOTS:
    void setUsed(bool used);
    void setScale(float scale);
    void setFormFactor(qtmir::FormFactor formFactor);
    void setCurrentModeIndex(uint32_t currentModeIndex);
    void setActive(bool active);

private Q_SLOTS:
    void updateScreenModes();

private:
    QList<qtmir::ScreenMode*> m_modes;
    QPointer<QScreen> m_screen;
};

#endif // SCREEN_H
