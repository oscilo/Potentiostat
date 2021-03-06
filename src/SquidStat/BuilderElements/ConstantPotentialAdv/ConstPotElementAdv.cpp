#include "ConstPotElementAdv.h"

#include <ExternalStructures.h>

#define BUILDER_ELEMENTS_USER_INPUTS
#include <ExperimentUIHelper.h>


#define TOP_WIDGET_NAME			"constant-potential-adv-element"

#define CONST_POTENTIAL_OBJ_NAME	"constant-potential"
#define POTENTIAL_V_OCP_OBJ_NAME	"potential-vs-ocp"
#define DURATION_OBJ_NAME         "duration"
#define DURATION_UNITS_OBJ_NAME   "duration-units"
#define SAMPLING_INTERVAL_OBJ_NAME		"sampling-interval"
#define CURRENT_RANGE_OBJ_NAME  "current-range-mode"
#define CURRENT_RANGE_VALUE_OBJ_NAME  "approx-current-max-value"
#define CURRENT_RANGE_UNITS_OBJ_NAME  "current-range-units"
#define DI_DT_MIN_OBJ_NAME        "dIdt-min"
#define DI_DT_UNITS_OBJ_NAME      "dIdt-units"
#define MINIMUM_CURRENT_OBJ_NAME  "minimum-current"
#define MINIMUM_CURRENT_UNITS_OBJ_NAME  "minimum-current-units"


#define CONST_POTENTIAL_DEFAULT	0.5
#define DURATION_DEFAULT  60
#define SAMPLING_INTERVAL_DEFAULT		0.1
#define DI_DT_MIN_DEFAULT 1
#define CURRENT_RANGE_VALUE_DEFAULT 100
#define MINIMUM_CURRENT_DEFAULT 0

