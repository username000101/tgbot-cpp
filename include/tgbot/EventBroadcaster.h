#ifndef TGBOT_EVENTBROADCASTER_H
#define TGBOT_EVENTBROADCASTER_H

#include "tgbot/export.h"
#include "tgbot/types/Message.h"
#include "tgbot/types/InlineQuery.h"
#include "tgbot/types/ChosenInlineResult.h"
#include "tgbot/types/CallbackQuery.h"
#include "tgbot/types/ShippingQuery.h"
#include "tgbot/types/PreCheckoutQuery.h"
#include "tgbot/types/Poll.h"
#include "tgbot/types/PollAnswer.h"
#include "tgbot/types/ChatMemberUpdated.h"
#include "tgbot/types/ChatJoinRequest.h"

#include <iostream>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>


namespace TgBot {

class EventHandler;

/**
 * @brief This class holds all event listeners.
 *
 * @ingroup general
 */
class TGBOT_API EventBroadcaster {

    friend EventHandler;

public:
    typedef void(*MessageListener) (const Message::Ptr);
    typedef void(*InlineQueryListener) (const InlineQuery::Ptr);
    typedef void(*ChosenInlineResultListener) (const ChosenInlineResult::Ptr);
    typedef void(*CallbackQueryListener) (const CallbackQuery::Ptr);
    typedef void(*ShippingQueryListener) (const ShippingQuery::Ptr);
    typedef void(*PreCheckoutQueryListener) (const PreCheckoutQuery::Ptr);
    typedef void(*PollListener) (const Poll::Ptr);
    typedef void(*PollAnswerListener) (const PollAnswer::Ptr);
    typedef void(*ChatMemberUpdatedListener) (const ChatMemberUpdated::Ptr);
    typedef void(*ChatJoinRequestListener) (const ChatJoinRequest::Ptr);

    /**
     * @brief Registers listener which receives new incoming message of any kind - text, photo, sticker, etc.
     * @param listener Listener.
     */
    inline void onAnyMessage(const MessageListener& listener) {
        _onAnyMessageListener = listener;
    }

    /**
     * @brief Registers listener which receives all messages with commands (messages with leading '/' char).
     * @param commandName Command name which listener can handle.
     * @param listener Listener. Pass nullptr to remove listener of command
     */
    inline void onCommand(const std::string& commandName, MessageListener listener) {
        if (listener) {
            _onCommandListeners[commandName] = listener;
        } else {
            _onCommandListeners.erase(commandName);
        }
    }

    /**
    * @brief Registers listener which receives all messages with commands (messages with leading '/' char).
    * @param commandsList Commands names which listener can handle.
    * @param listener Listener. Pass nullptr to remove listener of commands
    */
    inline void onCommand(const std::initializer_list<std::string>& commandsList, MessageListener listener) {
        if (listener) {
            for (const auto& command : commandsList) {
                _onCommandListeners[command] = listener;
            }
        } else {
            for (const auto& command : commandsList) {
                _onCommandListeners.erase(command);
            }
        }
    }

    /**
     * @brief Registers listener which receives all messages with commands (messages with leading '/' char) which haven't been handled by other listeners.
     * @param listener Listener.
     */
    inline void onUnknownCommand(const MessageListener& listener) {
        _onUnknownCommandListener = listener;
    }

    /**
     * @brief Registers listener which receives all messages without commands (messages with no leading '/' char)
     * @param listener Listener.
     */
    inline void onNonCommandMessage(const MessageListener& listener) {
        _onNonCommandMessageListener = listener;
    }

    /**
     * @brief Registers listener which receives new versions of a message that is known to the bot and was edited
     * @param listener Listener.
     */
    inline void onEditedMessage(const MessageListener& listener) {
        _onEditedMessageListener = listener;
    }

    /**
     * @brief Registers listener which receives new incoming inline queries
     * @param listener Listener.
     */
    inline void onInlineQuery(const InlineQueryListener& listener) {
        _onInlineQueryListener = listener;
    }

