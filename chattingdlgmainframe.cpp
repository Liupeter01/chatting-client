#include "chattingdlgmainframe.h"
#include "chattingcontactlist.h"
#include "chattinghistorywidget.h"
#include "loadingwaitdialog.h"
#include "tcpnetworkconnection.h"
#include "tools.h"
#include "ui_chattingdlgmainframe.h"
#include <QAction>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QPoint>
#include <QRandomGenerator>
#include <QtEndian>

ChattingDlgMainFrame::ChattingDlgMainFrame(QWidget *parent)
    : m_send_status(false) /*wait for data status is false*/
      ,
      QDialog(parent), ui(new Ui::ChattingDlgMainFrame), m_curQLabel(nullptr),
      m_dlgMode(
          ChattingDlgMode::ChattingDlgChattingMode) /*chatting mode by default*/
{
  ui->setupUi(this);

  /*register signal for ui display*/
  registerSignal();

  /*register Qaction for search edit ui widget*/
  registerSearchEditAction();

  /*register search edit signal*/
  registerSearchEditSignal();

  /* install event filter
   * clean text inside search_edit when mouse moving outside the widget area
   */
  this->installEventFilter(this);

  /*constraint the length of username when client try to search*/
  ui->search_user_edit->setMaxLength(20);

  /*set show list to hidden status*/
  // ui->show_lists->setHidden(true);

  /*chat list test*/
  addItemToChatListTest();

  /*after switch status, then switch window*/
  switchRelevantListWidget();

  /*show chatting page as default*/
  switchChattingPage();

  /*load qicon for chatting main frame*/
  Tools::loadIconResources({"add_friend_normal.png", "add_friend_hover.png",
                            "add_friend_clicked.png"});

  /*set default button icon*/
  Tools::setPushButtonIcon(ui->search_user_button, "add_friend_normal.png");

  /*load qimage for side bar*/
  Tools::loadImgResources({"chat_icon_normal.png", "chat_icon_hover.png",
                           "chat_icon_clicked.png", "contact_list_normal.png",
                           "contact_list_hover.png",
                           "contact_list_clicked.png"},
                          (ui->my_chat->width() + ui->my_chat->width()) / 2,
                          (ui->my_chat->height() + ui->my_chat->height()) / 2);

  /*set chatting page as default*/
  Tools::setQLableImage(ui->my_chat, "chat_icon_normal.png");
  Tools::setQLableImage(ui->my_contact, "contact_list_normal.png");

  emit ui->my_chat->clicked();

  /*add label to global control*/
  addLabel(ui->my_chat);
  addLabel(ui->my_contact);
}

void ChattingDlgMainFrame::registerSignal() {
  /*when the text input changed inside search widget, then trigger slot and
   * switch list widget*/
  connect(ui->search_user_edit, &QLineEdit::textChanged, this,
          &ChattingDlgMainFrame::slot_search_text_changed);

  connect(ui->search_user_button, &ButtonDisplaySwitching::clicked, this,
          &ChattingDlgMainFrame::updateSearchUserButton);
  connect(ui->search_user_button, &ButtonDisplaySwitching::update_display, this,
          &ChattingDlgMainFrame::updateSearchUserButton);
  connect(ui->my_chat, &SideBarWidget::clicked, this, [this]() {
    /*update UI display*/
    this->updateMyChat();

    /*when chat button was clicked, then display chat list*/
    this->slot_display_chat_list();
  });

  connect(ui->my_chat, &SideBarWidget::update_display, this,
          &ChattingDlgMainFrame::updateMyChat);

  connect(ui->my_contact, &SideBarWidget::clicked, this, [this]() {
    /*update UI display*/
    this->updateMyContact();

    /*when contact button was clicked, then display contact list*/
    this->slot_display_contact_list();
  });

  connect(ui->my_contact, &SideBarWidget::update_display, this,
          &ChattingDlgMainFrame::updateMyContact);

  connect(ui->chat_list, &MainFrameShowLists::signal_load_more_record, this,
          &ChattingDlgMainFrame::slot_load_more_record);

  connect(ui->contact_list, &ChattingContactList::signal_switch_addnewuser,
          this, &ChattingDlgMainFrame::switchNewUserPage);

  /*connect signal<->slot when item was clicked in the QListWidget*/
  connect(ui->search_list, &QListWidget::itemClicked, this,
          &ChattingDlgMainFrame::slot_list_item_clicked);

  /*connect signal<->slot when slot_search_username was triggered*/
  connect(ui->search_list, &MainFrameSearchLists::signal_waiting_for_data, this,
          &ChattingDlgMainFrame::slot_waiting_for_data);
}

