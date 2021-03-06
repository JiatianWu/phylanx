# Copyright (c) 2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(tests

    avg_pool2d_operation
    avg_pool3d_operation
    batch_dot_operation
    bias_add_operation
    conv1d_operation
    conv2d_operation
    conv2d_transpose_operation
    ctc_decode_operation
    elu_operation
    hard_sigmoid_operation
    l2_normalize_operation
    max_pool2d_operation
    max_pool3d_operation
    one_hot_operation
    relu_operation
    resize_operation
    separable_conv1d_operation
    sigmoid_operation
    softmax_operation
    softplus_operation
    softsign_operation
    spatial_2d_padding_operation
    switch_operation
   )

foreach(test ${tests})
  set(sources ${test}.cpp)

  source_group("Source Files" FILES ${sources})

  # add executable
  add_phylanx_executable(${test}_test
    SOURCES ${sources}
    ${${test}_FLAGS}
    EXCLUDE_FROM_ALL
    FOLDER "Tests/Unit/Plugins/KerasSupport")

  add_phylanx_unit_test("plugins.keras_support" ${test} ${${test}_PARAMETERS})

  add_phylanx_pseudo_target(tests.unit.plugins.keras_support.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.keras_support
    tests.unit.plugins.keras_support.${test})
  add_phylanx_pseudo_dependencies(tests.unit.plugins.keras_support.${test}
    ${test}_test_exe)

endforeach()
