/**
 ** This file is part of the UDTStudio project.
 ** Copyright 2019-2020 UniSwarm
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

#include "nodemanagerwidget.h"

#include <QFormLayout>

NodeManagerWidget::NodeManagerWidget(QWidget *parent)
    : NodeManagerWidget(nullptr, parent)
{
}

NodeManagerWidget::NodeManagerWidget(Node *node, QWidget *parent)
    : QWidget(parent)
{
    _node = nullptr;
    createWidgets();
    setNode(node);
}

Node *NodeManagerWidget::node() const
{
    return _node;
}

void NodeManagerWidget::snotify(void *object, quint16 index, quint8 subindexDevice, const QByteArray &data)
{
    static_cast<NodeManagerWidget *>(object)->notify(index, subindexDevice, data);
}

void NodeManagerWidget::notify(quint16 index, quint8 subindexDevice, const QByteArray &data)
{
    if (index == 0x1000 && subindexDevice == 0x00)
    {
        _index1000Label->setText(data.toHex(' ').toUpper());
    }
}

void NodeManagerWidget::setNode(Node *node)
{
    if (node != _node)
    {
        if (_node)
        {
            disconnect(_node, &Node::statusChanged, this, &NodeManagerWidget::updateData);
        }
    }
    _node = node;
    if (_node)
    {
        connect(_node, &Node::statusChanged, this, &NodeManagerWidget::updateData);
    }
    _groupBox->setEnabled(_node);
    updateData();
}

void NodeManagerWidget::updateData()
{
    if (_node)
    {
        _nodeNameEdit->setText(_node->name());
        _nodeStatusLabel->setText(_node->statusStr());
    }
}

void NodeManagerWidget::start()
{
    if (_node)
    {
        _node->sendStart();
    }
}

void NodeManagerWidget::stop()
{
    if (_node)
    {
        _node->sendStop();
    }
}

void NodeManagerWidget::resetCom()
{
    if (_node)
    {
        _node->sendResetComm();
    }
}

void NodeManagerWidget::resetNode()
{
    if (_node)
    {
        _node->sendResetNode();
    }
}

void NodeManagerWidget::test()
{
    if (_node)
    {
        // TODO move to register fonction
        _node->nodeOd()->_object = this;
        _node->nodeOd()->_notifyIndex = 0x1000;
        _node->nodeOd()->_notifySubIndex = 0x00;
        _node->nodeOd()->_notify = NodeManagerWidget::snotify;

        _node->readObject(0x1000, 0x00);
    }
}

void NodeManagerWidget::createWidgets()
{
    QAction *action;
    QLayout *layout = new QVBoxLayout();
    layout->setMargin(0);

    _groupBox = new QGroupBox(tr("Node"));
    QFormLayout *layoutGroupBox = new QFormLayout();

    _toolBar = new QToolBar(tr("Node commands"));
    action = _toolBar->addAction(tr("start"));
    connect(action, &QAction::triggered, this, &NodeManagerWidget::start);
    action = _toolBar->addAction(tr("stop"));
    connect(action, &QAction::triggered, this, &NodeManagerWidget::stop);
    action = _toolBar->addAction(tr("resetCom"));
    connect(action, &QAction::triggered, this, &NodeManagerWidget::resetCom);
    action = _toolBar->addAction(tr("resetNode"));
    connect(action, &QAction::triggered, this, &NodeManagerWidget::resetNode);
    action = _toolBar->addAction(tr("test"));
    connect(action, &QAction::triggered, this, &NodeManagerWidget::test);
    layoutGroupBox->addRow(_toolBar);

    _nodeNameEdit = new QLineEdit();
    layoutGroupBox->addRow(tr("Name :"), _nodeNameEdit);

    _nodeStatusLabel = new QLabel();
    layoutGroupBox->addRow(tr("Status :"), _nodeStatusLabel);

    _index1000Label = new QLabel();
    layoutGroupBox->addRow(tr("0x1000 :"), _index1000Label);

    _groupBox->setLayout(layoutGroupBox);
    layout->addWidget(_groupBox);

    setLayout(layout);
}