    /**
     * @brief Registers listener which receives the results of an inline query that was chosen by a user and sent to their chat partner.
     * Please see https://core.telegram.org/bots/inline#collecting-feedback for details on how to enable these updates for your bot.
     *
     * @param listener Listener.
     */
    inline void onChosenInlineResult(const ChosenInlineResultListener& listener){
        _onChosenInlineResultListener = listener;
    }

    /**
     * @brief Registers listener which receives new incoming callback queries
     * @param listener Listener.
     */
    inline void onCallbackQuery(CallbackQueryListener listener){
        if (!_onCallbackQueryListeners.empty())
            _onCallbackQueryListeners.clear();
        _onCallbackQueryListeners.push_back(listener);
    }


    /**
     * @brief Registers listener which receives new incoming shipping queries.
     * Only for invoices with flexible price
     *
     * @param listener Listener.
     */
    inline void onShippingQuery(const ShippingQueryListener& listener){
        _onShippingQueryListener = listener;
    }

    /**
     * @brief Registers listener which receives new incoming pre-checkout queries.
     * Contains full information about checkout
     *
     * @param listener Listener.
     */
    inline void onPreCheckoutQuery(const PreCheckoutQueryListener& listener){
        _onPreCheckoutQueryListener = listener;
    }

    /**
     * @brief Registers listener which receives new poll states.
     * Bots receive only updates about stopped polls and polls, which are sent by the bot
     *
     * @param listener Listener.
     */
    inline void onPoll(PollListener listener){
        _onPollListeners.push_back(listener);
    }

    /**
     * @brief Registers listener which receives an answer if a user changed their answer in a non-anonymous poll.
     * Bots receive new votes only in polls that were sent by the bot itself.
     *
     * @param listener Listener.
     */
    inline void onPollAnswer(PollAnswerListener listener){
        _onPollAnswerListeners.push_back(listener);
    }

    /**
     * @brief Registers listener which receives the bot's chat member status if it was updated in a chat.
     * For private chats, this update is received only when the bot is blocked or unblocked by the user.
     *
     * @param listener Listener.
     */

    inline void onMyChatMember(const ChatMemberUpdatedListener& listener){
        _onMyChatMemberListener = listener;
    }

    /**
     * @brief Registers listener which receives a status if a chat member's status was updated in a chat.
     * The bot must be an administrator in the chat and must explicitly specify “chat_member” in the list of allowedUpdates to receive these updates.
     *
     * @param listener Listener.
     */
    inline void onChatMember(ChatMemberUpdatedListener listener){
        _onChatMemberListener = listener;
    }

    /**
     * @brief Registers listener which receives requests to join the chat.
     * The bot must have the canInviteUsers administrator right in the chat to receive these updates.
     *
     * @param listener Listener.
     */
    inline void onChatJoinRequest(ChatJoinRequestListener listener){
        _onChatJoinRequestListener = listener;
    }

private:
    template<typename ListenerType, typename ObjectType>
    inline void broadcast(const std::vector<ListenerType>& listeners, const ObjectType object) const {
        if (!object)
            return;
        for (auto& iter : listeners) {
            if (!iter)
                std::cout << "tgbot-cpp: " << __FUNCTION__ << ":" << __LINE__ << ": Invalid handler provided" << std::endl;
            else if (!std::is_invocable_v<decltype(iter), CallbackQuery::Ptr>)
                std::cout << "tgbot-cpp: " << __FUNCTION__ << ":" << __LINE__ << ": Non-invocable object provided(" << typeid(iter).name() << std::endl;
            else
                iter(object);
        }
    }

    inline void broadcastAnyMessage(const Message::Ptr& message) const {
        broadcast<MessageListener, Message::Ptr>({_onAnyMessageListener}, message);
    }

    inline bool broadcastCommand(const std::string& command, const Message::Ptr& message) const {
        auto iter = _onCommandListeners.find(command);
        if (iter == _onCommandListeners.end()) {
            return false;
        }
        iter->second(message);
        return true;
    }

    inline void broadcastUnknownCommand(const Message::Ptr& message) const {
        broadcast<MessageListener, Message::Ptr>({_onUnknownCommandListener}, message);
    }

