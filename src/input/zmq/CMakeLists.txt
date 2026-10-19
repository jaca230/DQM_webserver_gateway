add_dqm_gateway_plugin(dqm_gateway_zmq_source register_plugin.cpp
  zmq_publisher_source.cpp
  zmq_publisher_source_config.cpp)
target_include_directories(dqm_gateway_zmq_source_objects PRIVATE ${CPPZMQ_INCLUDE_DIR})
target_link_libraries(dqm_gateway_zmq_source_objects
  PRIVATE Threads::Threads ${ZMQ_LIBRARY} ${BSD_LIBRARY})
target_link_libraries(dqm_gateway_zmq_source
  PRIVATE Threads::Threads ${ZMQ_LIBRARY} ${BSD_LIBRARY})
