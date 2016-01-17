#include "portdialog.hh"
#include <QSerialPortInfo>
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPushButton>


NewDeviceDialog::NewDeviceDialog(QWidget *parent) :
  QDialog(parent)
{
  setWindowTitle(tr("Configure a new device"));
  setMinimumWidth(460);

  _name = new QLineEdit(tr("Device name"));
  _name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  _ports = new QComboBox();
  _ports->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  _secret = new QLineEdit();
  _secret->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

  foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
    _ports->addItem(info.portName(), info.systemLocation());
  }

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QFormLayout *layout = new QFormLayout();
  layout->addRow(tr("Name"), _name);
  layout->addRow(tr("Interface"), _ports);
  layout->addRow(tr("Secret"), _secret);
  layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

  QVBoxLayout *box = new QVBoxLayout();
  box->addLayout(layout);
  box->addWidget(buttonBox);
  setLayout(box);
}

QString
NewDeviceDialog::systemLocation() const {
  return _ports->currentData().toString();
}

QString
NewDeviceDialog::name() const {
  return _name->text();
}

QByteArray
NewDeviceDialog::secret() const {
  return QByteArray::fromHex(_secret->text().toLocal8Bit());
}


DeviceDialog::DeviceDialog(QSettings &settings, QWidget *parent)
  : QDialog(parent), _settings(settings)
{
  setWindowTitle(tr("Select a device"));
  setMinimumWidth(340);

  QLabel *label = new QLabel(tr("Select a device:"));
  QFont font = label->font();
  font.setPointSize(18);
  label->setFont(font);

  _devices = new QComboBox();
  int size = _settings.beginReadArray("devices");
  for (int i=0; i<size; i++) {
    _settings.setArrayIndex(i);
    _devices->addItem(_settings.value("name").toString(), QVariant(i));
  }
  _settings.endArray();

  QPushButton *add = new QPushButton("+");

  QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  QHBoxLayout *selBox = new QHBoxLayout();
  selBox->addWidget(_devices,1);
  selBox->addWidget(add,0);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(label);
  layout->addLayout(selBox);
  layout->addWidget(bbox);
  setLayout(layout);

  QObject::connect(bbox, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
  QObject::connect(add, SIGNAL(clicked(bool)), this, SLOT(onNewDevice()));
}

void
DeviceDialog::onNewDevice() {
  NewDeviceDialog dialog;
  if (QDialog::Accepted != dialog.exec()) { return; }

  int idx = _devices->count();
  _devices->addItem(dialog.name(), idx);

  _settings.beginWriteArray("devices");
  _settings.setArrayIndex(idx);
  _settings.setValue("name", dialog.name());
  _settings.setValue("port", dialog.systemLocation());
  _settings.setValue("secret", dialog.secret().toHex());
  _settings.endArray();

  _devices->setCurrentIndex(idx);
}

QString
DeviceDialog::name() const {
  return _devices->currentText();
}

QString
DeviceDialog::systemLocation() const {
  _settings.beginReadArray("devices");
  _settings.setArrayIndex(_devices->currentIndex());
  QString loc = _settings.value("port").toString();
  _settings.endArray();
  return loc;
}

QByteArray
DeviceDialog::secret() const {
  _settings.beginReadArray("devices");
  _settings.setArrayIndex(_devices->currentIndex());
  QByteArray sec = QByteArray::fromHex(_settings.value("secret").toString().toLocal8Bit());
  _settings.endArray();
  return sec;
}
