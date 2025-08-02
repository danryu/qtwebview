// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlinuxwebview_p.h"
#include <private/qwebviewplugin_p.h>
#include <QtCore/qbytearray.h>

QT_BEGIN_NAMESPACE

class QLinuxWebViewPlugin : public QWebViewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWebViewPluginInterface_iid FILE "linux.json")

public:
    QAbstractWebView *create(const QString &key) const override
    {
        return (key == QLatin1String("webview")) ? new QLinuxWebViewPrivate() : nullptr;
    }

    void prepare() const override
    {
        // Initialize GLib/GTK if needed
        // Note: In a real application, GTK should be initialized by the main application
    }
};

QT_END_NAMESPACE

#include "qlinuxwebviewplugin.moc"