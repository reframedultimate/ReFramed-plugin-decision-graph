cmake_minimum_required (VERSION 3.25)

project ("ogdf"
    LANGUAGES CXX)

include(CheckCXXSourceCompiles)

list (INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/modules")

set (OGDF_LIB_TYPE "STATIC" CACHE STRING "Build as a SHARED or STATIC library")
option (OGDF_PIC "Generate position independent code" OFF)
set_property (CACHE OGDF_LIB_TYPE PROPERTY STRINGS "SHARED" "STATIC")
set (OGDF_MEMORY_MANAGER "POOL_TS" CACHE STRING "Memory manager to be used")
set_property (CACHE OGDF_MEMORY_MANAGER PROPERTY STRINGS "POOL_TS" "POOL_NTS" "MALLOC_TS")
set (OGDF_DEBUG_MODE "REGULAR" CACHE STRING "Whether to use (heavy) OGDF assertions in debug mode")
set_property (CACHE OGDF_DEBUG_MODE PROPERTY STRINGS "NONE" "REGULAR" "HEAVY")
mark_as_advanced (CACHE OGDF_DEBUG_MODE)
option (OGDF_USE_ASSERT_EXCEPTIONS "Whether to throw an exception on failed assertions." OFF)
set (OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE "OFF" CACHE
    STRING "Which library (libdw, libbdf, libunwind) to use in case a stack trace should be written \
    to a failed assertion exceptions's what(). Library must be found by CMake to be able to use it.")
if (OGDF_USE_ASSERT_EXCEPTIONS)
    set_property (CACHE OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE PROPERTY STRINGS "OFF")
else ()
    unset (OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE CACHE)
endif ()
option (OGDF_WARNING_ERRORS "Whether to treat compiler warnings as errors; may break compilation!" OFF)
mark_as_advanced (OGDF_WARNING_ERRORS)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND OGDF_MEMORY_MANAGER STREQUAL MALLOC_TS)
    option (OGDF_LEAK_CHECK "Whether to use the address sanitizer for the MALLOC_TS memory manager." OFF)
else()
    unset (OGDF_LEAK_CHECK CACHE)
endif()

# set debug mode
if (OGDF_DEBUG_MODE STREQUAL "HEAVY")
    set (OGDF_HEAVY_DEBUG ON)
endif ()
if (NOT OGDF_DEBUG_MODE STREQUAL NONE AND (MULTICONFIG_BUILD OR CMAKE_BUILD_TYPE MATCHES Debug))
    set(OGDF_DEBUG ON)
endif()

# find available packages for stack traces
if (OGDF_USE_ASSERT_EXCEPTIONS)
      find_package (Libdw)
      if (LIBDW_FOUND)
          set_property (CACHE OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE APPEND PROPERTY STRINGS "ON_LIBDW")
      endif ()
      find_package (Libbfd)
      if (LIBBFD_FOUND)
          set_property (CACHE OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE APPEND PROPERTY STRINGS "ON_LIBBFD")
      endif ()
      find_package (Libunwind)
      if (LIBUNWIND_FOUND)
          set_property (CACHE OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE APPEND PROPERTY STRINGS "ON_LIBUNWIND")
      endif()
endif()

# static analysis using clang-tidy
option(OGDF_ENABLE_CLANG_TIDY "Enable static analysis using clang-tidy" OFF)

if(OGDF_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY};-extra-arg=-Wno-unknown-warning-option)
    else()
        message(WARNING "clang-tidy not found!")
    endif()
endif()

# autogen header variables for debug mode
set (SHOW_STACKTRACE 0)
if (OGDF_DEBUG AND OGDF_USE_ASSERT_EXCEPTIONS)
    check_cxx_source_compiles ("
#include <string>
int main() {
    std::string copy(__PRETTY_FUNCTION__);
    return 0;
}" 
    compiler_has_pretty_function)

    if (compiler_has_pretty_function)
        set (OGDF_FUNCTION_NAME "__PRETTY_FUNCTION__")
    else () # fallback to C++11 standard
        set (OGDF_FUNCTION_NAME "__func__")
    endif ()
    if (OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE MATCHES LIBDW)
        set (SHOW_STACKTRACE 1)
        set (BACKWARD_HAS_DW 1)
    elseif (OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE MATCHES LIBBFD)
        set (SHOW_STACKTRACE 1)
        set (BACKWARD_HAS_BFD 1)
    elseif (OGDF_USE_ASSERT_EXCEPTIONS_WITH_STACK_TRACE MATCHES LIBUNWIND)
        set (SHOW_STACKTRACE 1)
        set (BACKWARD_HAS_UNWIND 1)
    endif()
