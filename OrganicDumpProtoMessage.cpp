#include "OrganicDumpProtoMessage.h"

#include <glog/logging.h>

#include "TlsConnection.h"
#include "TlsUtilities.h"

namespace
{
using organicdump_proto::ClientType;
using organicdump_proto::MessageType;
using organicdump_proto::BasicResponse;
using organicdump_proto::Hello;
using organicdump_proto::MessageType;
using organicdump_proto::RegisterRpi;
using organicdump_proto::RegisterSoilMoistureSensor;
using organicdump_proto::SendSoilMoistureMeasurement;
using organicdump_proto::UpdatePeripheralOwnership;

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

OrganicDumpProtoMessage::OrganicDumpProtoMessage(RegisterRpi msg)
  : type{MessageType::REGISTER_RPI},
    register_rpi{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(RegisterSoilMoistureSensor msg)
  : type{MessageType::REGISTER_SOIL_MOISTURE_SENSOR},
    register_soil_moisture_sensor{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(UpdatePeripheralOwnership msg)
  : type{MessageType::UPDATE_PERIPHERAL_OWNERSHIP},
    update_peripheral_ownership{std::move(msg)} {}

OrganicDumpProtoMessage::OrganicDumpProtoMessage(SendSoilMoistureMeasurement msg)
  : type{MessageType::SEND_SOIL_MOISTURE_MEASUREMENT},
    send_soil_moisture_measurement{std::move(msg)} {}

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
    case MessageType::REGISTER_RPI:
      register_rpi.~RegisterRpi();
      break;
    case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
      register_soil_moisture_sensor.~RegisterSoilMoistureSensor();
      break;
    case MessageType::UPDATE_PERIPHERAL_OWNERSHIP:
      update_peripheral_ownership.~UpdatePeripheralOwnership();
      break;
    case MessageType::SEND_SOIL_MOISTURE_MEASUREMENT:
      send_soil_moisture_measurement.~SendSoilMoistureMeasurement();
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

  // Transfer resource ownership
  type = other->type;
  switch (type)
  {
    case MessageType::HELLO:
      new (&hello) Hello{std::move(other->hello)};
      other->hello.~Hello();
      break;
    case MessageType::REGISTER_RPI:
      new (&register_rpi) RegisterRpi{std::move(other->register_rpi)};
      other->register_rpi.~RegisterRpi();
      break;
    case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
      new (&register_soil_moisture_sensor) RegisterSoilMoistureSensor{
          std::move(other->register_soil_moisture_sensor)};
      other->register_soil_moisture_sensor.~RegisterSoilMoistureSensor();
      break;
    case MessageType::UPDATE_PERIPHERAL_OWNERSHIP:
      new (&update_peripheral_ownership) UpdatePeripheralOwnership{
          std::move(other->update_peripheral_ownership)};
      other->update_peripheral_ownership.~UpdatePeripheralOwnership();
      break;
    case MessageType::SEND_SOIL_MOISTURE_MEASUREMENT:
      new (&send_soil_moisture_measurement) SendSoilMoistureMeasurement{
          std::move(other->send_soil_moisture_measurement)};
      other->send_soil_moisture_measurement.~SendSoilMoistureMeasurement();
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

  // Restore |other| to default state
  other->type = MessageType::HELLO;
  new (&other->hello) Hello{};
}

bool SendTlsProtobufMessage(
    TlsConnection *cxn,
    OrganicDumpProtoMessage *organic_dump_msg,
    bool *out_cxn_closed)
{
    assert(cxn);
    assert(organic_dump_msg);

    uint8_t msg_type = static_cast<uint8_t>(organic_dump_msg->type);

    google::protobuf::Message *msg = nullptr;
    switch (organic_dump_msg->type)
    {
        case MessageType::HELLO:
          msg = &organic_dump_msg->hello;
          break;
        case MessageType::REGISTER_RPI:
          msg = &organic_dump_msg->register_rpi;
          break;
        case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
          msg = &organic_dump_msg->register_soil_moisture_sensor;
          break;
        case MessageType::UPDATE_PERIPHERAL_OWNERSHIP:
          msg = &organic_dump_msg->update_peripheral_ownership;
          break;
        case MessageType::SEND_SOIL_MOISTURE_MEASUREMENT:
          msg = &organic_dump_msg->send_soil_moisture_measurement;
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
    uint8_t buffer[kRxBufferSize];

    // First read header
    network::ProtobufMessageHeader header;
    if (!ReadTlsProtobufMessageHeader(cxn, &header, out_cxn_closed)) {

      if (*out_cxn_closed)
      {
        return true;
      }

      LOG(ERROR) << "Failed to read TLS Protobuf message header";
      return false;
    }

    MessageType type = static_cast<MessageType>(header.type);
    bool success = false;

    if (header.size > kRxBufferSize)
    {
        LOG(ERROR) << "Failed to handle TLS Protobuf message. Max size: "
                   << kRxBufferSize << ". Actual size: " << header.size;
        return false;
    }

    bool is_successful = true;
    switch (type)
    {
        case MessageType::HELLO:
        {
          Hello hello;
          is_successful = ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &hello,
              out_cxn_closed);
          *out_msg = OrganicDumpProtoMessage{std::move(hello)};
          break;
        }
        case MessageType::REGISTER_RPI:
        {
          RegisterRpi register_pi;
          is_successful = ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &register_pi,
              out_cxn_closed);
          *out_msg = OrganicDumpProtoMessage{std::move(register_pi)};
          break;
        }
        case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
        {
          RegisterSoilMoistureSensor register_sensor;
          is_successful = ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &register_sensor,
              out_cxn_closed);
          *out_msg = OrganicDumpProtoMessage{std::move(register_sensor)};
          break;
        }
        case MessageType::UPDATE_PERIPHERAL_OWNERSHIP:
        {
          UpdatePeripheralOwnership update_ownership;
          is_successful = ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &update_ownership,
              out_cxn_closed);
          *out_msg = OrganicDumpProtoMessage{std::move(update_ownership)};
          break;
        }
        case MessageType::SEND_SOIL_MOISTURE_MEASUREMENT:
        {
          SendSoilMoistureMeasurement measurement;
          is_successful = ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &measurement,
              out_cxn_closed);
          *out_msg = OrganicDumpProtoMessage{std::move(measurement)};
          break;
        }
        case MessageType::BASIC_RESPONSE:
        {
          BasicResponse response;
          is_successful = ReadTlsProtobufMessageBody(
              cxn,
              buffer,
              header.size,
              &response,
              out_cxn_closed);
          *out_msg = OrganicDumpProtoMessage{std::move(response)};
          break;
        }
        default:
          LOG(ERROR) << "Unknown message type: " << static_cast<int>(out_msg->type);
          assert(false);
          return false;
    }

    return is_successful || out_cxn_closed;
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
      case MessageType::REGISTER_RPI:
          return "REGISTER_RPI";
      case MessageType::REGISTER_SOIL_MOISTURE_SENSOR:
          return "REGISTER_SOIL_MOISTURE_SENSOR";
      case MessageType::UPDATE_PERIPHERAL_OWNERSHIP:
          return "UPDATE_PERIPHERAL_OWNERSHIP";
      case MessageType::SEND_SOIL_MOISTURE_MEASUREMENT:
          return "SEND_SOIL_MOISTURE_MEASUREMENT";
      case MessageType::BASIC_RESPONSE:
          return "BASIC_RESPONSE";
      default:
          return "UNKNOWN";
  }
}

}; // namespace organicdump
