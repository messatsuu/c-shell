#!/usr/bin/env bats

setup() {
    bats_require_minimum_version 1.5.0
}

@test "subshells environment separation" {
    run ./bin/main -c 'cd /tmp && (cd .. && pwd) && pwd'

    [ "$output" == "$(printf "/\n/tmp\n")" ]
}

@test "parent-shell environment sharing" {
    run ./bin/main -c 'export my_var=420 ; (echo $my_var)'

    [ "$output" == "420" ]

    run ./bin/main -c '(export another_var=420) ; echo -n $another_var'

    [ "$output" == "" ]
}
