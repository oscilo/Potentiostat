#include "DCPotentialSweep.h"
#include <ExternalStructures.h>

#define BUILDER_ELEMENTS_USER_INPUTS
#include <ExperimentUIHelper.h>

#define TOP_WIDGET_NAME			"potential-sweep-element"

#define VSTART_OBJ_NAME           "starting-voltage"
#define VSTART_VS_OCP_OBJ_NAME    "starting-voltage-vs-ocp"
#define VEND_OBJ_NAME             "ending-voltage"
#define VEND_VS_OCP_OBJ_NAME      "ending-voltage-vs-ocp"
#define SLEWRATE_OBJ_NAME         "slewrate"
#define SAMPLING_MODE_OBJ_NAME    "sampling-mode"
#define SAMPLING_INT_OBJ_NAME     "sampling-interval"
#define SAMPLING_INT_UNITS_OBJ_NAME       "sampling-interval-units"
#define MAX_CURRENT_OBJ_NAME      "max-current"
#define MAX_CURRENT_UNITS_OBJ_NAME "max-current-units"
#define MIN_CURRENT_OBJ_NAME      "min-current"
#define MIN_CURRENT_UNITS_OBJ_NAME  "min-current-units"
#define AUTORANGE_MODE_OBJ_NAME   "autorange-mode"

#define VSTART_DEFAULT	    -0.1
#define VEND_DEFAULT        0.1
#define SLEWRATE_DEFAULT		10
#define SAMPLING_INT_DEFAULT 0.1
#define MAX_CURRENT_DEFAULT 100
#define MIN_CURRENT_DEFAULT 0

