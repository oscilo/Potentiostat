#include "MainWindowUI.h"
#include "MainWindow.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_scale_widget.h>
#include <qwt_text_label.h>

#include "UIHelper.hpp"

#include "Log.h"

#include "ListSeriesData.h"

#include <QButtonGroup>

#include <QEvent>
#include <QKeyEvent>

#include <QIntValidator>
#include <QListView>
#include <QTabWidget>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QCheckBox>

#include <QStandardItemModel>

#include <QPixmap>
#include <QLine>

#include <QTime>
#include <QDateTime>
#include <QFileDialog>
#include <QStringList>
#include <QSettings>
#include <QColorDialog>
#include <QMessageBox>

#include <QXmlStreamReader>

#include <QEventLoop>

#include <qtcsv/reader.h>

#include <QDrag>
#include <QMimeData>

#define ELEMENT_MIME_TYPE	"image/element"

#define EXPERIMENT_VIEW_ALL_CATEGORY	"View All"
#define NONE_Y_AXIS_VARIABLE			"None"

#define DEFAULT_MAJOR_CURVE_COLOR		QColor(42, 127, 220)
#define DEFAULT_MINOR_CURVE_COLOR		QColor(208, 35, 39)

#define CURVE_PARAMS_PRI_PEN_COLOR		"curve-params-pri-pen-color"
#define CURVE_PARAMS_PRI_PEN_WIDTH		"curve-params-pri-pen-width"
#define CURVE_PARAMS_PRI_PEN_STYLE		"curve-params-pri-pen-style"
#define CURVE_PARAMS_PRI_STYLE			"curve-params-pri-style"
#define CURVE_PARAMS_PRI_SYMBOL_WIDTH	"curve-params-pri-symbol-width"
#define CURVE_PARAMS_PRI_SYMBOL_STYLE	"curve-params-pri-symbol-style"

#define CURVE_PARAMS_SEC_PEN_COLOR		"curve-params-sec-pen-color"
#define CURVE_PARAMS_SEC_PEN_WIDTH		"curve-params-sec-pen-width"
#define CURVE_PARAMS_SEC_PEN_STYLE		"curve-params-sec-pen-style"
#define CURVE_PARAMS_SEC_STYLE			"curve-params-sec-style"
#define CURVE_PARAMS_SEC_SYMBOL_WIDTH	"curve-params-sec-symbol-width"
#define CURVE_PARAMS_SEC_SYMBOL_STYLE	"curve-params-sec-symbol-style"

#define PAUSE_EXP_BUTTON_TEXT		"Pause Experiment"
#define RESUME_EXP_BUTTON_TEXT		"Resume Experiment"

MainWindowUI::MainWindowUI(MainWindow *mainWindow) :
	mw(mainWindow)
{
	prebuiltExperimentData.userInputs = 0;

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
	widgetsLayout->addWidget(GetBuildExperimentTab());
	widgetsLayout->addWidget(GetNewDataWindowTab());

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

	pbt = OBJ_NAME(PBT("Build an Experiment"), "bar-button");
	pbt->setCheckable(true);
	buttonGroup->addButton(pbt);
	barLayout->addWidget(pbt);

	CONNECT(pbt, &QPushButton::toggled, [=](bool checked) {
		if (!checked) {
			return;
		}

		widgetsLayout->setCurrentWidget(GetBuildExperimentTab());
	});

	pbt = OBJ_PROP(OBJ_NAME(PBT("New Data Window"), "bar-button"), "order", "last");
	pbt->setCheckable(true);
	buttonGroup->addButton(pbt);
	barLayout->addWidget(pbt);
	
	ui.newDataTab.newDataTabButton = pbt;
	
	CONNECT(pbt, &QPushButton::toggled, [=](bool checked) {
		if (!checked) {
			return;
		}

		widgetsLayout->setCurrentWidget(GetNewDataWindowTab());
	});

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
	lay->setColumnStretch(0, 1);
	lay->setColumnStretch(1, 1);

	return w;
}
QWidget* MainWindowUI::GetSearchHardwareWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	//QPushButton *searchByVendor;
	//QPushButton *searchViaHandshake;
	//QPushButton *selectHardware;
	//QPushButton *requestCalibration;
	//QComboBox *hwList;
	//QLineEdit *channelEdit;

	w = WDG();
	QGridLayout *lay = NO_SPACING(NO_MARGIN(new QGridLayout(w)));

	//lay->addWidget(OBJ_NAME(LBL("Select hardware"), "heading-label"), 0, 0, 1, 2);
	//lay->addWidget(OBJ_NAME(LBL("COM port:"), "regular-comment-label"), 1, 0);
	//lay->addWidget(OBJ_NAME(LBL("Channel:"), "regular-comment-label"), 2, 0);
	//lay->addWidget(hwList = CMB(), 1, 1);
	//lay->addWidget(channelEdit = LED(), 2, 1);
	//QListView *hwListComboList = OBJ_NAME(new QListView, "combo-list");
	//hwList->setView(hwListComboList);

	QGridLayout *buttonLay = NO_SPACING(NO_MARGIN(new QGridLayout));
	/*
	buttonLay->addWidget(searchByVendor = OBJ_NAME(PBT("Search by vendor"), "secondary-button"), 0, 1);
	buttonLay->addWidget(searchViaHandshake = OBJ_NAME(PBT("Search via handshake"), "secondary-button"), 0, 2);
	//*/
	//buttonLay->addWidget(selectHardware = OBJ_NAME(PBT("Select this hardware"), "secondary-button"), 1, 1);
	//buttonLay->addWidget(requestCalibration = OBJ_NAME(PBT("Request calibration"), "secondary-button"), 1, 2);
	buttonLay->setColumnStretch(0, 1);
	buttonLay->setColumnStretch(3, 1);

	lay->addLayout(buttonLay, 3, 0, 1, 2);


	//CONNECT(searchByVendor, &QPushButton::clicked, mw, &MainWindow::SearchHwVendor);
	//CONNECT(searchViaHandshake, &QPushButton::clicked, mw, &MainWindow::SearchHwHandshake);

	/*
	CONNECT(mw, &MainWindow::AddNewInstruments, [=](const QStringList &newLines) {
		hwList->addItems(newLines);
	});
	CONNECT(mw, &MainWindow::RemoveDisconnectedInstruments, [=](const QStringList &linesToDelete) {
		for (int i = 0; i < hwList->count();) {
			if (linesToDelete.contains(hwList->itemText(i))) {
				hwList->removeItem(i);
			}
			else {
				++i;
			}
		}
	});
	//*/

	/*
	CONNECT(mw, &MainWindow::HardwareFound, [=](const InstrumentList &instrumentList) {
		hwList->clear();

		foreach(const InstrumentInfo &info, instrumentList) {
			LOG() << info.serialPortName << ": " << info.serial;
			hwList->addItem(info.serialPortName, QVariant::fromValue(info));
		}
	});
	//*/

	/*
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
	//*/

	/*
	CONNECT(selectHardware, &QPushButton::clicked, [=]() {
		mw->SelectHardware(hwList->currentText(), channelEdit->text().toInt());
	});
	//*/

	//channelEdit->setValidator(new QIntValidator(0, MAX_CHANNEL_VALUE));
	//channelEdit->setText("0");

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
	/*
	QVBoxLayout *lay = NO_MARGIN(NO_SPACING(new QVBoxLayout(w)));

	QwtPlot *plot = new QwtPlot();
	//plot->setAxisScale(QwtPlot::xBottom, 0, 100000);
	plot->setAxisScale(QwtPlot::yLeft, 0, 1050);
	//plot->setAxisAutoScale(QwtPlot::yLeft, true);
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
	//*/
	/*
	CONNECT(mw, &MainWindow::DataArrived, [=](quint8 channel, const ExperimentalData &expData) {
		static QVector<qreal> xPlotData, yPlotData;
		qreal x = expData.timestamp / 100000UL;
		qreal y = expData.adcData.ewe;
		xPlotData.append(x);
		yPlotData.append(y);

		curve->setSamples(xPlotData, yPlotData);
		plot->replot();
	});
	//*/
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
	w->setMinimumHeight(50);
	QHBoxLayout *buttonLay = NO_SPACING(NO_MARGIN(new QHBoxLayout(w)));
	//buttonLay->addWidget(startExperiment = OBJ_NAME(PBT("Start Experiment"), "control-button-blue"));
	//buttonLay->addWidget(stopExperiment = OBJ_NAME(PBT("Stop Experiment"), "control-button-red"));

	//CONNECT(startExperiment, &QPushButton::clicked, mw, &MainWindow::StartExperiment);
	//CONNECT(stopExperiment, &QPushButton::clicked, mw, &MainWindow::StopExperiment);

	return w;
}
QWidget* MainWindowUI::CreateBuildExpContainerWidget(const MainWindowUI::BuilderContainer &bc) {
	if (bc.type != BuilderContainer::SET) {
		LOG() << "It is not a SET!";
		return 0;
	}

	auto containerWdg = OBJ_NAME(new QFrame(mw), "node-builder-container");
	auto containerWdgLay = NO_MARGIN(NO_SPACING(new QVBoxLayout(containerWdg)));

	for (auto it = bc.elements.begin(); it != bc.elements.end(); ++it) {
		if (!it->w) {
			return 0;
		}

		containerWdgLay->addWidget(it->w);
	}
	QSpinBox *mult;
	containerWdgLay->addWidget(mult = OBJ_NAME(new QSpinBox(), "node-builder-multiplier-container"));

	mult->setMinimum(1);
	mult->setMaximum(99999);
	mult->setValue(bc.repetition);

	return containerWdg;
}
QWidget* MainWindowUI::CreateBuildExpElementWidget(const MainWindowUI::BuilderContainer &bc) {
	if (bc.type != BuilderContainer::ELEMENT) {
		LOG() << "It is not an ELEMENT!";
		return 0;
	}

	auto *w = OBJ_NAME(new QFrame(), "node-builder-owner");

	auto lay = NO_SPACING(NO_MARGIN(new QVBoxLayout(w)));
	
	QSpinBox *mult;
	QLabel *label;

	lay->addWidget(label = OBJ_NAME(new QLabel, "node-builder-label"));
	lay->addWidget(mult = OBJ_NAME(new QSpinBox(), "node-builder-multiplier-single"));

	label->setPixmap(QPixmap(":/GUI/Resources/node-pic-example.png"));

	mult->setMinimum(1);
	mult->setMaximum(99999);
	mult->setValue(bc.repetition);

	return w;
}
#include <functional>
#include <QPainter>
enum LineDirection : quint8 {
	LD_NONE = 0,
	LD_LEFT = 1,
	LD_RIGHT = 2,
	LD_TOP = 4,
	LD_BOTTOM = 8
};
class ElementEventFiler : public QObject {
public:
	ElementEventFiler(QObject *parent, quint8 ld, std::function<void(QWidget*, quint8, quint8)> lineDrawer) :
		QObject(parent), _lineDrawer(lineDrawer), _lineDirection(ld), _dropDirection(LD_NONE) {}

	bool eventFilter(QObject *obj, QEvent *e) {
		switch(e->type()) {
			case QEvent::Paint: {
				QWidget *w = qobject_cast<QWidget*>(obj);
				if (w) {
					_lineDrawer(w, _lineDirection, _dropDirection);
					return true;
				}
			} break;

			case QEvent::DragEnter: {
				auto de = (QDragEnterEvent*)e;
				if (de->mimeData()->hasFormat(ELEMENT_MIME_TYPE)) {
					de->accept();
				}
				else {
					de->ignore();
				}
				return true;
			} break;
			
			case QEvent::DragMove: {
				auto dm = (QDragMoveEvent*)e;
				auto w = qobject_cast<QWidget*>(obj);

				if (!w) {
					dm->ignore();
					return false;
				}

				if (dm->mimeData()->hasFormat(ELEMENT_MIME_TYPE)) {
					_dropDirection = FindDirection(w, dm->pos());

					if (0 == (_lineDirection & _dropDirection)) {
						dm->ignore();
					}
					else {
						dm->setDropAction(Qt::MoveAction);
						dm->accept();
					}

					w->update();
				}
				else {
					dm->ignore();
				}
				return true;
			} break;

			case QEvent::DragLeave: {
				_dropDirection = LD_NONE;
				auto w = qobject_cast<QWidget*>(obj);

				if (w) {
					w->update();
				}

			} break;
		}
		return false;
	}

private:
	quint8 FindDirection(QWidget *w, const QPoint &pos) {
		if (!w) {
			return LD_NONE;
		}

		auto m = w->contentsMargins();
		auto r = w->rect();
		
		Qt::Alignment align;

		if (pos.x() < m.left()) {
			align |= Qt::AlignLeft;
		}
		else if (pos.x() < (r.width() - m.right())) {
			align |= Qt::AlignHCenter;
		}
		else {
			align |= Qt::AlignRight;
		}

		if (pos.y() < m.top()) {
			align |= Qt::AlignTop;
		}
		else if (pos.y() < (r.height() - m.bottom())) {
			align |= Qt::AlignVCenter;
		}
		else {
			align |= Qt::AlignBottom;
		}

		if (align.testFlag(Qt::AlignTop) && align.testFlag(Qt::AlignHCenter)) {
			return LD_TOP;
		}
		if (align.testFlag(Qt::AlignBottom) && align.testFlag(Qt::AlignHCenter)) {
			return LD_BOTTOM;
		}
		if (align.testFlag(Qt::AlignLeft) && align.testFlag(Qt::AlignVCenter)) {
			return LD_LEFT;
		}
		if (align.testFlag(Qt::AlignRight) && align.testFlag(Qt::AlignVCenter)) {
			return LD_RIGHT;
		}

		return LD_NONE;
	}

