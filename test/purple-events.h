#ifndef _PURPLE_EVENTS_H
#define _PURPLE_EVENTS_H

#include <purple.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <queue>
#include <iostream>
#include <map>

struct PurpleEvent;

extern "C" {
    extern PurpleGroup standardPurpleGroup;
}

class PurpleEventReceiver {
public:
    void addEvent(std::unique_ptr<PurpleEvent> event);

    // Check that given events in that order, and no others, are in the queue, and clear the queue
    template<typename... EventTypes>
    void verifyEvents(const PurpleEvent &event, EventTypes... args)
    {
        verifyEvent(event);
        verifyEvents(args...);
    }

    void verifyEvents2(std::initializer_list<std::unique_ptr<PurpleEvent>> events);
    void verifyNoEvents();
    void discardEvents();

    void inputEnter(const gchar *value);
    void inputCancel();
    void requestedAction(const char *button);
    PurpleXfer *getLastXfer() { return lastXfer; }
    void addCommand(const char *command, PurpleCmdFunc handler, void *data);
    void runCommand(const char *command, PurpleConversation *conv, std::vector<std::string> arguments);
private:
    void verifyEvent(const PurpleEvent &event);
    void verifyEvents()
    {
        verifyNoEvents();
    }

    std::queue<std::unique_ptr<PurpleEvent>> m_events;
    void      *inputUserData = NULL;
    GCallback  inputOkCb     = NULL;
    GCallback  inputCancelCb = NULL;
    PurpleXfer *lastXfer     = NULL;

    std::vector<std::pair<std::string, PurpleRequestActionCb>> actionCallbacks;
    void *actionUserData = NULL;
    std::map<std::string, std::pair<PurpleCmdFunc, void *>> commands;
};

void nodeMenuAction(PurpleBlistNode *node, GList *actions, const char *label);

extern PurpleEventReceiver g_purpleEvents;

enum class PurpleEventType: uint8_t {
    AccountSetAlias,
    ShowAccount,
    AddBuddy,
    AddChat,
    RemoveChat,
    AliasChat,
    HideAccount,
    RemoveBuddy,
    AliasBuddy,
    ConnectionError,
    ConnectionSetState,
    ConnectionUpdateProgress,
    NewConversation,
    ConversationWrite,
    ConvSetTitle,
    NotifyMessage,
    UserStatus,
    RequestInput,
    RequestAction,
    JoinChatFailed,
    ServGotChat,
    ServGotIm,
    ServGotJoinedChat,
    BuddyTypingStart,
    BuddyTypingStop,
    PresentConversation,
    ChatAddUser,
    ChatClearUsers,
    ChatSetTopic,
    XferAccepted,
    XferStart,
    XferProgress,
    XferCompleted,
    XferEnd,
    XferLocalCancel,
    XferRemoteCancel,
    SetUserPhoto,
    RoomlistInProgress,
    RoomlistAddRoom
};

struct PurpleEvent {
    PurpleEventType type;

    PurpleEvent(PurpleEventType type) : type(type) {}
    virtual ~PurpleEvent() {}
    std::string toString() const;
};

struct AccountSetAliasEvent: PurpleEvent {
    PurpleAccount *account;
    std::string    alias;

    AccountSetAliasEvent(PurpleAccount *account, const std::string &alias)
    : PurpleEvent(PurpleEventType::AccountSetAlias), account(account), alias(alias) {}
};

struct ShowAccountEvent: PurpleEvent {
    PurpleAccount *account;

    ShowAccountEvent(PurpleAccount *account)
    : PurpleEvent(PurpleEventType::ShowAccount), account(account) {}
};

struct AddBuddyEvent: PurpleEvent {
    std::string      username;
    std::string      alias;
    PurpleAccount   *account;
    PurpleContact   *contact;
    PurpleGroup     *group;
    PurpleBlistNode *node;

    AddBuddyEvent(const std::string &username, const char *alias, PurpleAccount *account,
                  PurpleContact *contact, PurpleGroup *group, PurpleBlistNode *node)
    : PurpleEvent(PurpleEventType::AddBuddy), username(username), alias(alias ? alias : ""),
      account(account), contact(contact), group(group), node(node) {}

    AddBuddyEvent(const std::string &username, const std::string &alias, PurpleAccount *account,
                  PurpleContact *contact, PurpleGroup *group, PurpleBlistNode *node)
    : AddBuddyEvent(username, alias.c_str(), account, contact, group, node) {}
};

struct AddChatEvent: PurpleEvent {
    std::string      name;
    std::string      alias;
    PurpleAccount   *account;
    PurpleGroup     *group;
    PurpleBlistNode *node;

