#include "RTMaterialMonitor.h"

RTMaterialMonitor::RTMaterialMonitor(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pPageMgr = nullptr;

	startClient();
}

RTMaterialMonitor::~RTMaterialMonitor()
{
}

void RTMaterialMonitor::startClient()
{
	if (!m_pPageMgr)
	{
		m_pPageMgr = new rtPageManager();
	}

	m_pPageMgr->initOpcuaClient();
}
