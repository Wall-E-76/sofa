cmake_minimum_required(VERSION 3.1)
project(runSofa)

set(RESOURCE_FILES
    resources/docs/runsofa.html
    )

if(APPLE)
    set(RC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/runSofa.icns")
else()
    set(RC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/runSofa.rc")
endif()

include(cmake/GeneratePluginConfig.cmake)
# if MSVC then plugins are located in bin/ instead of lib/ on Mac and Linux
if(MSVC)
    set(_pluginLocation "bin")
else()
    set(_pluginLocation "lib")
endif()
set(_configPluginFileName plugin_list.conf)
set(_defaultConfigPluginFileName "${_configPluginFileName}.default")
set(_defaultConfigPluginFilePath "${CMAKE_BINARY_DIR}/${_pluginLocation}/${_defaultConfigPluginFileName}")
sofa_generate_plugin_config(${_defaultConfigPluginFilePath})
message("Write Plugin list at ${_defaultConfigPluginFilePath}")

if(APPLE)
    set_source_files_properties(${RC_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(APPLE AND SOFA_BUILD_APP_BUNDLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${RC_FILES} ${RESOURCE_FILES} Main.cpp )
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/runSofa.plist")
else()
    add_executable(${PROJECT_NAME} ${RC_FILES} ${RESOURCE_FILES} Main.cpp)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC "CONFIG_PLUGIN_FILENAME=${_configPluginFileName}")
target_compile_definitions(${PROJECT_NAME} PUBLIC "DEFAULT_CONFIG_PLUGIN_FILENAME=${_defaultConfigPluginFileName}")
target_link_libraries(${PROJECT_NAME} SofaComponentAdvanced SofaComponentMisc)
target_link_libraries(${PROJECT_NAME} SofaSimulationGraph)
target_link_libraries(${PROJECT_NAME} SofaGuiMain)

if(SOFA_BUILD_TESTS)
    find_package(SofaTest QUIET)
    if(SofaTest_FOUND)
        add_subdirectory(runSofa_test)
    endif()
endif()

# Create build and install versions of .ini file for resources finding
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/etc/${PROJECT_NAME}.ini.in" "${CMAKE_BINARY_DIR}/etc/${PROJECT_NAME}.ini")
set(RESOURCES_DIR "../share/sofa/gui/runSofa")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/etc/${PROJECT_NAME}.ini.in" "${CMAKE_BINARY_DIR}/etc/installed${PROJECT_NAME}.ini")
install(FILES "${CMAKE_BINARY_DIR}/etc/installed${PROJECT_NAME}.ini" DESTINATION etc RENAME ${PROJECT_NAME}.ini COMPONENT applications)

install(DIRECTORY "resources/" DESTINATION "share/sofa/gui/runSofa" COMPONENT resources)
install(FILES "${_defaultConfigPluginFilePath}" DESTINATION ${_pluginLocation}/ COMPONENT applications)

sofa_install_targets(SofaGui runSofa runSofa)

if(APPLE AND SOFA_BUILD_RELEASE_PACKAGE)
    if(SOFA_BUILD_APP_BUNDLE)
        install(CODE "file(WRITE \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/../Resources/qt.conf\" \"[Paths]\\n  Plugins = MacOS/bin\")" COMPONENT applications)
    endif()

    # postInstall.cmake will be executed at the end of install
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch")
    add_custom_target(post_install ALL
        COMMENT "${PROJECT_NAME}: Patching cmake_install.cmake (to run post install script)."
        COMMAND
            test ! -e ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            && cat ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake > ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            
            && printf \"\\n\" >> ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            && printf \"execute_process ( \" >> ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            && printf 'COMMAND' >> ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            && printf \" /bin/bash \\\"${CMAKE_SOURCE_DIR}/scripts/macos-postinstall-fixup.sh\\\" \\\"\\\$$ENV\{DESTDIR\}\\\$$\{CMAKE_INSTALL_PREFIX\}\\\" ) \" >> ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            && printf \"\\n\" >> ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch
            
            && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmakepatch ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
            || true
        )
endif()
