#include "ExampleExperiment.h"

#include <ExternalStructures.h>
#include <ExperimentUIHelper.h>


#define TOP_WIDGET_NAME			"example-experiment"

#define START_VOLTAGE_OBJ_NAME	"start-voltage"
#define END_VOLTAGE_OBJ_NAME	"end-voltage"
#define VOLTAGE_STEP_OBJ_NAME	"voltage-step"
#define REPEATS_OBJ_NAME		"repeats"

#define START_VOLTAGE_DEFAULT	0
#define END_VOLTAGE_DEFAULT		1024
#define VOLTAGE_STEP_DEFAULT	1
#define REPEATS_DEFAULT			3

#define PLOT_VAR_TIMESTAMP				"Timestamp"
#define PLOT_VAR_TIMESTAMP_NORMALIZED	"Timestamp (normalized)"
#define PLOT_VAR_EWE					"Ewe"
#define PLOT_VAR_CURRENT				"Current"
#define PLOT_VAR_ECE					"Ece"
#define PLOT_VAR_CURRENT_INTEGRAL		"Integral d(Current)/d(time)"

QString ExampleExperiment::GetShortName() const {
	return "Example Experiment";
}
QString ExampleExperiment::GetFullName() const {
	return "Linear Sweep Voltammetry";
}
QString ExampleExperiment::GetDescription() const {
	return "This experiment sweeps the <b>potential</b> of the working electrode from E1 to E2 at constant scan rate dE/dT"
		"<br>This experiment sweeps the <b>potential</b> of the working electrode from E1 to E2 at constant scan rate dE/dT"
		"<br>This experiment sweeps the <b>potential</b> of the working electrode from E1 to E2 at constant scan rate dE/dT"
		"<br>This experiment sweeps the <b>potential</b> of the working electrode from E1 to E2 at constant scan rate dE/dT"
		"<br>This experiment sweeps the <b>potential</b> of the working electrode from E1 to E2 at constant scan rate dE/dT";
}
QStringList ExampleExperiment::GetCategory() const {
	return QStringList() <<
		"Example Category" <<
		"Example Category 2";
}
ExperimentTypeList ExampleExperiment::GetTypes() const {
	return ExperimentTypeList() << ET_DC;
}
QPixmap ExampleExperiment::GetImage() const {
	return QPixmap(":/Experiments/ExampleExperiment");
}
/*
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegExpValidator>
//*/
QWidget* ExampleExperiment::CreateUserInput() const {
	USER_INPUT_START(TOP_WIDGET_NAME);

	int row = 0;
	/*
	_INSERT_RIGHT_ALIGN_COMMENT("Input mask ", row, 0);
	auto test = new QLineEdit();
	lay->addWidget(test, row, 1);
	test->setInputMask("00:00:00");

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Input mask + blank char ", row, 0);
	test = new QLineEdit();
	lay->addWidget(test, row, 1);
	test->setInputMask("00 \\d\\ays 00:00:00;_");

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("RegExp validator + placeholder ", row, 0);
	test = new QLineEdit();
	lay->addWidget(test, row, 1);
	test->setPlaceholderText("hh:mm:ss");
	test->setValidator(new QRegExpValidator(QRegExp("[0-9]{0,2}:[0-9]{0,2}:[0-9]{1,2}")));

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Int validator [0; 1024] ", row, 0);
	test = new QLineEdit();
	lay->addWidget(test, row, 1);
	test->setValidator(new QIntValidator(0, 1024));

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Double validator [-1.000; 1.000] ", row, 0);
	test = new QLineEdit();
	lay->addWidget(test, row, 1);
	auto validator = new QDoubleValidator(-1.0, 1.0, 3, test);
	validator->setNotation(QDoubleValidator::StandardNotation);
	test->setValidator(validator);

	++row;
	//*/
	_INSERT_RIGHT_ALIGN_COMMENT("Start Voltage = ", row, 0);
	_INSERT_TEXT_INPUT(START_VOLTAGE_DEFAULT, START_VOLTAGE_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("End Voltage = ", row, 0);
	_INSERT_TEXT_INPUT(END_VOLTAGE_DEFAULT, END_VOLTAGE_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("dV/dt = ", row, 0);
	_INSERT_TEXT_INPUT(VOLTAGE_STEP_DEFAULT, VOLTAGE_STEP_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Repeats = ", row, 0);
	_INSERT_TEXT_INPUT(REPEATS_DEFAULT, REPEATS_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("", row, 2);

	++row;
	_INSERT_VERTICAL_SPACING(row);
	//*
	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Test radio 1", row, 0);
	_START_RADIO_BUTTON_GROUP("Test radio 1 id");
		_INSERT_RADIO_BUTTON("Ref", row, 1);
		_INSERT_RADIO_BUTTON("Open circuit", row, 2);
	_END_RADIO_BUTTON_GROUP();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Test radio 2", row, 0);
	_START_RADIO_BUTTON_GROUP("Test radio 2 id");
		_INSERT_RADIO_BUTTON("Ref", row, 1);
		_INSERT_RADIO_BUTTON("Open circuit", row, 2);
	_END_RADIO_BUTTON_GROUP();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Test drop down", row, 0);
	_START_DROP_DOWN("Test drop down id", row, 1);
		_ADD_DROP_DOWN_ITEM("Item 1");
		_ADD_DROP_DOWN_ITEM("Item 2");
		_ADD_DROP_DOWN_ITEM("Item 3");
	_END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Test radio layout", row, 0);
	_START_RADIO_BUTTON_GROUP_HORIZONTAL_LAYOUT("Test radio layout id", row, 1);
		_INSERT_RADIO_BUTTON_LAYOUT("Ref");
		_INSERT_RADIO_BUTTON_LAYOUT("Open circuit");
	_END_RADIO_BUTTON_GROUP_LAYOUT();
	//*/
	_SET_ROW_STRETCH(++row, 1);
	_SET_COL_STRETCH(3, 1);

	USER_INPUT_END();
}
NodesData ExampleExperiment::GetNodesData(QWidget *wdg, const CalibrationData &calData, const HardwareVersion &hwVersion) const {
	NODES_DATA_START(wdg, TOP_WIDGET_NAME);
	/*
	QString selectedRadio1;
	QString selectedRadio2;
	GET_SELECTED_RADIO(selectedRadio1, "Test radio 1 id");
	GET_SELECTED_RADIO(selectedRadio2, "Test radio 2 id");


	QString selectedDropDown;
	GET_SELECTED_DROP_DOWN(selectedDropDown, "Test drop down id");
	//*/

	/*
	qint32 startVoltage;
	qint32 endVoltage;
	qint32 voltageStep;
	qint32 repeats;
	GET_TEXT_INPUT_VALUE(startVoltage, START_VOLTAGE_OBJ_NAME);
	GET_TEXT_INPUT_VALUE(endVoltage, END_VOLTAGE_OBJ_NAME);
	GET_TEXT_INPUT_VALUE(voltageStep, VOLTAGE_STEP_OBJ_NAME);
	GET_TEXT_INPUT_VALUE(repeats, REPEATS_OBJ_NAME);
	//*/

	/*
	exp.isHead = true;
	exp.isTail = false;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 100000;
	exp.tMax = 10000000000;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 15625;
	exp.samplingParams.ADCBufferSizeEven = 20;
	exp.samplingParams.ADCBufferSizeOdd = 20;
	exp.samplingParams.PointsIgnored = 0;
	exp.samplingParams.DACMultEven = 20;
	exp.samplingParams.DACMultOdd = 20;
	exp.DCSweep_pot.VStartUserInput = startVoltage;
	exp.DCSweep_pot.VStartVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VEndUserInput = endVoltage;
	exp.DCSweep_pot.VEndVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VStep = voltageStep;
	exp.DCSweep_pot.Imax = 32767;
	exp.DCSweep_pot.IRangeMax = RANGE0;
	exp.DCSweep_pot.Imin = 0;
	exp.DCSweep_pot.IRangeMin = RANGE7;
	exp.MaxPlays = repeats;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = true;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 100000;
	exp.tMax = 100000000;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 15625;
	exp.samplingParams.ADCBufferSizeEven = 20;
	exp.samplingParams.ADCBufferSizeOdd = 20;
	exp.samplingParams.PointsIgnored = 0;
	exp.samplingParams.DACMultEven = 20;
	exp.samplingParams.DACMultOdd = 20;
	exp.DCSweep_pot.VStartUserInput = endVoltage;
	exp.DCSweep_pot.VStartVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VEndUserInput = startVoltage;
	exp.DCSweep_pot.VEndVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VStep = voltageStep;
	exp.DCSweep_pot.Imax = 32767;
	exp.DCSweep_pot.IRangeMax = RANGE0;
	exp.DCSweep_pot.Imin = 0;
	exp.DCSweep_pot.IRangeMin = RANGE7;
	exp.MaxPlays = repeats;
	exp.branchHeadIndex = 0;
	PUSH_NEW_NODE_DATA();
	//*/

	/*
	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 1e7;
	exp.tMax = 2e8;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 400000;
	exp.samplingParams.ADCBufferSizeEven = 20;
	exp.samplingParams.ADCBufferSizeOdd = 20;
	exp.samplingParams.DACMultEven = 20;
	exp.samplingParams.DACMultOdd = 20;
	exp.DCPoint_pot.VPointUserInput = 200;
	exp.DCPoint_pot.VPointVsOCP = false;
	exp.DCPoint_pot.Imax = 32767;
	exp.DCPoint_pot.IrangeMax = RANGE0;
	exp.DCPoint_pot.Imin = 0;
	exp.DCPoint_pot.IrangeMin = RANGE7;
	//exp.samplingParams.isDACStatic = true;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 1e7;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 100000;
	exp.samplingParams.ADCBufferSizeEven = 10;
	exp.samplingParams.ADCBufferSizeOdd = 10;
	exp.samplingParams.DACMultEven = 10;
	exp.samplingParams.DACMultOdd = 10;
	exp.DCSweep_pot.VStartUserInput = 200;
	exp.DCSweep_pot.VStartVsOCP = false;
	exp.DCSweep_pot.VEndUserInput = 600;
	exp.DCSweep_pot.VEndVsOCP = false;
	exp.DCSweep_pot.VStep = 1;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = true;
	exp.isTail = false;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 1e7;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 100000;
	exp.samplingParams.ADCBufferSizeEven = 10;
	exp.samplingParams.ADCBufferSizeOdd = 10;
	exp.samplingParams.DACMultEven = 10;
	exp.samplingParams.DACMultOdd = 10;
	exp.DCSweep_pot.VStartUserInput = 600;
	exp.DCSweep_pot.VStartVsOCP = false;
	exp.DCSweep_pot.VEndUserInput = 100;
	exp.DCSweep_pot.VEndVsOCP = false;
	exp.DCSweep_pot.VStep = 1;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = true;
	exp.branchHeadIndex = 2;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 1e7;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 100000;
	exp.samplingParams.ADCBufferSizeEven = 10;
	exp.samplingParams.ADCBufferSizeOdd = 10;
	exp.samplingParams.DACMultEven = 10;
	exp.samplingParams.DACMultOdd = 10;
	exp.DCSweep_pot.VStartUserInput = 100;
	exp.DCSweep_pot.VStartVsOCP = false;
	exp.DCSweep_pot.VEndUserInput = 600;
	exp.DCSweep_pot.VEndVsOCP = false;
	exp.DCSweep_pot.VStep = 1;
	exp.MaxPlays = 3;
	PUSH_NEW_NODE_DATA();
	//*/

	exp.isHead = true;
	exp.isTail = false;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 100000;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	exp.samplingParams.ADCTimerDiv = 3;
	exp.samplingParams.ADCTimerPeriod = 15625;
	exp.samplingParams.ADCBufferSizeEven = 20;
	exp.samplingParams.ADCBufferSizeOdd = 20;
	exp.samplingParams.PointsIgnored = 0;
	exp.samplingParams.DACMultEven = 5;
	exp.samplingParams.DACMultOdd = 5;
	exp.DCSweep_pot.VStartUserInput = 0;
	exp.DCSweep_pot.VStartVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VEndUserInput = 1024;
	exp.DCSweep_pot.VEndVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VStep = 1;
  exp.DCSweep_pot.Imax = 1e10;
	exp.MaxPlays = 3;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = true;
	exp.nodeType = DCNODE_SWEEP_POT;
	exp.tMin = 100000;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	exp.samplingParams.ADCTimerDiv = 2;
	exp.samplingParams.ADCTimerPeriod = 15625;
	exp.samplingParams.ADCBufferSizeEven = 20;
	exp.samplingParams.ADCBufferSizeOdd = 20;
	exp.samplingParams.PointsIgnored = 0;
	exp.samplingParams.DACMultEven = 10;
	exp.samplingParams.DACMultOdd = 10;
	exp.DCSweep_pot.VStartUserInput = 0;
	exp.DCSweep_pot.VStartVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VEndUserInput = 512;
	exp.DCSweep_pot.VEndVsOCP = false; //todo: get user input here
	exp.DCSweep_pot.VStep = 1;
  exp.DCSweep_pot.Imax = 1e10;
	exp.MaxPlays = 1;
	exp.branchHeadIndex = 0;
	PUSH_NEW_NODE_DATA();

	exp.nodeType = END_EXPERIMENT_NODE;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}

QStringList ExampleExperiment::GetXAxisParameters(ExperimentType type) const {
	QStringList ret;

	if (type == ET_DC) {
		ret <<
			PLOT_VAR_TIMESTAMP <<
			PLOT_VAR_TIMESTAMP_NORMALIZED <<
			PLOT_VAR_EWE <<
			PLOT_VAR_CURRENT;
	}

	return ret;
}
QStringList ExampleExperiment::GetYAxisParameters(ExperimentType type) const {
	QStringList ret;

	if (type == ET_DC) {
		ret <<
			PLOT_VAR_EWE <<
			PLOT_VAR_CURRENT <<
			PLOT_VAR_ECE <<
			PLOT_VAR_CURRENT_INTEGRAL;
	}

	return ret;
}
void ExampleExperiment::PUSH_NEW_DC_DATA_DEFINITION {
	static QMap<DataMap*, qreal> timestampOffset;
	qreal timestamp = (qreal)expData.timestamp / 100000000UL;

	if (container[PLOT_VAR_CURRENT_INTEGRAL].data.isEmpty()) {
		PUSH_BACK_DATA(PLOT_VAR_CURRENT_INTEGRAL, expData.ADCrawData.current / timestamp);
	}
	else {
		qreal newVal = container[PLOT_VAR_CURRENT_INTEGRAL].data.last();
		newVal += (container[PLOT_VAR_CURRENT].data.last() + expData.ADCrawData.current) * (timestamp + container[PLOT_VAR_TIMESTAMP].data.last()) / 2.;
		PUSH_BACK_DATA(PLOT_VAR_CURRENT_INTEGRAL, newVal);
	}

	PUSH_BACK_DATA(PLOT_VAR_TIMESTAMP, timestamp);
	PUSH_BACK_DATA(PLOT_VAR_EWE, expData.ADCrawData.ewe);
	PUSH_BACK_DATA(PLOT_VAR_ECE, expData.ADCrawData.ece);
	PUSH_BACK_DATA(PLOT_VAR_CURRENT, expData.ADCrawData.current);

	if (!timestampOffset.contains(&container)) {
		timestampOffset[&container] = timestamp;
	}
	PUSH_BACK_DATA(PLOT_VAR_TIMESTAMP_NORMALIZED, timestamp - timestampOffset[&container]);
}
void ExampleExperiment::SaveDcDataHeader(QFile &saveFile, const ExperimentNotes &notes) const {
	SAVE_DATA_HEADER_START();

	SAVE_DC_DATA_HEADER(PLOT_VAR_TIMESTAMP);
	SAVE_DC_DATA_HEADER(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DC_DATA_HEADER(PLOT_VAR_EWE);
	SAVE_DC_DATA_HEADER(PLOT_VAR_CURRENT);
	SAVE_DC_DATA_HEADER(PLOT_VAR_ECE);
	SAVE_DC_DATA_HEADER(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_HEADER_END();
}

void ExampleExperiment::SaveDcData(QFile &saveFile, const DataMap &container) const {
	SAVE_DATA_START();

	SAVE_DATA(PLOT_VAR_TIMESTAMP);
	SAVE_DATA(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DATA(PLOT_VAR_EWE);
	SAVE_DATA(PLOT_VAR_CURRENT);
	SAVE_DATA(PLOT_VAR_ECE);
	SAVE_DATA(PLOT_VAR_CURRENT_INTEGRAL);
	
	SAVE_DATA_END();
}