void ChattingDlgMainFrame::registerSearchEditAction() {
  /*add a search icon*/
  m_searchAction = new QAction(ui->search_user_edit);
  m_searchAction->setIcon(
      Tools::loadIcon(QT_DEMO_HOME "/res/search.png").value());

  /*put it on the front position of line edit*/
  ui->search_user_edit->addAction(m_searchAction, QLineEdit::LeadingPosition);
  ui->search_user_edit->setPlaceholderText(QString("Searching"));

  /*add a transparent cancel button*/
  m_cancelAction = new QAction(ui->search_user_edit);
  m_cancelAction->setIcon(
      Tools::loadIcon(QT_DEMO_HOME "/res/close_transparent.png").value());

  /*put it on the back position of line edit*/
  ui->search_user_edit->addAction(m_cancelAction, QLineEdit::TrailingPosition);

  // connect(ui->search_user_edit, )
}

void ChattingDlgMainFrame::registerSearchEditSignal() {
  /*when user input sth, then change transparent icon to visible icon*/
  connect(ui->search_user_edit, &QLineEdit::textChanged, this,
          [this](const QString &str) {
            m_cancelAction->setIcon(
                Tools::loadIcon(str.isEmpty()
                                    ? QT_DEMO_HOME "/res/close_transparent.png"
                                    : QT_DEMO_HOME "/res/close_search.png")
                    .value());
          });

  /*when user trigger cancel button, then clear all the text*/
  connect(m_cancelAction, &QAction::triggered, [this]() {
    /*clear username search text*/
    ui->search_user_edit->clear();

    /*switch to transparent icon, because there is no input*/
    m_cancelAction->setIcon(
        Tools::loadIcon(QT_DEMO_HOME "/res/close_transparent.png").value());

    /**/
    ui->search_user_edit->clearFocus();

    /*set show list to hidden status*/
    ui->search_list->setHidden(true);
  });
}

void ChattingDlgMainFrame::updateSearchUserButton() {
  auto state = ui->search_user_button->getState();
  /*if it is selected, then it gets the highest proity*/
  if (state.select == PushButtonState::SelectedStatus::ENABLED) {
    setCursor(Qt::PointingHandCursor);
    Tools::setPushButtonIcon(ui->search_user_button, "add_friend_clicked.png");
  } else {
    /*currently, its not selected! switch to hover
     *if it is not hovered! then switch to normal
     */
    Tools::setPushButtonIcon(ui->search_user_button,

                             state.hover ==
                                     PushButtonState::HoverStatus::DISABLED
                                 ? "add_friend_normal.png"
                                 : "add_friend_hover.png");

    if (state.hover == PushButtonState::HoverStatus::ENABLED) {
      setCursor(Qt::PointingHandCursor);
    } else {
      unsetCursor();
    }
  }
}

void ChattingDlgMainFrame::switchRelevantListWidget() {
  /*accroding to m_dlgMode mode*/
  switch (m_dlgMode) {
  case ChattingDlgMode::ChattingDlgChattingMode:
    ui->chat_list->show();
    ui->contact_list->hide();
    ui->search_list->hide();
    break;
  case ChattingDlgMode::chattingDlgContactMode:
    ui->chat_list->hide();
    ui->contact_list->show();
    ui->search_list->hide();
    break;
  case ChattingDlgMode::ChattingDlgSearchingMode:
    ui->chat_list->hide();
    ui->contact_list->hide();
    ui->search_list->show();
    break;
  default:
    break;
  }
}

