// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLINUXWEBVIEW_P_H
#define QLINUXWEBVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtGui/qwindow.h>
#include <QtCore/qpointer.h>

#include <private/qabstractwebview_p.h>

#include <webkit2/webkit2.h>

QT_BEGIN_NAMESPACE

class QLinuxWebViewSettingsPrivate : public QAbstractWebViewSettings
{
    Q_OBJECT
public:
    explicit QLinuxWebViewSettingsPrivate(WebKitWebView *webView, QObject *p = nullptr);

    bool localStorageEnabled() const override;
    bool javaScriptEnabled() const override;
    bool localContentCanAccessFileUrls() const override;
    bool allowFileAccess() const override;

public Q_SLOTS:
    void setLocalContentCanAccessFileUrls(bool enabled) override;
    void setJavaScriptEnabled(bool enabled) override;
    void setLocalStorageEnabled(bool enabled) override;
    void setAllowFileAccess(bool enabled) override;

private:
    WebKitWebView *m_webView = nullptr;
    bool m_allowFileAccess = false;
    bool m_localContentCanAccessFileUrls = false;
};

class QLinuxWebViewPrivate : public QAbstractWebView
{
    Q_OBJECT
public:
    explicit QLinuxWebViewPrivate(QObject *p = nullptr);
    ~QLinuxWebViewPrivate() override;

    QString httpUserAgent() const override;
    void setHttpUserAgent(const QString &httpUserAgent) override;
    QUrl url() const override;
    void setUrl(const QUrl &url) override;
    bool canGoBack() const override;
    bool canGoForward() const override;
    QString title() const override;
    int loadProgress() const override;
    bool isLoading() const override;

    QWindow *nativeWindow() const override { return m_window; }
    QAbstractWebViewSettings *getSettings() const override;

public Q_SLOTS:
    void goBack() override;
    void goForward() override;
    void reload() override;
    void stop() override;
    void loadHtml(const QString &html, const QUrl &baseUrl = QUrl()) override;
    void setCookie(const QString &domain, const QString &name, const QString &value) override;
    void deleteCookie(const QString &domain, const QString &name) override;
    void deleteAllCookies() override;

protected:
    void runJavaScriptPrivate(const QString& script, int callbackId) override;

private Q_SLOTS:
    void onLoadStarted();
    void onLoadFinished(bool ok);
    void onLoadProgressChanged(double progress);
    void onTitleChanged();
    void onUrlChanged();

private:
    static void loadChangedCallback(WebKitWebView *webView, WebKitLoadEvent loadEvent, gpointer userData);
    static void progressChangedCallback(WebKitWebView *webView, GParamSpec *paramSpec, gpointer userData);
    static void titleChangedCallback(WebKitWebView *webView, GParamSpec *paramSpec, gpointer userData);
    static void uriChangedCallback(WebKitWebView *webView, GParamSpec *paramSpec, gpointer userData);

    void connectSignals();
    void disconnectSignals();

public:
    WebKitWebView *m_webView = nullptr;
    QLinuxWebViewSettingsPrivate *m_settings = nullptr;
    QWindow *m_window = nullptr;
    bool m_isLoading = false;
    QString m_title;
    QUrl m_url;
    int m_loadProgress = 0;
};

QT_END_NAMESPACE

#endif // QLINUXWEBVIEW_P_H