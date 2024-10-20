#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "tcpnetworkconnection.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_login(nullptr),
      m_register(nullptr) /*we don't need to allocate memory at the beginning*/
      ,
      m_reset(nullptr) /*we don't need to allocate memory at the beginning*/
{
  ui->setupUi(this);

  switchingToLoginDialog();

  // Just For Testing
  // emit TCPNetworkConnection::get_instance() ->
  // signal_switch_chatting_dialog();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::displayDefaultWindow(QWidget *window) {
  setCentralWidget(window);
  window->show();
}

void MainWindow::setFramelessWindow(QDialog *dialog) {
  dialog->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
}

void MainWindow::switchingToRegInterface() {
  m_register = new registerinterface(this);

  /*m_register has to be created to prevent from UB*/
  connect(m_register, &registerinterface::switchToLogin, this,
          &MainWindow::switchingToLoginDialog);

  setFixedSize(m_register->minimumSize());
  setFramelessWindow(m_register);
  displayDefaultWindow(m_register);
}

void MainWindow::switchingToLoginDialog() {
  m_login = new LoginInterface(this);

  /*connect register event*/
  connect(this->m_login, &LoginInterface::switchWindow, this,
          &MainWindow::switchingToRegInterface);

  /*connect retset account info event*/
  connect(this->m_login, &LoginInterface::switchReset, this,
          &MainWindow::switchingToResetDialog);

  /*connect switch to chatting main frame info event*/
  connect(TCPNetworkConnection::get_instance().get(),
          &TCPNetworkConnection::signal_switch_chatting_dialog, this,
          &MainWindow::swithcingToChattingInf);

  setFixedSize(m_login->minimumSize());
  setFramelessWindow(m_login);
  displayDefaultWindow(m_login);
}

void MainWindow::switchingToResetDialog() {
  m_reset = new ResetPasswdInterface(this);

  /*when user press forgotpassword label, then switch ResetPasswdInterface*/
  connect(this->m_reset, &ResetPasswdInterface::switchToLogin, this,
          &MainWindow::switchingToLoginDialog);

  setFixedSize(m_reset->minimumSize());
  setFramelessWindow(m_reset);
  displayDefaultWindow(m_reset);
}

void MainWindow::swithcingToChattingInf() {
  m_chattingMainFrame = new ChattingDlgMainFrame(this);

  setFixedSize(m_chattingMainFrame->maximumSize());

  setFramelessWindow(m_chattingMainFrame);
  displayDefaultWindow(m_chattingMainFrame);
}