QString DCPotentialSweepElement::GetFullName() const {
	return "Potential sweep";
}
QStringList DCPotentialSweepElement::GetCategory() const {
	return QStringList() <<
		"Potentiostatic control" <<
		"Basic voltammetry";
}
QPixmap DCPotentialSweepElement::GetImage() const {
	return QPixmap(":/GUI/Resources/element.png");
}
ExperimentType DCPotentialSweepElement::GetType() const {
	return ET_DC;
}
QWidget* DCPotentialSweepElement::CreateUserInput(UserInput &inputs) const {
	USER_INPUT_START(TOP_WIDGET_NAME);

	int row = 0;
	_INSERT_RIGHT_ALIGN_COMMENT("Starting Potential = ", row, 0);
	_INSERT_TEXT_INPUT(VSTART_DEFAULT, VSTART_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

  ++row;
  _INSERT_RIGHT_ALIGN_COMMENT("With respect to ", row, 0);
  _START_DROP_DOWN(VSTART_VS_OCP_OBJ_NAME, row, 1);
  _ADD_DROP_DOWN_ITEM("reference");
  _ADD_DROP_DOWN_ITEM("open circuit");
  _END_DROP_DOWN();

  ++row;
  _INSERT_RIGHT_ALIGN_COMMENT("Ending potential = ", row, 0);
  _INSERT_TEXT_INPUT(VEND_DEFAULT, VEND_OBJ_NAME, row, 1);
  _INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

  ++row;
  _INSERT_RIGHT_ALIGN_COMMENT("Slew rate = ", row, 0);
  _INSERT_TEXT_INPUT(SLEWRATE_DEFAULT, SLEWRATE_OBJ_NAME, row, 1);
  _INSERT_LEFT_ALIGN_COMMENT("mV/s", row, 2);

  ++row;
  _INSERT_RIGHT_ALIGN_COMMENT("Maximum current: ", row, 0);
  _INSERT_TEXT_INPUT(MAX_CURRENT_DEFAULT, MAX_CURRENT_OBJ_NAME, row, 1);
  _START_DROP_DOWN(MAX_CURRENT_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("mA");
  _ADD_DROP_DOWN_ITEM("uA");
  _ADD_DROP_DOWN_ITEM("nA");
  _END_DROP_DOWN();

  ++row;
  _INSERT_RIGHT_ALIGN_COMMENT("Minimum current: ", row, 0);
  _INSERT_TEXT_INPUT(MIN_CURRENT_DEFAULT, MIN_CURRENT_OBJ_NAME, row, 1);
  _START_DROP_DOWN(MIN_CURRENT_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("mA");
  _ADD_DROP_DOWN_ITEM("uA");
  _ADD_DROP_DOWN_ITEM("nA");
  _END_DROP_DOWN();

  ++row;
  _INSERT_VERTICAL_SPACING(row);

  ++row;
  _INSERT_LEFT_ALIGN_COMMENT("<b>Sampling interval", row, 1);
  
  ++row;
  _START_RADIO_BUTTON_GROUP(SAMPLING_MODE_OBJ_NAME);
  _INSERT_RADIO_BUTTON("Auto-calculate (recommended)", row, 0);
  ++row;
  _INSERT_RADIO_BUTTON("Fixed interval: ", row, 0);
  _END_RADIO_BUTTON_GROUP();
  _INSERT_TEXT_INPUT(SAMPLING_INT_DEFAULT, SAMPLING_INT_OBJ_NAME, row, 1);
  _START_DROP_DOWN(SAMPLING_INT_UNITS_OBJ_NAME, row, 2);
  _ADD_DROP_DOWN_ITEM("s");
  _ADD_DROP_DOWN_ITEM("min");
  _ADD_DROP_DOWN_ITEM("hr");
  _END_DROP_DOWN();

  ++row;
  _INSERT_VERTICAL_SPACING(row);

  ++row;
  _INSERT_LEFT_ALIGN_COMMENT("<b>Current ranging</b>", row, 1);

  ++row;
  _START_RADIO_BUTTON_GROUP_HORIZONTAL_LAYOUT(AUTORANGE_MODE_OBJ_NAME, row, 0);
  _INSERT_RADIO_BUTTON_LAYOUT("Autorange");
  _INSERT_RADIO_BUTTON_LAYOUT("Fixed range (based on maximum current)");
  _END_RADIO_BUTTON_GROUP();

	_SET_ROW_STRETCH(++row, 1);
	_SET_COL_STRETCH(2, 1);

	USER_INPUT_END();
}

NodesData DCPotentialSweepElement::GetNodesData(const UserInput &inputs, const CalibrationData &calData, const HardwareVersion &hwVersion) const
{
  NODES_DATA_START(wdg, TOP_WIDGET_NAME);

  double VStart = inputs[VSTART_OBJ_NAME].toDouble();
  bool VStartVsOCP = inputs[VSTART_VS_OCP_OBJ_NAME].toString().contains("open circuit");
  double VEnd = inputs[VEND_OBJ_NAME].toDouble();
  bool VEndVsOCP = inputs[VEND_VS_OCP_OBJ_NAME].toString().contains("open circuit");
  double dVdt = inputs[SLEWRATE_OBJ_NAME].toDouble();
  QString samplingMode_str = inputs[SAMPLING_MODE_OBJ_NAME].toString();
  double samplingInterval = inputs[SAMPLING_INT_OBJ_NAME].toDouble();
  QString samplingIntUnits_str = inputs[SAMPLING_INT_UNITS_OBJ_NAME].toString();
  double maxCurrent = inputs[MAX_CURRENT_OBJ_NAME].toDouble();
  QString maxCurrentUnits_str = inputs[MAX_CURRENT_UNITS_OBJ_NAME].toString();
  double minCurrent = inputs[MIN_CURRENT_OBJ_NAME].toDouble();
  QString minCurrentUnits_str = inputs[MIN_CURRENT_UNITS_OBJ_NAME].toString();
  QString currentRangeMode_str = inputs[AUTORANGE_MODE_OBJ_NAME].toString();
  currentRange_t currentRangeMode;

  samplingInterval = samplingMode_str.contains("Auto") ? 0 : inputs[SAMPLING_INT_OBJ_NAME].toDouble();
  if (samplingIntUnits_str.contains("s"))
    samplingInterval *= 1;
  else if (samplingIntUnits_str.contains("min"))
    samplingInterval *= 60;
  else if (samplingIntUnits_str.contains("h"))
    samplingInterval *= 3600;
  
  if (maxCurrentUnits_str.contains("mA"))
    maxCurrent *= 1;
  else if (maxCurrentUnits_str.contains("uA"))
    maxCurrent *= 1e-3;
  else if (maxCurrentUnits_str.contains("nA"))
    maxCurrent *= 1e-6;

  if (minCurrentUnits_str.contains("mA"))
    minCurrent *= 1;
  else if (minCurrentUnits_str.contains("uA"))
    minCurrent *= 1e-3;
  else if (minCurrentUnits_str.contains("nA"))
    minCurrent *= 1e-6;


  if (currentRangeMode_str.contains("Autorange"))
    currentRangeMode = AUTORANGE;
  else
    currentRangeMode = ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, maxCurrent);

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 0;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
  exp.currentRangeMode = currentRangeMode;
  uint32_t numPoints = ExperimentCalcHelperClass::GetSamplingParams_potSweep(hwVersion.hwModel, &calData, &exp, dVdt, samplingInterval);
  (const_cast<DCPotentialSweepElement*>(this))->setNumIgnoredPoints(numPoints);
	exp.DCSweep_pot.VStartUserInput = ExperimentCalcHelperClass::GetBINVoltage(&calData, VStart);
  exp.DCSweep_pot.VStartVsOCP = VStartVsOCP;
  exp.DCSweep_pot.VEndUserInput = ExperimentCalcHelperClass::GetBINVoltage(&calData, VEnd);
  exp.DCSweep_pot.VEndVsOCP = VEndVsOCP;
  exp.DCSweep_pot.IRangeMax = ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, maxCurrent);
  exp.DCSweep_pot.Imax = ExperimentCalcHelperClass::GetBINCurrent(&calData, exp.DCSweep_pot.IRangeMax, maxCurrent);
  exp.DCSweep_pot.IRangeMin = currentRangeMode == AUTORANGE ? ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, minCurrent) : exp.DCSweep_pot.IRangeMax;
  exp.DCSweep_pot.Imin = ExperimentCalcHelperClass::GetBINCurrent(&calData, exp.DCSweep_pot.IRangeMin, minCurrent);
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}

void DCPotentialSweepElement::setNumIgnoredPoints(uint32_t numPoints)
{
  numIgnoredPoints = numPoints;
}