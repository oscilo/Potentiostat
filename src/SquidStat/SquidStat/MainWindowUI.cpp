#include "MainWindowUI.h"
#include "MainWindow.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

#include "UIHelper.hpp"

#include "Log.h"

#include "ExperimentReader.h"

#include <QButtonGroup>

#include <QIntValidator>
#include <QListView>
#include <QTabWidget>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QScrollArea>

#include <QStandardItemModel>

#include <QPixmap>

#define PREBUILT_EXP_DIR	"./prebuilt/"

QWidget* MainWindowUI::PrebuiltExpCreateGroupHeader(const ExperimentNode_t *node) {
	auto ret = OBJ_NAME(LBL("Unknown node type"), "heading-label");

	switch (node->nodeType) {
		case DCNODE_SWEEP:
			ret->setText("DC Sweep");
			break;
		default:
			break;
	}

	return ret;
}

QWidget* MainWindowUI::PrebuiltExpCreateParamsInput(ExperimentNode_t *node) {
	auto *ret = WDG();
	auto *lay = NO_SPACING(NO_MARGIN(new QGridLayout(ret)));
	SavedInputs savedInputs;
	QWidget *w;

	savedInputs.node = node;

#define INSERT_LED(left_comment, right_comment, default_value, row)		\
	lay->addWidget(OBJ_PROP(OBJ_NAME(LBL(left_comment), "experiment-params-comment"), "comment-placement", "left"),	row, 0);\
	lay->addWidget(w = new QLineEdit(QString("%1").arg(default_value)),														row, 1);\
	lay->addWidget(OBJ_PROP(OBJ_NAME(LBL(right_comment), "experiment-params-comment"), "comment-placement", "right"),				row, 2); \
	savedInputs.input[QString(#default_value)] = w;

	switch (node->nodeType) {
		case DCNODE_SWEEP:
			INSERT_LED("Start Voltage", "V", node->DCSweep.VStart, 0);
			INSERT_LED("End Voltage", "V", node->DCSweep.VEnd, 1);
			INSERT_LED("dV/dt", "", node->DCSweep.dVdt, 2);
			break;
		default:
			break;
	}

	prebuiltExperimentData.inputsList << savedInputs;

	return ret;
}
void MainWindowUI::FillNodeParameters() {
	try {
	#define GET_VALUE_FROM_LED(field, getter) \
		field = qobject_cast<QLineEdit*>(input.input[QString(#field)])->text().getter();
		foreach(SavedInputs input, prebuiltExperimentData.inputsList) {
			ExperimentNode_t *node = input.node;
			QWidget *w;
			switch (node->nodeType) {
			case DCNODE_SWEEP:
				GET_VALUE_FROM_LED(node->DCSweep.VStart, toInt);
				GET_VALUE_FROM_LED(node->DCSweep.VEnd, toInt);
				GET_VALUE_FROM_LED(node->DCSweep.dVdt, toInt);
				break;
			default:
				break;
			}
		}
	}
	catch (...) {
		;
	}
}

MainWindowUI::MainWindowUI(MainWindow *mainWindow) :
	mw(mainWindow)
{
	mw->setObjectName("mainUI");
	mw->ApplyStyle();
}
MainWindowUI::~MainWindowUI() {
}
void MainWindowUI::CreateUI() {
	CreateCentralWidget();
}
void MainWindowUI::CreateCentralWidget() {
	QWidget *centralWidget = OBJ_NAME(WDG(), "central-widget");
	QGridLayout *centralLayout = NO_SPACING(NO_MARGIN(new QGridLayout(centralWidget)));
	mw->setCentralWidget(centralWidget);

	centralLayout->addWidget(GetApplyStyleButton(),		0, 0);
	centralLayout->addWidget(GetMainTabWidget(),		1, 0);
}
QWidget* MainWindowUI::GetApplyStyleButton() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	w = OBJ_NAME(WDG(), "apply-button-owner");
	QHBoxLayout *lay = NO_SPACING(NO_MARGIN(new QHBoxLayout(w)));

	auto *pbt = OBJ_NAME(PBT("Apply stylesheet"), "apply-button");

	CONNECT(pbt, &QPushButton::clicked, mw, &MainWindow::ApplyStyle);

	lay->addWidget(pbt);
	lay->addWidget(OBJ_NAME(PBT("SweepVoltammetry.csv"), "apply-button"));
	lay->addStretch(1);

	return w;
}
QWidget* MainWindowUI::GetMainTabWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	w = WDG();
	auto *lay = NO_SPACING(NO_MARGIN(new QVBoxLayout(w)));
	auto *barWidget = OBJ_NAME(WDG(), "top-tab-bar-owner");
	auto *barLayout = NO_SPACING(NO_MARGIN(new QHBoxLayout(barWidget)));
	auto *widgetsLayout = NO_MARGIN(new QStackedLayout);

	lay->addWidget(barWidget);
	lay->addLayout(widgetsLayout);

	widgetsLayout->addWidget(GetOldSearchHardwareTab());
	widgetsLayout->addWidget(GetRunExperimentTab());

	QButtonGroup *buttonGroup = new QButtonGroup(mw);
	
	auto *pbt = OBJ_PROP(OBJ_NAME(PBT("Search the Hardware"), "bar-button"), "order", "first");
	pbt->setCheckable(true);
	pbt->setChecked(true);
	buttonGroup->addButton(pbt);
	barLayout->addWidget(pbt);

	CONNECT(pbt, &QPushButton::toggled, [=](bool checked) {
		if (!checked) {
			return;
		}
		
		widgetsLayout->setCurrentWidget(GetOldSearchHardwareTab());
	});

	pbt = OBJ_NAME(PBT("Run an Experiment"), "bar-button");
	pbt->setCheckable(true);
	buttonGroup->addButton(pbt);
	barLayout->addWidget(pbt);

	CONNECT(pbt, &QPushButton::toggled, [=](bool checked) {
		if (!checked) {
			return;
		}

		widgetsLayout->setCurrentWidget(GetRunExperimentTab());
	});

	pbt = OBJ_PROP(OBJ_NAME(PBT("New Data Window"), "bar-button"), "order", "last");
	pbt->setCheckable(true);
	buttonGroup->addButton(pbt);
	barLayout->addWidget(pbt);

	barLayout->addStretch(1);

	return w;
}
QWidget* MainWindowUI::GetOldSearchHardwareTab() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	w = WDG();
	QGridLayout *lay = NO_SPACING(NO_MARGIN(new QGridLayout(w)));

	lay->addWidget(GetSearchHardwareWidget(), 1, 0);
	lay->addWidget(GetLogWidget(), 2, 0);
	lay->addWidget(GetPlotWidget(), 1, 1, 2, 1);
	lay->addWidget(GetControlButtonsWidget(), 3, 0, 1, 2);

	return w;
}
QWidget* MainWindowUI::GetRunExperimentTab() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	QLabel *descrIcon;
	QLabel *descrName;
	QLabel *descrText;

	w = WDG();
	QHBoxLayout *lay = NO_SPACING(NO_MARGIN(new QHBoxLayout(w)));

	auto *experimentListOwner = OBJ_PROP(OBJ_NAME(WDG(), "experiment-list-owner"), "widget-type", "left-grey");
	auto *experimentListLay = NO_SPACING(NO_MARGIN(new QVBoxLayout(experimentListOwner)));
	auto *experimentList = OBJ_PROP(OBJ_NAME(new QListView, "experiment-list"), "widget-type", "left-grey");
	experimentList->setEditTriggers(QAbstractItemView::NoEditTriggers);

	experimentListLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Experiments"), "heading-label"), "widget-type", "left-grey"));
	experimentListLay->addWidget(experimentList);

	auto *descriptionHelpLay = NO_SPACING(NO_MARGIN(new QVBoxLayout()));

	auto *descriptionWidget = OBJ_NAME(WDG(), "experiment-description-owner");
	auto *descriptionWidgetLay = NO_SPACING(NO_MARGIN(new QVBoxLayout(descriptionWidget)));

	descriptionWidgetLay->addWidget(descrIcon = OBJ_NAME(LBL(""), "experiment-description-icon"));
	descriptionWidgetLay->addWidget(descrName = OBJ_NAME(LBL(""), "experiment-description-name"));
	descriptionWidgetLay->addWidget(descrText = OBJ_NAME(LBL(""), "experiment-description-text"));
	descriptionWidgetLay->addStretch(1);

	descriptionHelpLay->addWidget(OBJ_NAME(WDG(), "experiment-description-spacing-top"));
	descriptionHelpLay->addWidget(descriptionWidget);
	descriptionHelpLay->addWidget(OBJ_NAME(WDG(), "experiment-description-spacing-bottom"));

	auto *paramsWidget = WDG();
	auto *paramsWidgetLay = NO_SPACING(NO_MARGIN(new QGridLayout(paramsWidget)));

	auto *startExpPbt = OBJ_PROP(OBJ_NAME(PBT("Start Experiment"), "primary-button"), "button-type", "experiment-start-pbt");
	startExpPbt->hide();
	auto *buttonLay = NO_SPACING(NO_MARGIN(new QHBoxLayout()));

	buttonLay->addStretch(1);
	buttonLay->addWidget(startExpPbt);
	buttonLay->addStretch(1);

	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-top"),	0, 0, 1, 3);
	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-bottom"), 4, 0, 1, 3);
	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-left"),	1, 0, 2, 1);
	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-right"),	1, 3, 2, 1);
	paramsWidgetLay->addLayout(buttonLay, 3, 1);
	paramsWidgetLay->setRowStretch(2, 1);

	auto *scrollAreaWidget = WDG();
	QVBoxLayout *paramsLay = NO_SPACING(NO_MARGIN(new QVBoxLayout(scrollAreaWidget)));

	QScrollArea *scrollArea = OBJ_NAME(new QScrollArea(), "experiment-params-scroll-area");
	paramsWidgetLay->addWidget(scrollArea, 2, 1);

	lay->addWidget(experimentListOwner);
	lay->addLayout(descriptionHelpLay);
	lay->addWidget(paramsWidget);


	scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(scrollAreaWidget);
	
	CONNECT(mw, &MainWindow::PrebuiltExperimentsFound, [=](const QList<ExperimentContainer> &expList) {
		QStandardItemModel *model = new QStandardItemModel(4, 1);
		
		int row = 0;

		foreach(const ExperimentContainer &exp, expList) {
			model->setItem(row++, new QStandardItem(exp.shortName));
		}
		
		experimentList->setModel(model);
	});

	CONNECT(experimentList, &QListView::clicked, [=](const QModelIndex &index) {
		mw->PrebuiltExperimentSelected(index.row());
	});

	CONNECT(mw, &MainWindow::PrebuiltExperimentSetDescription, [=](const ExperimentContainer &ec) {
		descrName->setText(ec.name);
		descrText->setText(ec.description);
		descrIcon->setPixmap(QPixmap(PREBUILT_EXP_DIR + ec.imagePath));
		startExpPbt->show();
	});

	CONNECT(mw, &MainWindow::PrebuiltExperimentSetParameters, [=](const QList<ExperimentNode_t*> &nodeList) {
		prebuiltExperimentData.inputsList.clear();
		foreach(QWidget *wdg, prebuiltExperimentData.paramWidgets) {
			paramsLay->removeWidget(wdg);
			wdg->deleteLater();
		}
		prebuiltExperimentData.paramWidgets.clear();

		int row = 0;
		foreach(ExperimentNode_t *node, nodeList) {
			auto header = PrebuiltExpCreateGroupHeader(node);
			auto params = PrebuiltExpCreateParamsInput(node);

			paramsLay->addWidget(header);
			paramsLay->addWidget(params);

			prebuiltExperimentData.paramWidgets << header;
			prebuiltExperimentData.paramWidgets << params;
		}
		auto stretchWdg = WDG();
		paramsLay->addWidget(stretchWdg, 1);
		prebuiltExperimentData.paramWidgets << stretchWdg;
	});

	CONNECT(startExpPbt, &QPushButton::clicked, [=]() {
		FillNodeParameters();

		mw->StartExperiment();
	});

	return w;
}
QWidget* MainWindowUI::GetSearchHardwareWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	QPushButton *searchByVendor;
	QPushButton *searchViaHandshake;
	QPushButton *selectHardware;
	QPushButton *requestCalibration;
	QComboBox *hwList;
	QLineEdit *channelEdit;

	w = WDG();
	QGridLayout *lay = NO_SPACING(NO_MARGIN(new QGridLayout(w)));

	lay->addWidget(OBJ_NAME(LBL("Select hardware"), "heading-label"), 0, 0, 1, 2);
	lay->addWidget(OBJ_NAME(LBL("COM port:"), "regular-comment-label"), 1, 0);
	lay->addWidget(OBJ_NAME(LBL("Channel:"), "regular-comment-label"), 2, 0);
	lay->addWidget(hwList = CMB(), 1, 1);
	lay->addWidget(channelEdit = LED(), 2, 1);
	QListView *hwListComboList = OBJ_NAME(new QListView, "combo-list");
	hwList->setView(hwListComboList);

	QGridLayout *buttonLay = NO_SPACING(NO_MARGIN(new QGridLayout));
	buttonLay->addWidget(searchByVendor = OBJ_NAME(PBT("Search by vendor"), "secondary-button"), 0, 1);
	buttonLay->addWidget(searchViaHandshake = OBJ_NAME(PBT("Search via handshake"), "secondary-button"), 0, 2);
	buttonLay->addWidget(selectHardware = OBJ_NAME(PBT("Select this hardware"), "secondary-button"), 1, 1);
	buttonLay->addWidget(requestCalibration = OBJ_NAME(PBT("Request calibration"), "secondary-button"), 1, 2);
	buttonLay->setColumnStretch(0, 1);
	buttonLay->setColumnStretch(3, 1);

	lay->addLayout(buttonLay, 3, 0, 1, 2);


	CONNECT(searchByVendor, &QPushButton::clicked, mw, &MainWindow::SearchHwVendor);
	CONNECT(searchViaHandshake, &QPushButton::clicked, mw, &MainWindow::SearchHwHandshake);

	CONNECT(mw, &MainWindow::HardwareFound, [=](const InstrumentList &instrumentList) {
		hwList->clear();

		foreach(const InstrumentInfo &info, instrumentList) {
			LOG() << info.portName << ": " << info.serial;
			hwList->addItem(info.portName, QVariant::fromValue(info));
		}
	});

	CONNECT(hwList, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index) {
			if (-1 != index) {
				currentInstrument.instrumentInfo = hwList->itemData(index).value<InstrumentInfo>();
			}
		}
	);

	CONNECT(channelEdit, &QLineEdit::textChanged, [=](const QString &str) {
		currentInstrument.channel = str.toInt();
	});

	CONNECT(selectHardware, &QPushButton::clicked, [=]() {
		mw->SelectHardware(currentInstrument.instrumentInfo, currentInstrument.channel);
	});
	CONNECT(requestCalibration, &QPushButton::clicked, mw, &MainWindow::RequestCalibration);

	channelEdit->setValidator(new QIntValidator(0, MAX_CHANNEL_VALUE));
	channelEdit->setText("0");

	return w;
}
QWidget* MainWindowUI::GetLogWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	QTextEdit *log;

	w = WDG();
	QVBoxLayout *lay = NO_SPACING(NO_MARGIN(new QVBoxLayout(w)));

	lay->addWidget(OBJ_NAME(LBL("Log"), "heading-label"));
	lay->addWidget(log = TED());

	log->setReadOnly(true);
	
	SetLogSignalEmitterParent(mw);
	CONNECT(GetLogSignalEmitter(), &LogSignalEmitter::SendLog, log, &QTextEdit::append);
	
	return w;
}
QWidget* MainWindowUI::GetPlotWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	w = WDG();
	QVBoxLayout *lay = NO_MARGIN(NO_SPACING(new QVBoxLayout(w)));

	QwtPlot *plot = new QwtPlot();
	//plot->setAxisScale(QwtPlot::xBottom, 0, 100000);
	plot->setAxisScale(QwtPlot::yLeft, 0, 1050);
	QwtText title;
	title.setFont(QFont("Segoe UI", 14));
	//title.setText("Frequency (Hz)");
	title.setText("Timestamp (ms)");
	plot->setAxisTitle(QwtPlot::xBottom, title);
	//title.setText(QString("Impedance (`") + QChar(0x03a9) + QString(")"));
	title.setText("Current (ewe)");
	plot->setAxisTitle(QwtPlot::yLeft, title);

	plot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);

	lay->addWidget(plot);

	QwtPlotCurve *curve = new QwtPlotCurve("Impedance 'Filename.csv'");
	curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
	curve->setPen(QColor(42, 127, 220), 1);
	curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	curve->attach(plot);

	CONNECT(mw, &MainWindow::DataArrived, [=](quint8 channel, const ExperimentalData &expData) {
		static QVector<qreal> xPlotData, yPlotData;
		qreal x = expData.timestamp / 100000UL;
		qreal y = expData.adcData.ewe;
		xPlotData.append(x);
		yPlotData.append(y);

		curve->setSamples(xPlotData, yPlotData);
		plot->replot();
	});

	return w;
}
QWidget* MainWindowUI::GetControlButtonsWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	QPushButton *startExperiment;
	QPushButton *stopExperiment;

	w = WDG();
	QHBoxLayout *buttonLay = NO_SPACING(NO_MARGIN(new QHBoxLayout(w)));
	buttonLay->addWidget(startExperiment = OBJ_NAME(PBT("Start Experiment"), "control-button-blue"));
	buttonLay->addWidget(stopExperiment = OBJ_NAME(PBT("Stop Experiment"), "control-button-red"));

	//CONNECT(startExperiment, &QPushButton::clicked, mw, &MainWindow::StartExperiment);
	//CONNECT(stopExperiment, &QPushButton::clicked, mw, &MainWindow::StopExperiment);

	return w;
}