endif()

# autogen header variables if libs are shared
if (OGDF_LIB_TYPE STREQUAL "SHARED")
    set (OGDF_DLL 1)
endif ()

# autogen header variables for mallinfo2
check_cxx_source_compiles ("
#include <malloc.h>
int main() {
    struct mallinfo2 mi = mallinfo2();
    return 0;
}" 
has_mallinfo2)
if (has_mallinfo2)
    set (OGDF_HAS_MALLINFO2 1)
endif ()

# autogen header variables for SSE3
check_cxx_source_compiles ("
#include <pmmintrin.h>
int main() {
    __m128d a = _mm_set1_pd(42), b = _mm_set1_pd(23);
    _mm_hadd_pd(a, b);
    return 0;
}" 
has_sse3_pmmintrin)
if (NOT has_sse3_pmmintrin)
    check_cxx_source_compiles ("
#include <intrin.h>
int main() {
    __m128d a = _mm_set1_pd(42), b = _mm_set1_pd(23);
    _mm_hadd_pd(a, b);
    return 0;
}" 
    has_sse3_intrin)
endif()
if (has_sse3_intrin)
    set (OGDF_SSE3_EXTENSIONS <intrin.h>)
elseif (has_sse3_pmmintrin)
    set (OGDF_SSE3_EXTENSIONS <pmmintrin.h>)
else ()
    message (STATUS "SSE3 could not be activated")
endif ()

# autogen header variables for Linux-specific CPU_SET, etc.
check_cxx_source_compiles ("
#include <sched.h>
int main() {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    return 0;
}" 
has_linux_cpu_macros)
if (has_linux_cpu_macros)
    set (OGDF_HAS_LINUX_CPU_MACROS 1)
endif ()

# add stack trace settings
if (BACKWARD_HAS_DW)
    target_include_directories (OGDF SYSTEM PUBLIC ${LIBDW_INCLUDE_DIR})
elseif (BACKWARD_HAS_BFD)
    target_include_directories (OGDF SYSTEM PUBLIC ${LIBBFD_INCLUDE_DIR} ${LIBDL_INCLUDE_DIR})
elseif (BACKWARD_HAS_UNWIND)
    target_include_directories (OGDF SYSTEM PUBLIC ${LIBUNWIND_INCLUDE_DIR})
endif()
if(SHOW_STACKTRACE)
    if(BACKWARD_HAS_DW)
        target_link_libraries (OGDF PUBLIC ${LIBDW_LIBRARY})
    elseif(BACKWARD_HAS_BFD)
        target_link_libraries (OGDF PUBLIC ${LIBBFD_LIBRARY} ${LIBDL_LIBRARY})
    elseif(BACKWARD_HAS_UNWIND)
        target_link_libraries (OGDF PUBLIC ${LIBUNWIND_LIBRARY}    )
    endif()
endif()

configure_file ("templates/config_autogen.h.in" "include/ogdf/basic/internal/config_autogen.h")

