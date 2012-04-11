/*
  This file is part of the PhantomJS project from Ofi Labs.

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

#include "repl.h"

#include <QDesktopServices>
#include <QTimer>
#include <QDir>
#include <QRegExp>

#include "consts.h"
#include "terminal.h"
#include "utils.h"

#define PROMPT                          "phantomjs> "
#define HISTORY_FILENAME                "phantom_repl_history"

// Only with word characters, spaces and the dot ('.')
//  we can still attempt to offer a completion to the user
#define REGEXP_NON_COMPLETABLE_CHARS    "[^\\w\\s\\.]"

// JS Code to find possible completions
#define JS_RETURN_POSSIBLE_COMPLETIONS "REPL._getCompletions(%1, \"%2\");"

// JS Code to evaluate User Input and prettify the expression result
#define JS_EVAL_USER_INPUT \
    "try { " \
        "REPL._lastEval = eval(\"%1\");" \
        "console.log(JSON.stringify(REPL._lastEval, REPL._expResStringifyReplacer, '   ')); " \
    "} catch(e) { " \
        "if (e instanceof TypeError) { " \
            "console.error(\"'%1' is a cyclic structure\"); " \
        "} else { " \
            "console.error(e.message);" \
        "}" \
    "} "


// public:
bool REPL::instanceExists()
{
    return REPL::getInstance() != NULL;
}

REPL *REPL::getInstance(QWebFrame *webframe, Phantom *parent)
{
    static REPL *singleton = NULL;
    if (!singleton && webframe && parent) {
        // This will create the singleton only when all the parameters are given
        singleton = new REPL(webframe, parent);
    }
    return singleton;
}

// private:
REPL::REPL(QWebFrame *webframe, Phantom *parent)
    : QObject(parent),
      m_looping(true)
{
    m_webframe = webframe;
    m_parentPhantom = parent;
    m_historyFilepath = QString("%1/%2").arg(
                QDesktopServices::storageLocation(QDesktopServices::DataLocation),
                HISTORY_FILENAME).toLocal8Bit();

    // Ensure the location for the history file exists
    QDir().mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

    // Listen for Phantom exit(ing)
    connect(m_parentPhantom, SIGNAL(aboutToExit(int)), this, SLOT(stopLoop(int)));

    // Set the static callback to offer Completions to the User
    linenoiseSetCompletionCallback(REPL::offerCompletion);

    // Inject REPL utility functions
    m_webframe->evaluateJavaScript(Utils::readResourceFileUtf8(":/repl.js"), QString("phantomjs://repl.js"));

    // Start the REPL's loop
    QTimer::singleShot(0, this, SLOT(startLoop()));
}

void REPL::offerCompletion(const char *buf, linenoiseCompletions *lc)
{
    // IF there is a ( or ), then do nothing (we can't complete)
    QString buffer(buf);
    int lastIndexOfDot = -1;
    QString toInspect, toComplete;
    QRegExp nonCompletableChars(REGEXP_NON_COMPLETABLE_CHARS);

    // If we encounter a non acceptable character (see above)
    if (buffer.contains(nonCompletableChars)) {
        return;
    }

    // Decompose what user typed so far in 2 parts: what toInspect and what toComplete.
    lastIndexOfDot = buffer.lastIndexOf('.');
    if (lastIndexOfDot > -1) {
        toInspect = buffer.left(lastIndexOfDot);
        toComplete = buffer.right(buffer.length() - lastIndexOfDot -1);
    } else {
        // Nothing to inspect: use the global "window" object
        toInspect = "window";
        toComplete = buffer;
    }

    // This will return an array of String with the possible completions
    QStringList completions = REPL::getInstance()->m_webframe->evaluateJavaScript(
                QString(JS_RETURN_POSSIBLE_COMPLETIONS).arg(
                    toInspect,
                    toComplete),
                QString()
                ).toStringList();

    foreach (QString c, completions) {
        if (lastIndexOfDot > -1) {
            // Preserve the "toInspect" portion of the string to complete
            linenoiseAddCompletion(lc, QString("%1.%2").arg(toInspect, c).toLocal8Bit().data());
        } else {
            linenoiseAddCompletion(lc, c.toLocal8Bit().data());
        }
    }
}

// private slots:
void REPL::startLoop()
{
    char *userInput;

    // Load REPL history
    linenoiseHistoryLoad(m_historyFilepath.data());         //< requires "char *"
    while(m_looping && (userInput = linenoise(PROMPT)) != NULL) {
        if (userInput[0] != '\0') {
            // Send the user input to the main Phantom frame for evaluation
            m_webframe->evaluateJavaScript(
                        QString(JS_EVAL_USER_INPUT).arg(
                            QString(userInput).replace('"', "\\\"")), QString("phantomjs://repl-input"));

            // Save command in the REPL history
            linenoiseHistoryAdd(userInput);
            linenoiseHistorySave(m_historyFilepath.data()); //< requires "char *"
        }
        free(userInput);
    }

    // If still "looping", close Phantom (usually caused by "CTRL+C" / "CTRL+D")
    if (m_looping) {
        m_parentPhantom->exit();
    }
}

void REPL::stopLoop(const int code)
{
    Q_UNUSED(code);
    m_looping = false;
}
