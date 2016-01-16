#include "mainwindow.hh"
#include <QAction>
#include <QToolBar>
#include <QTableView>
#include <QSlider>
#include <QHeaderView>

#include "dayofweekdelegate.hh"
#include "configdialog.hh"
#include "alarmsettingwidget.hh"

MainWindow::MainWindow(Dawn &dawn, QWidget *parent) :
  QMainWindow(parent), _dawn(dawn)
{
  setMinimumSize(460, 320);
  setWindowTitle(tr("Dawn control"));
  setWindowIcon(QIcon("://icon.png"));

  QToolBar *toolbar = this->addToolBar(tr("Toolbar"));

  QAction *config = toolbar->addAction(QIcon(":/settings.png"), tr("Config"));

  QSlider *bright = new QSlider(Qt::Horizontal);
  bright->setMinimum(0);
  bright->setMaximum(0xffff);
  bright->setValue(_dawn.value());
  toolbar->addWidget(bright);

  QAction *quit = toolbar->addAction(QIcon(":/quit.png"), tr("Quit"));

  AlarmSettingsWidget *table = new AlarmSettingsWidget(_dawn);
  setCentralWidget(table);

  QObject::connect(quit, SIGNAL(triggered()), this, SLOT(close()));
  QObject::connect(config, SIGNAL(triggered()), this, SLOT(onConfig()));
  QObject::connect(bright, SIGNAL(valueChanged(int)), this, SLOT(onSetBrightness(int)));
}


void
MainWindow::onConfig() {
  ConfigDialog(_dawn).exec();
}

void
MainWindow::onSetBrightness(int value) {
  value = std::max(0, std::min(value, 0xffff));
  _dawn.setValue(value);
}
