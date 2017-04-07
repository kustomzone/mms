#include "MouseAlgosTab.h"

#include <limits>

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QRadioButton>
#include <QSlider>
#include <QSpacerItem>
#include <QSplitter>
#include <QVBoxLayout>

#include "ConfigDialog.h"
#include "Param.h"
#include "ProcessUtilities.h"
#include "SettingsMouseAlgos.h"
#include "SimUtilities.h"
#include "State.h"

namespace mms {

MouseAlgosTab::MouseAlgosTab() :
        m_comboBox(new QComboBox()),
        m_editButton(new QPushButton("Edit")),
        m_seedBox(new QSpinBox()),
        m_seedAutoUpdate(new QCheckBox("Auto-update")),
        m_buildButton(new QPushButton("Build")),
        m_buildOutput(new TextDisplay()),
        m_buildAutoClear(new QCheckBox("Auto-clear")),
        m_runButton(new QPushButton("Run")),
        m_runOutput(new TextDisplay()),
		m_runAutoClear(new QCheckBox("Auto-clear")) {

    // TODO: MACK - clean this up a little bit

	// Initialize the checkstates
	m_buildAutoClear->setCheckState(Qt::Checked);
	m_runAutoClear->setCheckState(Qt::Checked);

    // Set up the layout
    QVBoxLayout* layout = new QVBoxLayout();
    setLayout(layout);
    QHBoxLayout* topLayout = new QHBoxLayout();
    layout->addLayout(topLayout);

    // Add the import button
    QPushButton* importButton = new QPushButton("Import Mouse Algorithm");
    connect(importButton, &QPushButton::clicked, this, &MouseAlgosTab::import);
    topLayout->addWidget(importButton);

    // Add the combobox
    topLayout->addWidget(m_comboBox);

    // Create the algo buttons
    connect(m_editButton, &QPushButton::clicked, this, &MouseAlgosTab::edit);
    connect(m_buildButton, &QPushButton::clicked, this, &MouseAlgosTab::build);
    connect(m_runButton, &QPushButton::clicked, this, &MouseAlgosTab::run);

    // Add the algo buttons to the tab
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(m_runButton);
    buttonsLayout->addWidget(m_buildButton);
    buttonsLayout->addWidget(m_editButton);
    layout->addLayout(buttonsLayout);

    // Add the mouse algo config box
    QGroupBox* optionsBox = new QGroupBox("Mouse Options");
    layout->addWidget(optionsBox);
    QGridLayout* optionsLayout = new QGridLayout();
    optionsBox->setLayout(optionsLayout);

    // Add the random seed config field
    QLabel* seedLabel = new QLabel("Previous Seed");
    seedLabel->setAlignment(Qt::AlignCenter);
    m_seedBox->setRange(0, std::numeric_limits<int>::max());
	m_seedBox->setValue(SimUtilities::randomNonNegativeInt());
	m_seedAutoUpdate->setCheckState(Qt::Checked);
    optionsLayout->addWidget(seedLabel, 0, 0);
    optionsLayout->addWidget(m_seedBox, 1, 0);
    optionsLayout->addWidget(m_seedAutoUpdate, 1, 1);
    connect(m_seedAutoUpdate, &QCheckBox::stateChanged, this, [=](int state){
        QString text = (state == Qt::Checked ? "Previous Seed" : "Seed");
        seedLabel->setText(text);
        m_seedBox->setReadOnly(state == Qt::Checked);
    });

    // Add the speed adjustments
    double maxSpeed = P()->maxSimSpeed();
    QSlider* speedSlider = new QSlider(Qt::Horizontal);
    QLabel* speedLabel = new QLabel("Speed");
    QDoubleSpinBox* speedBox = new QDoubleSpinBox();
    speedBox->setRange(maxSpeed / 100.0, maxSpeed);
    optionsLayout->addWidget(speedLabel, 2, 0);
    optionsLayout->addWidget(speedSlider, 2, 1);
    optionsLayout->addWidget(speedBox, 2, 2);
    connect(
        speedSlider, &QSlider::valueChanged,
        this, [=](int value){
            double fraction = (value + 1.0) / 100.0;
            speedBox->setValue(fraction * maxSpeed);
            S()->setSimSpeed(fraction * maxSpeed);
        }
    );
    connect(
        speedBox,
        static_cast<void(QDoubleSpinBox::*)()>(&QDoubleSpinBox::editingFinished),
        this, [=](){
            double percent = speedBox->value() / speedBox->maximum() * 100.0;
            speedSlider->setValue(static_cast<int>(percent - 1.0));
            S()->setSimSpeed(speedBox->value());
        }
    );
    speedSlider->setValue(100.0 / speedBox->maximum() - 1.0);

    // Add the map layout buttons
    QGroupBox* zoomBox = new QGroupBox("Zoomed Map");
    zoomBox->setCheckable(true);
    optionsLayout->addWidget(zoomBox, 3, 0, 1, 3);
    QHBoxLayout* zoomBoxLayout = new QHBoxLayout();
    zoomBox->setLayout(zoomBoxLayout);
    connect(zoomBox, &QGroupBox::toggled, this, [=](bool on){
        S()->setLayoutType(on ? LayoutType::ZOOMED : LayoutType::FULL);
    });
    zoomBox->setChecked(false);
    
    // Add the zoomed map scale slider
    double maxZoomedMapScale = P()->maxZoomedMapScale();
    QSlider* scaleSlider = new QSlider(Qt::Horizontal);
    QLabel* scaleLabel = new QLabel("Scale");
    QDoubleSpinBox* scaleBox = new QDoubleSpinBox();
    scaleBox->setRange(maxZoomedMapScale / 100.0, maxZoomedMapScale);
    zoomBoxLayout->addWidget(scaleLabel);
    zoomBoxLayout->addWidget(scaleSlider);
    zoomBoxLayout->addWidget(scaleBox);
    connect(
        scaleSlider, &QSlider::valueChanged,
        this, [=](int value){
            double fraction = (value + 1.0) / 100.0;
            scaleBox->setValue(fraction * maxZoomedMapScale);
            S()->setZoomedMapScale(fraction * maxZoomedMapScale);
        }
    );
    connect(
        scaleBox,
        static_cast<void(QDoubleSpinBox::*)()>(&QDoubleSpinBox::editingFinished),
        this, [=](){
            double percent = scaleBox->value() / scaleBox->maximum() * 100.0;
            scaleSlider->setValue(static_cast<int>(percent - 1.0));
            S()->setZoomedMapScale(scaleBox->value());
        }
    );
    scaleSlider->setValue(10.0 / scaleBox->maximum() - 1.0);

    // Add the rotate zoomed map option
    QCheckBox* rotateZoomedMapCheckbox = new QCheckBox("Rotate Zoomed Map");
    zoomBoxLayout->addWidget(rotateZoomedMapCheckbox);
    connect(
        rotateZoomedMapCheckbox, &QCheckBox::stateChanged,
        this, [=](int state){
            S()->setRotateZoomedMap(state == Qt::Checked);
        }
    );

    // Add a spliter for the outputs
    QSplitter* splitter = new QSplitter();
    splitter->setOrientation(Qt::Vertical);
    splitter->setHandleWidth(6);
    layout->addWidget(splitter);

    // Add the build output and run output
	QVector<QPair<QString, TextDisplay*>> displays;
	for (int i = 0; i < 2; i += 1) {
		QString label = (i == 0 ? "Build Output" : "Run Output");
		QCheckBox* autoClear = (i == 0 ? m_buildAutoClear : m_runAutoClear);
		TextDisplay* textDisplay = (i == 0 ? m_buildOutput : m_runOutput);
		QWidget* container = new QWidget();
		QVBoxLayout* layout = new QVBoxLayout(container);
		layout->setContentsMargins(0, 0, 0, 0);
		QHBoxLayout* headerLayout = new QHBoxLayout();
		headerLayout->addWidget(new QLabel(label));
		QPushButton* clearButton = new QPushButton("Clear");
		connect(clearButton, &QPushButton::clicked, this, [=](){
			textDisplay->clear();
		});
		headerLayout->addWidget(clearButton);
		headerLayout->addWidget(autoClear);
		headerLayout->addSpacerItem(
			new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed)
		);
		layout->addLayout(headerLayout);
		layout->addWidget(textDisplay);
		splitter->addWidget(container);
	}

