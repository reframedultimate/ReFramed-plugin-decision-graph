cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "decision-graph")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

find_package (ReFramed REQUIRED)
include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    FORMS
        "forms/SequenceSearchView.ui"
    SOURCES
        "src/views/SequenceSearchView.cpp"
        "src/models/DecisionGraph.cpp"
        "src/models/GraphBuilder.cpp"
        "src/models/GraphQuery.cpp"
        "src/models/GraphQueryBuilder.cpp"
        "src/DecisionGraphPlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        "include/${PLUGIN_NAME}/listeners/GraphBuilderListener.hpp"
        "include/${PLUGIN_NAME}/models/DecisionGraph.hpp"
        "include/${PLUGIN_NAME}/models/GraphBuilder.hpp"
        "include/${PLUGIN_NAME}/models/GraphQuery.hpp"
        "include/${PLUGIN_NAME}/models/GraphQueryBuilder.hpp"
        "include/${PLUGIN_NAME}/views/SequenceSearchView.hpp"
        "include/${PLUGIN_NAME}/DecisionGraphPlugin.hpp"
    INCLUDE_DIRECTORIES
        "include"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/SequenceSearchView.hpp")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
    add_subdirectory ("thirdparty/ogdf.v2022.02")
    target_link_libraries (plugin-${PLUGIN_NAME}
        PRIVATE OGDF)
    set (COIN_PIC ON CACHE BOOL "" FORCE)
    set (OGDF_PIC ON CACHE BOOL "" FORCE)
endif ()

