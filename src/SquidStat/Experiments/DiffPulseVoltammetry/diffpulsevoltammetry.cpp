#include "diffpulsevoltammetry.h"

#include <ExternalStructures.h>
#include <ExperimentUIHelper.h>

#define TOP_WIDGET_NAME			"Normal-Pulse-Voltammetry"

#define START_VOLTAGE_OBJ_NAME	"start-voltage"
#define START_V_VS_OCP_OBJ_NAME	"start-voltage-vs-ocp"
#define FINAL_VOLTAGE_OBJ_NAME	"final-voltage"
#define FINAL_V_VS_OCP_OBJ_NAME	"final-voltage-vs-ocp"
#define VOLTAGE_STEP_OBJ_NAME   "voltage-step"
#define PULSE_HEIGHT_OBJ_NAME	"pulse-height"
#define PULSE_WIDTH_OBJ_NAME	"pulse-width"
#define PULSE_PERIOD_OBJ_NAME	"pulse-period"

#define START_VOLTAGE_DEFAULT	-0.5
#define FINAL_VOLTAGE_DEFAULT	0.5
#define VOLTAGE_STEP_DEFAULT	0.01
#define PULSE_HEIGHT_DEFAULT	0.1
#define PULSE_WIDTH_DEFAULT		50
#define PULSE_PERIOD_DEFAULT	100

#define PLOT_VAR_TIMESTAMP				"Timestamp"
#define PLOT_VAR_TIMESTAMP_NORMALIZED	"Elapsed time (s)"
#define PLOT_VAR_ELAPSED_TIME_HR      "Elapsed time (hr)"
#define PLOT_VAR_EWE					"Working electrode (V)"
#define PLOT_VAR_CURRENT				"Current (mA)"
#define PLOT_VAR_ECE					"Counter electrode (V)"
#define PLOT_VAR_CURRENT_INTEGRAL		"Cumulative charge (mAh)"

