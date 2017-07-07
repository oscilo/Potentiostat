#ifndef NORMALPULSEVOLTAMMETRYPLUGIN_H
#define NORMALPULSEVOLTAMMETRYPLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

class NormalPulseVoltammetryPlugin : public QObject, public QDesignerCustomWidgetInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface" FILE "cyclicvoltammetryplugin.json")
	Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
	NormalPulseVoltammetryPlugin(QObject *parent = 0);

	bool isContainer() const;
	bool isInitialized() const;
	QIcon icon() const;
	QString domXml() const;
	QString group() const;
	QString includeFile() const;
	QString name() const;
	QString toolTip() const;
	QString whatsThis() const;
	QWidget *createWidget(QWidget *parent);
	void initialize(QDesignerFormEditorInterface *core);

private:
	bool initialized;
};

#endif // NORMALPULSEVOLTAMMETRYPLUGIN_H
