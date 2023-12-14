#ifndef __TOURKIT_MESSAGETYPE_HPP__
#define __TOURKIT_MESSAGETYPE_HPP__

enum class MessageType {
    WAKE_UP,
    RELOADCONFIG,
    ARTNET_DMX_IN,
    ARTNET_DMX_OUT,
    ARTNET_RELOAD_CONFIG,
    NDI_FRAME_OUT,
    NDI_FRAME_IN,
    NDI_CREATE_SENDER,
    NDI_CLEAR_SENDERS
};

#endif