#include "controllerconfigdialog.h"
#include "ui_controllerconfigdialog.h"

#include <QRegularExpression>
#include <QLabel>
#include <QFile>
#include <QAbstractButton>
#include <QPushButton>
#include <QInputDialog> 

#include "AppConfig.h"
#include "PreferenceDefs.h"
#include "PathUtils.h"
#include "bindingmodel.h"
#include "ControllerInfo.h"
#include "inputeventselectiondialog.h"
#include "QStringUtils.h"

#include <boost/filesystem.hpp>
#include <iostream>

ControllerConfigDialog::ControllerConfigDialog(CInputBindingManager* inputBindingManager, CInputProviderQtKey* qtKeyInputProvider, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ControllerConfigDialog)
    , m_inputManager(inputBindingManager)
    , m_qtKeyInputProvider(qtKeyInputProvider)
{
	ui->setupUi(this);
	std::string profile = CAppConfig::GetInstance().GetPreferenceString(PREF_UI_GAMEPAD1_PROFILE);

	PrepareBindingsView();

	auto path = CGamePadConfig::GetProfilePath();
	if(boost::filesystem::is_directory(path))
	{
		for(auto& entry : boost::filesystem::directory_iterator(path))
		{
			auto profile = Framework::PathUtils::GetNativeStringFromPath(entry.path().stem());
			ui->comboBox->addItem(profile.c_str());
		}
	}

	auto index = ui->comboBox->findText(profile.c_str());
	if(index >= 0)
		ui->comboBox->setCurrentIndex(index);

}

ControllerConfigDialog::~ControllerConfigDialog()
{
	delete ui;
}

void ControllerConfigDialog::PrepareBindingsView()
{
	CBindingModel* model = new CBindingModel(this);
	model->Setup(m_inputManager);
	model->setHeaderData(0, Qt::Orientation::Horizontal, QVariant("Button"), Qt::DisplayRole);
	model->setHeaderData(1, Qt::Orientation::Horizontal, QVariant("Binding Type"), Qt::DisplayRole);
	model->setHeaderData(2, Qt::Orientation::Horizontal, QVariant("Binding Value"), Qt::DisplayRole);
	ui->tableView->setModel(model);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
	ui->tableView->resizeColumnsToContents();
}

void ControllerConfigDialog::on_buttonBox_clicked(QAbstractButton* button)
{
	if(button == ui->buttonBox->button(QDialogButtonBox::Ok))
	{
		m_inputManager->Save();
		CAppConfig::GetInstance().Save();
	}
	else if(button == ui->buttonBox->button(QDialogButtonBox::Apply))
	{
		m_inputManager->Save();
		CAppConfig::GetInstance().Save();
	}
	else if(button == ui->buttonBox->button(QDialogButtonBox::RestoreDefaults))
	{
		switch(ui->tabWidget->currentIndex())
		{
		case 0:
			AutoConfigureKeyboard(m_inputManager);
			static_cast<CBindingModel*>(ui->tableView->model())->Refresh();
			break;
		}
	}
	else if(button == ui->buttonBox->button(QDialogButtonBox::Cancel))
	{
		m_inputManager->Reload();
	}
}

void ControllerConfigDialog::on_tableView_doubleClicked(const QModelIndex& index)
{
	OpenBindConfigDialog(index.row());
}

void ControllerConfigDialog::on_ConfigAllButton_clicked()
{
	for(int i = 0; i < PS2::CControllerInfo::MAX_BUTTONS; ++i)
	{
		if(!OpenBindConfigDialog(i))
		{
			break;
		}
	}
}

