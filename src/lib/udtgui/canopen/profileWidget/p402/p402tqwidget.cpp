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

#include "p402tqwidget.h"

#include "canopen/datalogger/dataloggerwidget.h"
#include "canopen/indexWidget/indexlabel.h"
#include "canopen/indexWidget/indexspinbox.h"

#include "profile/p402/modetq.h"
#include "profile/p402/nodeprofile402.h"
#include "services/rpdo.h"
#include "services/tpdo.h"

#include <QPushButton>

P402TqWidget::P402TqWidget(QWidget *parent)
    : P402ModeWidget(parent)
{
    createWidgets();
    _nodeProfile402 = nullptr;
}

void P402TqWidget::readRealTimeObjects()
{
    if (_nodeProfile402 != nullptr)
    {
        _nodeProfile402->readRealTimeObjects();
    }
}

void P402TqWidget::readAllObjects()
{
    if (_nodeProfile402 != nullptr)
    {
        _nodeProfile402->readAllObjects();
    }
}

void P402TqWidget::reset()
{
    _modeTq->reset();
}

void P402TqWidget::setNode(Node *node, uint8_t axis)
{
    if (node == nullptr)
    {
        return;
    }

    if (axis > 8)
    {
        return;
    }

    if (node != nullptr)
    {
        setNodeInterrest(node);

        if (!node->profiles().isEmpty())
        {
            _nodeProfile402 = dynamic_cast<NodeProfile402 *>(node->profiles()[axis]);
            _modeTq = dynamic_cast<ModeTq *>(_nodeProfile402->mode(NodeProfile402::OperationMode::TQ));

            _torqueTargetObjectId = _modeTq->targetObjectId();
            registerObjId(_torqueTargetObjectId);

            _torqueDemandObjectId = _modeTq->torqueDemandObjectId();
            _torqueDemandLabel->setObjId(_torqueDemandObjectId);

            _torqueActualValueObjectId = _modeTq->torqueActualValueObjectId();
            _torqueActualValueLabel->setObjId(_torqueActualValueObjectId);

            _targetSlopeSpinBox->setObjId(_modeTq->targetSlopeObjectId());
            _maxTorqueSpinBox->setObjId(_modeTq->maxTorqueObjectId());

            int max = _nodeProfile402->node()->nodeOd()->value(_maxTorqueSpinBox->objId()).toInt();
            _targetTorqueSlider->setRange(-max, max);
            _targetTorqueSlider->setTickInterval(max / 10);
            _sliderMinLabel->setNum(-max);
            _sliderMaxLabel->setNum(max);
        }

        connect(_maxTorqueSpinBox, &QSpinBox::editingFinished, this, &P402TqWidget::maxTorqueSpinboxFinished);
    }
}

void P402TqWidget::targetTorqueSpinboxFinished()
{
    qint16 value = static_cast<qint16>(_targetTorqueSpinBox->value());
    _nodeProfile402->setTarget(value);
}

void P402TqWidget::targetTorqueSliderChanged()
{
    qint16 value = static_cast<qint16>(_targetTorqueSlider->value());
    _nodeProfile402->setTarget(value);
}

void P402TqWidget::maxTorqueSpinboxFinished()
{
    int max = _nodeProfile402->node()->nodeOd()->value(_maxTorqueSpinBox->objId()).toInt();
    _targetTorqueSlider->setRange(-max, max);
    _sliderMinLabel->setNum(-max);
    _sliderMaxLabel->setNum(max);
}

void P402TqWidget::setZeroButton()
{
    _nodeProfile402->setTarget(0);
}

void P402TqWidget::createDataLogger()
{
    DataLogger *dataLogger = new DataLogger();
    DataLoggerWidget *dataLoggerWidget = new DataLoggerWidget(dataLogger);
    dataLoggerWidget->setTitle(tr("Node %1 axis %2 TQ").arg(_nodeProfile402->nodeId()).arg(_nodeProfile402->axisId()));

    dataLogger->addData(_torqueActualValueObjectId);
    dataLogger->addData(_torqueTargetObjectId);
    dataLogger->addData(_torqueDemandObjectId);

    dataLoggerWidget->setAttribute(Qt::WA_DeleteOnClose);
    connect(dataLoggerWidget, &QObject::destroyed, dataLogger, &DataLogger::deleteLater);

    dataLoggerWidget->show();
    dataLoggerWidget->raise();
    dataLoggerWidget->activateWindow();
}

void P402TqWidget::mapDefaultObjects()
{
    NodeObjectId controlWordObjectId = _nodeProfile402->controlWordObjectId();
    QList<NodeObjectId> tqRpdoObjectList = {controlWordObjectId, _torqueTargetObjectId};
    _nodeProfile402->node()->rpdos().at(0)->writeMapping(tqRpdoObjectList);

    NodeObjectId statusWordObjectId = _nodeProfile402->statusWordObjectId();
    QList<NodeObjectId> tqTpdoObjectList = {statusWordObjectId, _torqueDemandObjectId};
    _nodeProfile402->node()->tpdos().at(0)->writeMapping(tqTpdoObjectList);
}

