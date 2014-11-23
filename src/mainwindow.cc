#include "mainwindow.hh"
#include <QAction>
#include <QToolBar>
#include <QTableView>
#include <QSlider>
#include "configdialog.hh"


MainWindow::MainWindow(Dawn &dawn, QWidget *parent) :
  QMainWindow(parent), _dawn(dawn)
{
  setMinimumSize(320, 240);
  setWindowTitle(tr("Dawn control"));

  QToolBar *toolbar = this->addToolBar(tr("Toolbar"));
  QAction *config = toolbar->addAction(tr("Config"));
  QSlider *bright = new QSlider(Qt::Horizontal);
  bright->setMinimum(0);
  bright->setMaximum(255);
  toolbar->addWidget(bright);
  QAction *quit = toolbar->addAction(tr("Quit"));

  QTableView *table = new QTableView();
  table->setModel(&_dawn);
  this->setCentralWidget(table);

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
  value = std::max(0, std::min(value, 255));
  _dawn.setValue(value);
}