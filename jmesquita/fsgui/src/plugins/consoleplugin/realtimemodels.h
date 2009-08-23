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
#ifndef REALTIMEMODELS_H
#define REALTIMEMODELS_H

#include <QSortFilterProxyModel>

class ChannelSortModel : public QSortFilterProxyModel {

    Q_OBJECT

public:
    ChannelSortModel(QObject *parent = 0);
    void setUUIDFilter(QList<QString> uuid);

protected:
     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    QList<QString> _uuid_list;
};

class EventSortModel : public QSortFilterProxyModel {

    Q_OBJECT

public:
    EventSortModel(QObject *parent = 0);
    void setUUIDFilter(QString uuid);

protected:
     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    QString _uuid;
};

#endif // REALTIMEMODELS_H
