#pragma once

#include "AbstractExperiment.h"
#include "ExperimentReader.h"

class CustomExperimentRunner : public QObject, public AbstractExperiment {
	Q_OBJECT

signals:
	void EditButtonClicked(const CustomExperiment &myId);

private slots:
	void EditButtonSlot();

public:
	CustomExperimentRunner(const CustomExperiment&);

	QString GetShortName() const;
	QString GetFullName() const;
	QString GetDescription() const;
	QStringList GetCategory() const;
	ExperimentTypeList GetTypes() const;
	QPixmap GetImage() const;

	QWidget* CreateUserInput() const;
	NodesData GetNodesData(QWidget*, const CalibrationData&, const HardwareVersion&) const;

	QStringList GetXAxisParameters(ExperimentType) const;
	QStringList GetYAxisParameters(ExperimentType) const;

	void PUSH_NEW_DC_DATA_DEFINITION;
	void SaveDcDataHeader(QFile&, const ExperimentNotes &notes) const;
	void SaveDcData(QFile&, const DataMap&) const;

	void PUSH_NEW_AC_DATA_DEFINITION;
	void SaveAcDataHeader(QFile&, const ExperimentNotes &notes) const;
	void SaveAcData(QFile&, const DataMap&) const;

private:
	CustomExperiment _ce;
};