    AddChatEvent(const std::string &name, const std::string &alias, PurpleAccount *account,
                 PurpleGroup *group, PurpleBlistNode *node)
    : PurpleEvent(PurpleEventType::AddChat), name(name), alias(alias), account(account),
      group(group), node(node) {}
};

struct RemoveChatEvent: public PurpleEvent {
    std::string name;
    std::string inviteLink;

    RemoveChatEvent(const std::string &name, const std::string &inviteLink)
    : PurpleEvent(PurpleEventType::RemoveChat), name(name), inviteLink(inviteLink) {}
};

struct AliasChatEvent: PurpleEvent {
    std::string name;
    std::string newAlias;

    AliasChatEvent(const std::string &username, const std::string &newAlias)
    : PurpleEvent(PurpleEventType::AliasChat), name(username), newAlias(newAlias) {}
};

struct HideAccountEvent: PurpleEvent {
    PurpleAccount *account;
};

struct RemoveBuddyEvent: PurpleEvent {
    PurpleAccount *account;
    std::string    username;

    RemoveBuddyEvent(PurpleAccount *account, const std::string &username)
    : PurpleEvent(PurpleEventType::RemoveBuddy), account(account), username(username) {}
};

struct AliasBuddyEvent: PurpleEvent {
    std::string username;
    std::string newAlias;

    AliasBuddyEvent(const std::string &username, const std::string &newAlias)
    : PurpleEvent(PurpleEventType::AliasBuddy), username(username), newAlias(newAlias) {}
};

struct ConnectionErrorEvent: PurpleEvent {
    PurpleConnection *connection;
    std::string       message;

    ConnectionErrorEvent(PurpleConnection *connection, const std::string &message)
    : PurpleEvent(PurpleEventType::ConnectionError), connection(connection), message(message) {}
};

struct ConnectionSetStateEvent: PurpleEvent {
    PurpleConnection      *connection;
    PurpleConnectionState  state;

    ConnectionSetStateEvent(PurpleConnection *connection, PurpleConnectionState state)
    : PurpleEvent(PurpleEventType::ConnectionSetState), connection(connection), state(state) {}
};

struct ConnectionUpdateProgressEvent: PurpleEvent {
    PurpleConnection *connection;
    size_t            step, stepCount;

    ConnectionUpdateProgressEvent(PurpleConnection *connection,
                                  size_t step, size_t stepCount)
    : PurpleEvent(PurpleEventType::ConnectionUpdateProgress), connection(connection),
      step(step), stepCount(stepCount) {}
};

struct NewConversationEvent: PurpleEvent {
    PurpleConversationType  type;
    PurpleAccount          *account;
    std::string             name;

    NewConversationEvent(PurpleConversationType type, PurpleAccount *account, const std::string &name)
    : PurpleEvent(PurpleEventType::NewConversation), type(type), account(account), name(name) {}
};

struct ConversationWriteEvent: PurpleEvent {
    std::string        conversation;
    std::string        username;
    std::string        message;
    PurpleMessageFlags flags;
    time_t             mtime;

    ConversationWriteEvent(const std::string &conversationName, const std::string &username,
                           const std::string &message, PurpleMessageFlags flags, time_t mtime)
    : PurpleEvent(PurpleEventType::ConversationWrite), conversation(conversationName),
    username(username), message(message), flags(flags), mtime(mtime) {}
};

struct ConvSetTitleEvent: PurpleEvent {
    std::string name;
    std::string newTitle;

    ConvSetTitleEvent(const std::string name, const std::string &newTitle)
    : PurpleEvent(PurpleEventType::ConvSetTitle), name(name), newTitle(newTitle) {}
};

struct NotifyMessageEvent: PurpleEvent {
    void                *handle;
    PurpleNotifyMsgType  type;
    std::string          title;
    std::string          primary;
    std::string          secondary;
};

struct UserStatusEvent: PurpleEvent {
    PurpleAccount        *account;
    std::string           username;
    PurpleStatusPrimitive status;

    UserStatusEvent(PurpleAccount *account, const std::string &username, PurpleStatusPrimitive status)
    : PurpleEvent(PurpleEventType::UserStatus), account(account), username(username), status(status) {}
};

struct RequestInputEvent: PurpleEvent {
    void               *handle;
    std::string         title, primary;
	std::string         secondary, default_value;
	GCallback           ok_cb;
	GCallback           cancel_cb;
	PurpleAccount      *account;
    std::string         username;
    PurpleConversation *conv;
	void               *user_data;

