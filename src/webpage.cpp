/*
  This file is part of the PhantomJS project from Ofi Labs.

  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>
  Copyright (C) 2011 Ivan De Marino <ivan.de.marino@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "webpage.h"
#include <math.h>

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QPainter>
#include <QPrinter>
#include <QWebElement>
#include <QWebFrame>
#include <QWebPage>
#include <QWebInspector>
#include <QNetworkProxy>

#include "networkaccessmanager.h"
#include "utils.h"

#include <gifwriter.h>

#include "consts.h"

// Ensure we have at least head and body.
#define BLANK_HTML "<html><head></head><body></body></html>"

class CustomPage: public QWebPage
{
    Q_OBJECT

public:
    CustomPage(WebPage *parent = 0)
        : QWebPage(parent)
        , m_webPage(parent)
    {
        m_userAgent = QWebPage::userAgentForUrl(QUrl());
        setForwardUnsupportedContent(true);
    }

public slots:
    bool shouldInterruptJavaScript() {
        QApplication::processEvents(QEventLoop::AllEvents, 42);
        return false;
    }

protected:

    QString chooseFile(QWebFrame *originatingFrame, const QString &oldFile) {
        Q_UNUSED(originatingFrame);
        Q_UNUSED(oldFile);
        return m_uploadFile;
    }

    void javaScriptAlert(QWebFrame *originatingFrame, const QString &msg) {
        Q_UNUSED(originatingFrame);
        m_webPage->emitAlert(msg);
    }

    void javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID) {
        Q_UNUSED(lineNumber);
        Q_UNUSED(sourceID);

        m_webPage->emitConsoleMessage(message);
    }

    void javaScriptError(const QWebPage::JavaScriptError& error) {
        m_webPage->emitError(error);
    }

    QString userAgentForUrl(const QUrl &url) const {
        Q_UNUSED(url);
        return m_userAgent;
    }

private:
    WebPage *m_webPage;
    QString m_userAgent;
    QString m_uploadFile;
    friend class WebPage;
};


WebPage::WebPage(QObject *parent, const Config *config, const QUrl &baseUrl)
    : REPLCompletable(parent)
{
    setObjectName("WebPage");
    m_webPage = new CustomPage(this);
    m_mainFrame = m_webPage->mainFrame();
    m_mainFrame->setHtml(BLANK_HTML, baseUrl);
    m_ee = false;

    connect(m_mainFrame, SIGNAL(javaScriptWindowObjectCleared()), SIGNAL(initialized()));
    connect(m_webPage, SIGNAL(loadStarted()), SIGNAL(loadStarted()), Qt::QueuedConnection);
    connect(m_webPage, SIGNAL(loadFinished(bool)), SLOT(finish(bool)), Qt::QueuedConnection);

    // Start with transparent background.
    QPalette palette = m_webPage->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    m_webPage->setPalette(palette);

    // Page size does not need to take scrollbars into account.
    m_mainFrame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    m_mainFrame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    m_webPage->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    m_webPage->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

    m_webPage->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    m_webPage->settings()->setOfflineWebApplicationCachePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

    m_webPage->settings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);

    m_webPage->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_webPage->settings()->setLocalStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

    // Custom network access manager to allow traffic monitoring.
    m_networkAccessManager = new NetworkAccessManager(this, config);
    m_webPage->setNetworkAccessManager(m_networkAccessManager);
    connect(m_networkAccessManager, SIGNAL(resourceRequested(QVariant)),
            SIGNAL(resourceRequested(QVariant)));
    connect(m_networkAccessManager, SIGNAL(resourceReceived(QVariant)),
            SIGNAL(resourceReceived(QVariant)));

    m_webPage->setViewportSize(QSize(400, 300));
}

QWebFrame *WebPage::mainFrame()
{
    return m_mainFrame;
}

QString WebPage::content() const
{
    return m_mainFrame->toHtml();
}

void WebPage::setProxyType(const QString value) {
    m_proxyType = value;
}

void WebPage::setProxy(const QString &value) {
    m_proxy = value;
}

void WebPage::setProxyAuth(const QString &proxyauth)
{
    m_proxyAuth = proxyauth;
}

void WebPage::setEE(const QString value) {
    if (value == "true")
        m_ee = true;
    else
        m_ee = false;
}

void WebPage::applyProxy() {
        QNetworkProxy::ProxyType networkProxyType = QNetworkProxy::HttpProxy;

        if (m_proxyType == "socks5") {
            networkProxyType = QNetworkProxy::Socks5Proxy;
        }

    QString proxyUser = m_proxyAuth;
    QString proxyPass = "";

    if (proxyUser.lastIndexOf(':') > 0) {
        proxyPass = proxyUser.mid(proxyUser.lastIndexOf(':') + 1).trimmed();
        proxyUser = proxyUser.left(proxyUser.lastIndexOf(':')).trimmed();
    }

    QString proxyHost = m_proxy;
    int proxyPort = 1080;

    if (proxyHost.lastIndexOf(':') > 0) {
        bool ok = true;
        int port = proxyHost.mid(proxyHost.lastIndexOf(':') + 1).toInt(&ok);
        if (ok) {
            proxyHost = proxyHost.left(proxyHost.lastIndexOf(':')).trimmed();
            proxyPort = port;
        }
    }

    /*printf("%s\n",(const char *)m_proxyAuth.toAscii().constData());
    printf("%s\n",(const char *)m_proxy.toAscii().constData());
    printf("%s\n",(const char *)m_proxyType.toAscii().constData());
    printf("%s\n",(const char *)proxyUser.toAscii().constData());
    printf("%s\n",(const char *)proxyPass.toAscii().constData());
    printf("%s\n",(const char *)proxyHost.toAscii().constData());
    printf("%d\n",proxyPort);
*/
    QNetworkProxy proxy;
     proxy.setType(networkProxyType);
     proxy.setHostName(proxyHost);
     proxy.setPort(proxyPort);
     proxy.setUser(proxyUser);
     proxy.setPassword(proxyPass);
 
     m_networkAccessManager->setProxy(proxy);
 //    printf("Set proxy!\n");
}

