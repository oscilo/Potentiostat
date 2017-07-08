#include "ChargeDischargeDC.h"

#include <ExternalStructures.h>
#include <ExperimentUIHelper.h>

#define TOP_WIDGET_NAME			"Charge-Discharge-DC"

#define CHG_CURRENT_OBJ_NAME	"charge-current"
#define DISCHG_CURRENT_OBJ_NAME	"discharge-current"
#define MIN_CHG_CURRENT_OBJ_NAME	"min-charge-current"
#define MIN_DISCHG_CURRENT_OBJ_NAME	"min-discharge-current"
#define UPPER_VOLTAGE_OBJ_NAME	"upper-voltage"
#define LOWER_VOLTAGE_OBJ_NAME  "lower-voltage"
#define SAMP_INTERVAL_OBJ_NAME	"sampling-interval"
#define REST_PERIOD_OBJ			"rest_period"
#define REST_PERIOD_INT_OBJ		"rest-period-interval"
#define CYCLES_OBJ_NAME			"cycles"

#define CHG_CURRENT_DEFAULT		100		//(in mA)
#define DISCHG_CURRENT_DEFAULT	-100	//(in mA)
#define MIN_CHG_CURRENT_DEFAULT	0.5
#define MIN_DISCHG_CURRENT_DEFAULT -0.5
#define UPPER_VOLTAGE_DEFAULT	3.6
#define LOWER_VOLTAGE_DEFAULT	2.7
#define SAMP_INTERVAL_DEFAULT	10
#define REST_PERIOD_DEFAULT		120
#define REST_PERIOD_INT_DEFAULT 1
#define CYCLES_DEFAULT			10

#define PLOT_VAR_TIMESTAMP				"Timestamp"
#define PLOT_VAR_TIMESTAMP_NORMALIZED	"Timestamp (normalized)"
#define PLOT_VAR_EWE					"Ewe"
#define PLOT_VAR_CURRENT				"Current"
#define PLOT_VAR_ECE					"Ece"
#define PLOT_VAR_CURRENT_INTEGRAL		"Integral d(Current)/d(time)"

