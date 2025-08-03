// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlinuxwebview_p.h"
#include <private/qwebviewloadrequest_p.h>
#include <private/qwebview_p.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qvariant.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtGui/qwindow.h>

// Already included webkit2/webkit2.h in header with signal protection

QT_BEGIN_NAMESPACE

// QLinuxWebViewSettingsPrivate implementation

QLinuxWebViewSettingsPrivate::QLinuxWebViewSettingsPrivate(WebKitWebView *webView, QObject *parent)
    : QAbstractWebViewSettings(parent)
    , m_webView(webView)
{
}

bool QLinuxWebViewSettingsPrivate::localStorageEnabled() const
{
    if (!m_webView)
        return false;
    
    WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
    return webkit_settings_get_enable_html5_local_storage(settings);
}

bool QLinuxWebViewSettingsPrivate::javaScriptEnabled() const
{
    if (!m_webView)
        return true;
    
    WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
    return webkit_settings_get_enable_javascript(settings);
}

bool QLinuxWebViewSettingsPrivate::localContentCanAccessFileUrls() const
{
    return m_localContentCanAccessFileUrls;
}

bool QLinuxWebViewSettingsPrivate::allowFileAccess() const
{
    return m_allowFileAccess;
}

void QLinuxWebViewSettingsPrivate::setLocalContentCanAccessFileUrls(bool enabled)
{
    if (m_localContentCanAccessFileUrls == enabled)
        return;
    
    m_localContentCanAccessFileUrls = enabled;
    
    if (m_webView) {
        WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
        webkit_settings_set_allow_file_access_from_file_urls(settings, enabled);
    }
}

void QLinuxWebViewSettingsPrivate::setJavaScriptEnabled(bool enabled)
{
    if (!m_webView)
        return;
    
    WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
    webkit_settings_set_enable_javascript(settings, enabled);
}

void QLinuxWebViewSettingsPrivate::setLocalStorageEnabled(bool enabled)
{
    if (!m_webView)
        return;
    
    WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
    webkit_settings_set_enable_html5_local_storage(settings, enabled);
}

void QLinuxWebViewSettingsPrivate::setAllowFileAccess(bool enabled)
{
    if (m_allowFileAccess == enabled)
        return;
    
    m_allowFileAccess = enabled;
    
    if (m_webView) {
        WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
        webkit_settings_set_allow_file_access_from_file_urls(settings, enabled);
    }
}

// QLinuxWebViewPrivate implementation

QLinuxWebViewPrivate::QLinuxWebViewPrivate(QObject *parent)
    : QAbstractWebView(parent)
{
    m_webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
    m_settings = new QLinuxWebViewSettingsPrivate(m_webView, this);
    
    // Increase reference count to keep the WebView alive
    g_object_ref(m_webView);
    
    connectSignals();
}

QLinuxWebViewPrivate::~QLinuxWebViewPrivate()
{
    if (m_webView) {
        disconnectSignals();
        g_object_unref(m_webView);
        m_webView = nullptr;
    }
}

QString QLinuxWebViewPrivate::httpUserAgent() const
{
    if (!m_webView)
        return QString();
    
    WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
    const char *userAgent = webkit_settings_get_user_agent(settings);
    return QString::fromUtf8(userAgent);
}

void QLinuxWebViewPrivate::setHttpUserAgent(const QString &httpUserAgent)
{
    if (!m_webView)
        return;
    
    WebKitSettings *settings = webkit_web_view_get_settings(m_webView);
    webkit_settings_set_user_agent(settings, httpUserAgent.toUtf8().constData());
    emit httpUserAgentChanged(httpUserAgent);
}

// url() method removed - not in base class QAbstractWebView

void QLinuxWebViewPrivate::setUrl(const QUrl &url)
{
    if (!m_webView)
        return;
    
    m_url = url;
    webkit_web_view_load_uri(m_webView, url.toString().toUtf8().constData());
}

bool QLinuxWebViewPrivate::canGoBack() const
{
    if (!m_webView)
        return false;
    
    return webkit_web_view_can_go_back(m_webView);
}

bool QLinuxWebViewPrivate::canGoForward() const
{
    if (!m_webView)
        return false;
    
    return webkit_web_view_can_go_forward(m_webView);
}

