cmake_minimum_required (VERSION 3.25)

set (PLUGIN_NAME "decision-graph")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

find_package (ReFramed REQUIRED)

include (ReFramedPlugin)
include (CheckIncludeFileCXX)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    FORMS
        "forms/SequenceSearchView.ui"
    SOURCES
        "src/models/Graph.cpp"
        "src/models/GraphModel.cpp"
        "src/models/Query.cpp"
        "src/models/RegionItem.cpp"
        "src/models/RegionScene.cpp"
        "src/models/Sequence.cpp"
        "src/models/SequenceSearchModel.cpp"
        "src/models/VisualizerInterface.cpp"
        "src/parsers/QueryParser.y"
        "src/parsers/QueryScanner.lex"
        "src/parsers/QueryASTNode.cpp"
        "src/views/DamageView.cpp"
        "src/views/GraphView.cpp"
        "src/views/HeatMapView.cpp"
        "src/views/PieChartView.cpp"
        "src/views/RegionEditorView.cpp"
        "src/views/SequenceSearchView.cpp"
        "src/views/ShieldHealthView.cpp"
        "src/views/StateListView.cpp"
        "src/views/TimingsView.cpp"
        "src/widgets/PropertyWidget.cpp"
        "src/widgets/PropertyWidget_Damage.cpp"
        "src/widgets/PropertyWidget_DamageConstraints.cpp"
        "src/widgets/PropertyWidget_Graph.cpp"
        "src/widgets/PropertyWidget_HeatMap.cpp"
        "src/widgets/PropertyWidget_PositionConstraints.cpp"
        "src/widgets/PropertyWidget_POV.cpp"
        "src/widgets/PropertyWidget_Query.cpp"
        "src/widgets/PropertyWidget_RelativeConstraints.cpp"
        "src/widgets/PropertyWidget_Shield.cpp"
        "src/widgets/PropertyWidget_ShieldConstraints.cpp"
        "src/widgets/PropertyWidget_Templates.cpp"
        "src/widgets/PropertyWidget_Timings.cpp"
        "src/util/Str.cpp"
        "src/DecisionGraphPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/listeners/GraphModelListener.hpp"
        "include/${PLUGIN_NAME}/listeners/SequenceSearchListener.hpp"
        "include/${PLUGIN_NAME}/models/Edge.hpp"
        "include/${PLUGIN_NAME}/models/Graph.hpp"
        "include/${PLUGIN_NAME}/models/GraphModel.hpp"
        "include/${PLUGIN_NAME}/models/Node.hpp"
        "include/${PLUGIN_NAME}/models/Query.hpp"
        "include/${PLUGIN_NAME}/models/RegionItem.hpp"
        "include/${PLUGIN_NAME}/models/RegionScene.hpp"
        "include/${PLUGIN_NAME}/models/Sequence.hpp"
        "include/${PLUGIN_NAME}/models/SequenceSearchModel.hpp"
        "include/${PLUGIN_NAME}/models/State.hpp"
        "include/${PLUGIN_NAME}/models/VisualizerInterface.hpp"
        "include/${PLUGIN_NAME}/views/DamageView.hpp"
        "include/${PLUGIN_NAME}/views/GraphView.hpp"
        "include/${PLUGIN_NAME}/views/HeatMapView.hpp"
        "include/${PLUGIN_NAME}/views/PieChartView.hpp"
        "include/${PLUGIN_NAME}/views/RegionEditorView.hpp"
        "include/${PLUGIN_NAME}/views/SequenceSearchView.hpp"
        "include/${PLUGIN_NAME}/views/ShieldHealthView.hpp"
        "include/${PLUGIN_NAME}/views/StateListView.hpp"
        "include/${PLUGIN_NAME}/views/TimingsView.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_Damage.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_DamageConstraints.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_HeatMap.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_PositionConstraints.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_RelativeConstraints.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_Shield.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_ShieldConstraints.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_Timings.hpp"
        "include/${PLUGIN_NAME}/parsers/QueryASTNode.hpp"
        "include/${PLUGIN_NAME}/util/Str.hpp"
        "include/${PLUGIN_NAME}/DecisionGraphPlugin.hpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/GraphView.hpp"
        "include/${PLUGIN_NAME}/views/PieChartView.hpp"
        "include/${PLUGIN_NAME}/views/SequenceSearchView.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_Graph.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_POV.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_Query.hpp"
        "include/${PLUGIN_NAME}/widgets/PropertyWidget_Templates.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
    option (${PLUGIN_NAME}_COUNTER_EXAMPLES "Have bison generate counter examples" OFF)

    ###########################################################################
    # Qt Charts library
    ###########################################################################

    find_package (Qt6 COMPONENTS Charts REQUIRED)
    target_link_libraries (plugin-${PLUGIN_NAME}
        PRIVATE Qt6::Charts)

    if (WIN32 OR CYGWIN)
        add_custom_command (TARGET plugin-${PLUGIN_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${REFRAMED_BUILD_PLUGINDEPSDIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt6Charts$<$<CONFIG:Debug>:d>.dll ${REFRAMED_BUILD_PLUGINDEPSDIR}
            COMMENT "Copying Qt binaries from '${Qt6_DIR}/../../bin/' to '${REFRAMED_BUILD_PLUGINDIR}'"
            VERBATIM)
        install (
            FILES ${Qt6_DIR}/../../../bin/Qt6Charts$<$<CONFIG:Debug>:d>.dll
            DESTINATION ${REFRAMED_INSTALL_PLUGINDEPSDIR})
    endif ()

    ###########################################################################
    # Parsers
    ###########################################################################

    find_package (FLEX 2.6 REQUIRED)
    find_package (BISON 3.8 REQUIRED)

    # These may not exist
    file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/parsers")
    file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include/${PLUGIN_NAME}/parsers")

    if (${PLUGIN_NAME}_COUNTER_EXAMPLES)
        set (BISON_COMPILE_FLAGS -Wcounterexamples)
    endif ()
    bison_target (${PLUGIN_NAME}-QueryParser
        "${PROJECT_SOURCE_DIR}/src/parsers/QueryParser.y"
        "${PROJECT_BINARY_DIR}/src/parsers/QueryParser.y.cpp"
        DEFINES_FILE "${PROJECT_BINARY_DIR}/include/${PLUGIN_NAME}/parsers/QueryParser.y.hpp"
        COMPILE_FLAGS ${BISON_COMPILE_FLAGS})
    flex_target (${PLUGIN_NAME}-QueryScanner
        "${PROJECT_SOURCE_DIR}/src/parsers/QueryScanner.lex"
        "${PROJECT_BINARY_DIR}/src/parsers/QueryScanner.lex.cpp"
        DEFINES_FILE "${PROJECT_BINARY_DIR}/include/${PLUGIN_NAME}/parsers/QueryScanner.lex.hpp")
    add_flex_bison_dependency (${PLUGIN_NAME}-QueryScanner ${PLUGIN_NAME}-QueryParser)

    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set_source_files_properties (${FLEX_${PLUGIN_NAME}-QueryScanner_OUTPUTS} PROPERTIES
            COMPILE_FLAGS "/wd4005")
    endif ()

    check_include_file_cxx ("unistd.h" HAVE_UNISTD_H)

    target_sources (plugin-${PLUGIN_NAME}
        PRIVATE
            ${BISON_${PLUGIN_NAME}-QueryParser_OUTPUTS}
            ${FLEX_${PLUGIN_NAME}-QueryScanner_OUTPUTS})
    target_include_directories (plugin-${PLUGIN_NAME}
        PRIVATE
            $<BUILD_INTERFACE:$<$<AND:$<PLATFORM_ID:Windows>,$<NOT:$<BOOL:${HAVE_UNISTD_H}>>>:${PROJECT_SOURCE_DIR}/include/win32_unistd>>)

    ###########################################################################
    # Graph layout library
    ###########################################################################

    set (COIN_PIC ON CACHE BOOL "" FORCE)
    set (OGDF_PIC ON CACHE BOOL "" FORCE)
    add_subdirectory ("thirdparty/ogdf.v2022.02")
    target_link_libraries (plugin-${PLUGIN_NAME}
        PRIVATE OGDF)
endif ()

