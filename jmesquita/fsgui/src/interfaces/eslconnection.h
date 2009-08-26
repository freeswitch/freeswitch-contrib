/*
 * Copyright (c) 2007, Anthony Minessale II
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributor(s):
 *
 * Joao Mesquita <jmesquita (at) freeswitch.org>
 *
 */

#ifndef ESLCONNECTION_H
#define ESLCONNECTION_H
#include <QThread>
#include <QtGui>
#include <esl.h>
#include "eslevent.h"

/*!
  \brief Objects of this type are queued on ESLconnection for further processing.
  */
class CommandTransaction : public QObject
{
    Q_OBJECT
public:
    /*!
      \brief CommandTransaction constructor
      \param command The command to be executed
      */
    CommandTransaction(QString command);
    /*!
      \brief Retrieves the command to be executed.
      \return The command string currently set
      */
    QString getCommand() { return _command; }
    /*!
      \brief Set/override the current command
      \param command The new command string
      */
    void setCommand(QString command) { _command = command; }
    /*!
      \brief After ESLconnection has properly executed the command,
        it will set the response event using this method. The object will later emit a signal with the event set.
       \param e Response event that is being set
      */
    void setResponse(ESLevent e);
signals:
    /*!
      \brief Signal is emitted when setResponse is called by ESLconnection.
      \param The event that has been set on setResponse
      */
    void gotResponse(ESLevent);
private:
    /*! Current command set. */
    QString _command;
};


/*!
  \brief This class is the interface to the esl library. It will process comands and events
  coming from the lib.
  */
class ESLconnection : public QThread
{
    Q_OBJECT

 public:
    /*!
      \brief Constructor
      \param host The host to connect to
      \param port The port to connect to
      \param password The password to be used
      \param name The name of this connection. This is used so that plugins can lookup esl connections.
      */
    ESLconnection(const char *host, const char *port, const char *password, const char *name);
    virtual ~ESLconnection();
    /*!
      \brief Adds a new command object to the queue of commands to be processed.
      \param The command to be queued.
      */
    void addCommand(CommandTransaction *);
    /*!
      \brief Method that returns if handle is connected to server
      \return 1 if connected and 0 if not
      */
    int isConnected();
    /*!
      \brief Connects the handle to server
      */
    void connect();
    /*!
      \brief Disconnects the handle from the server
      */
    int disconnect(void);
    /*!
      \brief Retrieves the name of this connection
      \return Connection name
      */
    QString getName();
    /*!
      \brief Sets the console log level on server side
      \param loglevel to be set
      */
    bool setConsoleLogLevel(int);
signals:
    /*!
      \brief Emitted when this object gets connected to server
      */
    void connected(void);
    /*!
      \brief Emitted when this object gets disconnected from server
      */
    void disconnected(void);
    /*!
      \brief Emitted when a connection failure happens
      \param The error description as informed by libesl
      */
    void connectionFailed(QString);
    /*!
      \brief Emitted when a console log message is received
      \param Event corresponding to the log message
      */
    void receivedLogMessage(ESLevent);
    /*!
      \brief Emitted when a channel message is received
      \param Event corresponding the channel event
      */
    void receivedChannelEvent(ESLevent);
protected:
    /*!
      \brief This is the thread loop. It will proceed with polling libesl for new incoming events and
      processing commands sent by FsGui and its plugins.
      */
    void run();
private:
    esl_handle_t *handle; /*< The handle that is used by libesl */
    QString *_host; /*< The hostname used */
    QString *_port; /*< The port used */
    QString *_pass; /*< The password used */
    QString *_name; /*< The connection name */
    QQueue<CommandTransaction *> _commandQueue; /*< The command queue, processed on ESLconnection::run() */
    QHash<QString, CommandTransaction *> _commandHash; /*< Data structure that will hash command resposes so it know where to dispatch events */
    QMutex _commandQueueMutex; /*< A mutex that will make sure no read/write problems occur on _commandHash */

    /*!
      \brief Process every received event so it knows where to dispatch
      \param The newly received event
      */
    void processReceivedEvent(ESLevent *);
    /*!
      \brief Sends a command and it blocks until respose event is reseived
      \param The command to be sent
      \return The response event
      */
    ESLevent *sendRecv(const char *cmd);
    /*!
      \brief Polls the handle for new events on a timely fashion
      \param ms Maximum number of miliseconds to block
      \return And event received or NULL
      */
    ESLevent *recvEventTimed(int ms);
    /*!
      \brief Sends a command and waits for server to respond with the command UUID
      \param cmd The command to send
      \return The event containing the command UUID header
      */
    ESLevent* bgapi(QString cmd);
    /*!
      \brief Sends a command without blocking
      \param cmd The command to send
      */
    void send(QString cmd);
};

/*!
  \brief This global function will set what is the log verbosity of the esl lib. Used for debugging only.
  \param level The log level to be set.
  */
void eslSetLogLevel(int level);

#endif // ESLCONNECTION_H
