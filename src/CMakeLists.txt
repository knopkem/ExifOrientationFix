include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# applying global policies
foreach(policy ${project_policies})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()


## #################################################################
## Sources
## #################################################################

SET(RESOURCES
    app.qrc
)

SET(HEADERS_WRAP
    mainWindow.h
    mover.h
)

SET(HEADERS 
    ${HEADERS_WRAP}
    simpleLog.h
)
 
SET(SOURCES 
    main.cpp
    mainWindow.cpp
    mover.cpp
    simpleLog.cpp
)
 
QT5_WRAP_CPP(HEADERS_MOC ${HEADERS_WRAP})
QT5_ADD_RESOURCES(RESOURCES_QRC ${RESOURCES})

if(WIN32)
  SET(SOURCES ${SOURCES} ${PROJECT_NAME}.rc)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_EXECUTABLE(${PROJECT_NAME} 
    ${DEPLOYMENT_SYSTEM} 
    ${HEADERS_MOC}
    ${HEADERS}
    ${SOURCES}
    ${RESOURCES_QRC}
)

qt5_use_modules(${PROJECT_NAME} Core Gui Widgets Concurrent)

# Group common files together in Visual Studio.
SOURCE_GROUP("Header Files" FILES ${HEADERS})
SOURCE_GROUP("Source Files" FILES ${SOURCES})
SOURCE_GROUP("Generated Files" FILES ${HEADERS_MOC})

install(TARGETS ${PROJECT_NAME}  RUNTIME DESTINATION  ${BIN_INSTALL_DIR} BUNDLE DESTINATION  ${BIN_INSTALL_DIR}  )

#deploy qt plugins
INSTALL( DIRECTORY ${QT5_ROOT_PREFIX}/plugins/platforms DESTINATION ${BIN_INSTALL_DIR} )
INSTALL( DIRECTORY ${QT5_ROOT_PREFIX}/plugins/imageformats DESTINATION ${BIN_INSTALL_DIR} )

# deploy using windeploy
if (false)
    # Retrieve the absolute path to qmake and then use that path to find
    # the binaries
    get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:exifOrientationFix>"
                "--dir" "${CMAKE_TEMP}"
    COMMENT "Running windeployqt..."
    )
    INSTALL( DIRECTORY ${CMAKE_TEMP}/ DESTINATION ${BIN_INSTALL_DIR} )
endif()