QString ChargeDischargeDC::GetShortName() const {
	return "Charge/Discharge";
}
QString ChargeDischargeDC::GetFullName() const {
	return "Charge/Discharge (DC only)";
}
QString ChargeDischargeDC::GetDescription() const {
	return "This experiment repeatedly charges and discharges the cell under test. For each charge and discharge phase, the cell undergoes a <b>constant current</b> segment and then a <b>constant voltage</b> segment.";
}
QStringList ChargeDischargeDC::GetCategory() const {
	return QStringList() <<
		"Energy storage";

}
QPixmap ChargeDischargeDC::GetImage() const {
	return QPixmap(":/GUI/Resources/experiment.png");
}
/*
#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegExpValidator>
//*/
QWidget* ChargeDischargeDC::CreateUserInput() const {
	USER_INPUT_START(TOP_WIDGET_NAME);

	int row = 0;
	_INSERT_RIGHT_ALIGN_COMMENT("Starting phase", row, 0);
	_START_DROP_DOWN("Starting phase selection id", row, 1);
	_ADD_DROP_DOWN_ITEM("Charge first");
	_ADD_DROP_DOWN_ITEM("Discharge first");
	_END_DROP_DOWN();

	++row;
	_INSERT_VERTICAL_SPACING(row);
	
	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Charging current = ", row, 0);
	_INSERT_TEXT_INPUT(CHG_CURRENT_DEFAULT, CHG_CURRENT_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("mA", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Upper voltage limit = ", row, 0);
	_INSERT_TEXT_INPUT(UPPER_VOLTAGE_DEFAULT, UPPER_VOLTAGE_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Minimum charging current = ", row, 0);
	_INSERT_TEXT_INPUT(MIN_CHG_CURRENT_DEFAULT, MIN_CHG_CURRENT_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("mA", row, 2);

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Discharging current = ", row, 0);
	_INSERT_TEXT_INPUT(DISCHG_CURRENT_DEFAULT, DISCHG_CURRENT_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("mA", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Lower voltage limit = ", row, 0);
	_INSERT_TEXT_INPUT(LOWER_VOLTAGE_DEFAULT, LOWER_VOLTAGE_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("V", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Minimum discharging current = ", row, 0);
	_INSERT_TEXT_INPUT(MIN_DISCHG_CURRENT_DEFAULT, MIN_DISCHG_CURRENT_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("mA", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Sampling interval = ", row, 0);
	_INSERT_TEXT_INPUT(SAMP_INTERVAL_DEFAULT, SAMP_INTERVAL_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("s", row, 2);

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Rest period duration = ", row, 0);
	_INSERT_TEXT_INPUT(REST_PERIOD_DEFAULT, REST_PERIOD_OBJ, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("s", row, 2);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Rest period sampling interval = ", row, 0);
	_INSERT_TEXT_INPUT(REST_PERIOD_INT_DEFAULT, REST_PERIOD_INT_OBJ, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("s", row, 2);

	++row;
	_INSERT_VERTICAL_SPACING(row);

	++row;
	_INSERT_RIGHT_ALIGN_COMMENT("Cycles = ", row, 0);
	_INSERT_TEXT_INPUT(CYCLES_DEFAULT, CYCLES_OBJ_NAME, row, 1);
	_INSERT_LEFT_ALIGN_COMMENT("", row, 2);
	
	_SET_COL_STRETCH(3, 2);
	_SET_COL_STRETCH(1, 0);
	USER_INPUT_END();
}
NodesData ChargeDischargeDC::GetNodesData(QWidget *wdg, const CalibrationData &calData, const HardwareVersion &hwVersion) const {
	NODES_DATA_START(wdg, TOP_WIDGET_NAME);
	/*
	QString selectedRadio1;
	QString selectedRadio2;
	GET_SELECTED_RADIO(selectedRadio1, "Test radio 1 id");
	GET_SELECTED_RADIO(selectedRadio2, "Test radio 2 id");


	QString selectedDropDown;
	GET_SELECTED_DROP_DOWN(selectedDropDown, "Test drop down id");
	//*/

	//TODO: what to do about cells hooked up backwards?
	//TODO: incorporate max charge/discharge capacity? Or incorporate this into another experiment?
	//TODO: make a const. power discharge node type
	//TODO: is there a constant voltage phase during discharge?

	bool chargeFirst;
	double upperVoltage;
	double lowerVoltage;
	double chgCurrent;
	double minChgCurrent;
	double dischgCurrent;
	double minDischgCurrent;
	double sampInterval;
	double restPeriodDuration;
	double restPeriodInterval;
	qint32 cycles;
	QString firstPhase;
	GET_SELECTED_DROP_DOWN(firstPhase, "Starting phase selection id");
	GET_TEXT_INPUT_VALUE_DOUBLE(upperVoltage, UPPER_VOLTAGE_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(lowerVoltage, LOWER_VOLTAGE_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(chgCurrent, CHG_CURRENT_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(dischgCurrent, DISCHG_CURRENT_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(minChgCurrent, MIN_CHG_CURRENT_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(minDischgCurrent, MIN_DISCHG_CURRENT_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(sampInterval, SAMP_INTERVAL_OBJ_NAME);
	GET_TEXT_INPUT_VALUE_DOUBLE(restPeriodDuration, REST_PERIOD_OBJ);
	GET_TEXT_INPUT_VALUE_DOUBLE(restPeriodInterval, REST_PERIOD_INT_OBJ);
	GET_TEXT_INPUT_VALUE(cycles, CYCLES_OBJ_NAME);
	if (firstPhase.contains("Charge first"))
	{
		chargeFirst = true;
	}

	exp.isHead = true;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_GALV;
	exp.tMin = 1e8;
	exp.tMax = 0xffffffffffffffff;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, sampInterval);
	exp.DCPoint_galv.Irange = chargeFirst ? ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, chgCurrent) : ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, dischgCurrent);
	exp.DCPoint_galv.IPoint = chargeFirst ? ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_galv.Irange, chgCurrent) : ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_galv.Irange, dischgCurrent);
	exp.DCPoint_galv.Vmax = ExperimentCalcHelperClass::GetVoltage(&calData, upperVoltage);
	exp.DCPoint_galv.Vmin = ExperimentCalcHelperClass::GetVoltage(&calData, lowerVoltage);
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 1e8;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, sampInterval);
	exp.DCPoint_pot.IrangeMax = ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, chargeFirst ? chgCurrent * 1.5 : dischgCurrent * 1.5);
	exp.DCPoint_pot.Imax = chargeFirst ? ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMax, chgCurrent * 1.5) : ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMax, chgCurrent * 1.5);
	exp.DCPoint_pot.IrangeMin = ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, chargeFirst ? minChgCurrent : minDischgCurrent);
	exp.DCPoint_pot.Imin = chargeFirst ? ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMin, minChgCurrent) : ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMin, minDischgCurrent);
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetVoltage(&calData, chargeFirst ? upperVoltage : lowerVoltage);
	exp.DCPoint_pot.VPointVsOCP = false;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = exp.isTail = false;
	exp.nodeType = DCNODE_OCP;
	exp.DCocp.Vmin = 0;
	exp.DCocp.Vmax = 0x7fff;
	//TODO exp.DCocp.dVdtMax = 0;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, restPeriodInterval);
	exp.tMin = 25e6;
	exp.tMax = restPeriodDuration * 1e8;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = true;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_GALV;
	exp.tMin = 1e8;
	exp.tMax = 0xffffffffffffffff;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, sampInterval);
	exp.DCPoint_galv.Irange = !chargeFirst ? ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, chgCurrent) : ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, dischgCurrent);
	exp.DCPoint_galv.IPoint = !chargeFirst ? ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_galv.Irange, chgCurrent) : ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_galv.Irange, dischgCurrent);
	exp.DCPoint_galv.Vmax = ExperimentCalcHelperClass::GetVoltage(&calData, upperVoltage);
	exp.DCPoint_galv.Vmin = ExperimentCalcHelperClass::GetVoltage(&calData, lowerVoltage);
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = false;
	exp.nodeType = DCNODE_POINT_POT;
	exp.tMin = 1e8;
	exp.tMax = 0xFFFFFFFFFFFFFFFF;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, sampInterval);
	exp.DCPoint_pot.IrangeMax = ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, !chargeFirst ? chgCurrent * 1.5 : dischgCurrent * 1.5);
	exp.DCPoint_pot.Imax = !chargeFirst ? ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMax, chgCurrent * 1.5) : ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMax, chgCurrent * 1.5);
	exp.DCPoint_pot.IrangeMin = ExperimentCalcHelperClass::GetCurrentRange(hwVersion.hwModel, &calData, !chargeFirst ? minChgCurrent : minDischgCurrent);
	exp.DCPoint_pot.Imin = !chargeFirst ? ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMin, minChgCurrent) : ExperimentCalcHelperClass::GetCurrent(&calData, exp.DCPoint_pot.IrangeMin, minDischgCurrent);
	exp.DCPoint_pot.VPointUserInput = ExperimentCalcHelperClass::GetVoltage(&calData,!chargeFirst ? upperVoltage : lowerVoltage);
	exp.DCPoint_pot.VPointVsOCP = false;
	exp.MaxPlays = 1;
	PUSH_NEW_NODE_DATA();

	exp.isHead = false;
	exp.isTail = true;
	exp.nodeType = DCNODE_OCP;
	exp.DCocp.Vmin = 0;
	exp.DCocp.Vmax = 0xffff;
	//exp.DCocp.dVdtMax = 0;
	ExperimentCalcHelperClass::GetSamplingParams_staticDAC(hwVersion.hwModel, &exp, restPeriodInterval);
	exp.tMin = 25e6;
	exp.tMax = restPeriodDuration * 1e8;
	exp.MaxPlays = cycles;
	PUSH_NEW_NODE_DATA();
	

	exp.nodeType = END_EXPERIMENT_NODE;
	PUSH_NEW_NODE_DATA();

	NODES_DATA_END();
}

