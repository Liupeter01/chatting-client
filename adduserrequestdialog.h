#ifndef ADDUSERREQUESTDIALOG_H
#define ADDUSERREQUESTDIALOG_H

#include "onceclickableqlabel.h"
#include "usertagwidget.h"
#include <QDialog>
#include <QString>
#include <map>
#include <vector>

namespace Ui {
class AddUserRequestDialog;
}

class AddUserRequestDialog : public QDialog {
  Q_OBJECT

public:
  explicit AddUserRequestDialog(QWidget *parent = nullptr);
  ~AddUserRequestDialog();

private:
  /*register signal<->slot*/
  void registerSignal();

  /*set up placeholder text*/
  void setupDefaultInfo();

  /*set up ui style*/
  void setupWindowStyle();

  /*customlized functions*/
  bool eventFilter(QObject *object, QEvent *event) override;

  /*add new tag to the list*/
  void addNewTag2Container(const QString &text);

  /*add new tag input tag widget*/
  void addNewTag2InputTag(const QString &text);

  /*add new tag to exisiting existing_tag_widget*/
  void addNewTag2ExistingTag(const QString &text);

  /*creat selected or existing tag by using smart pointer*/
  void createSelectedTag(const QString &text, UserTagWidget *widget);
  void createExistingTag(const QString &text, OnceClickableQLabel *widget);

  /*reset labels*/
  // void resetLabels();

  /*shutdown AddUserRequestDialog*/
  void closeDialog();

private slots:
  /*click confirm*/
  void on_confirm_button_clicked();

  /*click cancel*/
  void on_cancel_button_clicked();

  /*connect with show_more_label clickable qlabel*/
  void slot_show_more_label();

  /*user press enter at tag_input*/
  void slot_input_tag_press_enter();

  /*user press close icon on qlabel*/
  void slot_remove_selected_tag();

  /*user click existing tag*/
  void slot_change_by_existing_tag(QString str, LabelState state);

  /*user input text in tag_input, bind with textchange signal*/
  void slot_input_tag_textchange(const QString &text);

  /*user finished input text in tag_input*/
  void slot_input_tag_finished();

  /*user create a new friend tag, save it in existing by click*/
  // void slot_create_tag_by_click();

private:
  Ui::AddUserRequestDialog *ui;

  /*using a parameter to adjust height for user experience*/
  static constexpr std::size_t COMPENSATION_HEIGHT = 2;
  static constexpr std::size_t COMPENSATION_WIDTH = 2;

  /*
   * record the position of the existing label(existing_label)
   * record the position of the selected label(tag_label)
   */
  QPoint m_existing_cur_pos;
  QPoint m_selected_cur_pos;

  /*store all tags*/
  std::vector<QString> m_tagLists;

  /*labels that were already created! might store in server & local database*/
  std::vector<QString> m_existing_key;
  std::map<QString, std::shared_ptr<OnceClickableQLabel>> m_exist_label;

  /*labels that are going to be added to textedit widget*/
  std::vector<QString> m_selected_key;
  std::map<QString, std::shared_ptr<UserTagWidget>> m_selected_label;
};

#endif // ADDUSERREQUESTDIALOG_H
