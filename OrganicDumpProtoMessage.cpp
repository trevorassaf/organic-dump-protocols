#include "OrganicDumpProtoMessage.h"

#include "TlsConnection.h"
#include "TlsUtilities.h"

namespace
{
using organicdump_proto::MessageType;
using organicdump_proto::BasicResponse;
using organicdump_proto::Echo;
using organicdump_proto::Hello;
using organicdump_proto::MessageType;
using organicdump_proto::RegisterClient;
using organicdump_proto::RegisterSoilMoistureSensor;
using organicdump_proto::UpdatePeripheralAssociation;

using network::TlsConnection;

constexpr size_t kRxBufferSize = 10 * 1024;
} // namespace

namespace organicdump
{

bool SendTlsProtobufMessage(
    TlsConnection *cxn,
    OrganicDumpProtoMessage *organic_dump_msg,
    bool *out_cxn_closed)
{
    assert(cxn);
    assert(organic_dump_msg);
    assert(out_cxn_closed);

    uint8_t msg_type = static_cast<uint8_t>(organic_dump_msg->type);

    google::protobuf::Message *msg = nullptr;
    switch (organic_dump_msg->type)
    {
        case MessageType::SERVER_ECHO:
          msg = &organic_dump_msg->echo;
          break;
        case MessageType::HELLO:
          msg = &organic_dump_msg->hello;
          break;
        case MessageType::REGISTER_CLIENT:
          msg = &organic_dump_msg->register_client;
          break;
        case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
          msg = &organic_dump_msg->register_soil_moisture_sensor;
          break;
        case MessageType::UPDATE_PERIPHERAL_ASSOCIATION:
          msg = &organic_dump_msg->update_peripheral_association;
          break;
        case MessageType::CLIENT_ECHO:
          msg = &organic_dump_msg->echo;
          break;
        case MessageType::BASIC_RESPONSE:
          msg = &organic_dump_msg->basic_response;
          break;
        default:
          LOG(ERROR) << "Unknown message type: " << static_cast<int>(organic_dump_msg->type);
          assert(false);
          break;
    }

    return SendTlsProtobufMessage(cxn, msg_type, msg, out_cxn_closed);
}

bool ReadTlsProtobufMessage(
    TlsConnection *cxn,
    OrganicDumpProtoMessage *msg,
    bool *out_cxn_closed)
{
    assert(cxn);
    assert(msg);
    assert(out_cxn_closed);

    uint8_t buffer[kRxBufferSize];

    // First read header
    network::ProtobufMessageHeader header;
    if (!ReadTlsProtobufMessageHeader(cxn, &header)) {
      LOG(ERROR) << "Failed to read TLS Protobuf message header";
      return false;
    }

    msg->type = static_cast<MessageType>(header.type);
    bool success = false;

    if (header.size > kRxBufferSize)
    {
        LOG(ERROR) << "Failed to handle TLS Protobuf message. Max size: "
                   << kRxBufferSize << ". Actual size: " << header.size;
        return false;
    }

    switch (msg->type)
    {
        case MessageType::SERVER_ECHO:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->echo);
        case MessageType::HELLO:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->hello);
        case MessageType::REGISTER_CLIENT:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->register_client);
        case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->register_soil_moisture_sensor);
        case MessageType::UPDATE_PERIPHERAL_ASSOCIATION:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->update_peripheral_association);
        case MessageType::CLIENT_ECHO:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->echo);
        case MessageType::BASIC_RESPONSE:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &msg->basic_response);
        default:
          LOG(ERROR) << "Unknown message type: " << static_cast<int>(msg->type);
          assert(false);
          return false;
    }
}

}; // namespace organicdump
