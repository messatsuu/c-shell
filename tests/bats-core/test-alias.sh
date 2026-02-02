#!/usr/bin/env bats

setup() {
    bats_require_minimum_version 1.5.0
}

@test "alias output gets replaced in whole AST-Tree" {
    run ./bin/main -c "alias my_alias='echo works' && eval '(my_alias && my_alias) && my_alias && false || my_alias ; my_alias again'"
    [ "$output" == "$(printf 'works\nworks\nworks\nworks\nworks again\n')" ]
}

@test "test recursive alias output" {
    run ./bin/main -c "alias my_alias='echo works!' another_alias=my_alias && eval another_alias"

    [ "$output" == "works!" ]
}

@test "test infinite recursive alias gets stopped" {
    run -127 ./bin/main -c "alias foo=bar bar=foo && eval 'foo'"

    [ "$output" == "csh: Command not found: foo" ]
}

@test "test unaliasing an alias" {
    run -127 ./bin/main -c "alias my_alias='echo works!' && eval 'unalias my_alias' && eval 'my_alias'"

    [ "$output" == "csh: Command not found: my_alias" ]
}

@test "test unaliasing all aliases" {
    run -127 ./bin/main -c "alias my_alias='echo works!' another_alias='echo works!' && eval 'unalias -a' && eval 'my_alias ; another_alias'"

    EXPECTED_EXIT_CODE=127
    [ "$output" == "$(printf  "csh: Command not found: my_alias\n\ncsh: Command not found: another_alias")" ]
}