void WebPage::setContent(const QString &content)
{
    m_mainFrame->setHtml(content);
}

QString WebPage::plainText() const
{
    return m_mainFrame->toPlainText();
}

QString WebPage::libraryPath() const
{
   return m_libraryPath;
}

void WebPage::setLibraryPath(const QString &libraryPath)
{
   m_libraryPath = libraryPath;
}

void
WebPage::showInspector(const int port)
{
    m_webPage->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    m_inspector = new QWebInspector;
    m_inspector->setPage(m_webPage);

    if (port == -1)
        m_inspector->setVisible(true);
    else {
        m_webPage->setProperty("_q_webInspectorServerPort", port);
    }
}


void WebPage::applySettings(const QVariantMap &def)
{
    QWebSettings *opt = m_webPage->settings();

    opt->setAttribute(QWebSettings::AutoLoadImages, def[PAGE_SETTINGS_LOAD_IMAGES].toBool());
    opt->setAttribute(QWebSettings::PluginsEnabled, def[PAGE_SETTINGS_LOAD_PLUGINS].toBool());
    opt->setAttribute(QWebSettings::JavascriptEnabled, def[PAGE_SETTINGS_JS_ENABLED].toBool());
    opt->setAttribute(QWebSettings::XSSAuditingEnabled, def[PAGE_SETTINGS_XSS_AUDITING].toBool());
    opt->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, def[PAGE_SETTINGS_LOCAL_ACCESS_REMOTE].toBool());
    opt->setAttribute(QWebSettings::WebSecurityEnabled, def[PAGE_SETTINGS_WEB_SECURITY_ENABLED].toBool());

    if (def.contains(PAGE_SETTINGS_USER_AGENT))
        m_webPage->m_userAgent = def[PAGE_SETTINGS_USER_AGENT].toString();

    if (def.contains(PAGE_SETTINGS_USERNAME))
        m_networkAccessManager->setUserName(def[PAGE_SETTINGS_USERNAME].toString());

    if (def.contains(PAGE_SETTINGS_PASSWORD))
        m_networkAccessManager->setPassword(def[PAGE_SETTINGS_PASSWORD].toString());
}

