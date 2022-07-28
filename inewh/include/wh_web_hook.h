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
* This header defines the \ref Wh::WebHook class.
***********************************************************************************************************************/

/* .. sphinx-project inewh */

#ifndef WH_WEB_HOOK_H
#define WH_WEB_HOOK_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QDateTime>
#include <QUrl>

#include <cstdint>

#include "wh_common.h"

class QTimer;
class QDateTime;
class QByteArray;
class QNetworkAccessManager;
class QJsonObject;
class QJsonDocument;
class QNetworkReply;

namespace Wh {
    /**
     * Class that provides support for generic Inesonic web hooks.
     */
    class WH_PUBLIC_API WebHook:public QObject {
        Q_OBJECT

        public:
            /**
             * Constructor
             *
             * \param[in] networkAccessManager The network settings manager.
             *
             * \param[in] parent               Pointer to the parent object.
             */
            WebHook(QNetworkAccessManager* networkAccessManager, QObject* parent = Q_NULLPTR);

            /**
             * Constructor
             *
             * \param[in] networkAccessManager The network settings manager.
             *
             * \param[in] webhookSecret        The secret used to authenticate the request with the remote server.
             *
             * \param[in] parent               Pointer to the parent object.
             */
            WebHook(
                QNetworkAccessManager* networkAccessManager,
                const QByteArray&      webhookSecret,
                QObject*               parent = Q_NULLPTR
            );

            ~WebHook() override;

            /**
             * Method you can use to set the global timestamp secret.
             *
             * \param[in] newTimestampSecret The new webhook secret.
             */
            static void setTimestampSecret(const QByteArray& newTimestampSecret);

            /**
             * Method you can use to obtain the current global timestamp secret.
             *
             * \return Returns the current global timestamp secret.
             */
            static const QByteArray& timestampSecret();

            /**
             * Method you can use to set the global timestamp URL.
             *
             * \param[in] timestampWebhookUrl The timestamp webhook URL to be used to measure time deltas.
             */
            static void setTimestampUrl(const QUrl& timestampWebhookUrl);

            /**
             * Method you can use to obtain the current global timestamp URL.
             *
             * \return Returns the currently selected timestamp URL.
             */
            static const QUrl& timestampUrl();

            /**
             * Method you can use to force the time delta.  This method is primarily intended for test purposes.
             *
             * \param[in] newTimeDelta The new time delta to be applied.
             */
            static void setTimeDelta(long long newTimeDelta);

            /**
             * Method you can use to obtain the current measured time delta.
             *
             * \return Returns the current measured time delta.
             */
            static long long timeDelta();

        signals:
            /**
             * Signal that is emitted when a valid JSON response is received.
             *
             * \param[out] jsonDocument A JSON document holding the received response.
             */
            void jsonResponseReceived(const QJsonDocument& jsonDocument);

            /**
             * Signal that is emitted when a valid response is received.
             *
             * \param[out] rawData The raw response data.
             */
            void responseReceived(const QByteArray& rawData);

            /**
             * Signal that is emitted when no response has been received or an invalid response has been received.
             *
             * \param[out] networkError The last reported network error.  This is the value of
             *                          QNetworkReply::NetworkError cast to an integer.
             */
            void failedToSend(int networkError);

            /**
             * Signal that is emitted when the internal time delta is updated.  This signal is primarily intended for
             * test purposes.
             */
            void timeDeltaUpdated();

        public slots:
            /**
             * Slot you can trigger to send a message.
             *
             * \param[in] destinationUrl The URL where the message should be received.
             *
             * \param[in] jsonDocument   The JSON payload to be sent.
             */
            void send(const QUrl& destinationUrl, const QJsonDocument& jsonDocument);

            /**
             * Slot you can trigger to send a message.
             *
             * \param[in] destinationUrl The URL where the message should be received.
             *
             * \param[in] jsonObject   The JSON payload to be sent.
             */
            void send(const QUrl& destinationUrl, const QJsonObject& jsonObject);

            /**
             * Slot you can trigger to force a time delta adjustment.
             */
            void forceTimeDeltaAdjustment();

        protected:
            /**
             * Method you can overload to intercept valid responses.  The default implementation triggers the
             * \ref WebHook::jsonResponseReceived signal.
             *
             * \param[in] jsonDocument A JSON document holding the received response.
             */
            virtual void jsonResponseWasReceived(const QJsonDocument& jsonDocument);

            /**
             * Method you can overload to intercept non-JSON responses.  The default implementation triggers the
             * \ref WebHook::responseReceived signal.
             *
             * \param[in] rawData The raw response data.
             */
            virtual void responseWasReceived(const QByteArray& rawData);

            /**
             * Method you can overload to intercept failure notifications.  The default implementation triggers the
             * \ref WebHook::failedToSend signal.
             *
             * \param[in] networkError The last reported network error.  This is the value of
             *                         QNetworkReply::NetworkError cast to an integer.
             */
            virtual void failed(int networkError);

        private slots:
            /**
             * Slot that is triggered in response to a timestamp adjustment.
             */
            void timestampReplyReceived();

            /**
             * Slot that is triggered when a response to the requested message is received.
             */
            void messageResponseReceived();

            /**
             * Method that is called to send the requested message.
             */
            void doSend();

            /**
             * Method that is called to trigger a request for a timestamp adjustment.
             */
            void doTimestampAdjustment();

        private:
            /**
             * Method that does common configuration for this object.
             */
            void configure();

            /**
             * The maximum number of allowed retries.
             */
            static constexpr unsigned maximumNumberRetries = 4;

            /**
             * The global timestamp secret.
             */
            static QByteArray globalTimestampSecret;

            /**
             * The global timestamp URL.
             */
            static QUrl globalTimestampUrl;

            /**
             * The current time delta value.
             */
            static long long globalTimeDelta;

            /**
             * Timer used to trigger the message to be resent.
             */
            QTimer* resendTimer;

            /**
             * Timer used to trigger the time delta to be recalculated.
             */
            QTimer* timeDeltaTimer;

            /**
             * The current webhook secret.
             */
            QByteArray currentSecret;

            /**
             * The network access manager.
             */
            QNetworkAccessManager* currentNetworkAccessManager;

            /**
             * The currently in-flight reply we're waiting to receive.  A null pointer indicates that
             */
            QNetworkReply* pendingReply;

            /**
             * The number of remaining retries.
             */
            unsigned remainingRetries;

            /**
             * The current destination URL.
             */
            QUrl currentUrl;

            /**
             * The payload to be sent.
             */
            QByteArray currentPayload;
    };
}

#endif