	std::function<void(QWidget*, quint8, quint8)> _lineDrawer;
	quint8 _lineDirection;
	quint8 _dropDirection;
};
QLine GetTopLine(const QRect &rect, const QMargins &margins) {
	QPoint startPoint;
	startPoint.setX(rect.width() / 2);
	startPoint.setY(-margins.top());

	QPoint endPoint;
	endPoint.setX(rect.width() / 2);
	endPoint.setY(margins.top());

	return QLine(startPoint, endPoint);
}
QLine GetBottomLine(const QRect &rect, const QMargins &margins) {
	QPoint startPoint;
	startPoint.setX(rect.width() / 2);
	startPoint.setY(rect.height() - margins.bottom());

	QPoint endPoint;
	endPoint.setX(rect.width() / 2);
	endPoint.setY(rect.height() + margins.bottom());

	return QLine(startPoint, endPoint);
}
QLine GetLeftLine(const QRect &rect, const QMargins &margins) {
	QPoint startPoint;
	startPoint.setX(-margins.left());
	startPoint.setY(rect.height() / 2);

	QPoint endPoint;
	endPoint.setX(margins.left());
	endPoint.setY(rect.height() / 2);

	return QLine(startPoint, endPoint);
}
QLine GetRightLine(const QRect &rect, const QMargins &margins) {
	QPoint startPoint;
	startPoint.setX(rect.width() - margins.right());
	startPoint.setY(rect.height() / 2);

	QPoint endPoint;
	endPoint.setX(rect.width() + margins.right());
	endPoint.setY(rect.height() / 2);

	return QLine(startPoint, endPoint);
}
auto LineDrawer = [](QWidget *w, quint8 lineDirection, quint8 dropDirection) {
	QRect rect = w->geometry();
	QMargins margins = w->contentsMargins();

	QPainter painter(w);
	painter.setPen(QPen(QColor("#80939a"), 2));
	if (lineDirection & LD_LEFT)
		painter.drawLine(GetLeftLine(rect, margins));
	if (lineDirection & LD_RIGHT)
		painter.drawLine(GetRightLine(rect, margins));
	if (lineDirection & LD_TOP)
		painter.drawLine(GetTopLine(rect, margins));
	if (lineDirection & LD_BOTTOM)
		painter.drawLine(GetBottomLine(rect, margins));

	if (0 != (lineDirection & dropDirection)) {
		painter.setPen(QPen(QColor("#7b1a1e"), 2));
		if (dropDirection & LD_LEFT)
			painter.drawLine(QLine(0, 0, 0, rect.height()));
		if (dropDirection & LD_RIGHT)
			painter.drawLine(QLine(rect.width(), 0, rect.width(), rect.height()));
		if (dropDirection & LD_TOP)
			painter.drawLine(QLine(0, 0, rect.width(), 0));
		if (dropDirection & LD_BOTTOM)
			painter.drawLine(QLine(0, rect.height(), rect.width(), rect.height()));
	}
};
QWidget* MainWindowUI::CreateBuildExpHolderWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	builder.container.repetition = 20;
	builder.container.elements
		<< BuilderContainer(1)
		<< BuilderContainer(2)
		<< BuilderContainer(10, BuilderContainer::SET)
		<< BuilderContainer(15, BuilderContainer::SET)
		<< BuilderContainer(8)
		<< BuilderContainer(9)
		<< BuilderContainer(10);

	builder.container.elements[2].elements
		<< BuilderContainer(3)
		<< BuilderContainer(4)
		<< BuilderContainer(5);

	builder.container.elements[3].elements
		<< BuilderContainer(6)
		<< BuilderContainer(7);

	for (auto it = builder.container.elements.begin(); it != builder.container.elements.end(); ++it) {
		quint8 ld = LD_NONE;
		if (it != builder.container.elements.begin()) {
			ld |= LD_LEFT;
		}
		if (it != (--builder.container.elements.end())) {
			ld |= LD_RIGHT;
		}

		switch (it->type) {
			case BuilderContainer::ELEMENT:
				it->w = CreateBuildExpElementWidget(*it);
				it->w->setAcceptDrops(true);
				it->w->installEventFilter(new ElementEventFiler(it->w, ld, LineDrawer));
				break;

			case BuilderContainer::SET:
				for (auto cIt = it->elements.begin(); cIt != it->elements.end(); ++cIt) {
					quint8 vld = LD_NONE;
					if (cIt == it->elements.begin()) {
						vld = ld;
					}
					if (cIt != it->elements.begin()) {
						vld |= LD_TOP;
					}
					if (cIt != (--it->elements.end())) {
						vld |= LD_BOTTOM;
					}

					cIt->w = CreateBuildExpElementWidget(*cIt);
					cIt->w->setAcceptDrops(true);
					cIt->w->installEventFilter(new ElementEventFiler(cIt->w, vld, LineDrawer));
				}
				it->w = CreateBuildExpContainerWidget(*it);
				break;
		}
	}

	w = WDG();
	auto lay = NO_SPACING(NO_MARGIN(new QVBoxLayout(w)));

	QScrollArea *buildExpHolder;
	QSpinBox *mult;
	lay->addWidget(buildExpHolder = OBJ_NAME(new QScrollArea, "exp-builder-scroll-area"));
	lay->addWidget(mult = OBJ_NAME(new QSpinBox(), "exp-builder-global-multiplier"));

	mult->setMinimum(1);
	mult->setMaximum(99999);
	mult->setValue(builder.container.repetition);

	auto buildExpHolderOwner = OBJ_NAME(new QFrame(), "build-exp-holder");
	auto buildExpHolderOwnerLay = NO_SPACING(NO_MARGIN(new QGridLayout(buildExpHolderOwner)));

	buildExpHolder->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	buildExpHolder->setWidgetResizable(true);
	buildExpHolder->setWidget(buildExpHolderOwner);

	int column = 0;
	int maxRowSpan = 0;
	foreach(auto &elem, builder.container.elements) {
		int rowSpan = (elem.type == BuilderContainer::ELEMENT) ? 1 : (elem.elements.size() + 1);
		if (rowSpan > maxRowSpan) {
			maxRowSpan = rowSpan;
		}

		buildExpHolderOwnerLay->addWidget(elem.w, 0, column++, rowSpan, 1);
	}
	buildExpHolderOwnerLay->setRowStretch(maxRowSpan, 1);
	buildExpHolderOwnerLay->setColumnStretch(column, 1);
	
	return w;
}
class ElementListEventFiler : public QObject {
public:
	ElementListEventFiler(QObject *parent) : QObject(parent) {}