QString WebPage::userAgent() const
{
    return m_webPage->m_userAgent;
}

void WebPage::setViewportSize(const QVariantMap &size)
{
    int w = size.value("width").toInt();
    int h = size.value("height").toInt();
    if (w > 0 && h > 0)
        m_webPage->setViewportSize(QSize(w, h));
}

QVariantMap WebPage::viewportSize() const
{
    QVariantMap result;
    QSize size = m_webPage->viewportSize();
    result["width"] = size.width();
    result["height"] = size.height();
    return result;
}

void WebPage::setClipRect(const QVariantMap &size)
{
    int w = size.value("width").toInt();
    int h = size.value("height").toInt();
    int top = size.value("top").toInt();
    int left = size.value("left").toInt();

    if (w >= 0 && h >= 0)
        m_clipRect = QRect(left, top, w, h);
}

QVariantMap WebPage::clipRect() const
{
    QVariantMap result;
    result["width"] = m_clipRect.width();
    result["height"] = m_clipRect.height();
    result["top"] = m_clipRect.top();
    result["left"] = m_clipRect.left();
    return result;
}


void WebPage::setScrollPosition(const QVariantMap &size)
{
    int top = size.value("top").toInt();
    int left = size.value("left").toInt();
    m_scrollPosition = QPoint(left,top);
    m_mainFrame->setScrollPosition(m_scrollPosition);
}

QVariantMap WebPage::scrollPosition() const
{
    QVariantMap result;
    result["top"] = m_scrollPosition.y();
    result["left"] = m_scrollPosition.x();
    return result;
}

void WebPage::setPaperSize(const QVariantMap &size)
{
    m_paperSize = size;
}

QVariantMap WebPage::paperSize() const
{
    return m_paperSize;
}

QVariant WebPage::evaluate(const QString &code)
{
    if (m_ee)
        printf("\n*******\n%s\n********\n",code.toAscii().constData());
    QString function = "(" + code + ")()";
    return m_mainFrame->evaluateJavaScript(function, QString("phantomjs://webpage.evaluate()"));
}

QVariant WebPage::evaluate(const QString &code, const QString &params)
{
    QString function = "(" + code + ")(" + params + ")";
    if (m_ee)
        printf("\n*******\n%s\n********\n",function.toAscii().constData());
    return m_mainFrame->evaluateJavaScript(function, QString("phantomjs://webpage.evaluate()"));
}

void WebPage::emitAlert(const QString &msg)
{
    emit javaScriptAlertSent(msg);
}

void WebPage::emitConsoleMessage(const QString &message)
{
    emit javaScriptConsoleMessageSent(message);
}

void WebPage::emitError(const QWebPage::JavaScriptError& error)
{
    QList<QWebPage::JavaScriptFrame> backtrace = error.backtrace();
    QVariantList newBacktrace = QVariantList();

    for (int i = 0; i < backtrace.size(); ++i) {
        QWebPage::JavaScriptFrame frame = backtrace.at(i);

        QVariantMap newFrame = QVariantMap();
        newFrame["file"] = frame.file();
        newFrame["line"] = frame.line();
        newFrame["function"] = frame.function();

        newBacktrace << newFrame;
    }

    emit javaScriptErrorSent(error.message(), newBacktrace);
}

void WebPage::finish(bool ok)
{
    QString status = ok ? "success" : "fail";
    emit loadFinished(status);
}

void WebPage::setCustomHeaders(const QVariantMap &headers)
{
    m_networkAccessManager->setCustomHeaders(headers);
}

QVariantMap WebPage::customHeaders() const
{
    return m_networkAccessManager->customHeaders();
}

