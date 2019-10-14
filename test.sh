#!/bin/bash
try(){
    expected="$1"
    input="$2"

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
try 0 0
try 42 42
try 40 32+8
try 41 42+8-9
try 41 " 12 + 34 - 5 "
try 72 " 5+ 8 + 68 -9"
echo OK