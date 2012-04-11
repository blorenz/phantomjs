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

#ifndef PHANTOM_H
#define PHANTOM_H

#include <QPointer>

#include "csconverter.h"
#include "filesystem.h"
#include "encoding.h"
#include "config.h"
#include "replcompletable.h"
#include "system.h"

class WebPage;
class WebServer;

class Phantom: public REPLCompletable
{
    Q_OBJECT
    Q_PROPERTY(QStringList args READ args)
    Q_PROPERTY(QVariantMap defaultPageSettings READ defaultPageSettings)
    Q_PROPERTY(QString libraryPath READ libraryPath WRITE setLibraryPath)
    Q_PROPERTY(QString outputEncoding READ outputEncoding WRITE setOutputEncoding)
    Q_PROPERTY(QString scriptName READ scriptName)
    Q_PROPERTY(QVariantMap version READ version)
    Q_PROPERTY(QObject *page READ page)

public:
    Phantom(QObject *parent = 0);
    virtual ~Phantom();

    QStringList args() const;

    QVariantMap defaultPageSettings() const;

    QString outputEncoding() const;
    void setOutputEncoding(const QString &encoding);

    bool execute();
    int returnValue() const;

    QString libraryPath() const;
    void setLibraryPath(const QString &libraryPath);

    QString scriptName() const;

    QVariantMap version() const;

    QObject* page() const;
    

public slots:
    QObject *createWebPage();
    QObject *createWebServer();
    QObject *createFilesystem();
    QObject *createSystem();
    QString loadModuleSource(const QString &name);
    bool injectJs(const QString &jsFilePath);

    // exit() will not exit in debug mode. debugExit() will always exit.
    void exit(int code = 0);
    void debugExit(int code = 0);

    void setProxyType(const QString value);
    void setProxy(const QString &value);
    void setProxyAuth(const QString &proxyauth);
    void applyProxy();

signals:
    void aboutToExit(int code);

private slots:
    void printConsoleMessage(const QString &msg);

    void onInitialized();
private:
    void doExit(int code);
    virtual void initCompletions();

    Encoding m_scriptFileEnc;
    WebPage *m_page;
    bool m_terminated;
    int m_returnValue;
    QString m_script;
    QVariantMap m_defaultPageSettings;
    FileSystem *m_filesystem;
    System *m_system;
    QList<QPointer<WebPage> > m_pages;
    QList<QPointer<WebServer> > m_servers;
    Config m_config;
};

#endif // PHANTOM_H