void WebPage::openUrl(const QString &address, const QVariant &op, const QVariantMap &settings)
{
    QString operation;
    QByteArray body;

    applySettings(settings);
    m_webPage->triggerAction(QWebPage::Stop);

    if (op.type() == QVariant::String)
        operation = op.toString();

    if (op.type() == QVariant::Map) {
        operation = op.toMap().value("operation").toString();
        body = op.toMap().value("data").toByteArray();
    }

    if (operation.isEmpty())
        operation = "get";

    QNetworkAccessManager::Operation networkOp = QNetworkAccessManager::UnknownOperation;
    operation = operation.toLower();
    if (operation == "get")
        networkOp = QNetworkAccessManager::GetOperation;
    else if (operation == "head")
        networkOp = QNetworkAccessManager::HeadOperation;
    else if (operation == "put")
        networkOp = QNetworkAccessManager::PutOperation;
    else if (operation == "post")
        networkOp = QNetworkAccessManager::PostOperation;
    else if (operation == "delete")
        networkOp = QNetworkAccessManager::DeleteOperation;

    if (networkOp == QNetworkAccessManager::UnknownOperation) {
        m_mainFrame->evaluateJavaScript("console.error('Unknown network operation: " + operation + "');", QString());
        return;
    }

    if (address == "about:blank") {
        m_mainFrame->setHtml(BLANK_HTML);
    } else {
        QUrl url = QUrl::fromEncoded(QByteArray(address.toAscii()));

#if QT_VERSION == QT_VERSION_CHECK(4, 8, 0)
        // Assume local file if scheme is empty
        if (url.scheme().isEmpty()) {
            url.setPath(QFileInfo(url.toString()).absoluteFilePath().prepend("/"));
            url.setScheme("file");
        }
#endif

        m_mainFrame->load(QNetworkRequest(url), networkOp, body);
    }
}

void WebPage::release()
{
    deleteLater();
}

bool WebPage::render(const QString &fileName)
{
    if (m_mainFrame->contentsSize().isEmpty())
        return false;

    QFileInfo fileInfo(fileName);
    QDir dir;
    dir.mkpath(fileInfo.absolutePath());

    if (fileName.endsWith(".pdf", Qt::CaseInsensitive))
        return renderPdf(fileName);

    QImage buffer = renderImage();
    if (fileName.toLower().endsWith(".gif")) {
        return exportGif(buffer, fileName);
    }

    return buffer.save(fileName);
}

QImage WebPage::renderImage()
{
    QSize contentsSize = m_mainFrame->contentsSize();
    contentsSize -= QSize(m_scrollPosition.x(), m_scrollPosition.y());
    QRect frameRect = QRect(QPoint(0, 0), contentsSize);
    if (!m_clipRect.isNull())
        frameRect = m_clipRect;

    QSize viewportSize = m_webPage->viewportSize();
    m_webPage->setViewportSize(contentsSize);

    QImage buffer(frameRect.size(), QImage::Format_ARGB32);
    buffer.fill(qRgba(255, 255, 255, 0));

    QPainter painter;

    // We use tiling approach to work-around Qt software rasterizer bug
    // when dealing with very large paint device.
    // See http://code.google.com/p/phantomjs/issues/detail?id=54.
    const int tileSize = 4096;
    int htiles = (buffer.width() + tileSize - 1) / tileSize;
    int vtiles = (buffer.height() + tileSize - 1) / tileSize;
    for (int x = 0; x < htiles; ++x) {
        for (int y = 0; y < vtiles; ++y) {

            QImage tileBuffer(tileSize, tileSize, QImage::Format_ARGB32);
            tileBuffer.fill(qRgba(255, 255, 255, 0));

            // Render the web page onto the small tile first
            painter.begin(&tileBuffer);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter.translate(-frameRect.left(), -frameRect.top());
            painter.translate(-x * tileSize, -y * tileSize);
            m_mainFrame->render(&painter, QRegion(frameRect));
            painter.end();

            // Copy the tile to the main buffer
            painter.begin(&buffer);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawImage(x * tileSize, y * tileSize, tileBuffer);
            painter.end();
        }
    }

    m_webPage->setViewportSize(viewportSize);

    return buffer;
}

