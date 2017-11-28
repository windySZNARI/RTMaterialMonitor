#pragma once

#include <QtWidgets/QDialog>
#include "ui_RTMaterialMonitor.h"
#include "rtPageManager.h"

class RTMaterialMonitor : public QDialog
{
	Q_OBJECT

public:
	RTMaterialMonitor(QWidget *parent = Q_NULLPTR);
	~RTMaterialMonitor();

	void startClient();

private:
	Ui::RTMaterialMonitorClass ui;
	rtPageManager* m_pPageMgr;
};