QStringList ChargeDischargeDC::GetXAxisParameters() const {
	return QStringList() <<
		PLOT_VAR_TIMESTAMP <<
		PLOT_VAR_TIMESTAMP_NORMALIZED <<
		PLOT_VAR_EWE <<
		PLOT_VAR_CURRENT;
}
QStringList ChargeDischargeDC::GetYAxisParameters() const {
	return QStringList() <<
		PLOT_VAR_EWE <<
		PLOT_VAR_CURRENT <<
		PLOT_VAR_ECE <<
		PLOT_VAR_CURRENT_INTEGRAL;
}
void ChargeDischargeDC::PushNewData(const ExperimentalData &expData, DataMap &container, const CalibrationData&, const HardwareVersion &hwVersion) const {
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
void ChargeDischargeDC::SaveDataHeader(QFile &saveFile) const {
	SAVE_DATA_HEADER_START();

	SAVE_DATA_HEADER(PLOT_VAR_TIMESTAMP);
	SAVE_DATA_HEADER(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DATA_HEADER(PLOT_VAR_EWE);
	SAVE_DATA_HEADER(PLOT_VAR_CURRENT);
	SAVE_DATA_HEADER(PLOT_VAR_ECE);
	SAVE_DATA_HEADER(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_HEADER_END();
}

void ChargeDischargeDC::SaveData(QFile &saveFile, const DataMap &container) const {
	SAVE_DATA_START();

	SAVE_DATA(PLOT_VAR_TIMESTAMP);
	SAVE_DATA(PLOT_VAR_TIMESTAMP_NORMALIZED);
	SAVE_DATA(PLOT_VAR_EWE);
	SAVE_DATA(PLOT_VAR_CURRENT);
	SAVE_DATA(PLOT_VAR_ECE);
	SAVE_DATA(PLOT_VAR_CURRENT_INTEGRAL);

	SAVE_DATA_END();
}