#define PHANTOMJS_PDF_DPI 72            // Different defaults. OSX: 72, X11: 75(?), Windows: 96

qreal stringToPointSize(const QString &string)
{
    static const struct {
        QString unit;
        qreal factor;
    } units[] = {
        { "mm", 72 / 25.4 },
        { "cm", 72 / 2.54 },
        { "in", 72 },
        { "px", 72.0 / PHANTOMJS_PDF_DPI / 2.54 },
        { "", 72.0 / PHANTOMJS_PDF_DPI / 2.54 }
    };
    for (uint i = 0; i < sizeof(units) / sizeof(units[0]); ++i) {
        if (string.endsWith(units[i].unit)) {
            QString value = string;
            value.chop(units[i].unit.length());
            return value.toDouble() * units[i].factor;
        }
    }
    return 0;
}

qreal printMargin(const QVariantMap &map, const QString &key)
{
    const QVariant margin = map.value(key);
    if (margin.isValid() && margin.canConvert(QVariant::String)) {
        return stringToPointSize(margin.toString());
    } else {
        return 0;
    }
}

bool WebPage::renderPdf(const QString &fileName)
{
    QPrinter printer;
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setResolution(PHANTOMJS_PDF_DPI);
    QVariantMap paperSize = m_paperSize;

    if (paperSize.isEmpty()) {
        const QSize pageSize = m_mainFrame->contentsSize();
        paperSize.insert("width", QString::number(pageSize.width()) + "px");
        paperSize.insert("height", QString::number(pageSize.height()) + "px");
        paperSize.insert("margin", "0px");
    }

    if (paperSize.contains("width") && paperSize.contains("height")) {
        const QSizeF sizePt(ceil(stringToPointSize(paperSize.value("width").toString())),
                            ceil(stringToPointSize(paperSize.value("height").toString())));
        printer.setPaperSize(sizePt, QPrinter::Point);
    } else if (paperSize.contains("format")) {
        const QPrinter::Orientation orientation = paperSize.contains("orientation")
                && paperSize.value("orientation").toString().compare("landscape", Qt::CaseInsensitive) == 0 ?
                    QPrinter::Landscape : QPrinter::Portrait;
        printer.setOrientation(orientation);
        static const struct {
            QString format;
            QPrinter::PaperSize paperSize;
        } formats[] = {
            { "A0", QPrinter::A0 },
            { "A1", QPrinter::A1 },
            { "A2", QPrinter::A2 },
            { "A3", QPrinter::A3 },
            { "A4", QPrinter::A4 },
            { "A5", QPrinter::A5 },
            { "A6", QPrinter::A6 },
            { "A7", QPrinter::A7 },
            { "A8", QPrinter::A8 },
            { "A9", QPrinter::A9 },
            { "B0", QPrinter::B0 },
            { "B1", QPrinter::B1 },
            { "B2", QPrinter::B2 },
            { "B3", QPrinter::B3 },
            { "B4", QPrinter::B4 },
            { "B5", QPrinter::B5 },
            { "B6", QPrinter::B6 },
            { "B7", QPrinter::B7 },
            { "B8", QPrinter::B8 },
            { "B9", QPrinter::B9 },
            { "B10", QPrinter::B10 },
            { "C5E", QPrinter::C5E },
            { "Comm10E", QPrinter::Comm10E },
            { "DLE", QPrinter::DLE },
            { "Executive", QPrinter::Executive },
            { "Folio", QPrinter::Folio },
            { "Ledger", QPrinter::Ledger },
            { "Legal", QPrinter::Legal },
            { "Letter", QPrinter::Letter },
            { "Tabloid", QPrinter::Tabloid }
        };
        printer.setPaperSize(QPrinter::A4); // Fallback
        for (uint i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
            if (paperSize.value("format").toString().compare(formats[i].format, Qt::CaseInsensitive) == 0) {
                printer.setPaperSize(formats[i].paperSize);
                break;
            }
        }
    } else {
        return false;
    }

    if (paperSize.contains("border") && !paperSize.contains("margin")) {
        // backwards compatibility
        paperSize["margin"] = paperSize["border"];
    }

    qreal marginLeft = 0;
    qreal marginTop = 0;
    qreal marginRight = 0;
    qreal marginBottom = 0;

    if (paperSize.contains("margin")) {
        const QVariant margins = paperSize["margin"];
        if (margins.canConvert(QVariant::Map)) {
            const QVariantMap map = margins.toMap();
            marginLeft = printMargin(map, "left");
            marginTop = printMargin(map, "top");
            marginRight = printMargin(map, "right");
            marginBottom = printMargin(map, "bottom");
        } else if (margins.canConvert(QVariant::String)) {
            const qreal margin = stringToPointSize(margins.toString());
            marginLeft = margin;
            marginTop = margin;
            marginRight = margin;
            marginBottom = margin;
        }
    }

    printer.setPageMargins(marginLeft, marginTop, marginRight, marginBottom, QPrinter::Point);

    m_mainFrame->print(&printer);
    return true;
}

