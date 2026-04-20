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

function(run_expr_all args expr)
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

    set(EXPR_WORKBENCH_LAST_OUT "${_out}" PARENT_SCOPE)
    set(EXPR_WORKBENCH_LAST_ERR "${_err}" PARENT_SCOPE)
endfunction()

function(require_in_output pattern)
    string(FIND "${EXPR_WORKBENCH_LAST_OUT}" "${pattern}" _exp_pos)
    if(_exp_pos EQUAL -1)
        message(FATAL_ERROR "Expected substring not found: '${pattern}'\nstdout:\n${EXPR_WORKBENCH_LAST_OUT}")
    endif()
endfunction()

function(require_not_in_output pattern)
    string(FIND "${EXPR_WORKBENCH_LAST_OUT}" "${pattern}" _exp_pos)
    if(NOT _exp_pos EQUAL -1)
        message(FATAL_ERROR "Unexpected substring found: '${pattern}'\nstdout:\n${EXPR_WORKBENCH_LAST_OUT}")
    endif()
endfunction()

function(require_not_in_rewritten pattern)
    string(FIND "${EXPR_WORKBENCH_LAST_OUT}" "[rewritten]" _rw_pos)
    if(_rw_pos EQUAL -1)
        message(FATAL_ERROR "[rewritten] section not found.\nstdout:
${EXPR_WORKBENCH_LAST_OUT}")
    endif()

    string(LENGTH "${EXPR_WORKBENCH_LAST_OUT}" _out_len)
    string(SUBSTRING "${EXPR_WORKBENCH_LAST_OUT}" ${_rw_pos} ${_out_len} _rewritten_tail)

    string(FIND "${_rewritten_tail}" "${pattern}" _exp_pos)
    if(NOT _exp_pos EQUAL -1)
        message(FATAL_ERROR "Unexpected substring found in rewritten output: '${pattern}'\nstdout:
${EXPR_WORKBENCH_LAST_OUT}")
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

# Integration flow: parse + rewrite + ssa.
run_expr_all("--simplify;--ssa" "(a^b)&c&(a^c)")
require_in_output("[parsed]")
require_in_output("[rewritten]")
require_in_output("[ssa]")
require_in_output("result =")

# Integration flow: parse + rewrite + emit-c.
run_expr_all("--simplify;--emit-c" "(a^b)&c&(a^c)")
require_in_output("[c-expr]")
require_in_output("~a")

# Integration flow: parse + rewrite + verify.
run_expr_all("--simplify;--verify" "(a^b)&c&(a^c)")
require_in_output("[verify]")
require_in_output("cases=128, passed=128, failed=0")

# Equation mode with multiple equations.
run_expr_all("--eq;--simplify;--emit-c;--ssa" "lhs=a^b;rhs=a^b;mix=lhs^rhs")
require_in_output("[parsed]")
require_in_output("lhs = a ^ b")
require_in_output("rhs = a ^ b")
require_in_output("[rewritten]")
require_in_output("mix = 0")
require_in_output("[ssa]")
require_in_output("[c-expr]")

# Rewrite/eval/codegen regression bundle.
function(run_regression_case expr)
    run_expr_all("--simplify;--emit-c;--verify" "${expr}")
    require_in_output("[parsed]")
    require_in_output("[rewritten]")
    require_in_output("[c-expr]")
    require_in_output("[verify]")
    require_in_output("cases=128, passed=128, failed=0")
    require_not_in_output("unsupported")
    require_not_in_output("invalid")
endfunction()

# Sensitive simplify case.
run_regression_case("(a^b)&c&(a^c)")

# Mixed arithmetic/bitwise rewrites.
run_regression_case("(a + 3) ^ ((b & 255) + (a - a))")

# Rotate/shift edge cases.
run_regression_case("rotl(a, 32) ^ rotr(b, 64) ^ (c << 32) ^ (d >> 64)")

# CH simplify regression: functional forms must be eliminated after simplify.
run_expr_all("--simplify;--verify" "ch(a,b,c)")
require_in_output("[rewritten]")
require_in_output("[verify]")
require_in_output("cases=128, passed=128, failed=0")
require_not_in_rewritten("ch(")

run_expr_all("--simplify;--verify" "maj(a,b,c)")
require_in_output("[rewritten]")
require_in_output("[verify]")
require_in_output("cases=128, passed=128, failed=0")
require_not_in_rewritten("maj(")

# Constant check cases for simplify/verify parity.
run_expr_all("--simplify;--verify" "ch(a,50,30)^((a&50)^(~a&30))")
require_in_output("[rewritten]")
require_not_in_rewritten("ch(")
require_in_output("[verify]")
require_in_output("cases=128, passed=128, failed=0")

run_expr_all("--simplify;--verify" "maj(a,50,30)^((a&50)^(a&30)^(50&30))")
require_in_output("[rewritten]")
require_not_in_rewritten("maj(")
require_in_output("[verify]")
require_in_output("cases=128, passed=128, failed=0")