    inline void broadcastNonCommandMessage(const Message::Ptr& message) const {
        broadcast<MessageListener, Message::Ptr>({_onNonCommandMessageListener}, message);
    }

    inline void broadcastEditedMessage(const Message::Ptr& message) const {
        broadcast<MessageListener, Message::Ptr>({_onEditedMessageListener}, message);
    }

    inline void broadcastInlineQuery(const InlineQuery::Ptr& query) const {
        broadcast<InlineQueryListener, InlineQuery::Ptr>({_onInlineQueryListener}, query);
    }

    inline void broadcastChosenInlineResult(const ChosenInlineResult::Ptr& result) const {
        broadcast<ChosenInlineResultListener, ChosenInlineResult::Ptr>({_onChosenInlineResultListener}, result);
    }

    inline void broadcastCallbackQuery(const CallbackQuery::Ptr& result) const {
        broadcast<CallbackQueryListener, CallbackQuery::Ptr>({_onCallbackQueryListener}, result);
    }

    inline void broadcastShippingQuery(const ShippingQuery::Ptr& result) const {
        broadcast<ShippingQueryListener, ShippingQuery::Ptr>({_onShippingQueryListener}, result);
    }

    inline void broadcastPreCheckoutQuery(const PreCheckoutQuery::Ptr& result) const {
        broadcast<PreCheckoutQueryListener, PreCheckoutQuery::Ptr>({_onPreCheckoutQueryListener}, result);
    }

    inline void broadcastPoll(const Poll::Ptr& result) const {
        broadcast<PollListener, Poll::Ptr>(_onPollListeners, result);
    }

    inline void broadcastPollAnswer(const PollAnswer::Ptr& result) const {
        broadcast<PollAnswerListener, PollAnswer::Ptr>(_onPollAnswerListeners, result);
    }

    inline void broadcastMyChatMember(const ChatMemberUpdated::Ptr& result) const {
        broadcast<ChatMemberUpdatedListener, ChatMemberUpdated::Ptr>({_onMyChatMemberListener}, result);
    }

    inline void broadcastChatMember(const ChatMemberUpdated::Ptr& result) const {
        broadcast<ChatMemberUpdatedListener, ChatMemberUpdated::Ptr>({_onChatMemberListener}, result);
    }

    inline void broadcastChatJoinRequest(const ChatJoinRequest::Ptr& result) const {
        broadcast<ChatJoinRequestListener, ChatJoinRequest::Ptr>({_onChatJoinRequestListener}, result);
    }

    MessageListener _onAnyMessageListener;
    std::unordered_map<std::string, MessageListener> _onCommandListeners;
    std::vector<MessageListener> _onUnknownCommandListeners;
    std::vector<MessageListener> _onNonCommandMessageListeners;
    std::vector<MessageListener> _onEditedMessageListeners;
    std::vector<InlineQueryListener> _onInlineQueryListeners;
    std::vector<ChosenInlineResultListener> _onChosenInlineResultListeners;
    std::vector<CallbackQueryListener> _onCallbackQueryListeners;
    std::vector<ShippingQueryListener> _onShippingQueryListeners;
    std::vector<PreCheckoutQueryListener> _onPreCheckoutQueryListeners;
    MessageListener _onUnknownCommandListener;
    MessageListener _onNonCommandMessageListener;
    MessageListener _onEditedMessageListener;
    InlineQueryListener _onInlineQueryListener;
    CallbackQueryListener _onCallbackQueryListener;
    ChosenInlineResultListener _onChosenInlineResultListener;
    ShippingQueryListener _onShippingQueryListener;
    PreCheckoutQueryListener _onPreCheckoutQueryListener;
    std::vector<PollListener> _onPollListeners;
    std::vector<PollAnswerListener> _onPollAnswerListeners;
    ChatMemberUpdatedListener _onMyChatMemberListener;
    ChatMemberUpdatedListener _onChatMemberListener;
    ChatJoinRequestListener _onChatJoinRequestListener;
};

}

#endif //TGBOT_EVENTBROADCASTER_H
