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

#include "p402ppwidget.h"

#include "canopen/datalogger/dataloggerwidget.h"
#include "canopen/indexWidget/indexcheckbox.h"
#include "canopen/indexWidget/indexspinbox.h"

#include "profile/p402/modepp.h"
#include "profile/p402/nodeprofile402.h"
#include "services/rpdo.h"
#include "services/tpdo.h"

#include <QString>
#include <QStringList>
#include <QtMath>

#include <canopen/indexWidget/indexlabel.h>

P402PpWidget::P402PpWidget(QWidget *parent)
    : P402ModeWidget(parent)
{
    _modePp = nullptr;

    createWidgets();
}

void P402PpWidget::setIProfile(NodeProfile402 *nodeProfile402)
{
    if (nodeProfile402 == nullptr)
    {
        _modePp = nullptr;
        return;
    }
    _modePp = dynamic_cast<ModePp *>(_nodeProfile402->mode(NodeProfile402::OperationMode::PP));

    connect(_modePp, &ModePp::changeSetImmediatelyEvent, this, &P402PpWidget::changeSetImmediatelyPointEvent);
    connect(_modePp, &ModePp::absRelEvent, this, &P402PpWidget::absRelEvent);
    connect(_modePp, &ModePp::changeOnSetPointEvent, this, &P402PpWidget::changeOnSetPointEvent);

    changeSetImmediatelyPointEvent(_modePp->isChangeSetImmediately());
    absRelEvent(_modePp->isAbsRel());
    changeOnSetPointEvent(_modePp->isChangeOnSetPoint());

    _positionDemandValueLabel->setObjId(_modePp->positionDemandValueObjectId());
    _positionActualValueLabel->setObjId(_modePp->positionActualValueObjectId());

    _positionRangeLimitMinSpinBox->setObjId(_modePp->positionRangeLimitMinObjectId());
    _positionRangeLimitMaxSpinBox->setObjId(_modePp->positionRangeLimitMaxObjectId());
    registerObjId(_positionRangeLimitMaxSpinBox->objId());
    _softwarePositionLimitMinSpinBox->setObjId(_modePp->softwarePositionLimitMinObjectId());
    _softwarePositionLimitMaxSpinBox->setObjId(_modePp->softwarePositionLimitMaxObjectId());

    _profileVelocitySpinBox->setObjId(_modePp->profileVelocityObjectId());
    _maxProfileVelocitySpinBox->setObjId(_modePp->maxProfileVelocityObjectId());
    _maxMotorSpeedSpinBox->setObjId(_modePp->maxMotorSpeedObjectId());

    _profileAccelerationSpinBox->setObjId(_modePp->profileAccelerationObjectId());
    _maxAccelerationSpinBox->setObjId(_modePp->maxAccelerationObjectId());
    _profileDecelerationSpinBox->setObjId(_modePp->profileDecelerationObjectId());
    _maxDecelerationSpinBox->setObjId(_modePp->maxDecelerationObjectId());
    _quickStopDecelerationSpinBox->setObjId(_modePp->quickStopDecelerationObjectId());

    _homeOffsetSpinBox->setObjId(_modePp->homeOffsetObjectId());

    _polarityCheckBox->setObjId(_nodeProfile402->fgPolaritybjectId());
}

void P402PpWidget::goOneLineEditFinished()
{
    _modePp->newSetPoint(false);
    _nodeProfile402->setTarget(_goOneLineEdit->text().toInt());
    _modePp->newSetPoint(true);
}

void P402PpWidget::twoOneLineEditFinished()
{
    _modePp->newSetPoint(false);
    _nodeProfile402->setTarget(_goTwoLineEdit->text().toInt());
    _modePp->newSetPoint(true);
}

void P402PpWidget::changeSetImmediatelyPointCheckBoxRampClicked(bool ok)
{
    if (_nodeProfile402 != nullptr)
    {
        _modePp->setChangeSetImmediately(ok);
    }
    updateInformationLabel();
}

