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

try 0 "0;"
try 42 "42;"
try 40 "32+8;"
try 41 "42+8-9;"
try 41 " 12 + 34 - 5 ;"
try 72 " 5+ 8 + 68 -9;"
try 83 " 2 * 43 -7+4;"
try 4 "12*5/15;"
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 8 '-(30-14)/-2;'
try 9 '(+30-12)/2;'

try 0 '0==1;'
try 1 '42==42;'
try 1 '0!=1;'
try 0 '42!=42;'

try 1 '0<1;'
try 0 '1<1;'
try 0 '2<1;'
try 1 '0<=1;'
try 1 '1<=1;'
try 0 '2<=1;'

try 1 '1>0;'
try 0 '1>1;'
try 0 '1>2;'
try 1 '1>=0;'
try 1 '1>=1;'
try 0 '1>=2;'

try 2 "a=b=2;"
try 5 "a=3;b=2;a=a+b;"
try 23 "a=7;b=18;c=2;a=a+b-c;"
try 1 "a=3;b=2;a=a/b;"
try 6 "a=3;b=2;a=a*b;"
echo OK