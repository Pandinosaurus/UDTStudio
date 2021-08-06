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

#include "updateprocess.h"

#include "busdriver/canbussocketcan.h"
#include "canopen.h"
#include "canopenbus.h"
#include "utility/phantomremove.h"

#include <QCanBus>
#include <QFile>

UpdateProcess::UpdateProcess(quint8 busId, quint8 speed, quint8 nodeid, QString binary)
    : _busId(busId)
    , _speed(speed)
    , _nodeId(nodeid)
    , _binary(binary)
{
    _bus = nullptr;
    _node = nullptr;
}

int UpdateProcess::connectDevice()
{
#ifdef Q_OS_UNIX
    _bus = new CanOpenBus(new CanBusSocketCAN("can0"));
#endif

    if (_bus)
    {
        _bus->setBusName("Bus 1");
        CanOpen::addBus(_bus);
        connect(_bus, &CanOpenBus::nodeAdded, this, &UpdateProcess::nodeDetected);
        _bus->exploreBus();
    }
    else
    {
        return -1; // err << QCoreApplication::translate("main", "error (2): bus not connected") << Qt::endl;
    }
    return 0;
}

bool UpdateProcess::nodeDetected()
{
    if (!_bus->existNode(_nodeId))
    {
        emit connected(false);
        return false;
    }

    _node = _bus->node(_nodeId);

    _objectIdentityList = {{_node->busId(), _node->nodeId(), 0x1000, 0},
                           {_node->busId(), _node->nodeId(), 0x1018, 1},
                           {_node->busId(), _node->nodeId(), 0x1018, 2},
                           {_node->busId(), _node->nodeId(), 0x1018, 3},
                           {_node->busId(), _node->nodeId(), 0x1018, 4}};

    _bootloaderObjectId = {_node->busId(), _node->nodeId(), 0x2050, 0, QMetaType::Type::UShort};
    _programObjectId = {_node->busId(), _node->nodeId(), 0x1F50, 1, QMetaType::Type::QByteArray};
    _programControlObjectId = {_node->busId(), _node->nodeId(), 0x1F51, 1, QMetaType::Type::UChar};
    _manufacturerSoftwareVersionObjectId = {_node->busId(), _node->nodeId(), 0x100A, 0, QMetaType::Type::QByteArray};
    _firmwareBuildDateObjectId = {_node->busId(), _node->nodeId(), 0x2003, 0, QMetaType::Type::QByteArray};

    _timer = new QTimer();

    setNodeInterrest(_node);
    registerObjId({0x1F56, 1});
    registerObjId({0x1F50, 1});
    registerObjId({0x2050, 0});
    registerObjId({0x100A, 0});
    registerObjId({0x2003, 0});
    registerObjId({0x1F51, 1});
    registerObjId({0x1018, 255});

    emit connected(true);
    return true;
}

void UpdateProcess::sendSyncOne()
{
    _node->readObject(_programControlObjectId);
}

UpdateProcess::Status UpdateProcess::status() const
{
    return _status;
}

Node *UpdateProcess::node() const
{
    return _node;
}

int UpdateProcess::openUni(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }
    else
    {
        _uniBinary = new UniParser(fileName);
        _uniBinary->read();
        file.close();
    }

    return true;
}

bool UpdateProcess::update()
{
    _status = STATE_IN_PROGRESS;
    switch (_state)
    {
    case STATE_FREE:
        break;

    case STATE_CHECK_MODE:
        _node->readObject(_programControlObjectId);
        break;

    case STATE_STOP_PROGRAM:
    {
        emit downloadState("Status : stop program");
        emit downloadState("Status : check before flashing");
        QVariant mdata = 0;
        mdata.convert(_programControlObjectId.dataType());
        _node->writeObject(0x1F51, 1, mdata);
        break;
    }
    case STATE_CLEAR_PROGRAM:
    {
        QVariant mdata = 3;
        mdata.convert(_programControlObjectId.dataType());
        _node->writeObject(_programControlObjectId, mdata);

        _timer->singleShot(200, this, SLOT(sendSyncOne()));
        break;
    }
    case STATE_DOWNLOAD_PROGRAM:
    {
        emit downloadState("Status : download programm is starting");

        PhantomRemove prog;
        //        QByteArray block1 = prog.remove(_uniBinary->prog().mid(0, (int)_uniBinary->head().memoryBlockEnd1 - (int)_uniBinary->head().memoryBlockStart1));
        //        uint32_t start = _uniBinary->head().memoryBlockStart1;
        //        block1.prepend(reinterpret_cast<char*>(&start), sizeof(start));
        //        _node->updateFirmware(block1);

        QByteArray block2 = prog.remove(_uniBinary->prog().mid((int)_uniBinary->head().memoryBlockStart2 - (int)_uniBinary->head().memoryBlockStart1,
                                                               (int)_uniBinary->head().memoryBlockEnd2 - (int)_uniBinary->head().memoryBlockStart1));

        uint32_t start2 = _uniBinary->head().memoryBlockStart2;
        block2.prepend(reinterpret_cast<char *>(&start2), sizeof(start2));
        _node->updateFirmware(block2);

        break;
    }

    case STATE_FLASH_FINISHED:
    {
        // CHECK CS SEND
        emit downloadState("Status : download programm finished");
        _state = STATE_CHECK;
        break;
    }

    case STATE_CHECK:
    {
        _state = STATE_OK;
        update();
        break;
    }

    case STATE_OK:
        _state = STATE_FREE;
        emit downloadState("Status : Flashing successful");
        emit downloadFinished();
        _status = STATE_SUCCESSFUL;
        break;

    case STATE_NOT_OK:
        // Error status C, D, M, N, O
        _state = STATE_FREE;
        emit downloadState("Status : Error, check identity, not corresponding");
        emit downloadFinished();
        _status = STATE_NOT_SUCCESSFUL;
        break;
    }

    return true;
}

void UpdateProcess::odNotify(const NodeObjectId &objId, SDO::FlagsRequest flags)
{
    if ((objId.index() == _programControlObjectId.index()) && objId.subIndex() == 1)
    {

        if (flags == SDO::FlagsRequest::Error)
        {
            _state = STATE_NOT_OK;
            update();
        }
        else if (flags == SDO::FlagsRequest::Write)
        {
            //_node->readObject(_programControlObjectId);
        }
        else if (flags == SDO::FlagsRequest::Read)
        {
            uint8_t program = static_cast<uint8_t>(_node->nodeOd()->value(_programControlObjectId).toUInt());

            if (program == 0 || program == 2)
            {
                // BOOTLOADER MODE
                _state = STATE_CLEAR_PROGRAM;
                update();
            }
            else if (program == 3)
            {
                // BOOTLOADER MODE
                _state = STATE_DOWNLOAD_PROGRAM;
                update();
            }
            else
            {
                // APP MODE
                emit downloadState("Status : check before flashing");
                QVariant mdata = _uniBinary->head().deviceModel;
                mdata.convert(_bootloaderObjectId.dataType());
                _node->writeObject(_bootloaderObjectId, mdata);
            }
        }
    }

    if ((objId.index() == _bootloaderObjectId.index()) && objId.subIndex() == 0)
    {
        if (flags == SDO::FlagsRequest::Error)
        {
            _state = STATE_NOT_OK;
            update();
        }
        else
        {
            _state = STATE_CLEAR_PROGRAM;
            update();
        }
    }
}
