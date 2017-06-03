#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "InstrumentStructures.h"
#include "ExternalStructures.h"

#include <Experiment.h>

#include <QList>
#include <QUuid>

class MainWindowUI;
class InstrumentOperator;
class ExperimentContainer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void LoadFonts();
    void ApplyStyle();
	
	void LoadPrebuildExperiments();
	//void PrebuiltExperimentSelected(int);
	void PrebuiltExperimentSelected(const Experiment*);

	void SearchHwVendor();
	void SearchHwHandshake();

	void SelectHardware(const InstrumentInfo&, quint8 channel);
	void RequestCalibration();

	void StartExperiment(QWidget*);
	void StopExperiment(const QUuid&);

	void SaveData(const QVector<qreal> &xData, const QVector<qreal> &yData, const QString &fileName);

signals:
	void HardwareFound(const InstrumentList&);
	void DataArrived(const QUuid&, quint8 channel, const ExperimentalData &expData);

	//void PrebuiltExperimentsFound(const QList<ExperimentContainer>&);
	void PrebuiltExperimentsFound(const QList<Experiment*>&);
	//void PrebuiltExperimentSetDescription(const ExperimentContainer&);
	//void PrebuiltExperimentSetParameters(const QList<ExperimentNode_t*>&);

	void CreateNewDataWindow(const QUuid&, const QString&);

private:
	void CleanupCurrentHardware();
	void FillHardware(const InstrumentList &);

	MainWindowUI *ui;

	//InstrumentOperator *instrumentOperator;
	

	struct InstrumentHandler {
		InstrumentInfo info;
		InstrumentOperator *oper;
		struct {
			bool busy;
			QUuid id;
			quint8 channel;
		} experiment;
		QList<QMetaObject::Connection> connections;
	};

	struct {
		QList<InstrumentHandler> handlers;
		struct {
			QList<InstrumentHandler>::iterator handler;
			quint8 channel;
		} currentInstrument;
	} hardware;

	struct {
		//QList<ExperimentContainer> ecList;
		//int selectedEcIndex;
		const Experiment* selectedExp;
		QList<Experiment*> expList;
	} prebuiltExperiments;

	QList<InstrumentHandler>::iterator SearchForHandler(InstrumentOperator*);
};

#endif // MAINWINDOW_H
