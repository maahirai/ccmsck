#!/bin/bash
try(){
    input="$1"
    expected="$2"

    ./ccmsck "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected, but got $actual"
        exit 1
    fi
}

try 32+8 40
try 42+8-9 41

echo OK