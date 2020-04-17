#ifndef ORGANICDUMP_ORGANICDUMPMESSAGE_H
#define ORGANICDUMP_ORGANICDUMPMESSAGE_H

#include <string>

#include "organic_dump.pb.h"
#include "TlsConnection.h"

namespace organicdump
{

struct OrganicDumpProtoMessage
{
  organicdump_proto::MessageType type;

  union
  {
      organicdump_proto::Hello hello;
      organicdump_proto::RegisterRpi register_rpi;
      organicdump_proto::RegisterSoilMoistureSensor register_soil_moisture_sensor;
      organicdump_proto::UpdatePeripheralOwnership update_peripheral_ownership;
      organicdump_proto::SendSoilMoistureMeasurement send_soil_moisture_measurement;
      organicdump_proto::BasicResponse basic_response;
  };

  OrganicDumpProtoMessage();
  OrganicDumpProtoMessage(organicdump_proto::Hello msg);
  OrganicDumpProtoMessage(organicdump_proto::RegisterRpi msg);
  OrganicDumpProtoMessage(organicdump_proto::RegisterSoilMoistureSensor msg);
  OrganicDumpProtoMessage(organicdump_proto::UpdatePeripheralOwnership msg);
  OrganicDumpProtoMessage(organicdump_proto::SendSoilMoistureMeasurement msg);
  OrganicDumpProtoMessage(organicdump_proto::BasicResponse msg);
  OrganicDumpProtoMessage(OrganicDumpProtoMessage &&other);
  OrganicDumpProtoMessage &operator=(OrganicDumpProtoMessage &&other);
  ~OrganicDumpProtoMessage();

private:
  void CloseResources();
  void StealResources(OrganicDumpProtoMessage *other);

private:
  OrganicDumpProtoMessage(const OrganicDumpProtoMessage &other) = delete;
  OrganicDumpProtoMessage &operator=(const OrganicDumpProtoMessage &other) = delete;
};

bool SendTlsProtobufMessage(
    network::TlsConnection *cxn,
    OrganicDumpProtoMessage *msg,
    bool *out_cxn_closed=nullptr);

bool ReadTlsProtobufMessage(
    network::TlsConnection *cxn,
    OrganicDumpProtoMessage *out_msg,
    bool *out_cxn_closed=nullptr);

std::string ToString(organicdump_proto::ClientType type);
std::string ToString(organicdump_proto::MessageType type);

}; // namespace organicdump

#endif // ORGANICDUMP_ORGANICDUMPMESSAGE_H
