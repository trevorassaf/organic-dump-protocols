#INCLUDE(FindProtobuf)
#FIND_PACKAGE(Protobuf REQUIRED)
#ADD_LIBRARY(test_proto ${PROTO_HEADER} ${PROTO_SRC})
#INCLUDE_DIRECTORIES(test_proto ${PROTOBUF_INCLUDE_DIR})
#INCLUDE_DIRECTORIES(test_proto ${CMAKE_CURRENT_BINARY_DIR})
#INCLUDE_DIRECTORIES(test_proto "/home/bozkurtus/Documents/codez/personal-repos/organic-dump/organic-dump-project/code/organic-dump-protocols/repo/external/protobuf/repo/src")
#PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER test.proto)



#INCLUDE(FindProtobuf)
#FIND_PACKAGE(Protobuf REQUIRED)
#INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
#PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER test.proto)
#ADD_LIBRARY(test_proto ${PROTO_HEADER} ${PROTO_SRC})
#INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
#TARGET_INCLUDE_DIRECTORIES(test_proto PRIVATE "/home/bozkurtus/Documents/codez/personal-repos/organic-dump/organic-dump-project/code/organic-dump-protocols/repo/external/protobuf/repo/src")

cmake_minimum_required(VERSION 3.1)

include_directories(../../../external/protobuf/repo/src/google/protbuf
                    ../../../external/boringssl/repo/include
                    ../../../code/organic-dump-network/repo)

add_library(test_proto SHARED test.pb.cc)
target_link_libraries(test_proto protobuf::libprotobuf)

add_library(organic_dump_proto SHARED organic_dump.pb.cc OrganicDumpProtoMessage.cpp)
target_link_libraries(organic_dump_proto protobuf::libprotobuf)
target_link_libraries(organic_dump_proto glog::glog)
target_link_libraries(organic_dump_proto organic_dump_network)
