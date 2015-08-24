add_executable(YALS2 main.c)
target_link_libraries(YALS2)
install(TARGETS YALS2 RUNTIME DESTINATION ${BIN_DIR})
