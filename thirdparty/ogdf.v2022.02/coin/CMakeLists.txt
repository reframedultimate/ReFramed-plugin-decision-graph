cmake_minimum_required (VERSION 3.3)

project ("COIN"
    LANGUAGES CXX)

set (COIN_SOLVER "CLP" CACHE STRING "Linear program solver to be used by COIN")
set_property (CACHE COIN_SOLVER PROPERTY STRINGS "CLP" "CPX" "GRB")
option (COIN_PIC "Generate position independent code" OFF)

if (NOT COIN_SOLVER STREQUAL "CLP")
    set (COIN_EXTERNAL_SOLVER_INCLUDE_DIRECTORIES "" CACHE PATH "Locations of required header files for the external LP solver")
    set (COIN_EXTERNAL_SOLVER_LIBRARIES "" CACHE FILEPATH "Libraries for the external LP solver")
else ()
    unset (COIN_EXTERNAL_SOLVER_INCLUDE_DIRECTORIES CACHE)
    unset (COIN_EXTERNAL_SOLVER_LIBRARIES CACHE)
endif ()

add_library (COIN STATIC
    "src/Clp/ClpCholeskyBase.cpp"
    "src/Clp/ClpCholeskyDense.cpp"
    "src/Clp/ClpCholeskyTaucs.cpp"
    "src/Clp/Clp_C_Interface.cpp"
    "src/Clp/ClpConstraint.cpp"
    "src/Clp/ClpConstraintLinear.cpp"
    "src/Clp/ClpConstraintQuadratic.cpp"
    "src/Clp/ClpDualRowDantzig.cpp"
    "src/Clp/ClpDualRowPivot.cpp"
    "src/Clp/ClpDualRowSteepest.cpp"
    "src/Clp/ClpDummyMatrix.cpp"
    "src/Clp/ClpDynamicExampleMatrix.cpp"
    "src/Clp/ClpDynamicMatrix.cpp"
    "src/Clp/ClpEventHandler.cpp"
    "src/Clp/ClpFactorization.cpp"
    "src/Clp/ClpGubDynamicMatrix.cpp"
    "src/Clp/ClpGubMatrix.cpp"
    "src/Clp/ClpHelperFunctions.cpp"
    "src/Clp/ClpInterior.cpp"
    "src/Clp/ClpLinearObjective.cpp"
    "src/Clp/ClpLsqr.cpp"
    "src/Clp/ClpMatrixBase.cpp"
    "src/Clp/ClpMessage.cpp"
    "src/Clp/ClpModel.cpp"
    "src/Clp/ClpNetworkBasis.cpp"
    "src/Clp/ClpNetworkMatrix.cpp"
    "src/Clp/ClpNode.cpp"
    "src/Clp/ClpNonLinearCost.cpp"
    "src/Clp/ClpObjective.cpp"
    "src/Clp/ClpPackedMatrix.cpp"
    "src/Clp/ClpPdcoBase.cpp"
    "src/Clp/ClpPdco.cpp"
    "src/Clp/ClpPlusMinusOneMatrix.cpp"
    "src/Clp/ClpPredictorCorrector.cpp"
    "src/Clp/ClpPresolve.cpp"
    "src/Clp/ClpPrimalColumnDantzig.cpp"
    "src/Clp/ClpPrimalColumnPivot.cpp"
    "src/Clp/ClpPrimalColumnSteepest.cpp"
    "src/Clp/ClpQuadraticObjective.cpp"
    "src/Clp/ClpSimplex.cpp"
    "src/Clp/ClpSimplexDual.cpp"
    "src/Clp/ClpSimplexNonlinear.cpp"
    "src/Clp/ClpSimplexOther.cpp"
    "src/Clp/ClpSimplexPrimal.cpp"
    "src/Clp/ClpSolve.cpp"
    "src/Clp/Idiot.cpp"
    "src/Clp/IdiSolve.cpp"
    "src/Clp/MyEventHandler.cpp"
    "src/Clp/MyMessageHandler.cpp"
    "src/Clp/unitTest.cpp"

    "src/CoinUtils/CoinAlloc.cpp"
    "src/CoinUtils/CoinBuild.cpp"
    "src/CoinUtils/CoinDenseFactorization.cpp"
    "src/CoinUtils/CoinDenseVector.cpp"
    "src/CoinUtils/CoinError.cpp"
    "src/CoinUtils/CoinFactorization1.cpp"
    "src/CoinUtils/CoinFactorization2.cpp"
    "src/CoinUtils/CoinFactorization3.cpp"
    "src/CoinUtils/CoinFactorization4.cpp"
    "src/CoinUtils/CoinFileIO.cpp"
    "src/CoinUtils/CoinFinite.cpp"
    "src/CoinUtils/CoinIndexedVector.cpp"
    "src/CoinUtils/CoinLpIO.cpp"
    "src/CoinUtils/CoinMessage.cpp"
    "src/CoinUtils/CoinMessageHandler.cpp"
    "src/CoinUtils/CoinModel.cpp"
    "src/CoinUtils/CoinModelUseful2.cpp"
    "src/CoinUtils/CoinModelUseful.cpp"
    "src/CoinUtils/CoinMpsIO.cpp"
    "src/CoinUtils/CoinOslFactorization2.cpp"
    "src/CoinUtils/CoinOslFactorization3.cpp"
    "src/CoinUtils/CoinOslFactorization.cpp"
    "src/CoinUtils/CoinPackedMatrix.cpp"
    "src/CoinUtils/CoinPackedVectorBase.cpp"
    "src/CoinUtils/CoinPackedVector.cpp"
    "src/CoinUtils/CoinParam.cpp"
    "src/CoinUtils/CoinParamUtils.cpp"
    "src/CoinUtils/CoinPostsolveMatrix.cpp"
    "src/CoinUtils/CoinPrePostsolveMatrix.cpp"
    "src/CoinUtils/CoinPresolveDoubleton.cpp"
    "src/CoinUtils/CoinPresolveDual.cpp"
    "src/CoinUtils/CoinPresolveDupcol.cpp"
    "src/CoinUtils/CoinPresolveEmpty.cpp"
    "src/CoinUtils/CoinPresolveFixed.cpp"
    "src/CoinUtils/CoinPresolveForcing.cpp"
    "src/CoinUtils/CoinPresolveHelperFunctions.cpp"
    "src/CoinUtils/CoinPresolveImpliedFree.cpp"
    "src/CoinUtils/CoinPresolveIsolated.cpp"
    "src/CoinUtils/CoinPresolveMatrix.cpp"
    "src/CoinUtils/CoinPresolvePsdebug.cpp"
    "src/CoinUtils/CoinPresolveSingleton.cpp"
    "src/CoinUtils/CoinPresolveSubst.cpp"
    "src/CoinUtils/CoinPresolveTighten.cpp"
    "src/CoinUtils/CoinPresolveTripleton.cpp"
    "src/CoinUtils/CoinPresolveUseless.cpp"
    "src/CoinUtils/CoinPresolveZeros.cpp"
    "src/CoinUtils/CoinSearchTree.cpp"
    "src/CoinUtils/CoinShallowPackedVector.cpp"
    "src/CoinUtils/CoinSimpFactorization.cpp"
    "src/CoinUtils/CoinSnapshot.cpp"
    "src/CoinUtils/CoinStructuredModel.cpp"
    "src/CoinUtils/CoinWarmStartBasis.cpp"
    "src/CoinUtils/CoinWarmStartDual.cpp"
    "src/CoinUtils/CoinWarmStartPrimalDual.cpp"
    "src/CoinUtils/CoinWarmStartVector.cpp"

    "src/Osi/OsiAuxInfo.cpp"
    "src/Osi/OsiBranchingObject.cpp"
    "src/Osi/OsiChooseVariable.cpp"
    "src/Osi/OsiColCut.cpp"
    "src/Osi/OsiCut.cpp"
    "src/Osi/OsiCuts.cpp"
    "src/Osi/OsiNames.cpp"
    "src/Osi/OsiPresolve.cpp"
    "src/Osi/OsiRowCut.cpp"
    "src/Osi/OsiRowCutDebugger.cpp"
    "src/Osi/OsiSolverBranch.cpp"
    "src/Osi/OsiSolverInterface.cpp"

    "src/OsiClp/OsiClpSolverInterface.cpp"
    $<$<STREQUAL:${COIN_SOLVER},"GRB">:src/Osi/OsiGrbSolverInterface.cpp>
    $<$<STREQUAL:${COIN_SOLVER},"CPX">:src/Osi/OsiCpxSolverInterface.cpp>)