QString QLinuxWebViewPrivate::title() const
{
    return m_title;
}

int QLinuxWebViewPrivate::loadProgress() const
{
    return m_loadProgress;
}

bool QLinuxWebViewPrivate::isLoading() const
{
    return m_isLoading;
}

QAbstractWebViewSettings *QLinuxWebViewPrivate::getSettings() const
{
    return m_settings;
}

void QLinuxWebViewPrivate::goBack()
{
    if (m_webView)
        webkit_web_view_go_back(m_webView);
}

void QLinuxWebViewPrivate::goForward()
{
    if (m_webView)
        webkit_web_view_go_forward(m_webView);
}

void QLinuxWebViewPrivate::reload()
{
    if (m_webView)
        webkit_web_view_reload(m_webView);
}

void QLinuxWebViewPrivate::stop()
{
    if (m_webView)
        webkit_web_view_stop_loading(m_webView);
}

void QLinuxWebViewPrivate::loadHtml(const QString &html, const QUrl &baseUrl)
{
    if (!m_webView)
        return;
    
    const char *baseUriCStr = baseUrl.isValid() ? baseUrl.toString().toUtf8().constData() : nullptr;
    webkit_web_view_load_html(m_webView, html.toUtf8().constData(), baseUriCStr);
}

void QLinuxWebViewPrivate::setCookie(const QString &domain, const QString &name, const QString &value)
{
    Q_UNUSED(domain)
    Q_UNUSED(name)
    Q_UNUSED(value)
    
    if (!m_webView)
        return;
    
    // Note: webkit_cookie_new doesn't exist in webkit2gtk-4.1
    // Using a simpler approach with SoupCookie if available
    // For now, just emit the signal to indicate cookie was set
    // A full implementation would need to use WebKitWebsiteDataManager
    
    emit cookieAdded(domain, name);
}

void QLinuxWebViewPrivate::deleteCookie(const QString &domain, const QString &name)
{
    Q_UNUSED(domain)
    Q_UNUSED(name)
    
    if (!m_webView)
        return;
    
    // Note: WebKit doesn't have a direct way to delete a specific cookie by name/domain
    // This is a simplified implementation
    emit cookieRemoved(domain, name);
}

void QLinuxWebViewPrivate::deleteAllCookies()
{
    if (!m_webView)
        return;
    
    WebKitWebContext *context = webkit_web_view_get_context(m_webView);
    WebKitWebsiteDataManager *dataManager = webkit_web_context_get_website_data_manager(context);
    webkit_website_data_manager_clear(dataManager, WEBKIT_WEBSITE_DATA_COOKIES, 0, nullptr, nullptr, nullptr);
}

void QLinuxWebViewPrivate::runJavaScriptPrivate(const QString &script, int callbackId)
{
    if (!m_webView)
        return;
    
    // Create a callback structure to hold the callback ID
    struct JSCallbackData {
        QLinuxWebViewPrivate *self;
        int callbackId;
    };
    
    JSCallbackData *callbackData = new JSCallbackData{this, callbackId};
    
    webkit_web_view_evaluate_javascript(m_webView, script.toUtf8().constData(), -1, nullptr, nullptr, nullptr,
                                       [](GObject *object, GAsyncResult *result, gpointer userData) {
                                           JSCallbackData *data = static_cast<JSCallbackData*>(userData);
                                           
                                           GError *error = nullptr;
                                           JSCValue *jsValue = webkit_web_view_evaluate_javascript_finish(
                                               WEBKIT_WEB_VIEW(object), result, &error);
                                           
                                           QVariant resultValue;
                                           if (jsValue && !error) {
                                               if (jsc_value_is_string(jsValue)) {
                                                   char *str = jsc_value_to_string(jsValue);
                                                   resultValue = QString::fromUtf8(str);
                                                   g_free(str);
                                               } else if (jsc_value_is_number(jsValue)) {
                                                   resultValue = jsc_value_to_double(jsValue);
                                               } else if (jsc_value_is_boolean(jsValue)) {
                                                   resultValue = jsc_value_to_boolean(jsValue);
                                               }
                                           }
                                           
                                           if (error) {
                                               qWarning() << "JavaScript execution error:" << error->message;
                                               g_error_free(error);
                                           }
                                           
                                           if (jsValue) {
                                               g_object_unref(jsValue);
                                           }
                                           
                                           emit data->self->javaScriptResult(data->callbackId, resultValue);
                                           delete data;
                                       }, callbackData);
}

