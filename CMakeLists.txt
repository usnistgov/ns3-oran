# NIST-developed software is provided by NIST as a public service. You may use,
# copy and distribute copies of the software in any medium, provided that you
# keep intact this entire notice. You may improve,modify and create derivative
# works of the software or any portion of the software, and you may copy and
# distribute such modifications or works. Modified works should carry a notice
# stating that you changed the software and should note the date and nature of
# any such change. Please explicitly acknowledge the National Institute of
# Standards and Technology as the source of the software.
#
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO
# WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
# LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
# AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE
# OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT
# ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY
# REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF,
# INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY,
# OR USEFULNESS OF THE SOFTWARE.
#
# You are solely responsible for determining the appropriateness of using and
# distributing the software and you assume all risks associated with its use,
# including but not limited to the risks and costs of program errors,
# compliance with applicable laws, damage to or loss of data, programs or
# equipment, and the unavailability or interruption of operation. This
# software is not intended to be used in any situation where a failure could
# cause risk of injury or damage to property. The software developed by NIST
# employees is not subject to copyright protection within the United States.

if(NOT ${ENABLE_SQLITE})
  message(STATUS "ORAN requires sqlite3 library")
  return()
else()
  set(sqlite_libraries
      ${SQLite3_LIBRARIES}
  )
endif()

if(DEFINED ENV{LIBTORCHPATH})
  find_external_library(DEPENDENCY_NAME Torch
                        HEADER_NAME
                          torch/script.h
                          torch.h
                        LIBRARY_NAME torch
                        SEARCH_PATHS $ENV{LIBTORCHPATH})
else()
  find_external_library(DEPENDENCY_NAME Torch
                        HEADER_NAME
                          torch/script.h
                          torch.h
                        LIBRARY_NAME torch)
endif()

if(${Torch_FOUND})
  include_directories(${Torch_INCLUDE_DIRS})
  link_libraries(${Torch_LIBRARIES})
endif()

set(torch_libraries)
set(oran_torch_sources)
set(oran_torch_headers)
if(${Torch_FOUND})
  set(torch_libraries ${Torch_LIBRARIES})
  set(oran_torch_sources
      model/oran-lm-lte-2-lte-torch-handover.cc
  )
  set(oran_torch_headers
      model/oran-lm-lte-2-lte-torch-handover.h
  )
endif()

if(DEFINED ENV{LIBONNXPATH})
  find_external_library(DEPENDENCY_NAME OnnxRuntime
                        HEADER_NAME
                          cpu_provider_factory.h
                          onnxruntime_c_api.h
                          onnxruntime_cxx_api.h
                          onnxruntime_run_options_config_keys.h
                          onnxruntime_session_options_config_keys.h
                          provider_options.h
                        LIBRARY_NAME onnxruntime
                        SEARCH_PATHS $ENV{LIBONNXPATH})
else()
  find_external_library(DEPENDENCY_NAME OnnxRuntime
                        HEADER_NAME
                          cpu_provider_factory.h
                          onnxruntime_c_api.h
                          onnxruntime_cxx_api.h
                          onnxruntime_run_options_config_keys.h
                          onnxruntime_session_options_config_keys.h
                          provider_options.h
                        LIBRARY_NAME onnxruntime)
endif()

if(${OnnxRuntime_FOUND})
  include_directories(${OnnxRuntime_INCLUDE_DIRS})
  link_libraries(${OnnxRuntime_LIBRARIES})
endif()

set(onnxruntime_libraries)
set(oran_onnxruntime_sources)
set(oran_onnxruntime_headers)
if(${OnnxRuntime_FOUND})
  set(onnxruntime_libraries ${OnnxRuntime_LIBRARIES})
  set(oran_onnxruntime_sources
      model/oran-lm-lte-2-lte-onnx-handover.cc
  )
  set(oran_onnxruntime_headers
      model/oran-lm-lte-2-lte-onnx-handover.h
  )
