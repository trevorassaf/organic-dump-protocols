# PROTOCOLS

## Steps to build

1. Build protoc. Simply run `make protoc`

2. Compile the .proto file into cpp/h files. Run ` /home/bozkurtus/Documents/codez/personal-repos/organic-dump/organic-dump-project/out/external/protobuf/repo/cmake/protoc --cpp_out=. test.proto`

Or.. for the organic dump project

Run ` /home/bozkurtus/Documents/codez/personal-repos/organic-dump/organic-dump-project/out/external/protobuf/repo/cmake/protoc --cpp_out=. organic_dump.proto`

3. Link protobuf library to target. In relevant CMakeLists.txt, add the following: target_link_libraries(test_proto protobuf::libprotobuf)