void ChattingDlgMainFrame::updateMyChat() {
  auto state = ui->my_chat->getState();
  if (state.visiable == LabelState::VisiableStatus::ENABLED) {

    resetAllLabels(ui->my_chat);

    setCursor(Qt::PointingHandCursor);
    Tools::setQLableImage(ui->my_chat, "chat_icon_clicked.png");
  } else {
    Tools::setQLableImage(ui->my_chat,
                          state.hover == LabelState::HoverStatus::DISABLED
                              ? "chat_icon_normal.png"
                              : "chat_icon_hover.png");

    if (state.hover == LabelState::HoverStatus::ENABLED) {
      setCursor(Qt::PointingHandCursor);
    } else {
      unsetCursor();
    }
  }
}

void ChattingDlgMainFrame::updateMyContact() {
  auto state = ui->my_contact->getState();
  if (state.visiable == LabelState::VisiableStatus::ENABLED) {

    resetAllLabels(ui->my_contact);

    setCursor(Qt::PointingHandCursor);
    Tools::setQLableImage(ui->my_contact, "contact_list_clicked.png");
  } else {
    Tools::setQLableImage(ui->my_contact,
                          state.hover == LabelState::HoverStatus::DISABLED
                              ? "contact_list_normal.png"
                              : "contact_list_hover.png");

    if (state.hover == LabelState::HoverStatus::ENABLED) {
      setCursor(Qt::PointingHandCursor);
    } else {
      unsetCursor();
    }
  }
}

void ChattingDlgMainFrame::addLabel(SideBarWidget *widget) {
  m_qlabelSet.push_back(
      std::shared_ptr<SideBarWidget>(widget, [](SideBarWidget *widget) {}));
}

void ChattingDlgMainFrame::resetAllLabels(SideBarWidget *new_widget) {
  if (m_curQLabel == nullptr) {
    m_curQLabel = new_widget;
    return;
  }
  /*user push the same button*/
  if (m_curQLabel == new_widget) {
    return;
  }
  for (auto &label : m_qlabelSet) {
    /*do not clear new_widget's status*/
    if (label.get() != new_widget) {
      label->clearState();
    }
  }

  m_curQLabel = new_widget;
}

void ChattingDlgMainFrame::clearSearchByMousePos(QMouseEvent *event) {
  /*current mode has to be SearchingMode*/
  if (m_dlgMode != ChattingDlgMode::ChattingDlgSearchingMode) {
    return;
  }

  /*get mouse position inside search list*/
  auto mousePosGlob = event->globalPosition();
  auto mouseInsideSearch = ui->search_list->mapFromGlobal(mousePosGlob);

  /*if mouse position OUTSIDE search_list, then clear search edit text*/
  if (!ui->search_list->rect().contains(mouseInsideSearch.toPoint())) {
    ui->search_user_edit->clear();
  }
}

void ChattingDlgMainFrame::slot_search_text_changed() {
  qDebug() << "Search Text Changed!";

  /*clean all QLabel state!!*/
  for (auto &label : m_qlabelSet) {
    label->clearState();
  }

  /*switch status*/
  m_dlgMode = ChattingDlgMode::ChattingDlgSearchingMode;

  /*after switch status, then switch window*/
  switchRelevantListWidget();
}

void ChattingDlgMainFrame::slot_load_more_record() {
  m_loading = std::shared_ptr<LoadingWaitDialog>(new LoadingWaitDialog(this),
                                                 [](LoadingWaitDialog *) {});

  /*do not block the execute flow*/
  m_loading->setModal(true);
  m_loading->show();

  /*load more data to the list*/
  qDebug() << "load more data to the list";

  /*test*/
  addItemToChatListTest();

  m_loading->hide();
  m_loading->deleteLater();
}

void ChattingDlgMainFrame::slot_display_chat_list() {
  qDebug() << "Chat Button Clicked!";

  /*switch status*/
  m_dlgMode = ChattingDlgMode::ChattingDlgChattingMode;

  /*after switch status, then switch window*/
  switchRelevantListWidget();

  /*switch to chatting page*/
  switchChattingPage();
}

void ChattingDlgMainFrame::slot_display_contact_list() {
  qDebug() << "Contact Button Clicked!";

  /*switch status*/
  m_dlgMode = ChattingDlgMode::chattingDlgContactMode;

  /*after switch status, then switch window*/
  switchRelevantListWidget();
}