    RequestInputEvent(void *handle, const char *title, const char *primary,
                      const char *secondary, const char *default_value,
                      const char *ok_text, GCallback ok_cb,
                      const char *cancel_text, GCallback cancel_cb,
                      PurpleAccount *account, const char *who, PurpleConversation *conv,
                      void *user_data)
    : PurpleEvent(PurpleEventType::RequestInput),
      handle(handle),
      title(title ? title : ""),
      primary(primary ? primary : ""),
      secondary(secondary ? secondary : ""),
      default_value(default_value ? default_value : ""),
      ok_cb(ok_cb),
      cancel_cb(cancel_cb),
      account(account),
      username(who ? who : ""),
      conv(conv),
      user_data(user_data)
      {}
    RequestInputEvent(void *handle, PurpleAccount *account, const char *who, PurpleConversation *conv)
    : PurpleEvent(PurpleEventType::RequestInput),
      handle(handle),
      account(account),
      username(who ? who : ""),
      conv(conv)
      {}
};

struct RequestActionEvent: PurpleEvent {
    void               *handle;
    std::string         title, primary;
	std::string         secondary;
	PurpleAccount      *account;
    std::string         username;
    PurpleConversation *conv;
	void               *user_data;
    std::vector<std::string>           buttons;
	std::vector<PurpleRequestActionCb> callbacks;

    RequestActionEvent(void *handle, const char *title, const char *primary, const char *secondary,
                      PurpleAccount *account, const char *who, PurpleConversation *conv,
                      void *user_data, const std::vector<std::string> &buttons,
                      const std::vector<PurpleRequestActionCb> &callbacks)
    : PurpleEvent(PurpleEventType::RequestAction),
      handle(handle),
      title(title ? title : ""),
      primary(primary ? primary : ""),
      secondary(secondary ? secondary : ""),
      account(account),
      username(who ? who : ""),
      conv(conv),
      user_data(user_data),
      buttons(buttons),
      callbacks(callbacks)
      {}
    RequestActionEvent(void *handle, PurpleAccount *account, const char *who,
                       PurpleConversation *conv, unsigned actionCount)
    : PurpleEvent(PurpleEventType::RequestAction),
      handle(handle),
      account(account),
      username(who ? who : ""),
      conv(conv),
      buttons(actionCount, ""),
      callbacks(actionCount, nullptr)
      {}
};

struct JoinChatFailedEvent: PurpleEvent {
    PurpleConnection *connection;
};

struct ServGotChatEvent: public PurpleEvent {
    PurpleConnection  *connection;
    int                id;
    std::string        username;
    std::string        message;
    PurpleMessageFlags flags;
    time_t             mtime;

    ServGotChatEvent(PurpleConnection *connection, int id, const std::string &username, const std::string &message,
                   PurpleMessageFlags flags, time_t mtime)
    : PurpleEvent(PurpleEventType::ServGotChat), connection(connection), id(id), username(username),
      message(message), flags(flags), mtime(mtime) {}
};

struct ServGotImEvent: PurpleEvent {
    PurpleConnection  *connection;
    std::string        username;
    std::string        message;
    PurpleMessageFlags flags;
    time_t             mtime;

    ServGotImEvent(PurpleConnection *connection, const std::string &username, const std::string &message,
                   PurpleMessageFlags flags, time_t mtime)
    : PurpleEvent(PurpleEventType::ServGotIm), connection(connection), username(username),
      message(message), flags(flags), mtime(mtime) {}
};

struct ServGotJoinedChatEvent: public PurpleEvent {
    PurpleConnection  *connection;
    int                id;
    std::string        chatName;
    std::string        conversationTitle;

    ServGotJoinedChatEvent(PurpleConnection *connection, int id, const std::string &chatName,
                           const std::string &chatAlias)
    : PurpleEvent(PurpleEventType::ServGotJoinedChat), connection(connection), id(id),
      chatName(chatName), conversationTitle(chatAlias) {}
};

struct BuddyTypingStartEvent: PurpleEvent {
    PurpleConnection *connection;
    std::string       username;
    int               timeout;
    PurpleTypingState state;
};

struct BuddyTypingStopEvent: PurpleEvent {
    PurpleConnection *connection;
    std::string       username;
};

struct PresentConversationEvent: PurpleEvent {
    std::string name;
    PresentConversationEvent(const std::string &name)
    : PurpleEvent(PurpleEventType::PresentConversation), name(name) {}
};

struct ChatAddUserEvent: PurpleEvent {
    std::string chatName;
    std::string user;
    std::string extra_msg;
    PurpleConvChatBuddyFlags flags;
    bool new_arrival;

    ChatAddUserEvent(const std::string &chatName, const std::string &user, const std::string &extra_msg,
                      PurpleConvChatBuddyFlags flags, gboolean new_arrival)
    : PurpleEvent(PurpleEventType::ChatAddUser), chatName(chatName), user(user), extra_msg(extra_msg),
      flags(flags), new_arrival(new_arrival) {}
};