QString ConstPotElementAdv::GetFullName() const {
	return "Constant Potential, Advanced";
}
QStringList ConstPotElementAdv::GetCategory() const {
	return QStringList() <<
		"Potentiostatic control" <<
		"Basic voltammetry";
}
QPixmap ConstPotElementAdv::GetImage() const {
	return QPixmap(":/GUI/ConstantPotentialAdv");
}
ExperimentType ConstPotElementAdv::GetType() const {
	return ET_DC;
}
QWidget* ConstPotElementAdv::CreateUserInput(UserInput &inputs) const {
	USER_INPUT_START(TOP_WIDGET_NAME);

	int row = 0;
	_INSERT_RIGHT_ALIGN_COMMENT("Potential = ", row, 0);
	_INSERT_TEXT_INPUT(CONST_POTENTIAL_DEFAULT, CONST_POTENTIAL_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("With respect to ", row, 0);
	_START_DROP_DOWN_EXT(POTENTIAL_V_OCP_OBJ_NAME, row, 1, 1, 2);
	_ADD_DROP_DOWN_ITEM("reference");
	_ADD_DROP_DOWN_ITEM("open circuit");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Sampling interval: ", row, 0);
	_INSERT_TEXT_INPUT(SAMPLING_INTERVAL_DEFAULT, SAMPLING_INTERVAL_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("s", row, 2);

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_CENTERED_COMMENT("<b>Ending conditions</b>", row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Max duration: ", row, 0);
	_INSERT_TEXT_INPUT(DURATION_DEFAULT, DURATION_OBJ_NAME, row, 1);
	_START_DROP_DOWN(DURATION_UNITS_OBJ_NAME, row, 2);
	_ADD_DROP_DOWN_ITEM("s");
	_ADD_DROP_DOWN_ITEM("min");
	_ADD_DROP_DOWN_ITEM("hr");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Minimum current: ", row, 0);
	_INSERT_TEXT_INPUT(MINIMUM_CURRENT_DEFAULT, MINIMUM_CURRENT_OBJ_NAME, row, 1);
	_START_DROP_DOWN(MINIMUM_CURRENT_UNITS_OBJ_NAME, row, 2);
	_ADD_DROP_DOWN_ITEM("mA");
	_ADD_DROP_DOWN_ITEM("uA");
	_ADD_DROP_DOWN_ITEM("nA");
	_END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Minimum dI/dt", row, 0);
	_INSERT_TEXT_INPUT(DI_DT_MIN_DEFAULT, DI_DT_MIN_OBJ_NAME, row, 1);
	_START_DROP_DOWN(DI_DT_UNITS_OBJ_NAME, row, 2);
	_ADD_DROP_DOWN_ITEM("mA/s");
	_ADD_DROP_DOWN_ITEM("uA/s");
	_ADD_DROP_DOWN_ITEM("nA/s");
	_END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_CENTERED_COMMENT("<b>Current ranging</b>", row);

	++row;
	_START_RADIO_BUTTON_GROUP(CURRENT_RANGE_OBJ_NAME);
	_INSERT_RADIO_BUTTON("Autorange", row, 0);
	++row;
	_INSERT_RADIO_BUTTON("Upper current limit: ", row, 0);
	_END_RADIO_BUTTON_GROUP();
	_INSERT_TEXT_INPUT(CURRENT_RANGE_VALUE_DEFAULT, CURRENT_RANGE_VALUE_OBJ_NAME, row, 1);
	_START_DROP_DOWN(CURRENT_RANGE_UNITS_OBJ_NAME, row, 2);
	_ADD_DROP_DOWN_ITEM("mA");
	_ADD_DROP_DOWN_ITEM("uA");
	_ADD_DROP_DOWN_ITEM("nA");
	_END_DROP_DOWN();

	_SET_ROW_STRETCH(++row, 1);
	_SET_COL_STRETCH(4, 1);
	/*
	_SET_COL_STRETCH(0, 10);
	_SET_COL_STRETCH(1, 1);
	_SET_COL_STRETCH(2, 2);
	//*/

	USER_INPUT_END();
}
NodesData ConstPotElementAdv::GetNodesData(const UserInput &inputs, const CalibrationData &calData, const HardwareVersion &hwVersion) const {
	NODES_DATA_START(wdg, TOP_WIDGET_NAME);

  double VPoint = inputs[CONST_POTENTIAL_OBJ_NAME].toDouble();
  bool vsOCP = inputs[POTENTIAL_V_OCP_OBJ_NAME].toString().contains("open circuit");
  double samplingInterval = inputs[SAMPLING_INTERVAL_OBJ_NAME].toDouble();
  double duration = inputs[DURATION_OBJ_NAME].toDouble();
  double minimumCurrent = inputs[MINIMUM_CURRENT_OBJ_NAME].toDouble();
  double dIdtMin = inputs[DI_DT_MIN_OBJ_NAME].toDouble();
  QString dIdtUnits_str = inputs[DI_DT_UNITS_OBJ_NAME].toString();
  QString currentRangeMode_str = inputs[CURRENT_RANGE_OBJ_NAME].toString();
  currentRange_t currentRangeMode;
  double upperCurrentLimit = inputs[CURRENT_RANGE_VALUE_OBJ_NAME].toDouble();
  
  duration *= ExperimentCalcHelperClass::GetUnitsMultiplier(inputs[DURATION_UNITS_OBJ_NAME].toString());
  minimumCurrent *= ExperimentCalcHelperClass::GetUnitsMultiplier(inputs[MINIMUM_CURRENT_UNITS_OBJ_NAME].toString());
  dIdtMin *= ExperimentCalcHelperClass::GetUnitsMultiplier(inputs[DI_DT_UNITS_OBJ_NAME].toString());
  upperCurrentLimit *= ExperimentCalcHelperClass::GetUnitsMultiplier(inputs[CURRENT_RANGE_UNITS_OBJ_NAME].toString());
  
  if (currentRangeMode_str.contains("Autorange"))
    currentRangeMode = AUTORANGE;
  else
    currentRangeMode = ExperimentCalcHelperClass::GetMinCurrentRange(hwVersion.hwModel, &calData, upperCurrentLimit);

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 10 * MILLISECONDS;
	exp.tMax = (uint64_t)(duration * SECONDS);
  exp.currentRangeMode = currentRangeMode;
  ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, samplingInterval);
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, VPoint);
  exp.DCPoint_pot.VPointVsOCP = vsOCP;
  exp.DCPoint_pot.Imax = MAX_CURRENT;
  exp.DCPoint_pot.Imin = minimumCurrent;
  exp.DCPoint_pot.dIdtMin = dIdtMin;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}