void P402TqWidget::showDiagram()
{
    QPixmap tqModePixmap;
    QLabel *tqModeLabel;
    tqModeLabel = new QLabel();
    tqModeLabel->setAttribute(Qt::WA_DeleteOnClose);
    tqModePixmap.load(":/diagram/img/diagrams/402TQDiagram.png");
    tqModeLabel->setPixmap(tqModePixmap);
    tqModeLabel->setWindowTitle("402 TQ Diagram");
    tqModeLabel->show();
}

void P402TqWidget::createWidgets()
{
    // Group Box TQ mode
    QGroupBox *modeGroupBox = new QGroupBox(tr("Torque mode"));
    _modeLayout = new QFormLayout();

    createTargetWidgets();
    createInformationWidgets();
    createLimitWidgets();

    QFrame *frame = new QFrame();
    frame->setFrameStyle(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    _modeLayout->addRow(frame);

    createSlopeWidgets();

    modeGroupBox->setLayout(_modeLayout);

    // Create interface
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(modeGroupBox);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(widget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *vBoxLayout = new QVBoxLayout();
    vBoxLayout->addWidget(scrollArea);
    vBoxLayout->addLayout(createButtonWidgets());
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(vBoxLayout);
}

void P402TqWidget::createTargetWidgets()
{
    _targetTorqueSpinBox = new QSpinBox();
    _targetTorqueSpinBox->setRange(std::numeric_limits<qint16>::min(), std::numeric_limits<qint16>::max());
    _modeLayout->addRow(tr("&Target torque"), _targetTorqueSpinBox);

    QLayout *labelSliderLayout = new QHBoxLayout();

    _sliderMinLabel = new QLabel("min");
    labelSliderLayout->addWidget(_sliderMinLabel);
    labelSliderLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    labelSliderLayout->addWidget(new QLabel("0"));
    labelSliderLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));

    _sliderMaxLabel = new QLabel("max");
    labelSliderLayout->addWidget(_sliderMaxLabel);
    _modeLayout->addRow(labelSliderLayout);

    _targetTorqueSlider = new QSlider(Qt::Horizontal);
    _targetTorqueSlider->setTickPosition(QSlider::TicksBelow);
    _modeLayout->addRow(_targetTorqueSlider);

    connect(_targetTorqueSlider, &QSlider::valueChanged, this, &P402TqWidget::targetTorqueSliderChanged);
    connect(_targetTorqueSpinBox, &QSpinBox::editingFinished, this, &P402TqWidget::targetTorqueSpinboxFinished);

    QPushButton *setZeroButton = new QPushButton();
    setZeroButton->setText("Set to 0");
    connect(setZeroButton, &QPushButton::clicked, this, &P402TqWidget::setZeroButton);

    QLayout *setZeroLayout = new QHBoxLayout();
    setZeroLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    setZeroLayout->addWidget(setZeroButton);
    setZeroLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    _modeLayout->addRow(setZeroLayout);
}

void P402TqWidget::createInformationWidgets()
{
    _torqueDemandLabel = new IndexLabel();
    _modeLayout->addRow(tr("Torque demand:"), _torqueDemandLabel);

    _torqueActualValueLabel = new IndexLabel();
    _modeLayout->addRow(tr("Torque actual value:"), _torqueActualValueLabel);
}

void P402TqWidget::createLimitWidgets()
{
    _maxTorqueSpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Ma&x torque "), _maxTorqueSpinBox);
}

void P402TqWidget::createSlopeWidgets()
{
    _targetSlopeSpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Target &slope "), _targetSlopeSpinBox);
}

QHBoxLayout *P402TqWidget::createButtonWidgets() const
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(5);

    QPushButton *dataLoggerPushButton = new QPushButton(tr("Data logger"));
    connect(dataLoggerPushButton, &QPushButton::clicked, this, &P402TqWidget::createDataLogger);
    layout->addWidget(dataLoggerPushButton);

    QPushButton *mappingPdoPushButton = new QPushButton(tr("Map TQ to PDOs"));
    connect(mappingPdoPushButton, &QPushButton::clicked, this, &P402TqWidget::mapDefaultObjects);
    layout->addWidget(mappingPdoPushButton);

    QPushButton *imgPushButton = new QPushButton(tr("Diagram TQ mode"));
    connect(imgPushButton, &QPushButton::clicked, this, &P402TqWidget::showDiagram);
    layout->addWidget(imgPushButton);

    return layout;
}

void P402TqWidget::odNotify(const NodeObjectId &objId, NodeOd::FlagsRequest flags)
{
    if ((flags & NodeOd::FlagsRequest::Error) != 0)
    {
        return;
    }

    if ((_nodeProfile402->node() == nullptr) || (_nodeProfile402->node()->status() != Node::STARTED))
    {
        return;
    }

    if (objId == _torqueTargetObjectId)
    {
        int value = _nodeProfile402->node()->nodeOd()->value(objId).toInt();
        if (!_targetTorqueSpinBox->hasFocus())
        {
            _targetTorqueSpinBox->blockSignals(true);
            _targetTorqueSpinBox->setValue(value);
            _targetTorqueSpinBox->blockSignals(false);
        }
        if (!_targetTorqueSlider->isSliderDown())
        {
            _targetTorqueSlider->blockSignals(true);
            _targetTorqueSlider->setValue(value);
            _targetTorqueSlider->blockSignals(false);
        }
    }
}