endif()

build_lib(
  LIBNAME oran
  SOURCE_FILES
    model/oran-near-rt-ric.cc
    model/oran-lm.cc
    model/oran-lm-noop.cc
    model/oran-lm-lte-2-lte-distance-handover.cc
    model/oran-lm-lte-2-lte-rsrp-handover.cc
    model/oran-cmm.cc
    model/oran-cmm-handover.cc
    model/oran-cmm-noop.cc
    model/oran-cmm-single-command-per-node.cc
    model/oran-command.cc
    model/oran-command-lte-2-lte-handover.cc
    model/oran-report.cc
    model/oran-report-apploss.cc
    model/oran-report-lte-ue-rsrp-rsrq.cc
    model/oran-report-location.cc
    model/oran-report-lte-ue-cell-info.cc
    model/oran-reporter.cc
    model/oran-reporter-apploss.cc
    model/oran-reporter-lte-ue-rsrp-rsrq.cc
    model/oran-reporter-location.cc
    model/oran-reporter-lte-ue-cell-info.cc
    model/oran-data-repository.cc
    model/oran-data-repository-sqlite.cc
    model/oran-near-rt-ric-e2terminator.cc
    model/oran-e2-node-terminator.cc
    model/oran-e2-node-terminator-wired.cc
    model/oran-e2-node-terminator-lte-enb.cc
    model/oran-e2-node-terminator-lte-ue.cc
    model/oran-e2-node-terminator-container.cc
    model/oran-report-trigger.cc
    model/oran-report-trigger-periodic.cc
    model/oran-report-trigger-lte-ue-handover.cc
    model/oran-report-trigger-location-change.cc
    model/oran-query-trigger.cc
    model/oran-query-trigger-noop.cc
    model/oran-query-trigger-custom.cc
    helper/oran-helper.cc
    ${oran_onnxruntime_sources}
    ${oran_torch_sources}
  HEADER_FILES
    model/oran-near-rt-ric.h
    model/oran-lm.h
    model/oran-lm-noop.h
    model/oran-lm-lte-2-lte-distance-handover.h
    model/oran-lm-lte-2-lte-rsrp-handover.h
    model/oran-cmm.h
    model/oran-cmm-handover.h
    model/oran-cmm-noop.h
    model/oran-cmm-single-command-per-node.h
    model/oran-command.h
    model/oran-command-lte-2-lte-handover.h
    model/oran-report.h
    model/oran-report-apploss.h
    model/oran-report-lte-ue-rsrp-rsrq.h
    model/oran-report-location.h
    model/oran-report-lte-ue-cell-info.h
    model/oran-reporter.h
    model/oran-reporter-apploss.h
    model/oran-reporter-lte-ue-rsrp-rsrq.h
    model/oran-reporter-location.h
    model/oran-reporter-lte-ue-cell-info.h
    model/oran-data-repository.h
    model/oran-data-repository-sqlite.h
    model/oran-near-rt-ric-e2terminator.h
    model/oran-e2-node-terminator.h
    model/oran-e2-node-terminator-wired.h
    model/oran-e2-node-terminator-lte-enb.h
    model/oran-e2-node-terminator-lte-ue.h
    model/oran-e2-node-terminator-container.h
    model/oran-report-trigger.h
    model/oran-report-trigger-periodic.h
    model/oran-report-trigger-lte-ue-handover.h
    model/oran-report-trigger-location-change.h
    model/oran-query-trigger.h
    model/oran-query-trigger-custom.h
    helper/oran-helper.h
    ${oran_onnxruntime_headers}
    ${oran_torch_headers}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
    ${liblte}
    ${sqlite_libraries}
    ${torch_libraries}
    ${onnxruntime_libraries}
  TEST_SOURCES
    test/oran-test-suite.cc
)

target_compile_definitions(${liboran} PUBLIC ENABLE_ORAN)