struct ChatClearUsersEvent: PurpleEvent {
    std::string chatName;

    ChatClearUsersEvent(const std::string &chatName)
    : PurpleEvent(PurpleEventType::ChatClearUsers), chatName(chatName) {}
};

struct ChatSetTopicEvent: PurpleEvent {
    std::string chatName;
    std::string newTopic;
    std::string who;

    ChatSetTopicEvent(const std::string &chatName, const std::string &newTopic, const std::string &who)
    : PurpleEvent(PurpleEventType::ChatSetTopic), newTopic(newTopic), who(who) {}
};

struct XferAcceptedEvent: PurpleEvent {
    PurpleXfer *xfer = NULL;
    std::string filename;
    std::string *getFileName = NULL;

    XferAcceptedEvent(PurpleXfer *xfer, const std::string &filename)
    : PurpleEvent(PurpleEventType::XferAccepted), xfer(xfer), filename(filename) {}

    XferAcceptedEvent(const std::string &filename)
    : PurpleEvent(PurpleEventType::XferAccepted), filename(filename) {}

    XferAcceptedEvent(std::string *getFileName)
    : PurpleEvent(PurpleEventType::XferAccepted), getFileName(getFileName) {}
};

struct XferStartEvent: PurpleEvent {
    std::string filename;
    std::string *filenamePtr;

    XferStartEvent(const std::string &filename)
    : PurpleEvent(PurpleEventType::XferStart), filename(filename), filenamePtr(nullptr) {}
    XferStartEvent(std::string *filenamePtr)
    : PurpleEvent(PurpleEventType::XferStart), filenamePtr(filenamePtr) {}
};

struct XferProgressEvent: PurpleEvent {
    std::string filename;
    size_t      bytesSent;

    XferProgressEvent(const std::string &filename, size_t bytesSent)
    : PurpleEvent(PurpleEventType::XferProgress), filename(filename), bytesSent(bytesSent) {}
};

struct XferCompletedEvent: PurpleEvent {
    std::string filename;
    bool        completed;
    size_t      bytesSent;

    XferCompletedEvent(const std::string &filename, gboolean completed, size_t bytesSent)
    : PurpleEvent(PurpleEventType::XferCompleted), filename(filename), completed(completed != FALSE),
      bytesSent(bytesSent) {}
};

struct XferEndEvent: PurpleEvent {
    std::string filename;

    XferEndEvent(const std::string &filename)
    : PurpleEvent(PurpleEventType::XferEnd), filename(filename) {}
};

struct XferLocalCancelEvent: PurpleEvent {
    std::string filename;

    XferLocalCancelEvent(const std::string &filename)
    : PurpleEvent(PurpleEventType::XferLocalCancel), filename(filename) {}
};

struct XferRemoteCancelEvent: PurpleEvent {
    std::string filename;

    XferRemoteCancelEvent(const std::string &filename)
    : PurpleEvent(PurpleEventType::XferRemoteCancel), filename(filename) {}
};

struct SetUserPhotoEvent: PurpleEvent {
    PurpleAccount *account;
    std::string    username;
    std::vector<uint8_t> data;

    SetUserPhotoEvent(PurpleAccount *account, const std::string &username, const void *data, size_t datalen)
    : PurpleEvent(PurpleEventType::SetUserPhoto), username(username)
    {
        this->data.resize(datalen);
        memmove(this->data.data(), data, datalen);
    }
};

struct RoomlistInProgressEvent: PurpleEvent {
    PurpleRoomlist *list;
    bool inprogress;

    RoomlistInProgressEvent(PurpleRoomlist *list, bool inprogress)
    : PurpleEvent(PurpleEventType::RoomlistInProgress), list(list), inprogress(inprogress) {}
};

struct RoomlistAddRoomEvent: PurpleEvent {
    PurpleRoomlist *roomlist;
    std::vector<std::string> fieldValues;
    const char *fieldToCheck = NULL;
    const char *valueToCheck = NULL;

    RoomlistAddRoomEvent(PurpleRoomlist *roomlist, const std::vector<std::string> &values)
    : PurpleEvent(PurpleEventType::RoomlistAddRoom), roomlist(roomlist), fieldValues(values) {}

    RoomlistAddRoomEvent(PurpleRoomlist *roomlist, const char *fieldToCheck,
                         const char *valueToCheck)
    : PurpleEvent(PurpleEventType::RoomlistAddRoom), roomlist(roomlist), fieldToCheck(fieldToCheck),
      valueToCheck(valueToCheck) {}
};

#endif
