/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 Inesonic, LLC.
*
* MIT License:
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
*   documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
*   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
*   permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
*   Software.
*   
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
*   OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
*   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
********************************************************************************************************************//**
* \file
*
* This header provides tests for the \ref WebHook class.
***********************************************************************************************************************/

#ifndef TEST_WEB_HOOK_H
#define TEST_WEB_HOOK_H

#include <QObject>
#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QByteArray>
#include <QNetworkReply>

class QNetworkAccessManager;
class QSettings;
class QEventLoop;
class QJsonDocument;
class QByteArray;

namespace Wh {
    class WebHook;
}

class TestWebHook:public QObject {
    Q_OBJECT

    public:
        TestWebHook();

        ~TestWebHook() override;

    protected slots: // protected to keep the test framework from thinking these are test cases.
        void timeDeltaUpdated();

        void jsonResponseReceived(const QJsonDocument& jsonResponse);

        void responseReceived(const QByteArray& rawData);

        void failedToSend(int networkError);

    private slots:
        void initTestCase();

        void testTimeDelta();
        void testMessage();
        void testMessageWithDelta();

        void cleanupTestCase();

    private:
        static const QByteArray timeStampSecret;
        static const QString    timeStampWebHookUrl;

        static const QByteArray testSecret;
        static const QString    testWebHookUrl;

        QNetworkAccessManager*  networkAccessManager;
        Wh::WebHook*            webHook;

        QJsonDocument           reportedJsonData;
        QByteArray              reportedRawData;

        QEventLoop*             eventLoop;
        bool                    quitOnTimestampUpdate;
        bool                    operationFailed;
        bool                    receivedJsonData;
        bool                    receivedRawData;
        bool                    timeDeltaWasUpdated;
};

#endif