void ChattingDlgMainFrame::slot_list_item_clicked(
    QListWidgetItem *clicked_item) {
  qDebug() << "item clicked! ";

  /*get clicked customlized widget object*/
  QWidget *widget = ui->search_list->itemWidget(clicked_item);
  if (widget == nullptr) {
    qDebug() << "invalid click item! ";
    return;
  }
  auto item = reinterpret_cast<ListItemWidgetBase *>(widget);
  if (item->getItemType() == ListItemType::Default) {
    qDebug() << "[ListItemType::Default]:list item base class!";
    return;

  } else if (item->getItemType() == ListItemType::SearchUserId) {
    qDebug() << "[ListItemType::SearchUserId]:generate add new usr window!";

    /*get username info*/
    QJsonObject json_obj;
    json_obj["username"] = ui->search_user_edit->text();
    QJsonDocument doc(json_obj);

    /*it should be store as a temporary object, because send_buffer will modify
     * it!*/
    auto json_data = doc.toJson(QJsonDocument::Compact);

    SendNode<QByteArray, std::function<uint16_t(uint16_t)>> send_buffer(
        static_cast<uint16_t>(ServiceType::SERVICE_SEARCHUSERNAME), json_data,
        [](auto x) { return qToBigEndian(x); });

    /*after connection to server, send TCP request*/
    TCPNetworkConnection::get_instance()->send_data(std::move(send_buffer));

    /*
     * waiting for server reaction
     * 1.Send username verification request to server: chattingdlgmainframe ->
     * chattingserver 2.Server responses to client's mainframesearchlist
     * framework: chattingserver -> mainframesearchlist 3.Framework send a
     * cancel waiting signal to chattingdlgmaingframs: mainframesearchlist ->
     * chattingdlgmainframe 4.Cancel waiting: slot_waiting_for_data(false);
     */
    qDebug() << "[ListItemType::SearchUserId]:Waiting For Server Response!";
    waitForDataFromRemote(true);
  }
}

ChattingDlgMainFrame::~ChattingDlgMainFrame() {
  delete m_searchAction;
  delete m_cancelAction;
  delete ui;
}

void ChattingDlgMainFrame::addItemToChatListTest() {

  for (std::size_t i = 0; i < 40; ++i) {
    auto random = QRandomGenerator::global()->bounded(10000);
    ChattingHistoryWidget *new_inserted(new ChattingHistoryWidget());
    new_inserted->setItemDisplay(QString::number(random),
                                 QT_DEMO_HOME "/res/microsoft.png",
                                 QString::number(random));

    QListWidgetItem *item(new QListWidgetItem);
    item->setSizeHint(new_inserted->sizeHint());

    ui->chat_list->addItem(item);
    ui->chat_list->setItemWidget(item, new_inserted);
    ui->chat_list->update();
  }
}

bool ChattingDlgMainFrame::eventFilter(QObject *object, QEvent *event) {
  /*mouse button press event*/
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouse(reinterpret_cast<QMouseEvent *>(event));

    /*clear search_edit according to mouse position*/
    clearSearchByMousePos(mouse);
  }
  return QDialog::eventFilter(object, event);
}

/*
 * switch to chatting page by using stackedWidget
 */
void ChattingDlgMainFrame::switchChattingPage() {
  ui->stackedWidget->setCurrentWidget(ui->chattingpage);
}

/*
 * switch to new user page by using stackedWidget
 */
void ChattingDlgMainFrame::switchNewUserPage() {
  ui->stackedWidget->setCurrentWidget(ui->newuserpage);
}

/*wait for remote server data*/
void ChattingDlgMainFrame::waitForDataFromRemote(bool status) {
  /*is still in loading*/
  if (status) {
    m_loading = std::shared_ptr<LoadingWaitDialog>(new LoadingWaitDialog(this),
                                                   [](LoadingWaitDialog *) {});
    m_loading->setModal(true);
    m_loading->show();
    m_send_status = status;
  } else {
    m_loading->hide();
    m_loading->deleteLater();
  }
}

void ChattingDlgMainFrame::slot_waiting_for_data(bool status) {
  waitForDataFromRemote(status);
}