void ControllerConfigDialog::AutoConfigureKeyboard(CInputBindingManager* bindingManager)
{
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::START, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Return));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::SELECT, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Backspace));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::DPAD_LEFT, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Left));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::DPAD_RIGHT, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Right));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::DPAD_UP, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Up));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::DPAD_DOWN, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Down));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::SQUARE, CInputProviderQtKey::MakeBindingTarget(Qt::Key_A));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::CROSS, CInputProviderQtKey::MakeBindingTarget(Qt::Key_Z));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::TRIANGLE, CInputProviderQtKey::MakeBindingTarget(Qt::Key_S));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::CIRCLE, CInputProviderQtKey::MakeBindingTarget(Qt::Key_X));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::L1, CInputProviderQtKey::MakeBindingTarget(Qt::Key_1));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::L2, CInputProviderQtKey::MakeBindingTarget(Qt::Key_2));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::L3, CInputProviderQtKey::MakeBindingTarget(Qt::Key_3));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::R1, CInputProviderQtKey::MakeBindingTarget(Qt::Key_8));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::R2, CInputProviderQtKey::MakeBindingTarget(Qt::Key_9));
	bindingManager->SetSimpleBinding(0, PS2::CControllerInfo::R3, CInputProviderQtKey::MakeBindingTarget(Qt::Key_0));

	bindingManager->SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_LEFT_X,
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_F),
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_H));
	bindingManager->SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_LEFT_Y,
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_T),
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_G));

	bindingManager->SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_RIGHT_X,
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_J),
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_L));
	bindingManager->SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_RIGHT_Y,
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_I),
	                                        CInputProviderQtKey::MakeBindingTarget(Qt::Key_K));
}

int ControllerConfigDialog::OpenBindConfigDialog(int index)
{
	std::string button(PS2::CControllerInfo::m_buttonName[index]);
	std::transform(button.begin(), button.end(), button.begin(), ::toupper);

	InputEventSelectionDialog IESD(this);
	IESD.Setup(button.c_str(), m_inputManager, m_qtKeyInputProvider, static_cast<PS2::CControllerInfo::BUTTON>(index));
	auto res = IESD.exec();
	return res;
}

void ControllerConfigDialog::on_comboBox_currentIndexChanged(int index)
{
	auto profile = ui->comboBox->itemText(index).toStdString();
	std::cout << profile.c_str() << std::endl;
	m_inputManager->Load(profile.c_str());
	CAppConfig::GetInstance().SetPreferenceString(PREF_UI_GAMEPAD1_PROFILE, profile.c_str());
	if(index == 0)
	{
		AutoConfigureKeyboard(m_inputManager);
	}

	static_cast<CBindingModel*>(ui->tableView->model())->Refresh();
}

void ControllerConfigDialog::on_addProfileButton_clicked()
{
	std::string profile_name;

	while(profile_name.empty())
	{
		bool ok_pressed = false;
		QString name = QInputDialog::getText(this, tr("Enter Profile Name"), tr("Only letters, numbers and dash characters allowed.\nProfile name:"),
		QLineEdit::Normal, "", &ok_pressed);

		// if cancel, stop there or empty entry
		if(!ok_pressed || name.isEmpty())
			return;

		// TODO filter text remove invalid path character
		profile_name = name.toStdString();
	}

	{
		auto profile_path = CGamePadConfig::GetProfile(profile_name);
		if(!boost::filesystem::exists(profile_path))
		{
			ui->comboBox->addItem(profile_name.c_str());
		}

		auto index = ui->comboBox->findText(profile_name.c_str());
		if(index >= 0)
			ui->comboBox->setCurrentIndex(index);
	}
}

void ControllerConfigDialog::on_delProfileButton_clicked()
{

	int index = ui->comboBox->currentIndex();
	if(index == -1)
		//TODO invalid selection
		return;
	
	auto name = ui->comboBox->currentText();
	if(name.isEmpty() || name == "default")
	{
		//TODO allow to delete default? it will be created again anyway
		return;
	}
	
	std::string profile_name = name.toStdString();
	{
		auto profile_path = CGamePadConfig::GetProfile(profile_name);
		if(boost::filesystem::exists(profile_path))
		{
			boost::filesystem::remove(profile_path);
			ui->comboBox->removeItem(index);
		}
	}
}