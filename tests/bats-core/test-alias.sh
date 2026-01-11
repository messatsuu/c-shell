#!/usr/bin/env bats

EXPECTED_EXIT_CODE=0

##### teardown teardown
teardown() {
    [ "$status" -eq "$EXPECTED_EXIT_CODE" ]
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
    run ./bin/main -c "alias foo=bar bar=foo && eval 'foo'"

    # TODO: check stderr output
    # [ "$output" == "works!" ]
}

@test "test unaliasing an alias" {
    run ./bin/main -c "alias my_alias='echo works!' && eval 'unalias my_alias' && eval 'my_alias'"

    # TODO: check stderr output
    # [ "$output" == "works!" ]
}

@test "test unaliasing all aliases" {
    run ./bin/main -c "alias my_alias='echo works!' another_alias='echo works!' && eval 'unalias -a' && eval 'my_alias ; another_alias'"

    # TODO: check stderr output
    # [ "$output" == "works!" ]
}