void P402PpWidget::changeSetImmediatelyPointEvent(bool ok)
{
    _changeSetImmediatelyPointCheckBox->setChecked(ok);
    updateInformationLabel();
}

void P402PpWidget::absRelCheckBoxRampClicked(bool ok)
{
    if (_nodeProfile402 != nullptr)
    {
        _modePp->setAbsRel(ok);
    }
}

void P402PpWidget::absRelEvent(bool ok)
{
    _absRelCheckBox->setChecked(ok);
}

void P402PpWidget::changeOnSetPointCheckBoxRampClicked(bool ok)
{
    if (_nodeProfile402 != nullptr)
    {
        _modePp->setChangeOnSetPoint(ok);
    }
    updateInformationLabel();
}

void P402PpWidget::changeOnSetPointEvent(bool ok)
{
    //_changeOnSetPointCheckBox->setChecked(ok);
}

void P402PpWidget::updateInformationLabel()
{
    //    QString text;
    //    if (!_changeSetImmediatelyPointCheckBox->isChecked())
    //    {
    //        text = "Change set Immediately disabled";
    //    }
    //    _infoLabel->setText(text);
}

void P402PpWidget::createDataLogger()
{
    DataLogger *dataLogger = new DataLogger();
    DataLoggerWidget *dataLoggerWidget = new DataLoggerWidget(dataLogger);
    dataLoggerWidget->setTitle(tr("Node %1 axis %2 PP").arg(_nodeProfile402->nodeId()).arg(_nodeProfile402->axis()));

    dataLogger->addData(_modePp->positionDemandValueObjectId());
    dataLogger->addData(_modePp->targetObjectId());

    dataLoggerWidget->setAttribute(Qt::WA_DeleteOnClose);
    connect(dataLoggerWidget, &QObject::destroyed, dataLogger, &DataLogger::deleteLater);

    dataLoggerWidget->show();
    dataLoggerWidget->raise();
    dataLoggerWidget->activateWindow();
}

void P402PpWidget::mapDefaultObjects()
{
    NodeObjectId controlWordObjectId = _nodeProfile402->controlWordObjectId();
    QList<NodeObjectId> ppRpdoObjectList = {controlWordObjectId, _modePp->targetObjectId()};
    _nodeProfile402->node()->rpdos().at(0)->writeMapping(ppRpdoObjectList);

    NodeObjectId statusWordObjectId = _nodeProfile402->statusWordObjectId();
    QList<NodeObjectId> ppTpdoObjectList = {statusWordObjectId, _modePp->positionDemandValueObjectId()};
    _nodeProfile402->node()->tpdos().at(0)->writeMapping(ppTpdoObjectList);
}

void P402PpWidget::showDiagram()
{
    QPixmap ppModePixmap;
    QLabel *ppModeLabel;
    ppModeLabel = new QLabel();
    ppModeLabel->setAttribute(Qt::WA_DeleteOnClose);
    ppModePixmap.load(":/diagram/img/diagrams/402PPDiagram.png");
    ppModeLabel->setPixmap(ppModePixmap);
    ppModeLabel->setWindowTitle("402 PP Diagram");
    ppModeLabel->show();
}

