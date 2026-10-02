add_test(NAME asset_gltf_export
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/assetbrowser/test_gltf.py)
set_tests_properties(asset_gltf_export PROPERTIES LABELS tooling)
add_test(NAME asset_sky_export
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/assetbrowser/test_sky.py)
set_tests_properties(asset_sky_export PROPERTIES LABELS tooling)

if(TARGET rage-gte-replay)
    add_test(NAME gte_replay_usage COMMAND rage-gte-replay)
set_tests_properties(gte_replay_usage PROPERTIES LABELS functional)
    set_tests_properties(gte_replay_usage PROPERTIES WILL_FAIL TRUE)
endif()
add_executable(mesh_obj_tests mesh_obj_tests.c)
target_link_libraries(mesh_obj_tests PRIVATE rage-rmesh)
add_test(NAME mesh_obj COMMAND mesh_obj_tests)
set_tests_properties(mesh_obj PROPERTIES LABELS tooling)
