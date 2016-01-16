#ifndef PORTDIALOG_HH
#define PORTDIALOG_HH

#include <QDialog>
#include <QComboBox>
#include <QSettings>
#include <QLineEdit>

class NewDeviceDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NewDeviceDialog(QWidget *parent = 0);

  QString name() const;
  QString systemLocation() const;
  QByteArray secret() const;

protected:
  QLineEdit *_name;
  QComboBox *_ports;
  QLineEdit *_secret;
};


class DeviceDialog: public QDialog
{
  Q_OBJECT

public:
  DeviceDialog(QSettings &settings, QWidget *parent=0);

  QString name() const;
  QString systemLocation() const;
  QByteArray secret() const;

protected slots:
  void onNewDevice();

protected:
  QSettings &_settings;
  QComboBox *_devices;
};

#endif // PORTDIALOG_HH
