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

#ifndef QQUICKSCREENWINDOW_H
#define QQUICKSCREENWINDOW_H

#include <QQuickWindow>

namespace qtmir {

class QQuickScreenWindow : public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(QScreen *screen READ screen WRITE setScreen NOTIFY screenChanged)
    Q_PROPERTY(float scale READ scale NOTIFY scaleChanged)

public:
    explicit QQuickScreenWindow(QQuickWindow *parent = 0);

    QScreen *screen() const;
    void setScreen(QScreen *screen);

    qreal scale();
    Q_INVOKABLE bool setScale(const float scale);

Q_SIGNALS:
    void screenChanged(QScreen *screen);
    void scaleChanged(qreal scale);

private Q_SLOTS:
    void nativePropertyChanged(QPlatformWindow *window, const QString &propertyName);

private:
    float getScaleNativeProperty() const;
    float m_scale;
};

} //namespace qtmir

#endif // QQUICKSCREENWINDOW_H
