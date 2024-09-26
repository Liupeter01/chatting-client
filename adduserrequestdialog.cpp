#include "adduserrequestdialog.h"
#include "onceclickableqlabel.h"
#include "tools.h"
#include "ui_adduserrequestdialog.h"
#include <QDebug>
#include <QScrollBar>
#include <algorithm>

AddUserRequestDialog::AddUserRequestDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AddUserRequestDialog),
      m_existing_cur_pos(
          QPoint(COMPENSATION_WIDTH,
                 COMPENSATION_HEIGHT)) /*init existing tag current pos*/
      ,
      m_selected_cur_pos(
          QPoint(COMPENSATION_WIDTH, COMPENSATION_HEIGHT)) /*init selected tag*/
{
  ui->setupUi(this);

  /*register signal<->slot*/
  registerSignal();

  /*set up placeholder text*/
  setupDefaultInfo();

  /*set up ui style*/
  setupWindowStyle();

  /*load image for usertag widget*/
  Tools::loadImgResources({"unselect_tag.png"},
                          UserTagWidget::getImageSize().width(),
                          UserTagWidget::getImageSize().height());
}

AddUserRequestDialog::~AddUserRequestDialog() { delete ui; }

void AddUserRequestDialog::registerSignal() {
  /*bind with show more label click event*/
  connect(ui->show_more_label, &OnceClickableQLabel::clicked, this,
          &AddUserRequestDialog::slot_show_more_label);

  /*bind with press enter inside tag_input widget*/
  connect(ui->tag_input, &RestrictUserSearchingInput::returnPressed, this,
          &AddUserRequestDialog::slot_input_tag_press_enter);

  /*bind with textchange inside tag_input*/
  connect(ui->tag_input, &RestrictUserSearchingInput::textChanged, this,
          &AddUserRequestDialog::slot_input_tag_textchange);

  /*bind with editfinished event inside tag_input*/
  connect(ui->tag_input, &RestrictUserSearchingInput::editingFinished, this,
          &AddUserRequestDialog::slot_input_tag_finished);

  // connect(ui->tag_display, &OnceClickableQLabel::clicked, this,
  // &AddUserRequestDialog::slo)
}

void AddUserRequestDialog::setupDefaultInfo() {

  /**limit user input ammount to 21**/
  ui->nick_name_edit->setMaxLength(21);
  ui->request_msg_edit->setMaxLength(21);
  ui->tag_input->setMaxLength(21);

  /*input label
   * 1. setup start pos
   * 2. setup widget height
   */
  ui->tag_input->move(2, 2);
  ui->tag_input->setFixedHeight(36);

  /*hide status display bar*/
  ui->user_tag_display_bar->hide();

  ui->nick_name_edit->setPlaceholderText(QString("I'am test_friend"));
  ui->request_msg_edit->setPlaceholderText(QString("I'am test_sender"));
  ui->tag_input->setPlaceholderText(QString("Select or search tags"));
}

void AddUserRequestDialog::setupWindowStyle() {
  /*hide dialog title*/
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

  ui->scrollArea->horizontalScrollBar()->setHidden(true);
  ui->scrollArea->verticalScrollBar()->setHidden(true);
  ui->scrollArea->installEventFilter(this);

  this->setModal(true);
}