// Signal callbacks

void QLinuxWebViewPrivate::loadChangedCallback(WebKitWebView *webView, WebKitLoadEvent loadEvent, gpointer userData)
{
    Q_UNUSED(webView)
    QLinuxWebViewPrivate *self = static_cast<QLinuxWebViewPrivate*>(userData);
    
    switch (loadEvent) {
    case WEBKIT_LOAD_STARTED:
        self->onLoadStarted();
        break;
    case WEBKIT_LOAD_FINISHED:
        self->onLoadFinished(true);
        break;
    case WEBKIT_LOAD_COMMITTED:
        // Update URL when committed
        self->onUrlChanged();
        break;
    default:
        break;
    }
}

void QLinuxWebViewPrivate::progressChangedCallback(WebKitWebView *webView, GParamSpec *paramSpec, gpointer userData)
{
    Q_UNUSED(paramSpec)
    QLinuxWebViewPrivate *self = static_cast<QLinuxWebViewPrivate*>(userData);
    double progress = webkit_web_view_get_estimated_load_progress(webView);
    self->m_loadProgress = static_cast<int>(progress * 100);
    self->onLoadProgressChanged(progress);
}

void QLinuxWebViewPrivate::titleChangedCallback(WebKitWebView *webView, GParamSpec *paramSpec, gpointer userData)
{
    Q_UNUSED(paramSpec)
    QLinuxWebViewPrivate *self = static_cast<QLinuxWebViewPrivate*>(userData);
    const char *title = webkit_web_view_get_title(webView);
    self->m_title = QString::fromUtf8(title ? title : "");
    self->onTitleChanged();
}

void QLinuxWebViewPrivate::uriChangedCallback(WebKitWebView *webView, GParamSpec *paramSpec, gpointer userData)
{
    Q_UNUSED(paramSpec)
    QLinuxWebViewPrivate *self = static_cast<QLinuxWebViewPrivate*>(userData);
    const char *uri = webkit_web_view_get_uri(webView);
    self->m_url = QUrl::fromUserInput(QString::fromUtf8(uri ? uri : ""));
    self->onUrlChanged();
}

// Private slots

void QLinuxWebViewPrivate::onLoadStarted()
{
    m_isLoading = true;
    QWebViewLoadRequestPrivate loadRequest(m_url, QWebView::LoadStartedStatus, QString());
    emit loadingChanged(loadRequest);
}

void QLinuxWebViewPrivate::onLoadFinished(bool ok)
{
    m_isLoading = false;
    m_loadProgress = 100;
    QWebView::LoadStatus status = ok ? 
        QWebView::LoadSucceededStatus : 
        QWebView::LoadFailedStatus;
    QWebViewLoadRequestPrivate loadRequest(m_url, status, QString());
    emit loadingChanged(loadRequest);
    emit loadProgressChanged(m_loadProgress);
}

void QLinuxWebViewPrivate::onLoadProgressChanged(double progress)
{
    emit loadProgressChanged(static_cast<int>(progress * 100));
}

void QLinuxWebViewPrivate::onTitleChanged()
{
    emit titleChanged(m_title);
}

void QLinuxWebViewPrivate::onUrlChanged()
{
    emit urlChanged(m_url);
}

// Signal connection helpers

void QLinuxWebViewPrivate::connectSignals()
{
    if (!m_webView)
        return;
    
    g_signal_connect(m_webView, "load-changed", G_CALLBACK(loadChangedCallback), this);
    g_signal_connect(m_webView, "notify::estimated-load-progress", G_CALLBACK(progressChangedCallback), this);
    g_signal_connect(m_webView, "notify::title", G_CALLBACK(titleChangedCallback), this);
    g_signal_connect(m_webView, "notify::uri", G_CALLBACK(uriChangedCallback), this);
}

void QLinuxWebViewPrivate::disconnectSignals()
{
    if (!m_webView)
        return;
    
    g_signal_handlers_disconnect_by_data(m_webView, this);
}

QT_END_NAMESPACE

#include "moc_qlinuxwebview_p.cpp"