target_include_directories (COIN
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (COIN
    PRIVATE
        CLP_BUILD
        COINUTILS_BUILD
        OSI_BUILD
        __OSI_CLP__
        COMPILE_IN_CG
        COMPILE_IN_LP
        COMPILE_IN_TM
        HAVE_CONFIG_H
        _CRT_SECURE_NO_WARNINGS)
if (COIN_PIC)
    set_target_properties (COIN PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif ()

# external LP solver
if(COIN_EXTERNAL_SOLVER_LIBRARIES)
  target_link_libraries(COIN PUBLIC ${COIN_EXTERNAL_SOLVER_LIBRARIES})
  foreach(EXT_LIB ${COIN_EXTERNAL_SOLVER_LIBRARIES})
    if(NOT EXISTS ${EXT_LIB} OR IS_DIRECTORY ${EXT_LIB})
      message(SEND_ERROR "The provided library does not exist: ${EXT_LIB}")
    endif()
  endforeach()
endif()
if(COIN_EXTERNAL_SOLVER_INCLUDE_DIRECTORIES)
  foreach(EXT_DIR ${COIN_EXTERNAL_SOLVER_INCLUDE_DIRECTORIES})
    if(NOT IS_DIRECTORY ${EXT_DIR})
      message(SEND_ERROR "The provided directory is invalid: ${EXT_DIR}")
    endif()
  endforeach()
  target_include_directories(COIN SYSTEM PUBLIC ${COIN_EXTERNAL_SOLVER_INCLUDE_DIRECTORIES})
endif()