add_library (OGDF ${OGDF_LIB_TYPE}
    "src/planarlayout/PlanarDrawLayout.cpp"
    "src/planarlayout/MMCBBase.cpp"
    "src/planarlayout/BitonicOrdering.cpp"
    "src/planarlayout/GridLayoutModule.cpp"
    "src/planarlayout/ShellingOrderModule.cpp"
    "src/planarlayout/MixedModelCrossingsBeautifierModule.cpp"
    "src/planarlayout/FPPLayout.cpp"
    "src/planarlayout/SchnyderLayout.cpp"
    "src/planarlayout/LeftistOrdering.cpp"
    "src/planarlayout/PlanarStraightLayout.cpp"
    "src/planarlayout/mixed_model_layout/MixedModelBase.cpp"
    "src/planarlayout/mixed_model_layout/IOPoints.cpp"
    "src/planarlayout/MixedModelLayout.cpp"
    "src/planarlayout/ShellingOrder.cpp"
    "src/planarlayout/BiconnectedShellingOrder.cpp"
    "src/planarlayout/TriconnectedShellingOrder.cpp"
    "src/orthogonal/LongestPathCompaction.cpp"
    "src/orthogonal/OrthoRep.cpp"
    "src/orthogonal/edge_router/NodeInfo.cpp"
    "src/orthogonal/CompactionConstraintGraph.cpp"
    "src/orthogonal/OrthoShaper.cpp"
    "src/orthogonal/EdgeRouter.cpp"
    "src/orthogonal/OrthoLayout.cpp"
    "src/orthogonal/FlowCompaction.cpp"
    "src/orthogonal/internal/CommonCompactionConstraintGraphBase.cpp"
    "src/planarity/PlanarizerStarReinsertion.cpp"
    "src/planarity/embedding_inserter/VarEdgeInserterCore.cpp"
    "src/planarity/embedding_inserter/FixEdgeInserterCore.cpp"
    "src/planarity/embedding_inserter/VarEdgeInserterDynCore.cpp"
    "src/planarity/PlanarizationLayout.cpp"
    "src/planarity/planar_subgraph_fast/PlanarSubgraphPQTree.cpp"
    "src/planarity/PlanRepInc.cpp"
    "src/planarity/MMCrossingMinimizationModule.cpp"
    "src/planarity/BoyerMyrvold.cpp"
    "src/planarity/SimpleIncNodeInserter.cpp"
    "src/planarity/MMFixedEmbeddingInserter.cpp"
    "src/planarity/boyer_myrvold/BoyerMyrvoldInit.cpp"
    "src/planarity/boyer_myrvold/FindKuratowskis.cpp"
    "src/planarity/boyer_myrvold/BoyerMyrvoldPlanar.cpp"
    "src/planarity/EmbedderMaxFace.cpp"
    "src/planarity/MMSubgraphPlanarizer.cpp"
    "src/planarity/MultiEdgeApproxInserter.cpp"
    "src/planarity/VariableEmbeddingInserterDyn.cpp"
    "src/planarity/SimpleEmbedder.cpp"
    "src/planarity/ExtractKuratowskis.cpp"
    "src/planarity/MMVariableEmbeddingInserter.cpp"
    "src/planarity/PlanRepExpansion.cpp"
    "src/planarity/PlanarizerMixedInsertion.cpp"
    "src/planarity/PlanarizationGridLayout.cpp"
    "src/planarity/booth_lueker/PlanarPQTree.cpp"
    "src/planarity/booth_lueker/EmbedPQTree.cpp"
    "src/planarity/PlanRepLight.cpp"
    "src/planarity/PlanRep.cpp"
    "src/planarity/BoothLueker.cpp"
    "src/planarity/EmbedderMinDepthMaxFaceLayers.cpp"
    "src/planarity/EmbedderMinDepthMaxFace.cpp"
    "src/planarity/PlanarizerChordlessCycle.cpp"
    "src/planarity/planarization_layout/CliqueReplacer.cpp"
    "src/planarity/FixedEmbeddingInserter.cpp"
    "src/planarity/PlanarSubgraphBoyerMyrvold.cpp"
    "src/planarity/SubgraphPlanarizer.cpp"
    "src/planarity/EmbedderMaxFaceLayers.cpp"
    "src/planarity/VariableEmbeddingInserter.cpp"
    "src/planarity/EmbedderMinDepthPiTa.cpp"
    "src/planarity/EmbedderMinDepth.cpp"
    "src/planarity/EmbedderOptimalFlexDraw.cpp"
    "src/planarity/StarInserter.cpp"
    "src/planarity/TopologyModule.cpp"
    "src/planarity/embedder/CrossingStructure.cpp"
    "src/tree/RadialTreeLayout.cpp"
    "src/tree/TreeLayout.cpp"
    "src/tree/LCA.cpp"
    "src/decomposition/DynamicSPQRForest.cpp"
    "src/decomposition/DynamicSPQRTree.cpp"
    "src/decomposition/DynamicBCTree.cpp"
    "src/decomposition/BCTree.cpp"
    "src/decomposition/PlanarSPQRTree.cpp"
    "src/decomposition/StaticSPQRTree.cpp"
    "src/clique/CliqueFinderSPQR.cpp"
    "src/clique/CliqueFinderHeuristic.cpp"
    "src/clique/CliqueFinderModule.cpp"
    "src/graphalg/ConnectivityTester.cpp"
    "src/graphalg/MaxAdjOrdering.cpp"
    "src/graphalg/MinimumCut.cpp"
    "src/graphalg/PageRank.cpp"
    "src/graphalg/ShortestPathWithBFM.cpp"
    "src/graphalg/EdgeIndependentSpanningTrees.cpp"
    "src/graphalg/ConvexHull.cpp"
    "src/graphalg/Clusterer.cpp"
    "src/graphalg/ModifiedNibbleClusterer.cpp"
    "src/graphalg/Triconnectivity.cpp"
    "src/graphalg/Matching.cpp"
    "src/graphalg/GraphReduction.cpp"
    "src/augmentation/PlanarAugmentationFix.cpp"
    "src/augmentation/DfsMakeBiconnected.cpp"
    "src/augmentation/PlanarAugmentation.cpp"
    "src/cluster/ClusterGraphAttributes.cpp"
    "src/cluster/ClusterPlanarizationLayout.cpp"
    "src/cluster/MaximumCPlanarSubgraph.cpp"
    "src/cluster/internal/ChunkConnection.cpp"
    "src/cluster/internal/CutConstraint.cpp"
    "src/cluster/internal/EdgeVar.cpp"
    "src/cluster/internal/CPlanarityMaster.cpp"
    "src/cluster/internal/CPlanaritySub.cpp"
    "src/cluster/internal/ClusterKuratowskiConstraint.cpp"
    "src/cluster/internal/MinimalClusterConnection.cpp"
    "src/cluster/internal/MaxCPlanarMaster.cpp"
    "src/cluster/internal/MaxPlanarEdgesConstraint.cpp"
    "src/cluster/internal/MaxCPlanarSub.cpp"
    "src/cluster/internal/CP_MasterBase.cpp"
    "src/cluster/internal/CPlanarSubClusteredST.cpp"
    "src/cluster/ClusterPlanarity.cpp"
    "src/cluster/ClusterGraph.cpp"
    "src/cluster/CPlanarSubClusteredGraph.cpp"
    "src/cluster/ClusterOrthoShaper.cpp"
    "src/cluster/ClusterOrthoLayout.cpp"
    "src/cluster/ClusterAnalysis.cpp"
    "src/cluster/CPlanarEdgeInserter.cpp"
    "src/cluster/ClusterPlanRep.cpp"
    "src/cluster/CconnectClusterPlanarEmbed.cpp"
    "src/cluster/CconnectClusterPlanar.cpp"
    "src/cluster/HananiTutteCPlanarity.cpp"
    "src/upward/LayerBasedUPRLayout.cpp"
    "src/upward/FixedEmbeddingUpwardEdgeInserter.cpp"
    "src/upward/internal/UpwardPlanarityEmbeddedDigraph.cpp"
    "src/upward/internal/UpSAT.cpp"
    "src/upward/internal/UpwardPlanaritySingleSource.cpp"
    "src/upward/SubgraphUpwardPlanarizer.cpp"
    "src/upward/UpwardPlanRep.cpp"
    "src/upward/UpwardPlanarity.cpp"
    "src/upward/VisibilityLayout.cpp"
    "src/upward/ExpansionGraph.cpp"
    "src/upward/UpwardPlanarSubgraphModule.cpp"
    "src/upward/DominanceLayout.cpp"
    "src/upward/FaceSinkGraph.cpp"
    "src/upward/UpwardPlanarSubgraphSimple.cpp"
    "src/upward/FUPSSimple.cpp"
    "src/upward/MaximalFUPS.cpp"
    "src/basic/Logger.cpp"
    "src/basic/Graph.cpp"
    "src/basic/extended_graph_alg.cpp"
    "src/basic/NearestRectangleFinder.cpp"
    "src/basic/Layout.cpp"
    "src/basic/PreprocessorLayout.cpp"
    "src/basic/LayoutStatistics_intersect.cpp"
    "src/basic/STNumbering.cpp"
    "src/basic/memory/PoolMemoryAllocator.cpp"
    "src/basic/CombinatorialEmbedding.cpp"
    "src/basic/System.cpp"
    "src/basic/basic.cpp"
    "src/basic/graphics.cpp"
    "src/basic/EdgeComparerSimple.cpp"
    "src/basic/GF2Solver.cpp"
    "src/basic/LayoutStatistics.cpp"
    "src/basic/graph_generators/randomHierarchy.cpp"
    "src/basic/graph_generators/operations.cpp"
    "src/basic/graph_generators/randomized.cpp"
    "src/basic/graph_generators/deterministic.cpp"
    "src/basic/LayoutStandards.cpp"
    "src/basic/geometry.cpp"
    "src/basic/GraphAttributes.cpp"
    "src/basic/Stopwatch.cpp"
    "src/basic/internal/config.cpp"
    "src/basic/GraphCopy.cpp"
    "src/basic/AdjacencyOracle.cpp"
    "src/basic/Module.cpp"
    "src/basic/Hashing.cpp"
    "src/basic/EdgeComparer.cpp"
    "src/basic/simple_graph_alg.cpp"
    "src/basic/GridLayout.cpp"
    "src/basic/Math.cpp"
    "src/layered/OptimalHierarchyLayout.cpp"
    "src/layered/GreedyInsertHeuristic.cpp"
    "src/layered/CrossingsMatrix.cpp"
    "src/layered/BlockOrder.cpp"
    "src/layered/SugiyamaLayout.cpp"
    "src/layered/MedianHeuristic.cpp"
    "src/layered/GreedyCycleRemoval.cpp"
    "src/layered/OptimalRanking.cpp"
    "src/layered/FastHierarchyLayout.cpp"
    "src/layered/ExtendedNestingGraph.cpp"
    "src/layered/DfsAcyclicSubgraph.cpp"
    "src/layered/SplitHeuristic.cpp"
    "src/layered/CrossingMinInterfaces.cpp"
    "src/layered/LongestPathRanking.cpp"
    "src/layered/AcyclicSubgraphModule.cpp"
    "src/layered/SiftingHeuristic.cpp"
    "src/layered/HierarchyLayoutModule.cpp"
    "src/layered/OptimalHierarchyClusterLayout.cpp"
    "src/layered/GreedySwitchHeuristic.cpp"
    "src/layered/BarycenterHeuristic.cpp"
    "src/layered/FastSimpleHierarchyLayout.cpp"
    "src/layered/CoffmanGrahamRanking.cpp"
    "src/uml/OrthoLayoutUML.cpp"
    "src/uml/UmlModelGraph.cpp"
    "src/uml/UmlDiagramGraph.cpp"
    "src/uml/UMLCrossingMinimizationModule.cpp"
    "src/uml/VariableEmbeddingInserterUML.cpp"
    "src/uml/UMLGraph.cpp"
    "src/uml/PlanarizationLayoutUML_inc.cpp"
    "src/uml/VariableEmbeddingInserterDynUML.cpp"
    "src/uml/FixedEmbeddingInserterUML.cpp"
    "src/uml/PlanarizationLayoutUML.cpp"
    "src/uml/PlanRepUML.cpp"
    "src/uml/SubgraphPlanarizerUML.cpp"
    "src/packing/CCLayoutPackModule.cpp"
    "src/packing/TileToRowsCCPacker.cpp"
    "src/packing/ComponentSplitterLayout.cpp"
    "src/packing/SimpleCCPacker.cpp"
    "src/energybased/SpringEmbedderFRExact.cpp"
    "src/energybased/multilevel_mixer/BarycenterPlacer.cpp"
    "src/energybased/multilevel_mixer/SolarPlacer.cpp"
    "src/energybased/multilevel_mixer/LocalBiconnectedMerger.cpp"
    "src/energybased/multilevel_mixer/ModularMultilevelMixer.cpp"
    "src/energybased/multilevel_mixer/SolarMerger.cpp"
    "src/energybased/multilevel_mixer/EdgeCoverMerger.cpp"
    "src/energybased/multilevel_mixer/RandomMerger.cpp"
    "src/energybased/multilevel_mixer/ScalingLayout.cpp"
    "src/energybased/multilevel_mixer/CirclePlacer.cpp"
    "src/energybased/multilevel_mixer/MultilevelGraph.cpp"
    "src/energybased/multilevel_mixer/MatchingMerger.cpp"
    "src/energybased/multilevel_mixer/RandomPlacer.cpp"
    "src/energybased/multilevel_mixer/MedianPlacer.cpp"
    "src/energybased/multilevel_mixer/ZeroPlacer.cpp"
    "src/energybased/multilevel_mixer/IndependentSetMerger.cpp"
    "src/energybased/SpringEmbedderGridVariant.cpp"
    "src/energybased/FMMMLayout.cpp"
    "src/energybased/spring_embedder/SEGV_ForceModel.cpp"
    "src/energybased/DavidsonHarelLayout.cpp"
    "src/energybased/NodeRespecterLayout.cpp"
    "src/energybased/StressMinimization.cpp"
    "src/energybased/GEMLayout.cpp"
    "src/energybased/DavidsonHarel.cpp"
    "src/energybased/FastMultipoleEmbedder.cpp"
    "src/energybased/dtree/GalaxyLevel.cpp"
    "src/energybased/fast_multipole_embedder/FMEKernel.cpp"
    "src/energybased/fast_multipole_embedder/LinearQuadtree.cpp"
    "src/energybased/fast_multipole_embedder/GalaxyMultilevel.cpp"
    "src/energybased/fast_multipole_embedder/EdgeChain.cpp"
    "src/energybased/fast_multipole_embedder/FMEThread.cpp"
    "src/energybased/fast_multipole_embedder/WSPD.cpp"
    "src/energybased/fast_multipole_embedder/ArrayGraph.cpp"
    "src/energybased/fast_multipole_embedder/FMEMultipoleKernel.cpp"
    "src/energybased/fast_multipole_embedder/LinearQuadtreeBuilder.cpp"
    "src/energybased/fast_multipole_embedder/LinearQuadtreeExpansion.cpp"
    "src/energybased/MultilevelLayout.cpp"
    "src/energybased/SpringEmbedderKK.cpp"
    "src/energybased/PivotMDS.cpp"
    "src/energybased/davidson_harel/EnergyFunction.cpp"
    "src/energybased/davidson_harel/Overlap.cpp"
    "src/energybased/davidson_harel/PlanarityGrid.cpp"
    "src/energybased/davidson_harel/Planarity.cpp"
    "src/energybased/davidson_harel/NodePairEnergy.cpp"
    "src/energybased/davidson_harel/Attraction.cpp"
    "src/energybased/davidson_harel/UniformGrid.cpp"
    "src/energybased/davidson_harel/Repulsion.cpp"
    "src/energybased/fmmm/NewMultipoleMethod.cpp"
    "src/energybased/fmmm/EdgeAttributes.cpp"
    "src/energybased/fmmm/new_multipole_method/QuadTreeNM.cpp"
    "src/energybased/fmmm/new_multipole_method/QuadTreeNodeNM.cpp"
    "src/energybased/fmmm/numexcept.cpp"
    "src/energybased/fmmm/Set.cpp"
    "src/energybased/fmmm/FruchtermanReingold.cpp"
    "src/energybased/fmmm/MAARPacking.cpp"
    "src/energybased/fmmm/NodeAttributes.cpp"
    "src/energybased/fmmm/Multilevel.cpp"
    "src/energybased/TutteLayout.cpp"
    "src/lib/minisat/core/Solver.cpp"
    "src/lib/minisat/simp/SimpSolver.cpp"
    "src/lib/abacus/opensub.cpp"
    "src/lib/abacus/sparvec.cpp"
    "src/lib/abacus/optsense.cpp"
    "src/lib/abacus/sub.cpp"
    "src/lib/abacus/aba_constraint.cpp"
    "src/lib/abacus/variable.cpp"
    "src/lib/abacus/fixcand.cpp"
    "src/lib/abacus/tailoff.cpp"
    "src/lib/abacus/osiif.cpp"
    "src/lib/abacus/valbranchrule.cpp"
    "src/lib/abacus/lpmasterosi.cpp"
    "src/lib/abacus/conclass.cpp"
    "src/lib/abacus/convar.cpp"
    "src/lib/abacus/lp.cpp"
    "src/lib/abacus/setbranchrule.cpp"
    "src/lib/abacus/slackstat.cpp"
    "src/lib/abacus/global.cpp"
    "src/lib/abacus/column.cpp"
    "src/lib/abacus/master.cpp"
    "src/lib/abacus/lpvarstat.cpp"
    "src/lib/abacus/row.cpp"
    "src/lib/abacus/conbranchrule.cpp"
    "src/lib/abacus/vartype.cpp"
    "src/lib/abacus/abacusroot.cpp"
    "src/lib/abacus/fsvarstat.cpp"
    "src/lib/abacus/srowcon.cpp"
    "src/lib/abacus/infeascon.cpp"
    "src/lib/abacus/csense.cpp"
    "src/lib/abacus/branchrule.cpp"
    "src/lib/abacus/lpif.cpp"
    "src/lib/abacus/lpsub.cpp"
    "src/lib/abacus/history.cpp"
    "src/lib/pugixml/pugixml.cpp"
    "src/lpsolver/LPSolver.cpp"
    "src/external/backward.cpp"
    "src/external/Minisat.cpp"
    "src/misclayout/CircularLayout.cpp"
    "src/misclayout/LinearLayout.cpp"
    "src/misclayout/BertaultLayout.cpp"
    "src/misclayout/ProcrustesSubLayout.cpp"
    "src/misclayout/BalloonLayout.cpp"
    "src/hypergraph/HypergraphLayout.cpp"
    "src/hypergraph/EdgeStandardRep.cpp"
    "src/hypergraph/Hypergraph.cpp"
    "src/simultaneous/SimDrawManipulatorModule.cpp"
    "src/simultaneous/SimDrawCreatorSimple.cpp"
    "src/simultaneous/SimDrawCaller.cpp"
    "src/simultaneous/SimDrawColorizer.cpp"
    "src/simultaneous/SimDraw.cpp"
    "src/simultaneous/SimDrawCreator.cpp"
    "src/fileformats/GraphIO.cpp"
    "src/fileformats/GML.cpp"
    "src/fileformats/Utils.cpp"
    "src/fileformats/GraphIO_dl.cpp"
    "src/fileformats/GraphIO_hyper.cpp"
    "src/fileformats/TsplibXmlParser.cpp"
    "src/fileformats/GraphIO_gdf.cpp"
    "src/fileformats/DOT.cpp"
    "src/fileformats/GraphIO_tlp.cpp"
    "src/fileformats/GraphIO_gexf.cpp"
    "src/fileformats/GraphIO_leda.cpp"
    "src/fileformats/GraphIO_graphml.cpp"
    "src/fileformats/SvgPrinter.cpp"
    "src/fileformats/DotParser.cpp"
    "src/fileformats/TlpLexer.cpp"
    "src/fileformats/GDF.cpp"
    "src/fileformats/GraphIO_dot.cpp"
    "src/fileformats/DotLexer.cpp"
    "src/fileformats/GexfParser.cpp"
    "src/fileformats/GraphMLParser.cpp"
    "src/fileformats/TlpParser.cpp"
    "src/fileformats/GEXF.cpp"
    "src/fileformats/GraphIO_gml.cpp"
    "src/fileformats/Tlp.cpp"
    "src/fileformats/GraphML.cpp"
    "src/fileformats/GdfParser.cpp"
    "src/fileformats/GraphIO_g6.cpp"
    "src/fileformats/DLParser.cpp"
    "src/fileformats/GmlParser.cpp"
    "src/labeling/ELabelPosSimple.cpp")
