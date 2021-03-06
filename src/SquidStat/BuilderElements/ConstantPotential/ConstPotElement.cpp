#include "ConstPotElement.h"

#include <ExternalStructures.h>

#define BUILDER_ELEMENTS_USER_INPUTS
#include <ExperimentUIHelper.h>


#define TOP_WIDGET_NAME			"constant-potential-element"

#define CONST_POTENTIAL_OBJ_NAME	"constant-potential"
#define POTENTIAL_V_OCP_OBJ_NAME	"potential-vs-ocp"
#define DURATION_OBJ_NAME         "duration"
#define DURATION_UNITS_OBJ_NAME   "duration-units"
#define SAMPLING_INTERVAL_OBJ_NAME		"sampling-interval"

#define CONST_POTENTIAL_DEFAULT	0.5
#define DURATION_DEFAULT  60
#define SAMPLING_INTERVAL_DEFAULT		0.1

QString ConstPotElement::GetFullName() const {
	return "Constant Potential";
}
QStringList ConstPotElement::GetCategory() const {
	return QStringList() <<
		"Potentiostatic control" <<
		"Basic voltammetry";
}
QPixmap ConstPotElement::GetImage() const {
	return QPixmap(":/GUI/ConstantPotential");
}
ExperimentType ConstPotElement::GetType() const {
	return ET_DC;
}
QWidget* ConstPotElement::CreateUserInput(UserInput &inputs) const {
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
  _INSERT_RIGHT_ALIGN_COMMENT("Duration: ", row, 0);
  _INSERT_TEXT_INPUT(DURATION_DEFAULT, DURATION_OBJ_NAME, row, 1);
  _START_DROP_DOWN(DURATION_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("s");
  _ADD_DROP_DOWN_ITEM("min");
  _ADD_DROP_DOWN_ITEM("hr");
  _END_DROP_DOWN();

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Sampling interval: ", row, 0);
	_INSERT_TEXT_INPUT(SAMPLING_INTERVAL_DEFAULT, SAMPLING_INTERVAL_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("s", row, 2);

	_SET_ROW_STRETCH(++row, 1);
	_SET_COL_STRETCH(3, 1);
  //_SET_COL_STRETCH(1, 1);
  //_SET_COL_STRETCH(2, 2);

	USER_INPUT_END();
}
NodesData ConstPotElement::GetNodesData(const UserInput &inputs, const CalibrationData &calData, const HardwareVersion &hwVersion) const {
	NODES_DATA_START(wdg, TOP_WIDGET_NAME);

  QString vsOCP_string = inputs[POTENTIAL_V_OCP_OBJ_NAME].toString();
  bool vsOCP = vsOCP_string.contains("open circuit");
  double duration = inputs[DURATION_OBJ_NAME].toDouble();
  duration *= ExperimentCalcHelperClass::GetUnitsMultiplier(inputs[DURATION_UNITS_OBJ_NAME].toString());

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 10 * MILLISECONDS;
	exp.tMax = (uint64_t)(duration * SECONDS);
  exp.currentRangeMode = AUTORANGE;
  ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, inputs[SAMPLING_INTERVAL_OBJ_NAME].toDouble());
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetBINVoltageForDAC(&calData, inputs[CONST_POTENTIAL_OBJ_NAME].toDouble());
  exp.DCPoint_pot.VPointVsOCP = vsOCP;
  exp.DCPoint_pot.Imax = MAX_CURRENT;
  exp.DCPoint_pot.Imin = 0;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}