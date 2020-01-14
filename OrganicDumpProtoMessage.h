#ifndef ORGANICDUMP_ORGANICDUMPMESSAGE_H
#define ORGANICDUMP_ORGANICDUMPMESSAGE_H

#include "organic_dump.pb.h"

#include "TlsConnection.h"

namespace organicdump
{

struct OrganicDumpProtoMessage
{
  organicdump_proto::MessageType type;

  union
  {
      organicdump_proto::Echo echo;
      organicdump_proto::Hello hello;
      organicdump_proto::RegisterClient register_client;
      organicdump_proto::RegisterSoilMoistureSensor register_soil_moisture_sensor;
      organicdump_proto::UpdatePeripheralAssociation update_peripheral_association;
      organicdump_proto::BasicResponse basic_response;
  };
};

bool SendTlsProtobufMessage(
    network::TlsConnection *cxn,
    OrganicDumpProtoMessage *msg,
    bool *out_cxn_closed);

bool ReadTlsProtobufMessage(
    network::TlsConnection *cxn,
    OrganicDumpProtoMessage *msg,
    bool *out_again,
    bool *out_cxn_closed);

}; // namespace organicdump

#endif // ORGANICDUMP_ORGANICDUMPMESSAGE_H