target_compile_features (OGDF
    PUBLIC cxx_range_for)
target_compile_options (OGDF
    PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wextra -Wno-unused-parameters -Wno-unknown-pragmas -Wno-error=sign-compare -Wno-error=conversion -Wno-error=strict-aliasing>
        $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<CONFIG:Release>>:-Wno-error=unused-variable>
        $<$<AND:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,$<BOOL:${OGDF_WARNING_ERRORS}>>:-Werror>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-error=zero-length-array -Wno-error=uninitialized>
        $<$<CXX_COMPILER_ID:GNU>:-Wshadow -Wno-error=maybe-uninitialized -Wno-error=unused-but-set-parameter>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Release>>:-Wno-error=unused-but-set-variable -Wno-error=strict-overflow>
        $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4018 /wd4068 /wd4101 /wd4244 /wd4250 /wd4267 /wd4373 /wd4800>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<BOOL:${OGDF_WARNING_ERRORS}>>:/WX>
        $<$<AND:$<OR:$<BOOL:${has_sse3_pmmintrin}>,$<BOOL:${has_sse3_intrin}>>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:-march=native>)
set_target_properties (OGDF PROPERTIES 
    CXX_STANDARD 11)
set_target_properties (OGDF PROPERTIES
    POSITION_INDEPENDENT_CODE ${OGDF_PIC})
target_link_libraries (OGDF
    PUBLIC
        COIN
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:pthread>)
target_include_directories (OGDF
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)

