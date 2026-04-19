set(_tool "$ENV{BF_EXPR_WORKBENCH}")
if(_tool STREQUAL "")
    message(FATAL_ERROR "BF_EXPR_WORKBENCH env var is required")
endif()

function(run_expr args expr expect)
    set(_cmd_args ${args})
    execute_process(
        COMMAND "${_tool}" --expr "${expr}" ${_cmd_args}
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
    )

    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "bitflow_expr failed (rc=${_rc}) for args='${args}' expr='${expr}'\nstdout:\n${_out}\nstderr:\n${_err}")
    endif()

    string(FIND "${_err}" "Missing rule dependency" _dep_err)
    if(NOT _dep_err EQUAL -1)
        message(FATAL_ERROR "Unexpected dependency error for args='${args}' expr='${expr}'\nstderr:\n${_err}")
    endif()

    if(NOT "${expect}" STREQUAL "")
        string(FIND "${_out}" "${expect}" _exp_pos)
        if(_exp_pos EQUAL -1)
            message(FATAL_ERROR "Expected substring not found: '${expect}'\nargs='${args}' expr='${expr}'\nstdout:\n${_out}")
        endif()
    endif()
endfunction()

# Stage profile matrix should not trigger dependency errors.
run_expr("--normalize" "x" "[rewritten]")
run_expr("--simplify" "x" "[rewritten]")
run_expr("--factorize" "x" "[rewritten]")
run_expr("--normalize;--simplify" "x" "[rewritten]")
run_expr("--simplify;--factorize" "x" "[rewritten]")
run_expr("" "x" "[rewritten]")

# Simplify regression case: (a^b)&c&(a^c) should remain simplifiable.
run_expr("--simplify" "(a^b)&c&(a^c)" "b & c & ~a")
