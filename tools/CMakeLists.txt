set(MONK_TOOLS mferdump mferinfo mferanon mfer2csv)

foreach(TOOL ${MONK_TOOLS})
  add_executable(${TOOL} ${TOOL}.cpp)
  target_link_libraries(${TOOL} PRIVATE monk)
endforeach()

# corpus_test is a standalone tool, not installed, used for dataset validation
add_executable(corpus_test corpus_test.cpp)
target_link_libraries(corpus_test PRIVATE monk)

install(TARGETS ${MONK_TOOLS}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_TESTING)
  set(MWF "${MONK_TEST_DATA_FILE}")
  set(OUT "${CMAKE_CURRENT_BINARY_DIR}")

  # ---------------------------------------------------------------------------
  # mferdump
  # ---------------------------------------------------------------------------
  add_test(NAME tool_mferdump
    COMMAND $<TARGET_FILE:mferdump> "${MWF}")
  set_tests_properties(tool_mferdump PROPERTIES
    PASS_REGULAR_EXPRESSION "\\| Tag")

  add_test(NAME tool_mferdump_no_args
    COMMAND $<TARGET_FILE:mferdump>)
  set_tests_properties(tool_mferdump_no_args PROPERTIES
    WILL_FAIL TRUE)

  # ---------------------------------------------------------------------------
  # mferinfo
  # ---------------------------------------------------------------------------
  add_test(NAME tool_mferinfo
    COMMAND $<TARGET_FILE:mferinfo> "${MWF}")
  set_tests_properties(tool_mferinfo PROPERTIES
    PASS_REGULAR_EXPRESSION "Channel Count: 6")

  add_test(NAME tool_mferinfo_patient_id
    COMMAND $<TARGET_FILE:mferinfo> "${MWF}")
  set_tests_properties(tool_mferinfo_patient_id PROPERTIES
    PASS_REGULAR_EXPRESSION "Patient ID: 12345")

  add_test(NAME tool_mferinfo_json
    COMMAND $<TARGET_FILE:mferinfo> "${MWF}" --json)
  set_tests_properties(tool_mferinfo_json PROPERTIES
    PASS_REGULAR_EXPRESSION "\"channelCount\": 6")

  add_test(NAME tool_mferinfo_no_args
    COMMAND $<TARGET_FILE:mferinfo>)
  set_tests_properties(tool_mferinfo_no_args PROPERTIES
    WILL_FAIL TRUE)

  # ---------------------------------------------------------------------------
  # mferanon
  # ---------------------------------------------------------------------------
  add_test(NAME tool_mferanon
    COMMAND $<TARGET_FILE:mferanon> "${MWF}" -o "${OUT}/anon.mwf")

  # Re-read the anonymized file: patient ID must not contain the original value
  add_test(NAME tool_mferanon_id_zeroed
    COMMAND $<TARGET_FILE:mferinfo> "${OUT}/anon.mwf")
  set_tests_properties(tool_mferanon_id_zeroed PROPERTIES
    DEPENDS tool_mferanon
    FAIL_REGULAR_EXPRESSION "12345")

  # Anonymized file must still be parseable (channel count unchanged)
  add_test(NAME tool_mferanon_still_valid
    COMMAND $<TARGET_FILE:mferinfo> "${OUT}/anon.mwf")
  set_tests_properties(tool_mferanon_still_valid PROPERTIES
    DEPENDS tool_mferanon
    PASS_REGULAR_EXPRESSION "Channel Count: 6")

  add_test(NAME tool_mferanon_missing_output
    COMMAND $<TARGET_FILE:mferanon> "${MWF}")
  set_tests_properties(tool_mferanon_missing_output PROPERTIES
    WILL_FAIL TRUE)

  # ---------------------------------------------------------------------------
  # mfer2csv
  # ---------------------------------------------------------------------------
  add_test(NAME tool_mfer2csv
    COMMAND $<TARGET_FILE:mfer2csv> "${MWF}" -o "${OUT}/all_channels.csv")
  set_tests_properties(tool_mfer2csv PROPERTIES
    PASS_REGULAR_EXPRESSION "Complete")

  add_test(NAME tool_mfer2csv_channel_select
    COMMAND $<TARGET_FILE:mfer2csv> "${MWF}" -o "${OUT}/two_channels.csv" --channels 0,1)
  set_tests_properties(tool_mfer2csv_channel_select PROPERTIES
    PASS_REGULAR_EXPRESSION "Complete")

  add_test(NAME tool_mfer2csv_interval
    COMMAND $<TARGET_FILE:mfer2csv> "${MWF}" -o "${OUT}/interval.csv" --start 0 --end 5)
  set_tests_properties(tool_mfer2csv_interval PROPERTIES
    PASS_REGULAR_EXPRESSION "Complete")

  add_test(NAME tool_mfer2csv_missing_output
    COMMAND $<TARGET_FILE:mfer2csv> "${MWF}")
  set_tests_properties(tool_mfer2csv_missing_output PROPERTIES
    WILL_FAIL TRUE)
endif()
