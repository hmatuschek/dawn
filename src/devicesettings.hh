#ifndef DEVICESETTINGS_HH
#define DEVICESETTINGS_HH

#include <QHash>
#include <QString>
#include <QStringList>
#include <QByteArray>


/** Represents the list of configured devices. */
class DeviceSettings
{
public:
  /** Represents a single configured device. */
  class Device {
  public:
    /** Empty constructor. */
    Device();
    /** Constructs a device settings from the given name, port and secret. */
    Device(const QString &name, const QString &device, const QByteArray secret);
    /** Copy constructor. */
    Device(const Device &other);

    /** Assignment. */
    Device &operator= (const Device &other);

    /** Returns the name of the device setting. */
    const QString &name() const;
    /** Reuturns the port of the device. */
    const QString &device() const;
    /** Returns the secret shared with the device. */
    const QByteArray &secret() const;

  protected:
    /** Device name. */
    QString _name;
    /** Device port. */
    QString _device;
    /** Shared secret. */
    QByteArray _secret;
  };

public:
  /** Reads the device settings. */
  DeviceSettings();

  /** Returns true if the list contains at least one device. */
  bool hasDevices() const;
  /** Returns true if the list contains the given device. */
  bool hasDevice(const QString &name) const;
  /** Retunrs the device names. */
  QStringList devices() const;
  /** Retunrs the given device. */
  Device device(const QString &name) const;
  /** Adds a device to the list. */
  void add(const QString &name, const QString &device, const QByteArray &secret);

protected:
  /** The device table. */
  QHash<QString, Device> _devices;
};

#endif // DEVICESETTINGS_HH