    // Add the mouse algos
    refresh();
}

void MouseAlgosTab::import() {

    QVector<ConfigDialogField> fields = getFields();
    ConfigDialogField nameField = fields.at(0);
    ConfigDialogField dirPathField = fields.at(1);
    ConfigDialogField buildCommandField = fields.at(2);
    ConfigDialogField runCommandField = fields.at(3);
    ConfigDialogField mouseFilePathField = fields.at(4);

    ConfigDialog dialog(
        "Import",
        "Mouse Algorithm",
        {
            nameField,
            dirPathField,
            buildCommandField,
            runCommandField,
            mouseFilePathField,
        },
        false // No "Remove" button
    );

    // Cancel was pressed
    if (dialog.exec() == QDialog::Rejected) {
        return;
    }

    // Ok was pressed
    QString name = dialog.getValue(nameField.key);
    SettingsMouseAlgos::add(
        name,
        dialog.getValue(dirPathField.key),
        dialog.getValue(buildCommandField.key),
        dialog.getValue(runCommandField.key),
        dialog.getValue(mouseFilePathField.key)
    );

    // Update the mouse algos
    refresh(name);
}

void MouseAlgosTab::edit() {

    QString name = m_comboBox->currentText();

    QVector<ConfigDialogField> fields = getFields();
    ConfigDialogField nameField = fields.at(0);
    ConfigDialogField dirPathField = fields.at(1);
    ConfigDialogField buildCommandField = fields.at(2);
    ConfigDialogField runCommandField = fields.at(3);
    ConfigDialogField mouseFilePathField = fields.at(4);

    nameField.initialValue = name;
    dirPathField.initialValue = SettingsMouseAlgos::getDirPath(name);
    buildCommandField.initialValue = SettingsMouseAlgos::getBuildCommand(name);
    runCommandField.initialValue = SettingsMouseAlgos::getRunCommand(name);
    mouseFilePathField.initialValue =
        SettingsMouseAlgos::getMouseFilePath(name);

    ConfigDialog dialog(
        "Edit",
        "Mouse Algorithm",
        {
            nameField,
            dirPathField,
            buildCommandField,
            runCommandField,
            mouseFilePathField,
        },
        true // Include a "Remove" button
    );

    // Cancel was pressed
    if (dialog.exec() == QDialog::Rejected) {
        return;
    }

    // Remove was pressed
    if (dialog.removeButtonPressed()) {
        SettingsMouseAlgos::remove(name);
        refresh();
        return;
    }

    // OK was pressed
    QString newName = dialog.getValue(nameField.key);
    SettingsMouseAlgos::update(
        name,
        newName,
        dialog.getValue(dirPathField.key),
        dialog.getValue(buildCommandField.key),
        dialog.getValue(runCommandField.key),
        dialog.getValue(mouseFilePathField.key)
    );

    // Update the mouse algos
    refresh(newName);
}

