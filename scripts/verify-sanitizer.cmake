cmake_minimum_required(VERSION 3.22)
if(NOT DEFINED PIXAURA_PROBE OR NOT DEFINED PIXAURA_KIND OR NOT DEFINED PIXAURA_LOG_DIR)
    message(FATAL_ERROR "Probe, sanitizer kind and log directory are required")
endif()
if(PIXAURA_KIND STREQUAL "asan")
    set(expected_exit 86)
    set(expected_diagnostic "ERROR: AddressSanitizer: heap-buffer-overflow")
elseif(PIXAURA_KIND STREQUAL "ubsan")
    set(expected_exit 87)
    set(expected_diagnostic "runtime error: signed integer overflow")
else()
    message(FATAL_ERROR "Unknown sanitizer kind: ${PIXAURA_KIND}")
endif()
file(MAKE_DIRECTORY "${PIXAURA_LOG_DIR}")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
        "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=0:exitcode=${expected_exit}"
        "UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1:exitcode=${expected_exit}"
        "${PIXAURA_PROBE}"
    RESULT_VARIABLE probe_result
    OUTPUT_VARIABLE probe_stdout
    ERROR_VARIABLE probe_stderr
    TIMEOUT 30
)
set(probe_log "${probe_stdout}${probe_stderr}")
file(WRITE "${PIXAURA_LOG_DIR}/${PIXAURA_KIND}-negative.log" "exit=${probe_result}\n${probe_log}")
if(NOT "${probe_result}" STREQUAL "${expected_exit}" OR NOT probe_log MATCHES "${expected_diagnostic}")
    message(FATAL_ERROR "${PIXAURA_KIND} instrumentation unproven: expected exit ${expected_exit} and '${expected_diagnostic}'; got '${probe_result}'. See ${PIXAURA_LOG_DIR}/${PIXAURA_KIND}-negative.log")
endif()
if(PIXAURA_KIND STREQUAL "asan" AND NOT probe_log MATCHES "pixaura_get_core_info")
    message(FATAL_ERROR "ASan diagnostic did not identify the shared core write; ensure llvm-symbolizer is available")
endif()
message(STATUS "${PIXAURA_KIND} negative probe confirmed actual instrumentation (exit ${probe_result})")
