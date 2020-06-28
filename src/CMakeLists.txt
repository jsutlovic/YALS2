if (WIN32)
  file(GLOB_RECURSE YALS2_SOURCES "*.c")
else()
  file(GLOB YALS2_SOURCES "*.c")
endif()

add_executable(YALS2 ${YALS2_SOURCES})
if (WIN32)
target_link_libraries(YALS2 ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})
else()
target_link_libraries(YALS2 m ${SDL2_LIBRARIES} ${SDL2TTF_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
endif()

if (WIN32)
  # Copy DLLs to bin folder on Windows
  add_custom_command ( TARGET YALS2 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${SDL2_DLL}" $<TARGET_FILE_DIR:YALS2>)

  foreach(sdl2_ttf_dll ${SDL2_TTF_DLLS})
      add_custom_command ( TARGET YALS2 POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${sdl2_ttf_dll}" $<TARGET_FILE_DIR:YALS2>)
  endforeach()

  add_custom_command(TARGET YALS2 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${YALS2_SOURCE_DIR}/res/" $<TARGET_FILE_DIR:YALS2>)
endif()

install(TARGETS YALS2 RUNTIME DESTINATION ${BIN_DIR})