QString DiffPulseVoltammetry::GetShortName() const {
	return "Differential Pulse Voltammetry";
}
QString DiffPulseVoltammetry::GetFullName() const {
	return "Differential Pulse Voltammetry";
}
QString DiffPulseVoltammetry::GetDescription() const {
	return "In this experiment, the working electrode holds at a <b>starting potential</b> during the <b>quiet time</b>. Then it applies a train of pulses superimposed on a staircase waveform, with a uniform <b>voltage step</b> size. The potential continues to step until the <b>final potential</b> is reached. The <b>pulse width</b> is the amount of time between the rising and falling edge of a pulse. The <b>pulse period</b> is the amount of time between the beginning of one pulse and the beginning of the next.";
}
QStringList DiffPulseVoltammetry::GetCategory() const {
	return QStringList() <<
		"Basic voltammetry"<<
		"Pulse voltammetry";

}
ExperimentTypeList DiffPulseVoltammetry::GetTypes() const {
	return ExperimentTypeList() << ET_DC;
}
QPixmap DiffPulseVoltammetry::GetImage() const {
	return QPixmap(":/Experiments/DiffPulseVoltammetry");
}
/*
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegExpValidator>
//*/
QWidget* DiffPulseVoltammetry::CreateUserInput() const {
	USER_INPUT_START(TOP_WIDGET_NAME);

	int row = 0;
	_INSERT_RIGHT_ALIGN_COMMENT("Starting potential = ", row, 0);
	_INSERT_TEXT_INPUT(START_VOLTAGE_DEFAULT, START_VOLTAGE_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("with respect to", row, 0);
	_START_DROP_DOWN(START_V_VS_OCP_OBJ_NAME, row, 1);
	_ADD_DROP_DOWN_ITEM("open circuit");
	_ADD_DROP_DOWN_ITEM("reference");
	_END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Voltage step = ", row, 0);
	_INSERT_TEXT_INPUT(VOLTAGE_STEP_DEFAULT, VOLTAGE_STEP_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Final potential = ", row, 0);
	_INSERT_TEXT_INPUT(FINAL_VOLTAGE_DEFAULT, FINAL_VOLTAGE_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("with respect to ", row, 0);
	_START_DROP_DOWN(FINAL_V_VS_OCP_OBJ_NAME, row, 1);
	_ADD_DROP_DOWN_ITEM("open circuit");
	_ADD_DROP_DOWN_ITEM("reference");
	_END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Pulse height = ", row, 0);
	_INSERT_TEXT_INPUT(PULSE_HEIGHT_DEFAULT, PULSE_HEIGHT_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Pulse width = ", row, 0);
	_INSERT_TEXT_INPUT(PULSE_WIDTH_DEFAULT, PULSE_WIDTH_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("milliseconds", row, 2);
	
	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Pulse period = ", row, 0);
	_INSERT_TEXT_INPUT(PULSE_PERIOD_DEFAULT, PULSE_PERIOD_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("milliseconds", row, 2);
	
	_SET_COL_STRETCH(3, 2);
	_SET_COL_STRETCH(1, 0);
	_SET_ROW_STRETCH(++row, 1);
	USER_INPUT_END();
}
NodesData DiffPulseVoltammetry::GetNodesData(QWidget *wdg, const CalibrationData &calData, const HardwareVersion &hwVersion) const {
	NODES_DATA_START(wdg, TOP_WIDGET_NAME);

	double startVoltage;
	double VStep;
	double VFinal;
	double PulseHeight;
	double restTime = 1;
	quint32 pulseWidth;
	quint32 pulsePeriod;
	GET_TEXT_INPUT_VALUE_DOUBLE(startVoltage, START_VOLTAGE_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(VStep, VOLTAGE_STEP_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(VFinal, FINAL_VOLTAGE_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(PulseHeight, PULSE_HEIGHT_OBJ_NAME)
	GET_TEXT_INPUT_VALUE(pulseWidth, PULSE_WIDTH_OBJ_NAME);
	GET_TEXT_INPUT_VALUE(pulsePeriod, PULSE_PERIOD_OBJ_NAME);
  QString startVsOCP_str;
  QString endVsOCP_str;
  GET_SELECTED_DROP_DOWN(startVsOCP_str, START_V_VS_OCP_OBJ_NAME);
  GET_SELECTED_DROP_DOWN(endVsOCP_str, FINAL_V_VS_OCP_OBJ_NAME);
  
	if (pulseWidth >= pulsePeriod)
		pulsePeriod += pulseWidth;

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 100 * MILLISECONDS;
	exp.tMax = restTime * SECONDS;
  exp.currentRangeMode = AUTORANGE;
  ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, 0.1);
	exp.DCPoint_pot.VPointUserInput = (int16_t)(startVoltage * 3276.8);
	exp.DCPoint_pot.VPointVsOCP = startVsOCP_str.contains("open circuit");
  exp.DCPoint_pot.Imax = MAX_CURRENT;
	exp.DCPoint_pot.Imin = 0;
  exp.DCPoint_pot.dIdtMin = 0;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_DIFFPULSE_POT;
	exp.tMin = 10 * MILLISECONDS;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
  exp.currentRangeMode = AUTORANGE;
  ExperimentCalcHelperClass::GetSamplingParameters_pulse(hwVersion.hwModel, pulsePeriod, pulseWidth, &exp);
  exp.DCPulseDiff_pot.VStartUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, startVoltage);
	exp.DCPulseDiff_pot.VStartVsOCP = startVsOCP_str.contains("open circuit");
  exp.DCPulseDiff_pot.VEndUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, VFinal);
	exp.DCPulseDiff_pot.VEndVsOCP = endVsOCP_str.contains("open circuit");
  exp.DCPulseDiff_pot.VStep = (calData.m_DACdcN_V + calData.m_DACdcP_V) / 2 * VStep;
	exp.DCPulseDiff_pot.VHeight = (int16_t)(PulseHeight * (calData.m_DACdcN_V + calData.m_DACdcP_V) / 2);
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.nodeType = END_EXPERIMENT_NODE;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}

QStringList DiffPulseVoltammetry::GetXAxisParameters(ExperimentType type) const {
	QStringList ret;

	if (type == ET_DC) {
		ret <<
      PLOT_VAR_EWE <<
			PLOT_VAR_ELAPSED_TIME_HR <<
			PLOT_VAR_TIMESTAMP_NORMALIZED <<
			PLOT_VAR_CURRENT;
	}

	return ret;
}
QStringList DiffPulseVoltammetry::GetYAxisParameters(ExperimentType type) const {
	QStringList ret;

	if (type == ET_DC) {
		ret <<
			PLOT_VAR_CURRENT <<
			PLOT_VAR_EWE <<
			PLOT_VAR_ECE <<
			PLOT_VAR_CURRENT_INTEGRAL;
	}

	return ret;
}
void DiffPulseVoltammetry::PUSH_NEW_DC_DATA_DEFINITION {
	static QMap<DataMap*, qreal> timestampOffset;
	qreal timestamp = (qreal)expData.timestamp / SECONDS;
  ProcessedDCData processedData = ExperimentCalcHelperClass::ProcessDCDataPoint(&calData, expData);

	if (container[PLOT_VAR_CURRENT_INTEGRAL].data.isEmpty()) {
		PUSH_BACK_DATA(PLOT_VAR_CURRENT_INTEGRAL, 0);
	}
	else {
		qreal newVal = container[PLOT_VAR_CURRENT_INTEGRAL].data.last();
		newVal += (container[PLOT_VAR_CURRENT].data.last() + processedData.current) * (timestamp - container[PLOT_VAR_TIMESTAMP].data.last()) / 3600.0 / 2.;
		PUSH_BACK_DATA(PLOT_VAR_CURRENT_INTEGRAL, newVal);
	}

	PUSH_BACK_DATA(PLOT_VAR_TIMESTAMP, timestamp);
	PUSH_BACK_DATA(PLOT_VAR_EWE, processedData.EWE);
	PUSH_BACK_DATA(PLOT_VAR_ECE, processedData.ECE);
	PUSH_BACK_DATA(PLOT_VAR_CURRENT, processedData.current);

	if (!timestampOffset.contains(&container)) {
		timestampOffset[&container] = timestamp;
	}
	PUSH_BACK_DATA(PLOT_VAR_TIMESTAMP_NORMALIZED, timestamp - timestampOffset[&container]);
  PUSH_BACK_DATA(PLOT_VAR_ELAPSED_TIME_HR, (timestamp - timestampOffset[&container]) / 3600);
}
void DiffPulseVoltammetry::SaveDcDataHeader(QFile &saveFile, const ExperimentNotes &notes) const {
	SAVE_DATA_HEADER_START();

	SAVE_DC_DATA_HEADER(PLOT_VAR_ELAPSED_TIME_HR);
	SAVE_DC_DATA_HEADER(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DC_DATA_HEADER(PLOT_VAR_EWE);
	SAVE_DC_DATA_HEADER(PLOT_VAR_CURRENT);
	SAVE_DC_DATA_HEADER(PLOT_VAR_ECE);
	SAVE_DC_DATA_HEADER(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_HEADER_END();
}

void DiffPulseVoltammetry::SaveDcData(QFile &saveFile, const DataMap &container) const {
	SAVE_DATA_START();

	SAVE_DATA(PLOT_VAR_ELAPSED_TIME_HR);
	SAVE_DATA(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DATA(PLOT_VAR_EWE);
	SAVE_DATA(PLOT_VAR_CURRENT);
	SAVE_DATA(PLOT_VAR_ECE);
	SAVE_DATA(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_END();
}