bool AddUserRequestDialog::eventFilter(QObject *object, QEvent *event) {
  if (object == ui->scrollArea) {
    if (event->type() == QEvent::Enter) {
      ui->scrollArea->verticalScrollBar()->setHidden(false);
    } else if (event->type() == QEvent::Leave) {
      ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
  }
  return QDialog::eventFilter(object, event);
}

void AddUserRequestDialog::addNewTag2Container(const QString &text) {
  /*store all tags*/
  auto it = std::find(m_tagLists.begin(), m_tagLists.end(), text);
  if (it == m_tagLists.end()) {
    m_tagLists.push_back(text);
  }
}

void AddUserRequestDialog::addNewTag2InputTag(const QString &text) {
  /*clear input text*/
  ui->tag_input->clear();

  /*we find it inside select key*/
  auto it = std::find(m_selected_key.begin(), m_selected_key.end(), text);
  if (it != m_selected_key.end()) {
    return;
  }

  /*create a new widget*/
  UserTagWidget *tag(new UserTagWidget(ui->input_tag_widget));
  tag->setTagName(text);
  tag->setObjectName("UserDefTag");

  // connect signal slot for close signal
  connect(tag, &UserTagWidget::signal_close, this,
          &AddUserRequestDialog::slot_remove_selected_tag);

  /*add new tag to the vector container*/
  createSelectedTag(text, tag);

  /*calculate do we need to generate this object at a new line*/
  if (m_selected_cur_pos.x() + tag->width() > ui->input_tag_widget->width()) {
    m_selected_cur_pos.setX(COMPENSATION_WIDTH);
    m_selected_cur_pos.setY(m_selected_cur_pos.y() + tag->height() +
                            COMPENSATION_HEIGHT);
  }

  /*move it to new place*/
  tag->move(m_selected_cur_pos);
  tag->show();

  /*update m_selected_cur_pos to the next new place*/
  m_selected_cur_pos.setX(m_selected_cur_pos.x() + tag->width() +
                          COMPENSATION_WIDTH);
  if (m_selected_cur_pos.x() + COMPENSATION_WIDTH >
      ui->input_tag_widget->width()) {
    /*move tag_input widget position to avoid collison with UserTagWidget*/
    m_selected_cur_pos.setX(COMPENSATION_WIDTH);
    m_selected_cur_pos.setY(m_selected_cur_pos.y() + tag->height() +
                            COMPENSATION_HEIGHT);
  }

  /*move tag_input's position, adjust height*/
  ui->tag_input->setFixedHeight(tag->height() + COMPENSATION_HEIGHT);
  ui->tag_input->move(m_selected_cur_pos);

  /*extend the height of input tag widget*/
  if (ui->input_tag_widget->height() <
      m_selected_cur_pos.y() + tag->height() + COMPENSATION_HEIGHT) {
    ui->input_tag_widget->setFixedHeight(2 * ui->input_tag_widget->height());
  }
}

void AddUserRequestDialog::addNewTag2ExistingTag(const QString &text) {
  /*we find it inside existing key*/
  auto it = std::find(m_existing_key.begin(), m_existing_key.end(), text);
  if (it != m_existing_key.end()) {
    return;
  }

  auto label = m_exist_label.find(text);
  if (label != m_exist_label.end()) {
    /*if we found this text, then set the status to selected*/
    label->second->setCurrentState(LabelState::VisiableStatus::ENABLED);
    return;
  }

  /*add a new onceclickableqlabel for display existing tag*/
  OnceClickableQLabel *exist = new OnceClickableQLabel(ui->existing_tag_widget);
  exist->setText(text);
  exist->setObjectName("exist_tag");
  exist->setCurrentState(
      LabelState::VisiableStatus::ENABLED); // set it to selected

  connect(exist, &OnceClickableQLabel::clicked, this,
          &AddUserRequestDialog::slot_change_by_existing_tag);

  /*add new widget and text to specfic data structure*/
  createExistingTag(text, exist);

  /**/
  QFontMetricsF fontMetrics(exist->font());

  auto text_width = fontMetrics.horizontalAdvance(exist->text());
  auto text_height = fontMetrics.height();

  qDebug() << "text_width = " << text_width << "\ntext_height = " << text_height
           << "\n";

  /*calculate do we need to generate this object at a new line*/
  if (m_existing_cur_pos.x() + text_width > ui->existing_tag_widget->width()) {
    m_existing_cur_pos.setX(COMPENSATION_WIDTH);
    m_existing_cur_pos.setY(m_existing_cur_pos.y() + text_height +
                            COMPENSATION_HEIGHT);
  }

  /*move it to new place*/
  exist->move(m_existing_cur_pos);
  exist->show();

  /*update m_selected_cur_pos to the newest status*/
  m_existing_cur_pos.setX(m_existing_cur_pos.x() + text_width +
                          2 * COMPENSATION_WIDTH);
  m_existing_cur_pos.setY(m_existing_cur_pos.y());
}

void AddUserRequestDialog::createSelectedTag(const QString &text,
                                             UserTagWidget *widget) {
  m_selected_key.push_back(text);
  m_selected_label.insert(std::pair<QString, std::shared_ptr<UserTagWidget>>(
      text, std::shared_ptr<UserTagWidget>(widget, [](UserTagWidget *) {})));
}

void AddUserRequestDialog::createExistingTag(const QString &text,
                                             OnceClickableQLabel *widget) {
  m_existing_key.push_back(text);
  m_exist_label.insert(std::pair<QString, std::shared_ptr<OnceClickableQLabel>>(
      text, std::shared_ptr<OnceClickableQLabel>(
                widget, [](OnceClickableQLabel *) {})));
}

void AddUserRequestDialog::closeDialog() {
  qDebug() << "closing AddUserRequestDialog";
  this->hide();
  deleteLater();
}

void AddUserRequestDialog::on_confirm_button_clicked() { closeDialog(); }

void AddUserRequestDialog::on_cancel_button_clicked() { closeDialog(); }

void AddUserRequestDialog::slot_show_more_label() {}

void AddUserRequestDialog::slot_input_tag_press_enter() {
  auto text = ui->tag_input->text();

  /*no text*/
  if (text.isEmpty()) {
    return;
  }

  /*hide input notification widget*/
  ui->user_tag_display_bar->hide();

  /*store all tags*/
  addNewTag2Container(text);

  /*add it to new tag*/
  addNewTag2InputTag(text);

  /*add new tag to exisiting existing_tag_widget*/
  addNewTag2ExistingTag(text);
}

void AddUserRequestDialog::slot_remove_selected_tag() {}

void AddUserRequestDialog::slot_change_by_existing_tag(QString str,
                                                       LabelState state) {}

void AddUserRequestDialog::slot_input_tag_textchange(const QString &text) {
  ui->tag_input->setFocus(Qt::FocusReason::ActiveWindowFocusReason);
}

void AddUserRequestDialog::slot_input_tag_finished() {}
