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
        "src/models/Graph.cpp"
        "src/models/IncrementalData.cpp"
        "src/models/Query.cpp"
        "src/models/QueryBuilder.cpp"
        "src/models/Sequence.cpp"
        "src/DecisionGraphPlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        "include/${PLUGIN_NAME}/listeners/IncrementalDataListener.hpp"
        "include/${PLUGIN_NAME}/models/Edge.hpp"
        "include/${PLUGIN_NAME}/models/Graph.hpp"
        "include/${PLUGIN_NAME}/models/IncrementalData.hpp"
        "include/${PLUGIN_NAME}/models/Node.hpp"
        "include/${PLUGIN_NAME}/models/Query.hpp"
        "include/${PLUGIN_NAME}/models/QueryBuilder.hpp"
        "include/${PLUGIN_NAME}/models/Sequence.hpp"
        "include/${PLUGIN_NAME}/models/State.hpp"
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