	bool eventFilter(QObject *obj, QEvent *e) {
		if (e->type() == QEvent::MouseButtonPress) {
			QWidget *w = qobject_cast<QWidget*>(obj);
			if (!w) {
				return false;
			}

			QMouseEvent *me = (QMouseEvent*)e;

			if (me->button() == Qt::LeftButton) {
				auto margins = w->contentsMargins();
				auto rect = w->rect();

				QPoint startPoint;
				startPoint.setX(margins.left() - 1);
				startPoint.setY(margins.top() - 1);
				QPoint endPoint;
				endPoint.setX(rect.width() - margins.right());
				endPoint.setY(rect.height() - margins.bottom());

				auto mime = new QMimeData;
				mime->setData(ELEMENT_MIME_TYPE, QByteArray());

				QDrag *drag = new QDrag(obj);
				drag->setMimeData(mime);
				drag->setPixmap(w->grab(QRect(startPoint, endPoint)));
				drag->setHotSpot(me->pos() - QPoint(margins.left(), margins.top()));

				Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
			}

			return false;
		}
		return false;
	}
};
QWidget* MainWindowUI::CreateElementsListWidget() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}
	w = OBJ_PROP(OBJ_NAME(WDG(), "node-list-owner"), "widget-type", "left-grey");
	auto nodeListOwnerLay = NO_SPACING(NO_MARGIN(new QVBoxLayout(w)));

	auto elementsListHolder = OBJ_NAME(new QFrame(), "elements-list-holder");
	auto elementsListHolderLay = new QGridLayout(elementsListHolder);
	elementsListHolderLay->addWidget(OBJ_NAME(LBL("Drag and drop elements on right window"), "elements-list-descr-label"), 0, 0, 1, 2);
	int i = 0;
	for (; i < 9; ++i) {
		auto label = OBJ_NAME(new QLabel, "element-builder");
		label->setPixmap(QPixmap(":/GUI/Resources/node-pic-example.png"));

		label->installEventFilter(new ElementListEventFiler(label));

		elementsListHolderLay->addWidget(label, 1 + i / 2, i % 2);
	}
	elementsListHolderLay->setRowStretch(1 + i / 2 + i % 2, 1);

	QScrollArea *elementsListArea = OBJ_NAME(new QScrollArea(), "node-list-scroll-area");
	elementsListArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	elementsListArea->setWidgetResizable(true);
	elementsListArea->setWidget(elementsListHolder);

	nodeListOwnerLay->addWidget(elementsListArea);

	return w;
}
QWidget* MainWindowUI::GetBuildExperimentTab() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	w = WDG();
	QHBoxLayout *lay = NO_SPACING(NO_MARGIN(new QHBoxLayout(w)));

	auto nodeListOwner = CreateElementsListWidget();
	
	auto *expBuilderOwner = OBJ_NAME(WDG(), "experiment-builder-owner");
	auto expBuilderOwnerLay = NO_SPACING(NO_MARGIN(new QGridLayout(expBuilderOwner)));

	expBuilderOwnerLay->addWidget(OBJ_NAME(WDG(), "exp-builder-top-buttons-replacement"), 0, 0, 1, 3);
	expBuilderOwnerLay->addWidget(OBJ_NAME(WDG(), "exp-builder-right-buttons-replacement"), 1, 2, 2, 1);
	expBuilderOwnerLay->addWidget(OBJ_NAME(WDG(), "exp-builder-left-spacer"), 1, 0, 2, 1);
	expBuilderOwnerLay->addWidget(OBJ_NAME(WDG(), "exp-builder-bottom-spacer"), 3, 0, 1, 3);
	expBuilderOwnerLay->addWidget(CreateBuildExpHolderWidget(), 1, 1);

	auto *nodeParamsOwner = OBJ_NAME(WDG(), "node-params-owner");

	lay->addWidget(nodeListOwner);
	lay->addWidget(expBuilderOwner);
	lay->addWidget(nodeParamsOwner);

	return w;
}
bool MainWindowUI::GetExperimentNotes(QWidget *parent, MainWindowUI::ExperimentNotes &ret) {
	static bool dialogCanceled;
	dialogCanceled = true;

	static QMap<QString, qreal> references;
	references["Predefined 1"] = 1.0;
	references["Predefined 2"] = 1.1;
	references["Predefined 3"] = 1.2;

	QDialog* dialog = OBJ_NAME(new QDialog(parent, Qt::SplashScreen), "notes-dialog");

	auto electrodeCombo = CMB();
	QRadioButton *commRefRadio;
	QRadioButton *otherRefRadio;
	QLineEdit *otherRefLed;
	QLineEdit *potVsSheLed;
	QTextEdit *notesTed;

	QVBoxLayout *dialogLay = NO_SPACING(NO_MARGIN(new QVBoxLayout(dialog)));

	auto scrolledWidget = WDG();
	auto scrollArea = OBJ_NAME(new QScrollArea, "experimental-notes-scroll-area");
	scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(scrolledWidget);

	auto lay = new QGridLayout(scrolledWidget);

	lay->addWidget(OBJ_NAME(LBL("Experimental Notes"), "heading-label"), 0, 0, 1, 2);
	lay->addWidget(notesTed = OBJ_NAME(TED(), "experimental-notes-text-edit"), 1, 0, 1, 2);

	lay->addWidget(OBJ_NAME(LBL("Reference Electrode"), "heading-label"), 2, 0, 1, 2);
	lay->addWidget(commRefRadio = new QRadioButton("Common reference electrode"), 3, 0);
	lay->addWidget(otherRefRadio = new QRadioButton("Other reference electrode"), 4, 0);
	lay->addWidget(electrodeCombo, 3, 1);
	lay->addWidget(otherRefLed = LED(), 4, 1);
	lay->addWidget(OBJ_NAME(LBL("Potential vs SHE (V)"), "notes-dialog-right-comment"), 5, 0);
	lay->addWidget(potVsSheLed = LED(), 5, 1);
	lay->addWidget(OBJ_NAME(WDG(), "notes-dialog-right-spacing"), 0, 2, 6, 1);
	lay->addWidget(OBJ_NAME(LBL("Other parameters"), "heading-label"), 6, 0, 1, 2);
	int row = 7;
	lay->addWidget(OBJ_NAME(LBL("Working electrode"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Working electrode area (cm<sup>2</sup>)"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Counter electrode"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Counter electrode area (cm<sup>2</sup>)"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Solvent"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Electrolyte"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Electrolyte concentration (moles per liter)"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);
	lay->addWidget(OBJ_NAME(LBL("Atmosphere"), "notes-dialog-right-comment"), row, 0);
	lay->addWidget(LED(), row++, 1);

	QPushButton *okBut;
	QPushButton *cancelBut;

	auto buttonLay = new QHBoxLayout;
	buttonLay->addStretch(1);
	buttonLay->addWidget(okBut = OBJ_NAME(PBT("OK"), "secondary-button"));
	buttonLay->addWidget(cancelBut = OBJ_NAME(PBT("Cancel"), "secondary-button"));
	buttonLay->addStretch(1);

	dialogLay->addWidget(scrollArea);
	dialogLay->addWidget(OBJ_NAME(WDG(), "notes-dialog-bottom-spacing"));// , 6, 0, 1, -1);
	dialogLay->addLayout(buttonLay);// , 7, 0, 1, -1);
	dialogLay->addWidget(OBJ_NAME(WDG(), "notes-dialog-bottom-spacing"));// , 8, 0, 1, -1);

	QListView *electrodeComboList = OBJ_NAME(new QListView, "combo-list");
	electrodeCombo->setView(electrodeComboList);

	QList<QMetaObject::Connection> dialogConn;
	
	#define COMMON_REFERENCE_ELECTRODE_NAME	"common-reference-electrode-name"
	#define OTHER_REFERENCE_ELECTRODE_NAME	"other-reference-electrode-name"
	#define OTHER_REFERENCE_ELECTRODE_VALUE	"other-reference-electrode-value"

	dialogConn << CONNECT(commRefRadio, &QRadioButton::clicked, [=]() {
		QSettings settings(SQUID_STAT_PARAMETERS_INI, QSettings::IniFormat);
		settings.setValue(OTHER_REFERENCE_ELECTRODE_NAME, otherRefLed->text());
		settings.setValue(OTHER_REFERENCE_ELECTRODE_VALUE, potVsSheLed->text());
		
		otherRefLed->setDisabled(true);
		otherRefLed->setPlaceholderText("");
		otherRefLed->setText("");

		potVsSheLed->setPlaceholderText("");
		potVsSheLed->setReadOnly(true);
		potVsSheLed->setText("");

		electrodeCombo->addItems(references.keys());
		electrodeCombo->setEnabled(true);

		QString currentText = settings.value(COMMON_REFERENCE_ELECTRODE_NAME, "").toString();
		if (!currentText.isEmpty()) {
			electrodeCombo->setCurrentText(currentText);
		}
	});

	dialogConn << CONNECT(otherRefRadio, &QRadioButton::clicked, [=]() {
		QSettings settings(SQUID_STAT_PARAMETERS_INI, QSettings::IniFormat);
		settings.setValue(COMMON_REFERENCE_ELECTRODE_NAME, electrodeCombo->currentText());

		electrodeCombo->setDisabled(true);
		electrodeCombo->clear();

		otherRefLed->setEnabled(true);
		otherRefLed->setPlaceholderText("Type here electrode name");
		otherRefLed->setText(settings.value(OTHER_REFERENCE_ELECTRODE_NAME, "").toString());

		potVsSheLed->setPlaceholderText("Type here the value");
		potVsSheLed->setReadOnly(false);
		potVsSheLed->setText(settings.value(OTHER_REFERENCE_ELECTRODE_VALUE, "").toString());
	});

	dialogConn << CONNECT(electrodeCombo, &QComboBox::currentTextChanged, [=](const QString &key) {
		if (key.isEmpty()) {
			return;
		}
		QString text = QString("%1").arg(references[key]).replace(QChar('.'), QLocale().decimalPoint());
		potVsSheLed->setText(text);
	});

	dialogConn << CONNECT(okBut, &QPushButton::clicked, [=]() {
		dialogCanceled = false;
	});

	CONNECT(okBut, &QPushButton::clicked, dialog, &QDialog::accept);
	CONNECT(cancelBut, &QPushButton::clicked, dialog, &QDialog::reject);

	commRefRadio->click();

	dialog->exec();

	foreach(auto conn, dialogConn) {
		QObject::disconnect(conn);
	}

	ret.notes = notesTed->toPlainText();
	ret.refElectrode = commRefRadio->isChecked() ? electrodeCombo->currentText() : otherRefLed->text();
	ret.potential = potVsSheLed->text();

	dialog->deleteLater();

	return !dialogCanceled;
}
class ExperimentFilterModel : public QSortFilterProxyModel {
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
		QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

		if (!index.isValid()) {
			return false;
		}

		auto exp = index.data(Qt::UserRole).value<const AbstractExperiment*>();
		QString pattern = filterRegExp().pattern();

		QString descriptionPlain = "";
		QXmlStreamReader xml("<i>" + exp->GetDescription() + "</i>");
		while (!xml.atEnd()) {
			if (xml.readNext() == QXmlStreamReader::Characters) {
				descriptionPlain += xml.text();
			}
		}

		bool validCategory = false;
		if (_category == EXPERIMENT_VIEW_ALL_CATEGORY) {
			validCategory = true;
		}
		else {
			validCategory = exp->GetCategory().contains(_category);
		}

		return (exp->GetShortName().contains(pattern, filterCaseSensitivity()) ||
			exp->GetFullName().contains(pattern, filterCaseSensitivity()) ||
			descriptionPlain.contains(pattern, filterCaseSensitivity())) && validCategory;
	}

public:
	void SetCurrentCategory(const QString &category) {
		_category = category;
		invalidateFilter();
	}

private:
	QString _category;
};
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
	auto proxyModel = new ExperimentFilterModel;
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	proxyModel->setSourceModel(new QStandardItemModel(0, 0));
	experimentList->setModel(proxyModel);

	auto searchLay = NO_SPACING(NO_MARGIN(new QHBoxLayout));
	QLabel *searchLabel;
	QPushButton *searchClearPbt;
	QLineEdit *searchExpLed;
	searchLay->addWidget(searchLabel = OBJ_NAME(LBL(""), "search-experiments-label"));
	searchLay->addWidget(searchExpLed = OBJ_NAME(LED(), "search-experiments"));
	searchLay->addWidget(searchClearPbt = OBJ_NAME(PBT(""), "search-experiments-clear"));
	searchLay->addWidget(OBJ_NAME(WDG(), "search-experiments-spacing"));
	searchLabel->setPixmap(QPixmap(":/GUI/Resources/search-icon.png"));
	searchClearPbt->setIcon(QIcon(":/GUI/Resources/search-clear-button.png"));
	searchClearPbt->setIconSize(QPixmap(":/GUI/Resources/search-clear-button.png").size());
	searchClearPbt->hide();

	CONNECT(searchExpLed, &QLineEdit::textChanged, [=](const QString &text) {
		if (text.isEmpty()) {
			searchClearPbt->hide();
		}
		else {
			searchClearPbt->show();
		}
	});

	CONNECT(searchClearPbt, &QPushButton::clicked, [=]() {
		searchExpLed->clear();
	});

	CONNECT(searchExpLed, &QLineEdit::textChanged, proxyModel, &QSortFilterProxyModel::setFilterFixedString);

	auto selectCategoryLay = NO_SPACING(NO_MARGIN(new QHBoxLayout));
	auto selectCategory = OBJ_NAME(CMB(), "select-category");
	selectCategory->setView(OBJ_NAME(new QListView, "combo-list"));

	selectCategoryLay->addWidget(OBJ_NAME(WDG(), "search-experiments-spacing"));
	selectCategoryLay->addWidget(selectCategory);
	selectCategoryLay->addWidget(OBJ_NAME(WDG(), "search-experiments-spacing"));

	CONNECT(selectCategory, &QComboBox::currentTextChanged, [=](const QString &category) {
		proxyModel->SetCurrentCategory(category);
	});

	experimentListLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Categories"), "heading-label"), "widget-type", "left-grey"));
	experimentListLay->addLayout(selectCategoryLay);
	experimentListLay->addLayout(searchLay);
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
	startExpPbt->setIcon(QIcon(":/GUI/Resources/start.png"));
	startExpPbt->setIconSize(QPixmap(":/GUI/Resources/start.png").size());
	startExpPbt->hide();

	auto *pauseExpPbt = OBJ_PROP(OBJ_NAME(PBT("Pause Experiment"), "primary-button"), "button-type", "experiment-start-pbt");
	auto *stopExpPbt = OBJ_PROP(OBJ_NAME(PBT("Stop Experiment"), "primary-button"), "button-type", "experiment-start-pbt");

	pauseExpPbt->hide();
	stopExpPbt->hide();

	auto *buttonLay = NO_SPACING(NO_MARGIN(new QHBoxLayout()));

	buttonLay->addStretch(1);
	buttonLay->addWidget(startExpPbt);
	buttonLay->addWidget(pauseExpPbt);
	buttonLay->addWidget(stopExpPbt);
	buttonLay->addStretch(1);

	auto paramsHeadWidget = WDG();
	paramsHeadWidget->hide();

	auto paramsHeadWidgetLay = new QGridLayout(paramsHeadWidget);

	auto channelEdit = CMB();
	channelEdit->setView(OBJ_NAME(new QListView, "combo-list"));
	channelEdit->addItem("Channel 1", 0);
	//channelEdit->addItem("Channel 2", 1);

	auto hwList = OBJ_NAME(CMB(), "hw-list-combo");
	hwList->setView(OBJ_NAME(new QListView, "combo-list"));

	paramsHeadWidgetLay->addWidget(OBJ_NAME(LBL("Select Channel"), "heading-label"), 0, 0, 1, 3);
	paramsHeadWidgetLay->addWidget(hwList, 1, 0);
	paramsHeadWidgetLay->addWidget(channelEdit, 1, 1);
	paramsHeadWidgetLay->addWidget(OBJ_NAME(LBL("Parameters"), "heading-label"), 2, 0, 1, 3);
	paramsHeadWidgetLay->setColumnStretch(2, 1);

	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-top"), 0, 0, 1, 3);
	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-bottom"), 4, 0, 1, 3);
	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-left"), 1, 0, 2, 1);
	paramsWidgetLay->addWidget(OBJ_NAME(WDG(), "experiment-params-spacing-right"), 1, 3, 2, 1);
	paramsWidgetLay->addWidget(paramsHeadWidget, 1, 1);
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

	CONNECT(mw, &MainWindow::AddNewInstruments, [=](const QStringList &newLines) {
		hwList->addItems(newLines);
	});

	CONNECT(mw, &MainWindow::RemoveDisconnectedInstruments, [=](const QStringList &linesToDelete) {
		for (int i = 0; i < hwList->count();) {
			if (linesToDelete.contains(hwList->itemText(i))) {
				hwList->removeItem(i);
			}
			else {
				++i;
			}
		}
	});

	CONNECT(mw, &MainWindow::PrebuiltExperimentsFound, [=](const QList<AbstractExperiment*> &expList) {
		QStandardItemModel *model = new QStandardItemModel(expList.size(), 1);

		int row = 0;

		QStringList categoryStrList;
		foreach(const AbstractExperiment* exp, expList) {
			auto *item = new QStandardItem(exp->GetShortName());
			item->setData(QVariant::fromValue(exp), Qt::UserRole);
			
			model->setItem(row++, item);

			categoryStrList << exp->GetCategory();
		}
		categoryStrList << EXPERIMENT_VIEW_ALL_CATEGORY;
		categoryStrList.removeDuplicates();

		foreach(auto str, categoryStrList) {
			selectCategory->addItem(str);
		}
		selectCategory->setCurrentIndex(selectCategory->count() - 1);

		//experimentList->setModel(model);
		auto oldModel = proxyModel->sourceModel();
		proxyModel->setSourceModel(model);
		oldModel->deleteLater();
	});

	CONNECT(experimentList->selectionModel(), &QItemSelectionModel::currentChanged, [=](const QModelIndex &index, const QModelIndex &) {
		if (prebuiltExperimentData.userInputs) {
			paramsLay->removeWidget(prebuiltExperimentData.userInputs);
			prebuiltExperimentData.userInputs->deleteLater();
			prebuiltExperimentData.userInputs = 0;

			descrName->setText("");
			descrText->setText("");
			descrIcon->setPixmap(QPixmap());
		}

		if (index.isValid()) {
			auto exp = index.data(Qt::UserRole).value<const AbstractExperiment*>();

			descrName->setText(exp->GetFullName());
			descrText->setText(exp->GetDescription());
			descrIcon->setPixmap(exp->GetImage());

			prebuiltExperimentData.userInputs = exp->CreateUserInput();
			paramsLay->addWidget(prebuiltExperimentData.userInputs);
				
			mw->PrebuiltExperimentSelected(exp);

			paramsHeadWidget->show();

			if (hwList->count()) {
				mw->SelectHardware(hwList->currentText(), channelEdit->currentData().toInt());
				mw->UpdateCurrentExperimentState();
			}
		}
		else {
			startExpPbt->hide();
			pauseExpPbt->hide();
			stopExpPbt->hide();
			paramsHeadWidget->hide();
		}
	});

	CONNECT(hwList, &QComboBox::currentTextChanged, [=](const QString &hwName) {
		if (!experimentList->selectionModel()->currentIndex().isValid()) {
			return;
		}
		if (hwName.isEmpty()) {
			return;
		}

		mw->SelectHardware(hwList->currentText(), channelEdit->currentData().toInt());
		mw->UpdateCurrentExperimentState();
	});

	CONNECT(channelEdit, &QComboBox::currentTextChanged, [=](const QString &channelName) {
		if (!experimentList->selectionModel()->currentIndex().isValid()) {
			return;
		}
		mw->SelectHardware(hwList->currentText(), channelEdit->currentData().toInt());
		mw->UpdateCurrentExperimentState();
	});
	
	CONNECT(pauseExpPbt, &QPushButton::clicked, [=]() {
		if (pauseExpPbt->text() == PAUSE_EXP_BUTTON_TEXT) {
			mw->PauseExperiment(hwList->currentText(), channelEdit->currentData().toInt());
		}
		else {
			mw->ResumeExperiment(hwList->currentText(), channelEdit->currentData().toInt());
		}
	});

	CONNECT(stopExpPbt, &QPushButton::clicked, [=]() {
		mw->StopExperiment(hwList->currentText(), channelEdit->currentData().toInt());
	});

	CONNECT(startExpPbt, &QPushButton::clicked, [=]() {
		mw->StartExperiment(prebuiltExperimentData.userInputs);
		mw->UpdateCurrentExperimentState();
	});

	CONNECT(mw, &MainWindow::CurrentHardwareBusy, [=]() {
		startExpPbt->hide();
		pauseExpPbt->show();
		stopExpPbt->show();
	});

	CONNECT(mw, &MainWindow::CurrentExperimentCompleted, [=]() {
		startExpPbt->show();
		pauseExpPbt->hide();
		stopExpPbt->hide();
	});

	CONNECT(mw, &MainWindow::CurrentExperimentPaused, [=]() {
		pauseExpPbt->setText(RESUME_EXP_BUTTON_TEXT);
	});

	CONNECT(mw, &MainWindow::CurrentExperimentResumed, [=]() {
		pauseExpPbt->setText(PAUSE_EXP_BUTTON_TEXT);
	});

	return w;
}
bool MainWindowUI::ReadCsvFile(QWidget *parent, QList<MainWindowUI::CsvFileData> &dataList) {
	bool ret = false;

	QSettings settings(SQUID_STAT_PARAMETERS_INI, QSettings::IniFormat);
	QString dirName = settings.value(DATA_SAVE_PATH, "").toString();

	auto dialogRetList = QFileDialog::getOpenFileNames(parent, "Open experiment data", dirName, "Data files (*.csv)");

	if (dialogRetList.isEmpty()) {
		return ret;
	}

	settings.setValue(DATA_SAVE_PATH, QFileInfo(dialogRetList.first()).absolutePath());

	foreach(auto dialogRet, dialogRetList) {
		if (dialogRet.isEmpty()) {
			return ret;
		}

		if (!QFileInfo(dialogRet).isReadable()) {
			return ret;
		}

		CsvFileData data;
		ret = ReadCsvFile(dialogRet, data);

		if (!ret) {
			return ret;
		}

		dataList << data;
	}


	return ret;
}
bool MainWindowUI::ReadCsvFile(const QString &dialogRet, MainWindowUI::CsvFileData &data) {
	bool ret = false;
	QList<QStringList> readData = QtCSV::Reader::readToList(dialogRet, ";");

	if (readData.size() < 2) {
		return ret;
	}

	data.fileName = QFileInfo(dialogRet).fileName();

	QStringList hdrList = readData.front();
	readData.pop_front();
	QStringList axisList = readData.front();
	readData.pop_front();

	int hdrListSize = hdrList.size();

	if (hdrListSize != axisList.size()) {
		return ret;
	}

	for (int i = 0; i < hdrListSize; ++i) {
		const QString &varName(hdrList.at(i));

		if (axisList.at(i).contains('X')) {
			data.xAxisList << varName;
		}
		if (axisList.at(i).contains('Y')) {
			data.yAxisList << varName;
		}
	}

	QChar systemDecimalPoint = QLocale().decimalPoint();
	QChar cDecimalPoint = QLocale::c().decimalPoint();

	for (auto it = readData.begin(); it != readData.end(); ++it) {
		QStringList &list(*it);

		if (hdrListSize != list.size()) {
			return ret;
		}

		for (int i = 0; i < hdrListSize; ++i) {
			bool ok;
			qreal val = list[i].replace(systemDecimalPoint, cDecimalPoint).toFloat(&ok);
			if (!ok) {
				return ret;
			}

			if (data.container[hdrList.at(i)].data.isEmpty()) {
				data.container[hdrList.at(i)].max = val;
				data.container[hdrList.at(i)].min = val;
			}
			else {
				if (val > data.container[hdrList.at(i)].max) {
					data.container[hdrList.at(i)].max = val;
				}
				if (val < data.container[hdrList.at(i)].min) {
					data.container[hdrList.at(i)].min = val;
				}
			}

			data.container[hdrList.at(i)].data.push_back(val);
		}
	}
	
	ret = true;

	return ret;
}
bool MainWindowUI::ReadCsvFile(QWidget *parent, MainWindowUI::CsvFileData &data) {
	bool ret = false;
	QSettings settings(SQUID_STAT_PARAMETERS_INI, QSettings::IniFormat);
	QString dirName = settings.value(DATA_SAVE_PATH, "").toString();

	auto dialogRet = QFileDialog::getOpenFileName(parent, "Open experiment data", dirName, "Data files (*.csv)");

	if (dialogRet.isEmpty()) {
		return ret;
	}

	settings.setValue(DATA_SAVE_PATH, QFileInfo(dialogRet).absolutePath());

	if (!QFileInfo(dialogRet).isReadable()) {
		return ret;
	}


	ret = ReadCsvFile(dialogRet, data);

	return ret;
}
QWidget* MainWindowUI::GetNewDataWindowTab() {
	static QWidget *w = 0;

	if (w) {
		return w;
	}

	w = OBJ_NAME(WDG(), "new-data-window-owner");

	auto *lay = NO_SPACING(NO_MARGIN(new QVBoxLayout(w)));
	QTabWidget *docTabs = OBJ_NAME(new QTabWidget, "plot-tab");
	ui.newDataTab.docTabs = docTabs;
	
	lay->addWidget(docTabs);

	docTabs->addTab(WDG(), QIcon(":/GUI/Resources/new-tab.png"), "");

	static QPushButton *closeTabButton = 0;
	static QMetaObject::Connection closeTabButtonConnection;
	static int prevCloseTabButtonPos = -1;

	CONNECT(docTabs->tabBar(), &QTabBar::tabBarClicked, [=](int index) {
		if (index != docTabs->count() - 1) {
			return;
		}
		
		CsvFileData csvData;
		if (!ReadCsvFile(mw, csvData)) {
			return;
		}

		QString tabName = csvData.fileName;
		const QUuid id = QUuid::createUuid();

		auto dataTabWidget = CreateNewDataTabWidget(id,
			ET_SAVED,
			tabName,
			csvData.xAxisList,
			csvData.yAxisList,
			&csvData.container);

		docTabs->insertTab(docTabs->count() - 1, dataTabWidget, tabName);
		ui.newDataTab.newDataTabButton->click();
		docTabs->setCurrentIndex(docTabs->count() - 2);
	});

	CONNECT(docTabs, &QTabWidget::currentChanged, [=](int index) {
		if (index < 0) {
			return;
		}
		if (index >= docTabs->count() - 1) {
			docTabs->setCurrentIndex(prevCloseTabButtonPos);
			return;
		}

		if (closeTabButton) {
			docTabs->tabBar()->setTabButton(prevCloseTabButtonPos, QTabBar::RightSide, 0);
			QObject::disconnect(closeTabButtonConnection);
			closeTabButton->deleteLater();
		}

		docTabs->tabBar()->setTabButton(index, QTabBar::RightSide, closeTabButton = OBJ_NAME(PBT("x"), "close-document-pbt"));
		prevCloseTabButtonPos = index;

		closeTabButtonConnection = 
			CONNECT(closeTabButton, &QPushButton::clicked, [=]() {
				int currentIndex = docTabs->currentIndex();

				if ((-1 == currentIndex) || (currentIndex >= docTabs->count() - 1)) {
					return;
				}

				auto wdg = docTabs->widget(currentIndex);
				auto plot = wdg->findChild<QWidget*>("qwt-plot");

				if (0 != plot) {
					for (auto itId = dataTabs.plots.begin(); itId != dataTabs.plots.end(); ++itId) {
						bool found = false;
						for (auto it = itId.value().begin(); it != itId.value().end(); ++it) {
							if (it.value().plot != plot) {
								break;
							}

							foreach(auto conn, it.value().plotTabConnections) {
								QObject::disconnect(conn);
							}

							if (it.value().data.first().saveFile) {
								it.value().data.first().saveFile->close();
								it.value().data.first().saveFile->deleteLater();
								it.value().data.first().saveFile = 0;
							}

							mw->StopExperiment(itId.key());

							itId.value().remove(it.key());
							if (0 == itId.value().count()) {
								dataTabs.plots.remove(itId.key());
							}

							found = true;
							break;
						}

						if (found) {
							break;
						}
					}
				}

				if( (prevCloseTabButtonPos == (docTabs->count() - 2)) && (docTabs->count() > 2) ){
					--prevCloseTabButtonPos;
				}

				docTabs->tabBar()->setTabButton(prevCloseTabButtonPos, QTabBar::RightSide, 0);
				QObject::disconnect(closeTabButtonConnection);
				closeTabButton->deleteLater();
				closeTabButton = 0;
				docTabs->removeTab(currentIndex);
				wdg->deleteLater();
			});
	});

	CONNECT(mw, &MainWindow::CreateNewDataWindow, [=](const StartExperimentParameters &startParams) {
		//QString expName = exp->GetShortName();

		auto dataTabWidget = CreateNewDataTabWidget(startParams.id,
			startParams.type,
			startParams.exp->GetShortName(),
			startParams.exp->GetXAxisParameters(),
			startParams.exp->GetYAxisParameters());

		auto &handler(dataTabs.plots[startParams.id][startParams.type]);

		handler.exp = startParams.exp;
		handler.data.first().saveFile = startParams.file;
		handler.data.first().cal = startParams.cal;
		handler.data.first().hwVer = startParams.hwVer;

		docTabs->insertTab(docTabs->count() - 1, dataTabWidget, startParams.name);

		ui.newDataTab.newDataTabButton->click();
		docTabs->setCurrentIndex(docTabs->count() - 2);
	});

	CONNECT(mw, &MainWindow::ExperimentCompleted, [=](const QUuid &id) {
		for (auto it = dataTabs.plots[id].begin(); it != dataTabs.plots[id].end(); ++it) {
			PlotHandler &handler(it.value());
			DataMapVisualization &majorData(handler.data.first());

			if (majorData.saveFile) {
				majorData.saveFile->close();
				majorData.saveFile->deleteLater();
				majorData.saveFile = 0;
			}

			handler.plot->replot();
		}
	});

	CONNECT(mw, &MainWindow::DcDataArrived, [=](const QUuid &id, quint8 channel, const ExperimentalDcData &expData, bool paused) {
		if (!dataTabs.plots.keys().contains(id)) {
			return;
		}

		if (!dataTabs.plots[id].contains(ET_DC)) {
			return;
		}

		if (paused) {
			return;
		}

		PlotHandler &handler(dataTabs.plots[id][ET_DC]);
		DataMapVisualization &majorData(handler.data.first());

		handler.exp->PushNewDcData(expData, majorData.container, majorData.cal, majorData.hwVer);
		if (majorData.saveFile) {
			handler.exp->SaveDcData(*majorData.saveFile, majorData.container);
		}

		if (majorData.data[QwtPlot::xBottom] && majorData.data[QwtPlot::yLeft]) {
			majorData.curve1->setSamples(new ListSeriesData(*majorData.data[QwtPlot::xBottom], *majorData.data[QwtPlot::yLeft]));
			
			ApplyNewAxisParams(QwtPlot::yLeft, handler);
		}

		if (majorData.data[QwtPlot::xBottom] && majorData.data[QwtPlot::yRight]) {
			majorData.curve2->setSamples(new ListSeriesData(*majorData.data[QwtPlot::xBottom], *majorData.data[QwtPlot::yRight]));
			
			ApplyNewAxisParams(QwtPlot::yRight, handler);
		}

		ApplyNewAxisParams(QwtPlot::xBottom, handler);

		auto curStamp = QDateTime::currentMSecsSinceEpoch();
		if (curStamp > handler.plotCounter.stamp) {
			handler.plot->replot();
			handler.plotCounter.stamp = curStamp + 50;
		}
	});

	CONNECT(mw, &MainWindow::AcDataArrived, [=](const QUuid &id, quint8 channel, const QByteArray &expData, bool paused) {
		if (!dataTabs.plots.keys().contains(id)) {
			return;
		}

		if (!dataTabs.plots[id].contains(ET_AC)) {
			return;
		}

		if (paused) {
			return;
		}

		PlotHandler &handler(dataTabs.plots[id][ET_AC]);
		DataMapVisualization &majorData(handler.data.first());

		handler.exp->PushNewAcData(expData, majorData.container, majorData.cal, majorData.hwVer);
		if (majorData.saveFile) {
			handler.exp->SaveAcData(*majorData.saveFile, majorData.container);
		}

		if (majorData.data[QwtPlot::xBottom] && majorData.data[QwtPlot::yLeft]) {
			majorData.curve1->setSamples(new ListSeriesData(*majorData.data[QwtPlot::xBottom], *majorData.data[QwtPlot::yLeft]));

			ApplyNewAxisParams(QwtPlot::yLeft, handler);
		}

		if (majorData.data[QwtPlot::xBottom] && majorData.data[QwtPlot::yRight]) {
			majorData.curve2->setSamples(new ListSeriesData(*majorData.data[QwtPlot::xBottom], *majorData.data[QwtPlot::yRight]));

			ApplyNewAxisParams(QwtPlot::yRight, handler);
		}

		ApplyNewAxisParams(QwtPlot::xBottom, handler);

		auto curStamp = QDateTime::currentMSecsSinceEpoch();
		if (curStamp > handler.plotCounter.stamp) {
			handler.plot->replot();
			handler.plotCounter.stamp = curStamp + 50;
		}
	});

	return w;
}

