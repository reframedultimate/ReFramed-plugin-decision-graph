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
        "src/DecisionGraphPlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        "include/${PLUGIN_NAME}/views/SequenceSearchView.hpp"
        "include/${PLUGIN_NAME}/models/DecisionGraph.hpp"
        "include/${PLUGIN_NAME}/DecisionGraphPlugin.hpp"
    INCLUDE_DIRECTORIES
        "include"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/SequenceSearchView.hpp")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()

