#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include "dawn.hh"


class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(Dawn &dawn, QWidget *parent = 0);

protected slots:
  void onConfig();
  void onSetBrightness(int value);

protected:
  Dawn &_dawn;
};

#endif // MAINWINDOW_HH
