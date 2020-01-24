#include "OrganicDumpProtoMessage.h"

#include "TlsConnection.h"
#include "TlsUtilities.h"

namespace
{
using organicdump_proto::ClientType;
using organicdump_proto::MessageType;
using organicdump_proto::BasicResponse;
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

OrganicDumpProtoMessage::OrganicDumpProtoMessage()
  : OrganicDumpProtoMessage{Hello{}} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(Hello msg)
  : type{MessageType::HELLO},
    hello{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(RegisterClient msg)
  : type{MessageType::REGISTER_CLIENT},
    register_client{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(RegisterSoilMoistureSensor msg)
  : type{MessageType::REGISTER_SOIL_MOISTURE_SENSOR},
    register_soil_moisture_sensor{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(UpdatePeripheralAssociation msg)
  : type{MessageType::UPDATE_PERIPHERAL_ASSOCIATION},
    update_peripheral_association{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(BasicResponse msg)
  : type{MessageType::BASIC_RESPONSE},
    basic_response{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(OrganicDumpProtoMessage &&other) {
  StealResources(&other);
}

OrganicDumpProtoMessage &OrganicDumpProtoMessage::operator=(OrganicDumpProtoMessage &&other) {
  if (this != &other)
  {
    CloseResources();
    StealResources(&other);
  }
  return *this;
}

OrganicDumpProtoMessage::~OrganicDumpProtoMessage() {
  CloseResources();
}

void OrganicDumpProtoMessage::CloseResources()
{
  switch (type)
  {
    case MessageType::HELLO:
      hello.~Hello();
      break;
    case MessageType::REGISTER_CLIENT:
      register_client.~RegisterClient();
      break;
    case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
      register_soil_moisture_sensor.~RegisterSoilMoistureSensor();
      break;
    case MessageType::UPDATE_PERIPHERAL_ASSOCIATION:
      update_peripheral_association.~UpdatePeripheralAssociation();
      break;
    case MessageType::BASIC_RESPONSE:
      basic_response.~BasicResponse();
      break;
    default:
      LOG(ERROR) << "Unknown MessageType: " << static_cast<int>(type);
      assert(false);
      break;
  }
}

void OrganicDumpProtoMessage::StealResources(OrganicDumpProtoMessage *other)
{
  assert(other);

  type = other->type;
  switch (type)
  {
    case MessageType::HELLO:
      new (&hello) Hello{std::move(other->hello)};
      other->hello.~Hello();
      break;
    case MessageType::REGISTER_CLIENT:
      new (&register_client) RegisterClient{std::move(other->register_client)};
      other->register_client.~RegisterClient();
      break;
    case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
      new (&register_soil_moisture_sensor) RegisterSoilMoistureSensor{
          std::move(other->register_soil_moisture_sensor)};
      other->register_soil_moisture_sensor.~RegisterSoilMoistureSensor();
      break;
    case MessageType::UPDATE_PERIPHERAL_ASSOCIATION:
      new (&update_peripheral_association) UpdatePeripheralAssociation{
          std::move(other->update_peripheral_association)};
      other->update_peripheral_association.~UpdatePeripheralAssociation();
      break;
    case MessageType::BASIC_RESPONSE:
      new (&basic_response) BasicResponse{std::move(other->basic_response)};
      other->basic_response.~BasicResponse();
      break;
    default:
      LOG(ERROR) << "Unknown MessageType: " << static_cast<int>(type);
      assert(false);
      break;
  }
}

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
    OrganicDumpProtoMessage *out_msg,
    bool *out_cxn_closed)
{
    assert(cxn);
    assert(out_msg);
    assert(out_cxn_closed);

    uint8_t buffer[kRxBufferSize];

    // First read header
    network::ProtobufMessageHeader header;
    if (!ReadTlsProtobufMessageHeader(cxn, &header)) {
      LOG(ERROR) << "Failed to read TLS Protobuf message header";
      return false;
    }

    out_msg->type = static_cast<MessageType>(header.type);
    bool success = false;

    if (header.size > kRxBufferSize)
    {
        LOG(ERROR) << "Failed to handle TLS Protobuf message. Max size: "
                   << kRxBufferSize << ". Actual size: " << header.size;
        return false;
    }

    switch (out_msg->type)
    {
        case MessageType::HELLO:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &out_msg->hello);
        case MessageType::REGISTER_CLIENT:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &out_msg->register_client);
        case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &out_msg->register_soil_moisture_sensor);
        case MessageType::UPDATE_PERIPHERAL_ASSOCIATION:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &out_msg->update_peripheral_association);
        case MessageType::BASIC_RESPONSE:
          return ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &out_msg->basic_response);
        default:
          LOG(ERROR) << "Unknown message type: " << static_cast<int>(out_msg->type);
          assert(false);
          return false;
    }
}

std::string ToString(ClientType type) {
  switch (type) {
    case ClientType::CONTROL:
      return "CONTROL";
    case ClientType::RPI_POT:
      return "RPI";
    default:
      return "UNKNOWN";
  }
}

std::string ToString(MessageType type) {
  switch (type)
  {
      case MessageType::HELLO:
        return "HELLO";
      case MessageType::REGISTER_CLIENT:
          return "REGISTER_CLIENT";
      case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
          return "REGISTER_SOIL_MOISTURE_SENSOR";
      case MessageType::UPDATE_PERIPHERAL_ASSOCIATION:
          return "UPDATE_PERIPHERAL_ASSOCIATION";
      case MessageType::BASIC_RESPONSE:
          return "BASIC_RESPONSE";
      default:
          return "UNKNOWN";
  }
}

}; // namespace organicdump
