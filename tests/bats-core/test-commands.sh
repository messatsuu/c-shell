#!/usr/bin/env bats

EXPECTED_EXIT_CODE=0
##### teardown teardown
teardown() {
    [ "$status" -eq "$EXPECTED_EXIT_CODE" ]
}

@test "test echo output" {
    run ./bin/main -c 'echo test'

    [ "$output" == "test" ]
}

@test "test piped command output" {
    run ./bin/main -c 'echo -n foo; echo -n bar; echo baz'

    [ "$output" == "foobarbaz" ]
}

@test "test chained command output" {
    run ./bin/main -c 'false && echo AND || echo OR ; echo SEMI'

    [ "$output" == "$(printf "OR\nSEMI\n")" ]
}

@test "double-quote handling" {
    run ./bin/main -c 'export FOO=bar && echo \" ; echo "$FOO"'

    [ "$output" == "$(printf "\"\nbar\n")" ]
}

@test "single-quote handling" {
    run ./bin/main -c "export FOO=bar && echo \' ; echo \"\$FOO\""

    [ "$output" == "$(printf "\'\nbar\n")" ]
}

@test "argument handling" {
    run ./bin/main -c "printf '[%s] ' 'first argument' second 'third argument' \"fourth argument :)\" fifth \"sixth argument\""

    [ "$output" == "[first argument] [second] [third argument] [fourth argument :)] [fifth] [sixth argument] " ]
}

@test "output redirection" {
    run ./bin/main -c "echo we redirect > /tmp/redirect_output"

    [ "$output" == "" ]
    [ "$(cat /tmp/redirect_output)" == "$(printf "we redirect\n")" ]
}

@test "exit-code priority handling" {
    run ./bin/main -c "true | false"

    EXPECTED_EXIT_CODE=1
}

@test "subshells create seperated environment" {
    run ./bin/main -c 'cd /tmp && (cd .. && pwd) && pwd'

    [ "$output" == "$(printf "/\n/tmp\n")" ]
}

@test "subshell_retains_var_env" {
    run ./bin/main -c 'export my_var=420 ; (echo $my_var)'

    [ "$output" == "420" ]

    run ./bin/main -c '(export another_var=420) ; echo -n $another_var'

    [ "$output" == "" ]
}
