#ifndef CHATTINGDLGMAINFRAME_H
#define CHATTINGDLGMAINFRAME_H

#include <QDialog>
#include <QIcon>
#include <QLabel>
#include <QVector>
#include <atomic>
#include <memory>
#include <unordered_map>

class SideBarWidget;
class QMouseEvent;
class QListWidgetItem;
class LoadingWaitDialog;
struct UserNameCard;
class UserFriendRequest;
class FriendChattingHistory;
struct ChattingTextMsg;
enum class MsgType;

namespace Ui {
class ChattingDlgMainFrame;
}

class ChattingDlgMainFrame : public QDialog {
  Q_OBJECT

public:
  explicit ChattingDlgMainFrame(QWidget *parent = nullptr);
  virtual ~ChattingDlgMainFrame();

protected:
  /*chat list test*/
  void addItemToChatListTest();

  /*customlized functions*/
  bool eventFilter(QObject *object, QEvent *event) override;

private:
  void registerSignal();

  /*register action for search edit ui item*/
  void registerSearchEditAction();
  void registerSearchEditSignal();
  void updateSearchUserButton();

protected:
  /*windows switcher(accroding to m_dlgMode)*/
  void switchRelevantListWidget();

  /*register handler for chatting*/
  void updateMyChat();

  /*register handler for Contact list*/
  void updateMyContact();

  /*delegate sidebar widget*/
  void addLabel(SideBarWidget *widget);
  void resetAllLabels(SideBarWidget *new_widget);

  /*clear search_edit according to mouse position*/
  void clearSearchByMousePos(QMouseEvent *event);

  /* switch to chatting page by using stackedWidget */
  void switchChattingPage();

  /* switch to new user page by using stackedWidget */
  void switchNewUserPage();

  /*switch to user profile page by using stackedWidget*/
  void switchUserProfilePage();

  /*wait for remote server data*/
  void waitForDataFromRemote(bool status);

  /*
   * add chatting contact
   * and register current uuid to DS m_chatHisoryWidList
   */
  void addChattingHistory(std::shared_ptr<FriendChattingHistory> info);

  /*is there any existing uuid related to this QListWidgetItem*/
  bool alreadyExistInHistoryWidListList(const QString &uuid) const;

  /*load more chatting record*/
  void loadMoreChattingHistory();

  /*search in chatting history widget by using friends uuid*/
  std::optional<QListWidgetItem *>
  findChattingHistoryWidget(const QString &friend_uuid);

signals:
  void signal_log_out();

private slots:
  /*when server offline or kick out of the server*/
  void slot_connection_status(bool status);

  /*
   * waiting for data from remote server
   * status = true: activate
   * status = false: deactivate
   */
  void slot_waiting_for_data(bool status);

  /*search text changed*/
  void slot_search_text_changed();

  /* load more limited items */
  void slot_load_more_contact_list();
  void slot_load_more_chatting_history();
  void slot_load_more_friending_requests();

  /*when side bar button activated, then display relevant info on show_list*/
  void slot_display_chat_list();
  void slot_display_contact_list();

  /*
   * user click the item shown in the ListWidget
   * 1. ListItemType::Default: DO NOTHING
   * 2. ListItemType::SearchUserId: When User Start To Searching User ID
   * 3. ListItemType::ChattingHistory: when user press chatting record
   */
  void slot_list_item_clicked(QListWidgetItem *clicked_item);

  /*if target user has already became a auth friend with current user
   * then switch back to chatting dialog
   */
  void slot_switch_chat_item(std::shared_ptr<UserNameCard> info);

  /* switch to user profile page in stack widget*/
  void slot_switch_user_profile(std::shared_ptr<UserNameCard> info);

  /* switch to chatting page with friends info
   * But User Has to use special Data Structure to pass arguments
   */
  void
  slot_switch_chattingdlg_page(std::shared_ptr<FriendChattingHistory> info);

  /*receive friend request from another user, this func is only for notifying
   * and storing request instance*/
  void slot_incoming_friend_request(
      std::optional<std::shared_ptr<UserFriendRequest>> info);

  /*
   * expose chatting history data to main page
   * developers could update friend's request by using this signal
   */
  void slot_sync_chat_msg_on_local(MsgType msg_type,
                                   std::shared_ptr<ChattingTextMsg> msg);

  /*
   * sender sends chat msg to receiver
   * sender could be a user who is not in the chathistorywidget list
   * so we have to create a new widget for him
   */
  void
  slot_incoming_text_msg(MsgType msg_type,
                         std::optional<std::shared_ptr<ChattingTextMsg>> msg);

private:
  Ui::ChattingDlgMainFrame *ui;

  /*define how many chat recoreds are going to show up on chat record list*/
  static std::size_t CHATRECORED_PER_PAGE;
  std::size_t m_curr_chat_record_loaded = 0;

  /*reserve for search line edit*/
  QAction *m_searchAction;

  /*reserve for cancel user searching*/
  QAction *m_cancelAction;

  QVector<std::shared_ptr<SideBarWidget>> m_qlabelSet;

  /*
   * we use this to store chatting history widget
   * all the chats made by this user will shown here
   */
  std::unordered_map<QString, QListWidgetItem *> m_chatHistoryWidList;

  /*cur qlabel*/
  SideBarWidget *m_curQLabel;

  /*close status dialog*/
  std::shared_ptr<QDialog> m_Dlg;

  /*wait for remote server data status*/
  std::atomic<bool> m_send_status;

  /*LoadingWaitDialog*/
  std::shared_ptr<LoadingWaitDialog> m_loading;

  enum class ChattingDlgMode {
    ChattingDlgChattingMode,  // show multiple user chatting dialog
    ChattingDlgSearchingMode, // allow user to search
    chattingDlgContactMode    // show contacts
  } m_dlgMode;
};

#endif // CHATTINGDLGMAINFRAME_H