void WebPage::uploadFile(const QString &selector, const QString &fileName)
{
    QWebElement el = m_mainFrame->findFirstElement(selector);
    if (el.isNull())
        return;

    m_webPage->m_uploadFile = fileName;
    el.evaluateJavaScript(JS_ELEMENT_CLICK);
}

bool WebPage::injectJs(const QString &jsFilePath) {
    //printf("InjectJs\n%s\n%s\n",jsFilePath.toAscii().constData(), m_libraryPath.toAscii().constData());
    return Utils::injectJsInFrame(jsFilePath, m_libraryPath, m_mainFrame);
}

void WebPage::_appendScriptElement(const QString &scriptUrl) {
    m_mainFrame->evaluateJavaScript( QString(JS_APPEND_SCRIPT_ELEMENT).arg(scriptUrl), scriptUrl );
}

void WebPage::sendEvent(const QString &type, const QVariant &arg1, const QVariant &arg2)
{
    if (type == "mousedown" ||  type == "mouseup" || type == "mousemove") {
        QMouseEvent::Type eventType = QEvent::None;
        Qt::MouseButton button = Qt::LeftButton;
        Qt::MouseButtons buttons = Qt::LeftButton;

        if (type == "mousedown")
            eventType = QEvent::MouseButtonPress;
        if (type == "mouseup")
            eventType = QEvent::MouseButtonRelease;
        if (type == "mousemove") {
            eventType = QEvent::MouseMove;
            button = Qt::NoButton;
            buttons = Qt::NoButton;
        }
        Q_ASSERT(eventType != QEvent::None);

        int x = arg1.toInt();
        int y = arg2.toInt();
        QMouseEvent *event = new QMouseEvent(eventType, QPoint(x, y), button, buttons, Qt::NoModifier);
        QApplication::postEvent(m_webPage, event);
        QApplication::processEvents();
        return;
    }

    if (type == "click") {
        sendEvent("mousedown", arg1, arg2);
        sendEvent("mouseup", arg1, arg2);
        return;
    }
}

void WebPage::initCompletions()
{
    // Add completion for the Dynamic Properties of the 'webpage' object
    // properties
    addCompletion("clipRect");
    addCompletion("content");
    addCompletion("libraryPath");
    addCompletion("settings");
    addCompletion("viewportSize");
    // functions
    addCompletion("evaluate");
    addCompletion("includeJs");
    addCompletion("injectJs");
    addCompletion("open");
    addCompletion("release");
    addCompletion("render");
    addCompletion("sendEvent");
    addCompletion("uploadFile");
    // callbacks
    addCompletion("onAlert");
    addCompletion("onConsoleMessage");
    addCompletion("onInitialized");
    addCompletion("onLoadStarted");
    addCompletion("onLoadFinished");
    addCompletion("onResourceRequested");
    addCompletion("onResourceReceived");
    // proxies
    addCompletion("setProxy");
    addCompletion("setProxyAuth");
    addCompletion("setProxyType");
    addCompletion("applyProxy");
    addCompletion("setEE");
}

#include "webpage.moc"
