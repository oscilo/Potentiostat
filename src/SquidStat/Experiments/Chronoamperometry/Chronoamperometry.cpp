#include "Chronoamperometry.h"

#include <ExternalStructures.h>
#include <ExperimentUIHelper.h>

#define TOP_WIDGET_NAME			"Chronoamperometry"

#define V1_OBJECT_NAME			"voltage-1"
#define V1_VS_OCP_OBJ_NAME		"voltage-1-vs-ocp"
#define T1_OBJECT_NAME			"voltage-1-time"
#define T1_UNITS_OBJ_NAME   "voltage-1-time-units"
#define V2_OBJECT_NAME			"voltage-2"
#define V2_VS_OCP_OBJ_NAME		"voltage-2-vs-ocp"
#define T2_OBJECT_NAME			"voltage-2-time"
#define T2_UNITS_OBJ_NAME   "voltage-2-time-units"
#define V3_OBJECT_NAME			"voltage-3"
#define V3_VS_OCP_OBJ_NAME		"voltage-3-vs-ocp"
#define T3_OBJECT_NAME			"voltage-3-time"
#define T3_UNITS_OBJ_NAME   "voltage-3-time-units"
#define V4_OBJECT_NAME			"voltage-4"
#define V4_VS_OCP_OBJ_NAME		"voltage-4-vs-ocp"
#define T4_OBJECT_NAME			"voltage-4-time"
#define T4_UNITS_OBJ_NAME   "voltage-4-time-units"
#define SAMPLING_PERIOD_OBJ_NAME	"sampling-period"
#define CURRENT_RANGE_OBJ_NAME  "current-range"

#define V1_DEFAULT				0.1
#define T1_DEFAULT				10
#define V2_DEFAULT				0.2
#define T2_DEFAULT				0
#define V3_DEFAULT				0.3
#define T3_DEFAULT				0
#define V4_DEFAULT				0.4
#define T4_DEFAULT				0
#define SAMPLING_INT_DEFAULT	0.25

#define PLOT_VAR_TIMESTAMP				"Timestamp"
#define PLOT_VAR_TIMESTAMP_NORMALIZED	"Elapsed time (s)"
#define PLOT_VAR_ELAPSED_TIME_HR      "Elapsed time (hr)"
#define PLOT_VAR_EWE					"Working electrode (V)"
#define PLOT_VAR_CURRENT				"Current (mA)"
#define PLOT_VAR_ECE					"Counter electrode (V)"
#define PLOT_VAR_CURRENT_INTEGRAL		"Cumulative charge (mAh)"

QString Chronoamperometry::GetShortName() const {
	return "Chronoamperometry";
}
QString Chronoamperometry::GetFullName() const {
	return "Chronoamperometry/Chronocoulometry";
}
QString Chronoamperometry::GetDescription() const {
	return "This experiment holds the working electrode at a constant potential for a specified amount of time while recording the current and charge passed. Up to four potentials can be used sequentially.";
}
QStringList Chronoamperometry::GetCategory() const {
	return QStringList() <<
		"Basic voltammetry";

}
ExperimentTypeList Chronoamperometry::GetTypes() const {
	return ExperimentTypeList() << ET_DC;
}
QPixmap Chronoamperometry::GetImage() const {
	return QPixmap(":/Experiments/Chronoamperometry");
}

