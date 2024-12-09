#ifndef TCPNETWORKCONNECTION_H
#define TCPNETWORKCONNECTION_H

#include "def.hpp"
#include <MsgNode.hpp>
#include <QJsonObject>
#include <QObject> //connect
#include <QString>
#include <QTcpSocket>
#include <QUrl>
#include <QtEndian>
#include <functional>
#include <optional>
#include <singleton.hpp>

struct UserNameCard;
struct UserFriendRequest;
struct ChattingTextMsg;
enum class MsgType;

class TCPNetworkConnection
    : public QObject,
      public Singleton<TCPNetworkConnection>,
      public std::enable_shared_from_this<TCPNetworkConnection> {

  Q_OBJECT
  friend class Singleton<TCPNetworkConnection>;
  using Callbackfunction = std::function<void(QJsonObject &&)>;

public:
  virtual ~TCPNetworkConnection();

  /*use signal to trigger data sending*/
  void
  send_data(SendNode<QByteArray, std::function<uint16_t(uint16_t)>> &&data);

private:
  TCPNetworkConnection();

  void registerNetworkEvent();
  void registerSocketSignal();
  void registerCallback();
  void registerErrorHandling();

private slots:
  void slot_establish_long_connnection();

signals:
  void signal_establish_long_connnection();

  /*return connection status to login class*/
  void signal_connection_status(bool status);

  /*login to server failed*/
  void signal_login_failed(ServiceStatus status);

  /*if login success, then switch to chatting dialog*/
  void signal_switch_chatting_dialog();

  /*
   * client sent search username request to server
   * server return result back to client
   */
  void signal_search_username(std::optional<std::shared_ptr<UserNameCard>>,
                              ServiceStatus status);

  /* client who is going to receive new friend request*/
  void signal_incoming_friend_request(
      std::optional<std::shared_ptr<UserFriendRequest>> info);

  /*client who inited request will receive response from here*/
  void signal_sender_response(bool status);

  /*client who is going to confirm request will receive status from here*/
  void signal_confirm_response(bool status);

  /*server be able to send friend request list to this client*/
  void signal_init_friend_request_list();

  /*server be able to send authenticate friend list to this client*/
  void signal_init_auth_friend_list();

  /*
   * target user confirm to add this person as a friend
   * Server will response a message to both user to add this friend
   */
  void signal_add_authenticate_friend(
      std::optional<std::shared_ptr<UserNameCard>>);

  /*
   * sender sends chat msg to receiver
   * sender could be a user who is not in the chathistorywidget list
   * so we have to create a new widget for him
   */
  void signal_incoming_text_msg(MsgType type,
                                std::optional<std::shared_ptr<ChattingTextMsg>> chat);

private:
  /*establish tcp socket with server*/
  QTcpSocket m_socket;

  struct RecvInfo {
    uint16_t _id = 0;
    uint16_t _length = 0;
    QByteArray _msg;
  } m_received;

  /*create a connection buffer to store the data transfer from server*/
  RecvNode<QByteArray, std::function<uint16_t(uint16_t)>> m_buffer;

  /*according to service type to execute callback*/
  std::map<ServiceType, Callbackfunction> m_callbacks;
};

#endif // TCPNETWORKCONNECTION_H
