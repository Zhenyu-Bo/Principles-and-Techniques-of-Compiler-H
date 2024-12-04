#!/bin/bash
# change to your /full/path/to/antlr-4.13.1-complete.jar
export LD_LIBRARY_PATH='.:/usr/local/lib:$LD_LIBRARY_PATH'
export CLASSPATH='.:/usr/local/lib/antlr-4.13.1-complete.jar:$CLASSPATH'
antlr4='java org.antlr.v4.Tool'
grun='java org.antlr.v4.gui.TestRig'

if [[ $# != 1 ]]; then
    echo "Usage: ./run.sh java|cpp"
    echo "Example: cat test/test_cases/simple.c1 | ./run.sh java"
fi

rm -rf build
mkdir -p build

if [[ $1 == "java" ]]; then
    cd grammar
    $antlr4 C1Lexer.g4 -o ../build
    cd ../build
    javac *.java
    $grun C1Lexer tokens -tokens
elif [[ $1 == "cpp" ]]; then
    cd grammar
    $antlr4 C1Lexer.g4 -Dlanguage=Cpp -o ../build
    cd ../build
    # uncomment below if you run this file on your machine
    #cp ../Libs_for_c1r_ref/libantlr4-runtime.so.4.13.1 /usr/local/lib/
    #ln -snf /usr/local/lib/libantlr4-runtime.so.4.13.1 /usr/local/lib/libantlr4-runtime.so
    #g++ -std=c++17 ../test/lexer.cpp C1Lexer.cpp -I../include/antlr4-runtime-4.13.1 -I. -lantlr4-runtime -o c1lexer
    #./c1lexer
    g++ -std=c++17 ../test/lexer.cpp C1Lexer.cpp -I../include/antlr4-runtime-4.13.1 -I. -lantlr4-runtime -o c1lexer
    ./c1lexer
fi