template<typename T, typename F>
void ModifyObject(QObject *parent, F &lambda) {
	foreach(QObject* obj, parent->children()) {
		T* objT = qobject_cast<T*>(obj);
		if (objT) {
			lambda(objT);
		}

		ModifyObject<T>(obj, lambda);
	}
}
#include <QDialogButtonBox>
bool MainWindowUI::GetColor(QWidget *parent, QColor &color) {
	static bool ret;
	ret = false;

	QColorDialog colorDialog(DEFAULT_MAJOR_CURVE_COLOR, parent);
	colorDialog.setWindowFlags(Qt::SplashScreen);
	colorDialog.setOptions(QColorDialog::NoButtons | QColorDialog::DontUseNativeDialog);
	colorDialog.setCurrentColor(color);
	colorDialog.setCustomColor(0, DEFAULT_MAJOR_CURVE_COLOR);
	colorDialog.setCustomColor(1, DEFAULT_MINOR_CURVE_COLOR);

	QObject *dialogPtr = &colorDialog;

	QPushButton *ok;
	QPushButton *cancel;

	auto buttonLay = new QHBoxLayout;
	buttonLay->addStretch(1);
	buttonLay->addWidget(ok = PBT("Select"));
	buttonLay->addWidget(cancel = PBT("Cancel"));
	buttonLay->addStretch(1);

	CONNECT(ok, &QPushButton::clicked, &colorDialog, &QColorDialog::accept);
	CONNECT(cancel, &QPushButton::clicked, &colorDialog, &QColorDialog::reject);

	auto okConnection = CONNECT(ok, &QPushButton::clicked, [=]() {
		ret = true;
	});

	ModifyObject<QVBoxLayout>(dialogPtr, [=](QVBoxLayout *obj) {
		if (obj->parent() != dialogPtr) {
			return;
		}

		obj->addStretch(1);
		obj->addLayout(buttonLay);
	});

	ModifyObject<QAbstractButton>(dialogPtr, [](QAbstractButton *obj) {
		OBJ_NAME(obj, "secondary-button");
	});

	#define BASIC_HEADER "&Basic colors"
	#define CUSTOM_HEADER "&Custom colors"

	ModifyObject<QLabel>(dialogPtr, [](QLabel *obj) {
		if ((obj->text() == BASIC_HEADER) || (obj->text() == CUSTOM_HEADER)) {
			OBJ_NAME(obj, "heading-label");
		}
		else {
			OBJ_PROP(OBJ_NAME(obj, "experiment-params-comment"), "comment-placement", "left");
		}
	});


	ModifyObject<QFrame>(dialogPtr, [](QFrame *obj) {
		auto objParent = qobject_cast<QWidget*>(obj->parent());
	});

	colorDialog.exec();

	QObject::disconnect(okConnection);

	color = colorDialog.selectedColor();

	return ret;
}
bool MainWindowUI::GetNewPen(QWidget *parent, QMap<QString, MainWindowUI::CurveParameters> &curveParams) {
	static bool dialogCanceled;
	dialogCanceled = true;
	
	QDialog* dialog = OBJ_NAME(new QDialog(parent, Qt::SplashScreen), "curve-params-dialog");
	QList<QMetaObject::Connection> dialogConn;
	auto globalLay = NO_SPACING(NO_MARGIN(new QHBoxLayout(dialog)));

	auto lay = new QVBoxLayout();

	globalLay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-horizontal-spacing"));
	globalLay->addLayout(lay);
	globalLay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-horizontal-spacing"));

	QListView *fileList;

	lay->addWidget(OBJ_NAME(LBL("Data Set List"), "heading-label"));
	lay->addWidget(fileList = OBJ_NAME(new QListView, "curve-params-data-set-list"));

	fileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	auto optLay = new QHBoxLayout;
	QRadioButton *primBut;
	QRadioButton *secBut;
	optLay->addWidget(primBut = RBT("Primary curve"));
	optLay->addWidget(secBut = RBT("Secondary curve"));
	lay->addLayout(optLay);

	auto stackLay = NO_SPACING(NO_MARGIN(new QStackedLayout));

	QLabel *curveSettingLabel;
	lay->addWidget(curveSettingLabel = OBJ_NAME(LBL("Primary Curve"), "heading-label"));

	static CurveParameters currentParams;

	QSettings squidSettings(SQUID_STAT_PARAMETERS_INI, QSettings::IniFormat);

	currentParams.curve[CurveParameters::PRIMARY].pen.color = QColor(squidSettings.value(CURVE_PARAMS_PRI_PEN_COLOR, DEFAULT_MAJOR_CURVE_COLOR.name()).toString());
	currentParams.curve[CurveParameters::PRIMARY].pen.width = squidSettings.value(CURVE_PARAMS_PRI_PEN_WIDTH, 1.0).toDouble();
	currentParams.curve[CurveParameters::PRIMARY].pen.style = (Qt::PenStyle)squidSettings.value(CURVE_PARAMS_PRI_PEN_STYLE, (int)Qt::SolidLine).toInt();
	currentParams.curve[CurveParameters::PRIMARY].style = (QwtPlotCurve::CurveStyle)squidSettings.value(CURVE_PARAMS_PRI_STYLE, (int)QwtPlotCurve::Lines).toInt();
	currentParams.curve[CurveParameters::PRIMARY].symbol.width = squidSettings.value(CURVE_PARAMS_PRI_SYMBOL_WIDTH, 8.0).toDouble();
	currentParams.curve[CurveParameters::PRIMARY].symbol.style = (QwtSymbol::Style)squidSettings.value(CURVE_PARAMS_PRI_SYMBOL_STYLE, (int)QwtSymbol::NoSymbol).toInt();

	currentParams.curve[CurveParameters::SECONDARY].pen.color = QColor(squidSettings.value(CURVE_PARAMS_SEC_PEN_COLOR, DEFAULT_MINOR_CURVE_COLOR.name()).toString());
	currentParams.curve[CurveParameters::SECONDARY].pen.width = squidSettings.value(CURVE_PARAMS_SEC_PEN_WIDTH, 1.0).toDouble();
	currentParams.curve[CurveParameters::SECONDARY].pen.style = (Qt::PenStyle)squidSettings.value(CURVE_PARAMS_SEC_PEN_STYLE, (int)Qt::SolidLine).toInt();
	currentParams.curve[CurveParameters::SECONDARY].style = (QwtPlotCurve::CurveStyle)squidSettings.value(CURVE_PARAMS_SEC_STYLE, (int)QwtPlotCurve::Lines).toInt();
	currentParams.curve[CurveParameters::SECONDARY].symbol.width = squidSettings.value(CURVE_PARAMS_SEC_SYMBOL_WIDTH, 8.0).toDouble();
	currentParams.curve[CurveParameters::SECONDARY].symbol.style = (QwtSymbol::Style)squidSettings.value(CURVE_PARAMS_SEC_SYMBOL_STYLE, (int)QwtSymbol::NoSymbol).toInt();

	for(int what = CurveParameters::START; what < CurveParameters::TOTAL_CURVES; ++what) {
		auto paramsOwner = OBJ_NAME(WDG(), "curve-params-adjusting-owner");

		QwtPlot *smallPlot;
		QComboBox *penStyleCmb;
		QComboBox *curveSymbolCmb;
		QDoubleSpinBox *spin;
		QPushButton *colorPbt;
		QLabel *widthLbl;
		QSpinBox *widthSpin;
		auto paramsLay = NO_SPACING(NO_MARGIN(new QGridLayout(paramsOwner)));

		paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Color (click): "), "experiment-params-comment"), "comment-placement", "left"), 0, 0);
		paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Width: "), "experiment-params-comment"), "comment-placement", "left"), 1, 0);
		paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Line Style: "), "experiment-params-comment"), "comment-placement", "left"), 2, 0);
		paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Dot Style: "), "experiment-params-comment"), "comment-placement", "left"), 3, 0);
		paramsLay->addWidget(widthLbl = OBJ_PROP(OBJ_NAME(LBL("Width: "), "experiment-params-comment"), "comment-placement", "left"), 3, 2);
		paramsLay->addWidget(colorPbt = PBT(""), 0, 1);
		paramsLay->addWidget(spin = new QDoubleSpinBox, 1, 1);
		paramsLay->addWidget(penStyleCmb = CMB(), 2, 1);
		paramsLay->addWidget(curveSymbolCmb = CMB(), 3, 1);
		paramsLay->addWidget(widthSpin = new QSpinBox, 3, 3);
		paramsLay->addWidget(smallPlot = new QwtPlot, 0, 2, 3, 2);

		smallPlot->enableAxis(QwtPlot::yLeft, false);
		smallPlot->enableAxis(QwtPlot::xBottom, false);

		auto curve = CreateCurve(QwtPlot::yLeft, DEFAULT_MAJOR_CURVE_COLOR);
		curve->setSamples(QVector<qreal>() << 0.0 << 0.25 << 0.5 << 0.75 << 1.0, QVector<qreal>() << 0.0 << 0.25 << 0.5 << 0.75 << 1.0);
		curve->attach(smallPlot);

		curve->setPen(currentParams.curve[what].pen.color,
			currentParams.curve[what].pen.width,
			currentParams.curve[what].pen.style);

		curve->setStyle(currentParams.curve[what].style);

		auto curSymbol = new QwtSymbol(currentParams.curve[what].symbol.style);
		curSymbol->setPen(currentParams.curve[what].pen.color, 1, Qt::SolidLine);
		curSymbol->setColor(currentParams.curve[what].pen.color);
		curSymbol->setSize(currentParams.curve[what].symbol.width);
		
		curve->setSymbol(curSymbol);

		smallPlot->replot();

		colorPbt->setStyleSheet("background-color: " + currentParams.curve[what].pen.color.name() + ";");

		spin->setDecimals(1);
		spin->setSingleStep(0.1);
		spin->setValue(currentParams.curve[what].pen.width);

		widthSpin->setValue(currentParams.curve[what].symbol.width);

		penStyleCmb->setView(OBJ_NAME(new QListView, "combo-list"));
		{
			penStyleCmb->addItem("No Line", QVariant::fromValue<Qt::PenStyle>(Qt::NoPen));
			penStyleCmb->addItem("Solid", QVariant::fromValue<Qt::PenStyle>(Qt::SolidLine));
			penStyleCmb->addItem("Dash", QVariant::fromValue<Qt::PenStyle>(Qt::DashLine));
			penStyleCmb->addItem("Dot", QVariant::fromValue<Qt::PenStyle>(Qt::DotLine));
			penStyleCmb->addItem("Dash Dot", QVariant::fromValue<Qt::PenStyle>(Qt::DashDotLine));
			penStyleCmb->addItem("Dash Dot Dot", QVariant::fromValue<Qt::PenStyle>(Qt::DashDotDotLine));
		}

		if (QwtPlotCurve::Dots == currentParams.curve[what].style) {
			penStyleCmb->setCurrentIndex(0);
		}
		else {
			for (int i = 0; i < penStyleCmb->count(); ++i) {
				if (penStyleCmb->itemData(i).value<Qt::PenStyle>() == currentParams.curve[what].pen.style) {
					penStyleCmb->setCurrentIndex(i);
					break;
				}
			}
		}

		curveSymbolCmb->setView(OBJ_NAME(new QListView, "combo-list"));
		{
			curveSymbolCmb->addItem("NoSymbol", QVariant::fromValue<int>((int)QwtSymbol::NoSymbol));
			curveSymbolCmb->addItem("Ellipse", QVariant::fromValue<int>((int)QwtSymbol::Ellipse));
			curveSymbolCmb->addItem("Rect", QVariant::fromValue<int>((int)QwtSymbol::Rect));
			curveSymbolCmb->addItem("Diamond", QVariant::fromValue<int>((int)QwtSymbol::Diamond));
			curveSymbolCmb->addItem("Triangle", QVariant::fromValue<int>((int)QwtSymbol::Triangle));
			curveSymbolCmb->addItem("DTriangle", QVariant::fromValue<int>((int)QwtSymbol::DTriangle));
			curveSymbolCmb->addItem("UTriangle", QVariant::fromValue<int>((int)QwtSymbol::UTriangle));
			curveSymbolCmb->addItem("LTriangle", QVariant::fromValue<int>((int)QwtSymbol::LTriangle));
			curveSymbolCmb->addItem("RTriangle", QVariant::fromValue<int>((int)QwtSymbol::RTriangle));
			curveSymbolCmb->addItem("Cross", QVariant::fromValue<int>((int)QwtSymbol::Cross));
			curveSymbolCmb->addItem("XCross", QVariant::fromValue<int>((int)QwtSymbol::XCross));
			curveSymbolCmb->addItem("HLine", QVariant::fromValue<int>((int)QwtSymbol::HLine));
			curveSymbolCmb->addItem("VLine", QVariant::fromValue<int>((int)QwtSymbol::VLine));
			curveSymbolCmb->addItem("Star1", QVariant::fromValue<int>((int)QwtSymbol::Star1));
			curveSymbolCmb->addItem("Star2", QVariant::fromValue<int>((int)QwtSymbol::Star2));
			curveSymbolCmb->addItem("Hexagon", QVariant::fromValue<int>((int)QwtSymbol::Hexagon));
		}
		
		for (int i = 0; i < curveSymbolCmb->count(); ++i) {
			if ((QwtSymbol::Style)curveSymbolCmb->itemData(i).value<int>() == currentParams.curve[what].symbol.style) {
				curveSymbolCmb->setCurrentIndex(i);
				break;
			}
		}
				
		stackLay->addWidget(paramsOwner);

		dialogConn << CONNECT(curveSymbolCmb, &QComboBox::currentTextChanged, [=]() {
			auto symbolStyle = (QwtSymbol::Style)curveSymbolCmb->currentData().value<int>();
			auto symbol = new QwtSymbol(symbolStyle);

			widthSpin->setHidden(QwtSymbol::NoSymbol == symbolStyle);
			widthLbl->setHidden(QwtSymbol::NoSymbol == symbolStyle);

			currentParams.curve[what].symbol.style = symbolStyle;

			symbol->setPen(currentParams.curve[what].pen.color, 1, Qt::SolidLine);
			symbol->setColor(currentParams.curve[what].pen.color);
			symbol->setSize(currentParams.curve[what].symbol.width);

			curve->setSymbol(symbol);
			smallPlot->replot();
		});
		
		dialogConn << CONNECT(penStyleCmb, &QComboBox::currentTextChanged, [=]() {
			auto penStyle = penStyleCmb->currentData().value<Qt::PenStyle>();
			if (Qt::NoPen == penStyle) {
				currentParams.curve[what].style = QwtPlotCurve::Dots;
			}
			else {
				currentParams.curve[what].style = QwtPlotCurve::Lines;
				currentParams.curve[what].pen.style = penStyle;
				curve->setPen(curve->pen().color(), curve->pen().width(), penStyleCmb->currentData().value<Qt::PenStyle>());
			}
			curve->setStyle(currentParams.curve[what].style);
			smallPlot->replot();
		});

		dialogConn << CONNECT(spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double val) {
			currentParams.curve[what].pen.width = val;
			curve->setPen(curve->pen().color(), val, curve->pen().style());
			smallPlot->replot();
		});

		dialogConn << CONNECT(widthSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int val) {
			currentParams.curve[what].symbol.width = val;

			auto symbol = new QwtSymbol(currentParams.curve[what].symbol.style);
			symbol->setPen(currentParams.curve[what].pen.color, 1, Qt::SolidLine);
			symbol->setColor(currentParams.curve[what].pen.color);
			symbol->setSize(currentParams.curve[what].symbol.width);

			curve->setSymbol(symbol);

			smallPlot->replot();
		});

		dialogConn << CONNECT(colorPbt, &QPushButton::clicked, [=]() {
			QColor color = curve->pen().color();
			if (GetColor(dialog, color)) {
				colorPbt->setStyleSheet("background-color: " + color.name() + ";");

				currentParams.curve[what].pen.color = color;

				curve->setPen(color, curve->pen().width(), curve->pen().style());

				auto symbol = new QwtSymbol(currentParams.curve[what].symbol.style);
				symbol->setPen(currentParams.curve[what].pen.color, 1, Qt::SolidLine);
				symbol->setColor(currentParams.curve[what].pen.color);
				symbol->setSize(currentParams.curve[what].symbol.width);

				curve->setSymbol(symbol);

				smallPlot->replot();
			}
		});
	}

	primBut->setChecked(true);

	dialogConn << CONNECT(primBut, &QRadioButton::clicked, [=]() {
		curveSettingLabel->setText("Primary Curve");
		stackLay->setCurrentIndex(0);
	});
	dialogConn << CONNECT(secBut, &QRadioButton::clicked, [=]() {
		curveSettingLabel->setText("Secondary Curve");
		stackLay->setCurrentIndex(1);
	});

	lay->addLayout(stackLay);
	lay->addSpacing(40);

	/*
	auto adjustButtonLay = new QHBoxLayout;
	QPushButton *changeColorPbt;
	QPushButton *changeLinePbt;
	adjustButtonLay->addStretch(1);
	adjustButtonLay->addWidget(changeColorPbt = OBJ_NAME(PBT("Change Color"), "secondary-button"));
	adjustButtonLay->addWidget(changeLinePbt = OBJ_NAME(PBT("Change Line Style"), "secondary-button"));
	adjustButtonLay->addStretch(1);

	lay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-vertical-spacing"));
	lay->addLayout(adjustButtonLay);
	lay->addStretch(1);
	//*/

	auto buttonLay = new QHBoxLayout;
	QPushButton *okBut;
	QPushButton *cancelBut;
	buttonLay->addStretch(1);
	buttonLay->addWidget(okBut = OBJ_NAME(PBT("Apply"), "secondary-button"));
	buttonLay->addWidget(cancelBut = OBJ_NAME(PBT("Cancel"), "secondary-button"));
	buttonLay->addStretch(1);

	lay->addLayout(buttonLay);
	lay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-vertical-spacing"));

	QStandardItemModel *model = new QStandardItemModel(curveParams.size(), 1);
	int row = 0;
	foreach(auto key, curveParams.keys()) {
		auto *item = new QStandardItem(key);
		model->setItem(row++, item);
	}
	fileList->setModel(model);
	fileList->selectionModel()->select(fileList->model()->index(0, 0), QItemSelectionModel::Select);
	
	/*
	QMap<QString, CurveParameters> *curveParamsPtr = &curveParams;

	dialogConn << CONNECT(changeColorPbt, &QPushButton::clicked, [=]() {
		QColor color = DEFAULT_MAJOR_CURVE_COLOR;
		if (GetColor(dialog, color)) {
			foreach(const QModelIndex &index, fileList->selectionModel()->selectedIndexes()) {
				QString curName = index.data(Qt::DisplayRole).toString();
				(*curveParamsPtr)[curName].color1 = color;
			}
		}

		color = DEFAULT_MINOR_CURVE_COLOR;
		if (GetColor(dialog, color)) {
			foreach(const QModelIndex &index, fileList->selectionModel()->selectedIndexes()) {
				QString curName = index.data(Qt::DisplayRole).toString();
				(*curveParamsPtr)[curName].color2 = color;
			}
		}
	});

	dialogConn << CONNECT(changeLinePbt, &QPushButton::clicked, [=]() {
		;
	});
	//*/

	dialogConn << CONNECT(okBut, &QPushButton::clicked, [=]() {
		dialogCanceled = false;
	});

	CONNECT(okBut, &QPushButton::clicked, dialog, &QDialog::accept);
	CONNECT(cancelBut, &QPushButton::clicked, dialog, &QDialog::reject);

	dialog->exec();

	if (!dialogCanceled) {
		foreach(const QModelIndex &index, fileList->selectionModel()->selectedIndexes()) {
			QString curName = index.data(Qt::DisplayRole).toString();
			curveParams[curName].curve[CurveParameters::PRIMARY] = currentParams.curve[CurveParameters::PRIMARY];
			curveParams[curName].curve[CurveParameters::SECONDARY] = currentParams.curve[CurveParameters::SECONDARY];
		}
	}

	squidSettings.setValue(CURVE_PARAMS_PRI_PEN_COLOR, currentParams.curve[CurveParameters::PRIMARY].pen.color.name());
	squidSettings.setValue(CURVE_PARAMS_PRI_PEN_WIDTH, currentParams.curve[CurveParameters::PRIMARY].pen.width);
	squidSettings.setValue(CURVE_PARAMS_PRI_PEN_STYLE, (int)currentParams.curve[CurveParameters::PRIMARY].pen.style);
	squidSettings.setValue(CURVE_PARAMS_PRI_STYLE, (int)currentParams.curve[CurveParameters::PRIMARY].style);
	squidSettings.setValue(CURVE_PARAMS_PRI_SYMBOL_WIDTH, currentParams.curve[CurveParameters::PRIMARY].symbol.width);
	squidSettings.setValue(CURVE_PARAMS_PRI_SYMBOL_STYLE, (int)currentParams.curve[CurveParameters::PRIMARY].symbol.style);

	squidSettings.setValue(CURVE_PARAMS_SEC_PEN_COLOR, currentParams.curve[CurveParameters::SECONDARY].pen.color.name());
	squidSettings.setValue(CURVE_PARAMS_SEC_PEN_WIDTH, currentParams.curve[CurveParameters::SECONDARY].pen.width);
	squidSettings.setValue(CURVE_PARAMS_SEC_PEN_STYLE, (int)currentParams.curve[CurveParameters::SECONDARY].pen.style);
	squidSettings.setValue(CURVE_PARAMS_SEC_STYLE, (int)currentParams.curve[CurveParameters::SECONDARY].style);
	squidSettings.setValue(CURVE_PARAMS_SEC_SYMBOL_WIDTH, currentParams.curve[CurveParameters::SECONDARY].symbol.width);
	squidSettings.setValue(CURVE_PARAMS_SEC_SYMBOL_STYLE, (int)currentParams.curve[CurveParameters::SECONDARY].symbol.style);

	foreach(auto conn, dialogConn) {
		QObject::disconnect(conn);
	}

	dialog->deleteLater();

	return !dialogCanceled;
}
QwtPlotCurve* MainWindowUI::CreateCurve(int yAxisId, const QColor &color) {
	QwtPlotCurve *curve = new QwtPlotCurve("");

	curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
	curve->setPen(color, 1);
	curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
	curve->setYAxis(yAxisId);

	return curve;
}

