/**
 ** This file is part of the UDTStudio project.
 ** Copyright 2019-2021 UniSwarm
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "emergency.h"
#include "canopenbus.h"

#include <QDataStream>
#include <QDebug>
#include <QIODevice>

Emergency::Emergency(Node *node)
    : Service(node)
{
    _cobId = 0x80;
    _cobIds.append(_cobId + node->nodeId());
}

uint32_t Emergency::cobId() const
{
    return _cobId;
}

QString Emergency::type() const
{
    return QLatin1String("Emergency");
}

void Emergency::parseFrame(const QCanBusFrame &frame)
{
    uint16_t errorCode;
    u_int8_t errorClass;
    QByteArray errorDesc;

    QByteArray payload = frame.payload();
    QDataStream request(&payload, QIODevice::ReadOnly);
    request.setByteOrder(QDataStream::LittleEndian);

    request >> errorCode;
    request >> errorClass;
    errorDesc = payload.mid(3);
    qDebug() << "Emergency" << errorCode << errorClass << errorDesc;
}