QWidget* Chronoamperometry::CreateUserInput() const {
	USER_INPUT_START(TOP_WIDGET_NAME);

	int row = 0;

	_INSERT_RIGHT_ALIGN_COMMENT("Potential 1 = ", row, 0);
	_INSERT_TEXT_INPUT(V1_DEFAULT, V1_OBJECT_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("with respect to", row, 0);
	_START_DROP_DOWN(V1_VS_OCP_OBJ_NAME, row, 1);
	_ADD_DROP_DOWN_ITEM("reference");
	_ADD_DROP_DOWN_ITEM("open circuit");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Duration = ", row, 0);
	_INSERT_TEXT_INPUT(T1_DEFAULT, T1_OBJECT_NAME, row, 1);
  _START_DROP_DOWN(T1_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("s");
  _ADD_DROP_DOWN_ITEM("min");
  _ADD_DROP_DOWN_ITEM("hr");
  _END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Potential 2 = ", row, 0);
	_INSERT_TEXT_INPUT(V2_DEFAULT, V2_OBJECT_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("with respect to", row, 0);
	_START_DROP_DOWN(V2_VS_OCP_OBJ_NAME, row, 1);
	_ADD_DROP_DOWN_ITEM("reference");
	_ADD_DROP_DOWN_ITEM("open circuit");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Duration = ", row, 0);
	_INSERT_TEXT_INPUT(T2_DEFAULT, T2_OBJECT_NAME, row, 1);
  _START_DROP_DOWN(T2_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("s");
  _ADD_DROP_DOWN_ITEM("min");
  _ADD_DROP_DOWN_ITEM("hr");
  _END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Potential 3 = ", row, 0);
	_INSERT_TEXT_INPUT(V3_DEFAULT, V3_OBJECT_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("with respect to", row, 0);
	_START_DROP_DOWN(V3_VS_OCP_OBJ_NAME, row, 1);
	_ADD_DROP_DOWN_ITEM("reference");
	_ADD_DROP_DOWN_ITEM("open circuit");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Duration = ", row, 0);
	_INSERT_TEXT_INPUT(T3_DEFAULT, T3_OBJECT_NAME, row, 1);
  _START_DROP_DOWN(T3_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("s");
  _ADD_DROP_DOWN_ITEM("min");
  _ADD_DROP_DOWN_ITEM("hr");
  _END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Potential 4 = ", row, 0);
	_INSERT_TEXT_INPUT(V4_DEFAULT, V4_OBJECT_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("with respect to", row, 0);
	_START_DROP_DOWN(V4_VS_OCP_OBJ_NAME, row, 1);
	_ADD_DROP_DOWN_ITEM("reference");
	_ADD_DROP_DOWN_ITEM("open circuit");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Duration = ", row, 0);
	_INSERT_TEXT_INPUT(T4_DEFAULT, T4_OBJECT_NAME, row, 1);
  _START_DROP_DOWN(T4_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("s");
  _ADD_DROP_DOWN_ITEM("min");
  _ADD_DROP_DOWN_ITEM("hr");
  _END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Sampling interval: ", row, 0);
	_INSERT_TEXT_INPUT(SAMPLING_INT_DEFAULT, SAMPLING_PERIOD_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("s", row, 2);
	
	_SET_COL_STRETCH(3, 2);
	_SET_COL_STRETCH(1, 0);
	_SET_ROW_STRETCH(++row, 1);
	USER_INPUT_END();
}
NodesData Chronoamperometry::GetNodesData(QWidget *wdg, const CalibrationData &calData, const HardwareVersion &hwVersion) const {
	NODES_DATA_START(wdg, TOP_WIDGET_NAME);

	double v1, v2, v3, v4, t1, t2, t3, t4;
  QString t1Units_str, t2Units_str, t3Units_str, t4Units_str;
	GET_TEXT_INPUT_VALUE_DOUBLE(v1, V1_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(v2, V2_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(v3, V3_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(v4, V4_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(t1, T1_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(t2, T2_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(t3, T3_OBJECT_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(t4, T4_OBJECT_NAME);
  GET_SELECTED_DROP_DOWN(t1Units_str, T1_UNITS_OBJ_NAME);
  GET_SELECTED_DROP_DOWN(t2Units_str, T2_UNITS_OBJ_NAME);
  GET_SELECTED_DROP_DOWN(t3Units_str, T3_UNITS_OBJ_NAME);
  GET_SELECTED_DROP_DOWN(t4Units_str, T4_UNITS_OBJ_NAME);

	double dt;
	GET_TEXT_INPUT_VALUE_DOUBLE(dt, SAMPLING_PERIOD_OBJ_NAME);

	QString ocp1, ocp2, ocp3, ocp4;
	bool _ocp1, _ocp2, _ocp3, _ocp4;
	GET_SELECTED_DROP_DOWN(ocp1, V1_VS_OCP_OBJ_NAME);
	GET_SELECTED_DROP_DOWN(ocp2, V2_VS_OCP_OBJ_NAME);
	GET_SELECTED_DROP_DOWN(ocp3, V3_VS_OCP_OBJ_NAME);
	GET_SELECTED_DROP_DOWN(ocp4, V4_VS_OCP_OBJ_NAME);
	_ocp1 = ocp1.contains("open circuit");
	_ocp2 = ocp2.contains("open circuit");
	_ocp3 = ocp3.contains("open circuit");
	_ocp4 = ocp4.contains("open circuit");

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 0;
  exp.tMax = t1 * SECONDS * ExperimentCalcHelperClass::GetUnitsMultiplier(t1Units_str);
  exp.currentRangeMode = AUTORANGE;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, dt);
  exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, v1);
	exp.DCPoint_pot.VPointVsOCP = _ocp1;
	exp.DCPoint_pot.Imax = MAX_CURRENT;
	exp.DCPoint_pot.Imin = 0;
  exp.DCPoint_pot.dIdtMin = 0;
	exp.MaxPlays = 1;
  if (exp.tMax != 0)
    PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 0;
	exp.tMax = t2 * SECONDS * ExperimentCalcHelperClass::GetUnitsMultiplier(t2Units_str);
  exp.currentRangeMode = AUTORANGE;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, dt);
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, v2);
	exp.DCPoint_pot.VPointVsOCP = false;
	exp.DCPoint_pot.Imax = MAX_CURRENT;
	exp.DCPoint_pot.Imin = 0;
  exp.DCPoint_pot.dIdtMin = 0;
	exp.MaxPlays = 1;
  if (exp.tMax != 0)
    PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 0;
	exp.tMax = t3 * SECONDS * ExperimentCalcHelperClass::GetUnitsMultiplier(t3Units_str);
  exp.currentRangeMode = AUTORANGE;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, dt);
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, v3);
	exp.DCPoint_pot.VPointVsOCP = false;
	exp.DCPoint_pot.Imax = MAX_CURRENT;
	exp.DCPoint_pot.Imin = 0;
  exp.DCPoint_pot.dIdtMin = 0;
	exp.MaxPlays = 1;
  if (exp.tMax != 0)
    PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 0;
	exp.tMax = t4 * SECONDS * ExperimentCalcHelperClass::GetUnitsMultiplier(t4Units_str);
  exp.currentRangeMode = AUTORANGE;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, dt);
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, v4);
	exp.DCPoint_pot.VPointVsOCP = false;
	exp.DCPoint_pot.Imax = MAX_CURRENT;
	exp.DCPoint_pot.Imin = 0;
  exp.DCPoint_pot.dIdtMin = 0;
	exp.MaxPlays = 1;
  if (exp.tMax != 0)
    PUSH_NEW_NODE_DATA();

	exp.nodeType = END_EXPERIMENT_NODE;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}

QStringList Chronoamperometry::GetXAxisParameters(ExperimentType type) const {
	QStringList ret;

	if (type == ET_DC) {
		ret <<
      PLOT_VAR_ELAPSED_TIME_HR <<
			PLOT_VAR_TIMESTAMP_NORMALIZED <<
			PLOT_VAR_EWE <<
			PLOT_VAR_CURRENT;
	}

	return ret;
}
QStringList Chronoamperometry::GetYAxisParameters(ExperimentType type) const {
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
void Chronoamperometry::PUSH_NEW_DC_DATA_DEFINITION {
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
void Chronoamperometry::SaveDcDataHeader(QFile &saveFile, const ExperimentNotes &notes) const {
	SAVE_DATA_HEADER_START();

	SAVE_DC_DATA_HEADER(PLOT_VAR_ELAPSED_TIME_HR);
	SAVE_DC_DATA_HEADER(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DC_DATA_HEADER(PLOT_VAR_EWE);
	SAVE_DC_DATA_HEADER(PLOT_VAR_CURRENT);
	SAVE_DC_DATA_HEADER(PLOT_VAR_ECE);
	SAVE_DC_DATA_HEADER(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_HEADER_END();
}

void Chronoamperometry::SaveDcData(QFile &saveFile, const DataMap &container) const {
	SAVE_DATA_START();

	SAVE_DATA(PLOT_VAR_ELAPSED_TIME_HR);
	SAVE_DATA(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DATA(PLOT_VAR_EWE);
	SAVE_DATA(PLOT_VAR_CURRENT);
	SAVE_DATA(PLOT_VAR_ECE);
	SAVE_DATA(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_END();
}