void MouseAlgosTab::build() {
    const QString& name = m_comboBox->currentText();
	ProcessUtilities::build(
		name,
    	SettingsMouseAlgos::names(),
		SettingsMouseAlgos::getBuildCommand(name),
		SettingsMouseAlgos::getDirPath(name),
		m_buildAutoClear,
		m_buildOutput,
		m_buildButton,
		this
	);
}

void MouseAlgosTab::run() {

	// Clear the run output
	if (m_runAutoClear->isChecked()) {
		m_runOutput->clear();
	}

	// Perform some validation
    const QString& name = m_comboBox->currentText();
    if (!SettingsMouseAlgos::names().contains(name)) {
		m_runOutput->appendPlainText("[CORRUPT ALGORITHM CONFIG]\n");
        return;
    }
	QString runCommand = SettingsMouseAlgos::getRunCommand(name);
	if (runCommand.isEmpty()) {
		m_runOutput->appendPlainText("[EMPTY RUN COMMAND]\n");
        return;
	}
	QString dirPath = SettingsMouseAlgos::getDirPath(name);
	if (dirPath.isEmpty()) {
		m_runOutput->appendPlainText("[EMPTY DIRECTORY]\n");
        return;
	}
	QString mouseFilePath = SettingsMouseAlgos::getMouseFilePath(name);
	if (mouseFilePath.isEmpty()) {
		m_runOutput->appendPlainText("[EMPTY MOUSE FILE]\n");
        return;
	}

	// Update the random seed
	if (m_seedAutoUpdate->isChecked()) {
		m_seedBox->setValue(SimUtilities::randomNonNegativeInt());
	}

    // Once we're sure the necessary items exist, emit the signal
    emit mouseAlgoSelected(
        name,
        runCommand,
        dirPath,
        mouseFilePath,
        m_seedBox->value(),
        m_runOutput
    );
}

void MouseAlgosTab::refresh(const QString& name) {
    m_comboBox->clear();
    for (const QString& algoName : SettingsMouseAlgos::names()) {
        m_comboBox->addItem(algoName);
    }
    int index = m_comboBox->findText(name);
    if (index != -1) {
        m_comboBox->setCurrentIndex(index);
    }
    bool isEmpty = (m_comboBox->count() == 0);
    m_comboBox->setEnabled(!isEmpty);
    m_editButton->setEnabled(!isEmpty);
    m_buildButton->setEnabled(!isEmpty);
    m_runButton->setEnabled(!isEmpty);
}

QVector<ConfigDialogField> MouseAlgosTab::getFields() {

    ConfigDialogField nameField;
    nameField.key = "NAME";
    nameField.label = "Name";

    ConfigDialogField dirPathField;
    dirPathField.key = "DIR_PATH";
    dirPathField.label = "Directory";
    dirPathField.fileBrowser = true;
    dirPathField.onlyDirectories = true;

    ConfigDialogField buildCommandField;
    buildCommandField.key = "BUILD_COMMAND";
    buildCommandField.label = "Build Command";

    ConfigDialogField runCommandField;
    runCommandField.key = "RUN_COMMAND";
    runCommandField.label = "Run Command";

    ConfigDialogField mouseFilePathField;
    mouseFilePathField.key = "MOUSE_FILE_PATH";
    mouseFilePathField.label = "Mouse File";
    mouseFilePathField.fileBrowser = true;

    return {
        nameField,
        dirPathField,
        buildCommandField,
        runCommandField,
        mouseFilePathField,
    };
}

} // namespace mms