void P402PpWidget::createWidgets()
{
    // Group Box PP mode
    QGroupBox *modeGroupBox = new QGroupBox(tr("Profile position mode"));
    _modeLayout = new QFormLayout();

    createTargetWidgets();
    createInformationWidgets();
    createLimitWidgets();

    QFrame *frame = new QFrame();
    frame->setFrameStyle(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    _modeLayout->addRow(frame);

    createVelocityWidgets();

    frame = new QFrame();
    frame->setFrameStyle(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    _modeLayout->addRow(frame);

    createAccelDeccelWidgets();

    frame = new QFrame();
    frame->setFrameStyle(QFrame::HLine);
    frame->setFrameShadow(QFrame::Sunken);
    _modeLayout->addRow(frame);

    createHomePolarityWidgets();
    modeGroupBox->setLayout(_modeLayout);

    // Create interface
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(modeGroupBox);
    layout->addWidget(createControlWordWidgets());

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

void P402PpWidget::createTargetWidgets()
{
    _targetPositionLineEdit = new QLineEdit();

    QHBoxLayout *goLayout = new QHBoxLayout();
    _goOneLineEdit = new QLineEdit();
    _goOneLineEdit->setPlaceholderText(tr("Target one"));
    goLayout->addWidget(_goOneLineEdit);
    _goOnePushButton = new QPushButton(tr("Go one"));
    connect(_goOneLineEdit, &QLineEdit::returnPressed, this, &P402PpWidget::goOneLineEditFinished);
    connect(_goOnePushButton, &QPushButton::clicked, this, &P402PpWidget::goOneLineEditFinished);
    goLayout->addWidget(_goOnePushButton);

    _goTwoLineEdit = new QLineEdit();
    _goTwoLineEdit->setPlaceholderText(tr("Target two"));
    goLayout->addWidget(_goTwoLineEdit);
    _goTwoPushButton = new QPushButton(tr("Go two"));
    connect(_goTwoLineEdit, &QLineEdit::returnPressed, this, &P402PpWidget::twoOneLineEditFinished);
    connect(_goTwoPushButton, &QPushButton::clicked, this, &P402PpWidget::twoOneLineEditFinished);
    goLayout->addWidget(_goTwoPushButton);

    QLabel *labelPositionTarget = new QLabel(tr("Position &target:"));
    labelPositionTarget->setBuddy(_goOneLineEdit);
    _modeLayout->addRow(labelPositionTarget, goLayout);
}

void P402PpWidget::createInformationWidgets()
{
    _infoLabel = new QLabel();
    _infoLabel->setStyleSheet("QLabel { color : red; }");
    _modeLayout->addRow(tr("Information:"), _infoLabel);

    _positionDemandValueLabel = new IndexLabel();
    _modeLayout->addRow(tr("Position demand value:"), _positionDemandValueLabel);

    _positionActualValueLabel = new IndexLabel();
    _modeLayout->addRow(tr("Position actual value:"), _positionActualValueLabel);
}

void P402PpWidget::createLimitWidgets()
{
    // POSITION RANGE LIMIT
    QLayout *positionLayout = new QHBoxLayout();
    positionLayout->setSpacing(0);

    _positionRangeLimitMinSpinBox = new IndexSpinBox();
    positionLayout->addWidget(_positionRangeLimitMinSpinBox);
    QLabel *label = new QLabel(tr(":"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    positionLayout->addWidget(label);

    _positionRangeLimitMaxSpinBox = new IndexSpinBox();
    positionLayout->addWidget(_positionRangeLimitMaxSpinBox);
    label = new QLabel(tr("&Position range limit:"));
    label->setToolTip(tr("Min, max"));
    label->setBuddy(_positionRangeLimitMinSpinBox);
    _modeLayout->addRow(label, positionLayout);

    // SOFTWARE RANGE LIMIT
    QLayout *softwareLayout = new QHBoxLayout();
    softwareLayout->setSpacing(0);

    _softwarePositionLimitMinSpinBox = new IndexSpinBox();
    softwareLayout->addWidget(_softwarePositionLimitMinSpinBox);
    label = new QLabel(tr(":"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    softwareLayout->addWidget(label);

    _softwarePositionLimitMaxSpinBox = new IndexSpinBox();
    softwareLayout->addWidget(_softwarePositionLimitMaxSpinBox);
    label = new QLabel(tr("&Software position limit:"));
    label->setToolTip(tr("Min, max"));
    label->setBuddy(_softwarePositionLimitMinSpinBox);

    _modeLayout->addRow(label, softwareLayout);
}

void P402PpWidget::createVelocityWidgets()
{
    // Add Profile velocity (0x6081) and Max motor speed (0x6080)
    _profileVelocitySpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Profile velocity:"), _profileVelocitySpinBox);

    _endVelocitySpinBox = new IndexSpinBox();
    // qfLayout->addRow(tr("End velocity:"), _endVelocitySpinBox);

    // Add Max profile velocity (0x607F) and Max motor speed (0x6080)
    _maxProfileVelocitySpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Max profile velocity:"), _maxProfileVelocitySpinBox);

    _maxMotorSpeedSpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Max motor speed:"), _maxMotorSpeedSpinBox);
}

void P402PpWidget::createAccelDeccelWidgets()
{
    QLabel *label;

    // Add Profile acceleration (0x6083) and Max acceleration (0x60C5)
    QLayout *accelerationlayout = new QHBoxLayout();

    _profileAccelerationSpinBox = new IndexSpinBox();
    accelerationlayout->addWidget(_profileAccelerationSpinBox);

    _maxAccelerationSpinBox = new IndexSpinBox();

    label = new QLabel(tr("Max:"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    accelerationlayout->addWidget(label);
    accelerationlayout->addWidget(_maxAccelerationSpinBox);
    _modeLayout->addRow(tr("Profile acceleration:"), accelerationlayout);

    // Add Profile deceleration (0x6084) and Max deceleration (0x60C6)
    QLayout *decelerationlayout = new QHBoxLayout();

    _profileDecelerationSpinBox = new IndexSpinBox();
    decelerationlayout->addWidget(_profileDecelerationSpinBox);

    _maxDecelerationSpinBox = new IndexSpinBox();

    label = new QLabel(tr("Max:"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    decelerationlayout->addWidget(label);
    decelerationlayout->addWidget(_maxDecelerationSpinBox);
    _modeLayout->addRow(tr("Profile deceleration:"), decelerationlayout);

    // Add Quick stop deceleration (0x6085)
    _quickStopDecelerationSpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Quick stop deceleration:"), _quickStopDecelerationSpinBox);
}

void P402PpWidget::createHomePolarityWidgets()
{
    _homeOffsetSpinBox = new IndexSpinBox();
    _modeLayout->addRow(tr("Home offset:"), _homeOffsetSpinBox);

    _polarityCheckBox = new IndexCheckBox();
    _polarityCheckBox->setBitMask(NodeProfile402::FgPolarity::MASK_POLARITY_POSITION);
    _modeLayout->addRow(tr("Polarity:"), _polarityCheckBox);
}

QGroupBox *P402PpWidget::createControlWordWidgets()
{
    QGroupBox *groupBox = new QGroupBox(tr("Control word"));
    QFormLayout *layout = new QFormLayout();

    _changeSetImmediatelyPointCheckBox = new QCheckBox();
    layout->addRow(tr("Change set immediately (bit 5):"), _changeSetImmediatelyPointCheckBox);
    connect(_changeSetImmediatelyPointCheckBox, &QCheckBox::clicked, this, &P402PpWidget::changeSetImmediatelyPointCheckBoxRampClicked);
    groupBox->setLayout(layout);

    _absRelCheckBox = new QCheckBox();
    layout->addRow(tr("Abs / rel (bit 6):"), _absRelCheckBox);
    connect(_absRelCheckBox, &QCheckBox::clicked, this, &P402PpWidget::absRelCheckBoxRampClicked);

    return groupBox;
}

QHBoxLayout *P402PpWidget::createButtonWidgets() const
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(5);

    QPushButton *dataLoggerPushButton = new QPushButton(tr("Data logger"));
    connect(dataLoggerPushButton, &QPushButton::clicked, this, &P402PpWidget::createDataLogger);
    layout->addWidget(dataLoggerPushButton);

    QPushButton *mappingPdoPushButton = new QPushButton(tr("Map PP to PDOs"));
    connect(mappingPdoPushButton, &QPushButton::clicked, this, &P402PpWidget::mapDefaultObjects);
    layout->addWidget(mappingPdoPushButton);

    QPushButton *imgPushButton = new QPushButton(tr("Diagram PP mode"));
    connect(imgPushButton, &QPushButton::clicked, this, &P402PpWidget::showDiagram);
    layout->addWidget(imgPushButton);

    return layout;
}

void P402PpWidget::odNotify(const NodeObjectId &objId, NodeOd::FlagsRequest flags)
{
    Q_UNUSED(objId)
    Q_UNUSED(flags)
}
