#ifndef ORGANICDUMP_ORGANICDUMPMESSAGE_H
#define ORGANICDUMP_ORGANICDUMPMESSAGE_H

#include "organic_dump.pb.h"

namespace organicdump
{

struct OrganicDumpProtoMessage
{
  organicdump_proto::MessageType type;

  union
  {
      organicdump_proto::ServerEcho server_echo;
      organicdump_proto::Hello hello;
      organicdump_proto::RegisterClient register_client;
      organicdump_proto::RegisterSoilMoistureSensor register_soil_moisture_sensor;
      organicdump_proto::UpdatePeripheralAssociation update_peripheral_association;
      organicdump_proto::ClientEcho client_echo;
      organicdump_proto::BasicResponse basic_response;
  };
};

}; // namespace organicdump

#endif // ORGANICDUMP_ORGANICDUMPMESSAGE_H
