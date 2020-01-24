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
      organicdump_proto::RegisterClient register_client;
      organicdump_proto::RegisterSoilMoistureSensor register_soil_moisture_sensor;
      organicdump_proto::UpdatePeripheralAssociation update_peripheral_association;
      organicdump_proto::BasicResponse basic_response;
  };

  OrganicDumpProtoMessage();
  OrganicDumpProtoMessage(organicdump_proto::Hello msg);
  OrganicDumpProtoMessage(organicdump_proto::RegisterClient msg);
  OrganicDumpProtoMessage(organicdump_proto::RegisterSoilMoistureSensor msg);
  OrganicDumpProtoMessage(organicdump_proto::UpdatePeripheralAssociation msg);
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
    bool *out_cxn_closed);

bool ReadTlsProtobufMessage(
    network::TlsConnection *cxn,
    OrganicDumpProtoMessage *out_msg,
    bool *out_cxn_closed);

std::string ToString(organicdump_proto::ClientType type);
std::string ToString(organicdump_proto::MessageType type);

}; // namespace organicdump

#endif // ORGANICDUMP_ORGANICDUMPMESSAGE_H