class PlotEventFilter : public QObject {
public:
	PlotEventFilter(QObject *parent, std::function<void(void)> lambda) : QObject(parent), _lambda(lambda) {}

	bool eventFilter(QObject *obj, QEvent *e) {
		if ((e->type() == QEvent::MouseButtonDblClick) || (e->type() == QEvent::MouseButtonPress)) {
			_lambda();
			return true;
		}
		return false;
	}

private:
	std::function<void(void)> _lambda;
};
class LegendEventFilter : public QObject {
public:
	LegendEventFilter(QObject *parent, QwtPlot *plot) : QObject(parent), _plot(plot) {}

	bool eventFilter(QObject *obj, QEvent *e) {
		if (e->type() == QEvent::MouseButtonDblClick) {
			obj->deleteLater();
			_plot->insertLegend(0);
			_plot->replot();
			return true;
		}
		return false;
	}

private:
	QwtPlot *_plot;
};
class PopupDialogEventFilter : public QObject {
public:
	PopupDialogEventFilter(QObject *parent, std::function<void(QEvent*, bool&)> lambda) : QObject(parent), _lambda(lambda) {}

	bool eventFilter(QObject *obj, QEvent *e) {
		if (e->type() == QEvent::KeyPress) {
			bool ret;
			_lambda(e, ret);
			return ret;
		}
		return false;
	}
private:
	std::function<void(QEvent*, bool&)> _lambda;
};
QString MainWindowUI::GetNewTitle(QWidget *parent, const QString &oldText) {
	static bool dialogCanceled;
	dialogCanceled = false;

	QDialog* dialog = OBJ_NAME(new QDialog(parent, Qt::FramelessWindowHint | Qt::Popup), "plot-title-dialog");
	QList<QMetaObject::Connection> dialogConn;
	auto globalLay = NO_SPACING(NO_MARGIN(new QHBoxLayout(dialog)));

	auto lay = new QVBoxLayout();

	globalLay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-horizontal-spacing"));
	globalLay->addLayout(lay);
	globalLay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-horizontal-spacing"));

	QLineEdit *titleLed;

	auto paramsLay = new QHBoxLayout;
	paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Plot title: "), "experiment-params-comment"), "comment-placement", "left"));
	paramsLay->addWidget(titleLed = new QLineEdit(oldText));

	lay->addLayout(paramsLay);
	lay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-vertical-spacing"));

	dialog->installEventFilter(new PopupDialogEventFilter(dialog, [=](QEvent *e, bool &ret) {
		QKeyEvent *ke = (QKeyEvent*)e;

		ret = false;
		int key = ke->key();

		if (ke->matches(QKeySequence::Cancel)) {
			dialog->reject();
			dialogCanceled = true;
			ret = true;
		}
		if ((Qt::Key_Enter == key) || (Qt::Key_Return == key)) {
			dialog->accept();
			ret = true;
		}
	}));

	dialog->exec();

	QString ret;
	if (!dialogCanceled) {
		ret = titleLed->text();
	}

	foreach(auto conn, dialogConn) {
		QObject::disconnect(conn);
	}

	dialog->deleteLater();

	return ret;
}
bool MainWindowUI::GetNewAxisParams(QWidget *parent, MainWindowUI::AxisParameters &axisParams) {
	static bool dialogCanceled;
	dialogCanceled = false;

	QDialog* dialog = OBJ_NAME(new QDialog(parent, Qt::FramelessWindowHint | Qt::Popup), "axis-params-dialog");
	QList<QMetaObject::Connection> dialogConn;
	auto globalLay = NO_SPACING(NO_MARGIN(new QHBoxLayout(dialog)));

	auto lay = new QVBoxLayout();

	globalLay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-horizontal-spacing"));
	globalLay->addLayout(lay);
	globalLay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-horizontal-spacing"));

	QLineEdit *minLed;
	QLineEdit *maxLed;
	QLineEdit *stepLed;

	auto paramsLay = new QGridLayout;
	paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Min value: "), "experiment-params-comment"), "comment-placement", "left"), 1, 0);
	paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Max value: "), "experiment-params-comment"), "comment-placement", "left"), 2, 0);
	paramsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Step size: "), "experiment-params-comment"), "comment-placement", "left"), 3, 0);
	paramsLay->addWidget(minLed = LED(), 1, 1);
	paramsLay->addWidget(maxLed = LED(), 2, 1);
	paramsLay->addWidget(stepLed = LED(), 3, 1);

	minLed->setPlaceholderText("Auto");
	maxLed->setPlaceholderText("Auto");
	stepLed->setPlaceholderText("Auto");

	lay->addLayout(paramsLay);
	lay->addWidget(OBJ_NAME(WDG(), "curve-params-dialog-vertical-spacing"));
	
	if (!axisParams.min.autoScale) {
		minLed->setText(QString::number(axisParams.min.val).replace(QChar('.'), QLocale().decimalPoint()));
	}
	if (!axisParams.max.autoScale) {
		maxLed->setText(QString::number(axisParams.max.val).replace(QChar('.'), QLocale().decimalPoint()));
	}
	if (!axisParams.step.autoScale) {
		stepLed->setText(QString::number(axisParams.step.val).replace(QChar('.'), QLocale().decimalPoint()));
	}

	dialog->installEventFilter(new PopupDialogEventFilter(dialog, [=](QEvent *e, bool &ret) {
		QKeyEvent *ke = (QKeyEvent*)e;

		ret = false;
		int key = ke->key();

		if (ke->matches(QKeySequence::Cancel)) {
			dialog->reject();
			dialogCanceled = true;
			ret = true;
		}
		if( (Qt::Key_Enter == key) || (Qt::Key_Return == key) ) {
			dialog->accept();
			ret = true;
		}
	}));

	dialog->exec();

	QString resStr;
	#define GET_VAL(led, var)					\
		resStr = led->text();					\
		if (resStr.isEmpty()) {					\
			axisParams.var.autoScale = true;	\
		}										\
		else {									\
			axisParams.var.val = resStr.replace(QLocale().decimalPoint(), QChar('.')).toDouble(); \
			axisParams.var.autoScale = false;	\
		}

	if (!dialogCanceled) {
		GET_VAL(minLed, min);
		GET_VAL(maxLed, max);
		GET_VAL(stepLed, step);
	}

	foreach(auto conn, dialogConn) {
		QObject::disconnect(conn);
	}

	dialog->deleteLater();

	return !dialogCanceled;
}
bool MainWindowUI::ApplyNewAxisParams(QwtPlot::Axis axis, MainWindowUI::PlotHandler &handler) {
	bool ret = false;
	if ((axis == QwtPlot::yRight) && !handler.plot->axisEnabled(axis)) {
		return ret;
	}

	AxisParameters &axisParams(handler.axisParams[axis]);
	QwtPlot *plot = handler.plot;

	if (axisParams.min.autoScale && axisParams.max.autoScale && axisParams.step.autoScale) {
		if (!plot->axisAutoScale(axis)) {
			plot->setAxisAutoScale(axis);
			ret = true;
		}
	}
	else {
		double step = axisParams.step.autoScale ? (double)0.0 : axisParams.step.val;
		double min;
		double max;

		if (!axisParams.min.autoScale) {
			min = axisParams.min.val;
		}
		if (!axisParams.max.autoScale) {
			max = axisParams.max.val;
		}

		for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
			double curMin;
			double curMax;

			switch (axis) {
				case QwtPlot::xBottom:
				case QwtPlot::xTop:
					if (handler.plot->axisEnabled(QwtPlot::yRight)) {
						curMin = qMin(it->curve1->minXValue(), it->curve2->minXValue());
						curMax = qMax(it->curve1->maxXValue(), it->curve2->maxXValue());
					}
					else {
						curMin = it->curve1->minXValue();
						curMax = it->curve1->maxXValue();
					}
					break;
				case QwtPlot::yLeft:
					curMin = it->curve1->minYValue();
					curMax = it->curve1->maxYValue();
					break;
				case QwtPlot::yRight:
					curMin = it->curve2->minYValue();
					curMax = it->curve2->maxYValue();
					break;
			}

			if (it == handler.data.begin()) {
				if (axisParams.min.autoScale) {
					min = curMin;
				}
				if (axisParams.max.autoScale) {
					max = curMax;
				}
				continue;
			}
			if (axisParams.min.autoScale) {
				if (curMin < min) {
					min = curMin;
				}
			}

			if (axisParams.max.autoScale) {
				if (curMax > max) {
					max = curMax;
				}
			}
		}

		if (plot->axisAutoScale(axis) ||
			(min != plot->axisInterval(axis).minValue()) ||
			(max != plot->axisInterval(axis).maxValue()) ||
			(step != plot->axisStepSize(axis))) {
			plot->setAxisScale(axis, min, max, step);
			ret = true;
		}
	}
	/*
	else if (!axisParams.min.autoScale && !axisParams.max.autoScale) {
		double step = axisParams.step.autoScale ? (double)0.0 : axisParams.step.val;
		double min = axisParams.min.val;
		double max = axisParams.max.val;

		if (plot->axisAutoScale(axis) ||
			(min != plot->axisInterval(axis).minValue()) ||
			(max != plot->axisInterval(axis).maxValue()) ||
			(step != plot->axisStepSize(axis))) {
			plot->setAxisScale(axis, min, max, step);
			ret = true;
		}
	}
	else if (axisParams.min.autoScale) {
		double step = axisParams.step.autoScale ? (double)0.0 : axisParams.step.val;
		double min;
		double max = axisParams.max.val;
		for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
			double curMin;
			switch (axis) {
				case QwtPlot::xBottom:
				case QwtPlot::xTop:
					curMin = qMin(it->curve1->minXValue(), it->curve2->minXValue());
					break;
				case QwtPlot::yLeft:
					curMin = it->curve1->minYValue();
					break;
				case QwtPlot::yRight:
					curMin = it->curve2->minYValue();
					break;
			}

			if (it == handler.data.begin()) {
				min = curMin;
				continue;
			}

			if (curMin < min) {
				min = curMin;
			}
		}

		if (plot->axisAutoScale(axis) ||
			(min != plot->axisInterval(axis).minValue()) ||
			(max != plot->axisInterval(axis).maxValue()) ||
			(step != plot->axisStepSize(axis))) {
			plot->setAxisScale(axis, min, max, step);
			ret = true;
		}
	}
	else if (axisParams.max.autoScale) {
		double step = axisParams.step.autoScale ? (double)0.0 : axisParams.step.val;
		double max;
		double min = axisParams.min.val;
		for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
			double curMax;
			switch (axis) {
				case QwtPlot::xBottom:
				case QwtPlot::xTop:
					curMax = qMax(it->curve1->maxXValue(), it->curve2->maxXValue());
					break;
				case QwtPlot::yLeft:
					curMax = it->curve1->maxYValue();
					break;
				case QwtPlot::yRight:
					curMax = it->curve2->maxYValue();
					break;
			}

			if (it == handler.data.begin()) {
				max = curMax;
				continue;
			}

			if (curMax > max) {
				max = curMax;
			}
		}

		if (plot->axisAutoScale(axis) ||
			(min != plot->axisInterval(axis).minValue()) ||
			(max != plot->axisInterval(axis).maxValue()) ||
			(step != plot->axisStepSize(axis))) {
			plot->setAxisScale(axis, min, max, step);
			ret = true;
		}
	}
	//*/

	return ret;
}
QWidget* MainWindowUI::CreateNewDataTabWidget(const QUuid &id, ExperimentType type, const QString &expName, const QStringList &xAxisList, const QStringList &yAxisList, const DataMap *loadedContainerPtr) {
	QFont axisTitleFont("Segoe UI");
	axisTitleFont.setPixelSize(22);
	axisTitleFont.setBold(false);

	auto w = WDG();

	auto lay = NO_SPACING(NO_MARGIN(new QGridLayout(w)));

	QwtPlot *plot = OBJ_NAME(new QwtPlot(), "qwt-plot");
	plot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);


	QwtText plotTitle;
	plotTitle.setFont(axisTitleFont);
	plotTitle.setText(expName);
	plot->setTitle(plotTitle);

	QwtPlotCurve *curve1 = CreateCurve(QwtPlot::yLeft, DEFAULT_MAJOR_CURVE_COLOR);
	QwtPlotCurve *curve2 = CreateCurve(QwtPlot::yRight, DEFAULT_MINOR_CURVE_COLOR);
	curve1->attach(plot);

	auto settingsLay = NO_SPACING(NO_MARGIN(new QGridLayout));

	settingsLay->addWidget(OBJ_NAME(new QLabel(expName), "heading-label"), 0, 0, 1, -1);
	settingsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("X - axis = "), "experiment-params-comment"), "comment-placement", "left"), 1, 0);
	settingsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Y1 - axis = "), "experiment-params-comment"), "comment-placement", "left"), 2, 0);
	settingsLay->addWidget(OBJ_PROP(OBJ_NAME(LBL("Y2 - axis = "), "experiment-params-comment"), "comment-placement", "left"), 3, 0);

	auto xCombo = CMB();
	QListView *xComboList = OBJ_NAME(new QListView, "combo-list");
	xCombo->setView(xComboList);
	xCombo->addItems(xAxisList);

	auto y1Combo = CMB();
	QListView *y1ComboList = OBJ_NAME(new QListView, "combo-list");
	y1Combo->setView(y1ComboList);
	y1Combo->addItems(yAxisList);

	auto y2Combo = CMB();
	QListView *y2ComboList = OBJ_NAME(new QListView, "combo-list");
	y2Combo->setView(y2ComboList);
	y2Combo->addItems(QStringList() << NONE_Y_AXIS_VARIABLE << yAxisList);

	settingsLay->addWidget(xCombo, 1, 1);
	settingsLay->addWidget(y1Combo, 2, 1);
	settingsLay->addWidget(y2Combo, 3, 1);
	settingsLay->setColumnStretch(2, 1);

	QPushButton *addDataPbt;
	QPushButton *editLinesPbt;
	QPushButton *savePlotPbt;

	auto buttonLay = new QHBoxLayout;
	buttonLay->addStretch(1);
	buttonLay->addWidget(addDataPbt = OBJ_NAME(PBT("Add a Data File(s)"), "secondary-button"));
	buttonLay->addWidget(editLinesPbt = OBJ_NAME(PBT("Edit Workers && Lines"), "secondary-button"));
	buttonLay->addWidget(savePlotPbt = OBJ_NAME(PBT("Save Plot"), "secondary-button"));
	buttonLay->addStretch(1);

	settingsLay->addWidget(OBJ_NAME(WDG(), "settings-vertical-spacing"), 4, 0, 1, -1);
	settingsLay->addLayout(buttonLay, 5, 0, 1, -1);
	settingsLay->setRowStretch(6, 1);

	lay->addWidget(OBJ_NAME(WDG(), "new-data-tab-top-spacing"), 0, 0, 1, 1);
	lay->addWidget(OBJ_NAME(WDG(), "new-data-tab-left-spacing"), 1, 0, -1, 1);
	lay->addLayout(settingsLay, 1, 1);
	lay->addWidget(plot, 0, 2, 2, 1);
	lay->setColumnStretch(1, 1);
	lay->setColumnStretch(2, 1);

	auto controlButtonLay = new QHBoxLayout;
	QPushButton *pauseExperiment;
	QPushButton *stopExperiment;

	controlButtonLay->addWidget(pauseExperiment = OBJ_NAME(PBT(PAUSE_EXP_BUTTON_TEXT), "control-button-blue"));
	controlButtonLay->addWidget(stopExperiment = OBJ_NAME(PBT("Stop Experiment"), "control-button-red"));

	if (type == ET_SAVED) {
		pauseExperiment->hide();
		stopExperiment->hide();
	}

	lay->addLayout(controlButtonLay, 2, 0, 1, -1);

	PlotHandler plotHandler;
	plotHandler.plot = plot;
	plotHandler.varCombo[QwtPlot::xBottom] = xCombo;
	plotHandler.varCombo[QwtPlot::yLeft] = y1Combo;
	plotHandler.varCombo[QwtPlot::yRight] = y2Combo;
	plotHandler.exp = 0;
	plotHandler.data << DataMapVisualization();
	plotHandler.data.first().saveFile = 0;
	plotHandler.data.first().curve1 = curve1;
	plotHandler.data.first().curve2 = curve2;
	plotHandler.plotCounter.stamp = 0;

	plot->axisWidget(QwtPlot::yLeft)->installEventFilter(new PlotEventFilter(w, [=]() {
		PlotHandler &handler(dataTabs.plots[id][type]);
		
		if (GetNewAxisParams(mw, handler.axisParams[QwtPlot::yLeft])) {
			if (ApplyNewAxisParams(QwtPlot::yLeft, handler)) {
				plot->replot();
			}
		}
	}));

	plot->axisWidget(QwtPlot::yRight)->installEventFilter(new PlotEventFilter(w, [=]() {
		PlotHandler &handler(dataTabs.plots[id][type]);

		if (GetNewAxisParams(mw, handler.axisParams[QwtPlot::yRight])) {
			if (ApplyNewAxisParams(QwtPlot::yRight, handler)) {
				plot->replot();
			}
		}
	}));

	plot->axisWidget(QwtPlot::xBottom)->installEventFilter(new PlotEventFilter(w, [=]() {
		PlotHandler &handler(dataTabs.plots[id][type]);

		if (GetNewAxisParams(mw, handler.axisParams[QwtPlot::xBottom])) {
			if(ApplyNewAxisParams(QwtPlot::xBottom, handler)) {
				plot->replot();
			}
		}
	}));

	plot->legend()->installEventFilter(new LegendEventFilter(w, plot));

	plot->canvas()->installEventFilter(new PlotEventFilter(w, [=]() {
		if (0 == plot->legend()) {
			plot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
			plot->legend()->installEventFilter(new LegendEventFilter(w, plot));
			plot->replot();
		}
	}));

	plot->titleLabel()->installEventFilter(new PlotEventFilter(w, [=]() {
		QString newTitle = GetNewTitle(mw, plot->title().text());
		if (!newTitle.isEmpty()) {
			auto titleText = plot->title();
			titleText.setText(newTitle);
			plot->setTitle(titleText);
		}
	}));

	plotHandler.plotTabConnections << CONNECT(pauseExperiment, &QPushButton::clicked, [=]() {
		if (pauseExperiment->text() == PAUSE_EXP_BUTTON_TEXT) {
			mw->PauseExperiment(id);
		}
		else {
			mw->ResumeExperiment(id);
		}
	});

	plotHandler.plotTabConnections << CONNECT(stopExperiment, &QPushButton::clicked, [=]() {
		mw->StopExperiment(id);
	});

	plotHandler.plotTabConnections << CONNECT(mw, &MainWindow::ExperimentResumed, [=](const QUuid &extId) {
		if (id != extId)
			return;

		pauseExperiment->setText(PAUSE_EXP_BUTTON_TEXT);
	});

	plotHandler.plotTabConnections << CONNECT(mw, &MainWindow::ExperimentPaused, [=](const QUuid &extId) {
		if (id != extId)
			return;

		pauseExperiment->setText(RESUME_EXP_BUTTON_TEXT);
	});

	plotHandler.plotTabConnections << CONNECT(mw, &MainWindow::ExperimentCompleted, [=](const QUuid &extId) {
		if (id != extId)
			return;

		pauseExperiment->hide();
		stopExperiment->hide();
	});

	plotHandler.plotTabConnections << CONNECT(addDataPbt, &QPushButton::clicked, [=]() {
		QList<CsvFileData> csvDataList;

		if (!ReadCsvFile(mw, csvDataList)) {
			return;
		}

		PlotHandler &handler(dataTabs.plots[id][type]);
		QStringList firstDataKeys = handler.data.first().container.keys();
		firstDataKeys.removeAll(NONE_Y_AXIS_VARIABLE);
		firstDataKeys.sort();
		
		foreach(auto csvData, csvDataList) {
			QStringList curDataKeys = csvData.container.keys();
			curDataKeys.sort();

			if (curDataKeys != firstDataKeys) {
				LOG() << "Incompatible data sets were selected!";
				return;
			}
		}

		foreach(auto csvData, csvDataList) {
			DataMapVisualization data;
			data.container = csvData.container;
			data.saveFile = 0;
			data.curve1 = CreateCurve(QwtPlot::yLeft, DEFAULT_MAJOR_CURVE_COLOR);
			data.curve2 = CreateCurve(QwtPlot::yRight, DEFAULT_MINOR_CURVE_COLOR);
			data.name = csvData.fileName;

			handler.data << data;

			DataMapVisualization &currentData(handler.data.last());
			currentData.data[QwtPlot::xBottom] = &currentData.container[handler.varCombo[QwtPlot::xBottom]->currentText()];
			currentData.data[QwtPlot::yLeft] = &currentData.container[handler.varCombo[QwtPlot::yLeft]->currentText()];
			currentData.data[QwtPlot::yRight] = &currentData.container[handler.varCombo[QwtPlot::yRight]->currentText()];

			currentData.curve1->setSamples(new ListSeriesData(*currentData.data[QwtPlot::xBottom], *currentData.data[QwtPlot::yLeft]));
			currentData.curve2->setSamples(new ListSeriesData(*currentData.data[QwtPlot::xBottom], *currentData.data[QwtPlot::yRight]));

			currentData.curve1->setTitle(handler.varCombo[QwtPlot::yLeft]->currentText());
			currentData.curve2->setTitle(handler.varCombo[QwtPlot::yRight]->currentText());

			currentData.curve1->attach(handler.plot);

			if (handler.varCombo[QwtPlot::yRight]->currentText() != NONE_Y_AXIS_VARIABLE) {
				currentData.curve2->attach(handler.plot);
			}

			handler.plot->replot();
		}
	});

	plotHandler.plotTabConnections << CONNECT(editLinesPbt, &QPushButton::clicked, [=]() {
		PlotHandler &handler(dataTabs.plots[id][type]);
		QMap<QString, CurveParameters> currentParams;

		foreach(const DataMapVisualization &data, handler.data) {
			currentParams[data.name].curve[CurveParameters::PRIMARY].pen.color = data.curve1->pen().color();
			currentParams[data.name].curve[CurveParameters::PRIMARY].pen.width = data.curve1->pen().width();
			currentParams[data.name].curve[CurveParameters::PRIMARY].pen.style = data.curve1->pen().style();
			currentParams[data.name].curve[CurveParameters::PRIMARY].style = data.curve1->style();
			if (data.curve1->symbol()) {
				currentParams[data.name].curve[CurveParameters::PRIMARY].symbol.width = data.curve1->symbol()->size().width();
				currentParams[data.name].curve[CurveParameters::PRIMARY].symbol.style = data.curve1->symbol()->style();
			}
			else {
				currentParams[data.name].curve[CurveParameters::PRIMARY].symbol.width = 10;
				currentParams[data.name].curve[CurveParameters::PRIMARY].symbol.style = QwtSymbol::NoSymbol;
			}

			currentParams[data.name].curve[CurveParameters::SECONDARY].pen.color = data.curve2->pen().color();
			currentParams[data.name].curve[CurveParameters::SECONDARY].pen.width = data.curve2->pen().width();
			currentParams[data.name].curve[CurveParameters::SECONDARY].pen.style = data.curve2->pen().style();
			currentParams[data.name].curve[CurveParameters::SECONDARY].style = data.curve2->style();
			if (data.curve2->symbol()) {
				currentParams[data.name].curve[CurveParameters::SECONDARY].symbol.width = data.curve2->symbol()->size().width();
				currentParams[data.name].curve[CurveParameters::SECONDARY].symbol.style = data.curve2->symbol()->style();
			}
			else {
				currentParams[data.name].curve[CurveParameters::SECONDARY].symbol.width = 10;
				currentParams[data.name].curve[CurveParameters::SECONDARY].symbol.style = QwtSymbol::NoSymbol;
			}
		}

		if (GetNewPen(mw, currentParams)) {
			foreach(const DataMapVisualization &data, handler.data) {
				data.curve1->setPen(currentParams[data.name].curve[CurveParameters::PRIMARY].pen.color,
					currentParams[data.name].curve[CurveParameters::PRIMARY].pen.width,
					currentParams[data.name].curve[CurveParameters::PRIMARY].pen.style);
				data.curve2->setPen(currentParams[data.name].curve[CurveParameters::SECONDARY].pen.color,
					currentParams[data.name].curve[CurveParameters::SECONDARY].pen.width,
					currentParams[data.name].curve[CurveParameters::SECONDARY].pen.style);
				data.curve1->setStyle(currentParams[data.name].curve[CurveParameters::PRIMARY].style);
				data.curve2->setStyle(currentParams[data.name].curve[CurveParameters::SECONDARY].style);

				auto symbol = new QwtSymbol(currentParams[data.name].curve[CurveParameters::PRIMARY].symbol.style);
				symbol->setPen(currentParams[data.name].curve[CurveParameters::PRIMARY].pen.color, 1, Qt::SolidLine);
				symbol->setColor(currentParams[data.name].curve[CurveParameters::PRIMARY].pen.color);
				symbol->setSize(currentParams[data.name].curve[CurveParameters::PRIMARY].symbol.width);
				data.curve1->setSymbol(symbol);

				symbol = new QwtSymbol(currentParams[data.name].curve[CurveParameters::SECONDARY].symbol.style);
				symbol->setPen(currentParams[data.name].curve[CurveParameters::SECONDARY].pen.color, 1, Qt::SolidLine);
				symbol->setColor(currentParams[data.name].curve[CurveParameters::SECONDARY].pen.color);
				symbol->setSize(currentParams[data.name].curve[CurveParameters::SECONDARY].symbol.width);
				data.curve2->setSymbol(symbol);
			}
			handler.plot->replot();
		}
	});

	plotHandler.plotTabConnections << CONNECT(savePlotPbt, &QPushButton::clicked, [=]() {
		QSettings settings(SQUID_STAT_PARAMETERS_INI, QSettings::IniFormat);
		QString dirName = settings.value(DATA_SAVE_PATH, "").toString();

		QString fileName = QFileDialog::getSaveFileName(mw, "Saving plot", dirName, "Image file (*.png)");

		if (fileName.isEmpty()) {
			return;
		}

		settings.setValue(DATA_SAVE_PATH, QFileInfo(fileName).absolutePath());

		if (!plot->grab().save(fileName, "PNG")) {
			LOG() << "Error during saving plot into \"" << fileName << "\"";
		}
	});

	plotHandler.plotTabConnections << CONNECT(xCombo, &QComboBox::currentTextChanged, [=](const QString &curText) {
		PlotHandler &handler(dataTabs.plots[id][type]);

		QwtText title;
		title.setFont(axisTitleFont);
		title.setText(curText);
		handler.plot->setAxisTitle(QwtPlot::xBottom, title);

		for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
			it->data[QwtPlot::xBottom] = &it->container[curText];
			it->curve1->setSamples(new ListSeriesData(*it->data[QwtPlot::xBottom], *it->data[QwtPlot::yLeft]));
			it->curve2->setSamples(new ListSeriesData(*it->data[QwtPlot::xBottom], *it->data[QwtPlot::yRight]));
		}
		handler.plot->replot();
	});

	plotHandler.plotTabConnections << CONNECT(y1Combo, &QComboBox::currentTextChanged, [=](const QString &curText) {
		PlotHandler &handler(dataTabs.plots[id][type]);

		QwtText title;
		title.setFont(axisTitleFont);
		title.setText(curText);
		handler.plot->setAxisTitle(QwtPlot::yLeft, title);

		for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
			it->data[QwtPlot::yLeft] = &it->container[curText];
			it->curve1->setSamples(new ListSeriesData(*it->data[QwtPlot::xBottom], *it->data[QwtPlot::yLeft]));
			it->curve1->setTitle(curText);
		}
		handler.plot->replot();
	});

	plotHandler.plotTabConnections << CONNECT(y2Combo, &QComboBox::currentTextChanged, [=](const QString &curText) {
		PlotHandler &handler(dataTabs.plots[id][type]);

		QwtText title;
		title.setFont(axisTitleFont);
		title.setText(curText);
		handler.plot->setAxisTitle(QwtPlot::yRight, title);

		if (curText == NONE_Y_AXIS_VARIABLE) {
			handler.plot->enableAxis(QwtPlot::yRight, false);
			for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
				it->curve2->detach();
			}
		}
		else {
			handler.plot->enableAxis(QwtPlot::yRight);
			for (auto it = handler.data.begin(); it != handler.data.end(); ++it) {
				it->curve2->attach(handler.plot);

				it->data[QwtPlot::yRight] = &it->container[curText];

				it->curve2->setSamples(new ListSeriesData(*it->data[QwtPlot::xBottom], *it->data[QwtPlot::yRight]));
				it->curve2->setTitle(curText);
			}
		}

		handler.plot->replot();
	});

	dataTabs.plots[id][type] = plotHandler;
	DataMapVisualization &majorData(dataTabs.plots[id][type].data.first());

	if (loadedContainerPtr) {
		majorData.container = *loadedContainerPtr;
	}
	majorData.data[QwtPlot::xBottom] = &majorData.container[xCombo->currentText()];
	majorData.data[QwtPlot::yLeft] = &majorData.container[y1Combo->currentText()];
	majorData.data[QwtPlot::yRight] = &majorData.container[NONE_Y_AXIS_VARIABLE];
	majorData.curve1->setSamples(new ListSeriesData(*majorData.data[QwtPlot::xBottom], *majorData.data[QwtPlot::yLeft]));
	majorData.curve2->setSamples(new ListSeriesData(*majorData.data[QwtPlot::xBottom], *majorData.data[QwtPlot::yRight]));
	majorData.curve1->setTitle(y1Combo->currentText());
	majorData.curve2->setTitle(NONE_Y_AXIS_VARIABLE);
	majorData.name = expName;

	QwtText title;
	
	title.setFont(axisTitleFont);

	title.setText(xCombo->currentText());
	plot->setAxisTitle(QwtPlot::xBottom, title);

	//title.setText(QString("Impedance (`") + QChar(0x03a9) + QString(")"));
	title.setText(y1Combo->currentText());
	plot->setAxisTitle(QwtPlot::yLeft, title);

	title.setText(NONE_Y_AXIS_VARIABLE);
	plot->setAxisTitle(QwtPlot::yRight, title);
	
	return w;
}