# n-queens

## Requirements

* C++11 compiler, e.g GNU g++ / Intel icc. In my experiments I used the Intel compiler for perfomance reasons.

* [FastFlow](https://github.com/fastflow/fastflow), a library offering high-performance parallel patterns and building blocks in C++.

## Installation

Modify ```src/Makefile``` to provide the name of the chosen C++11 compiler and the home directory to FastFlow.
Then ```cd src/```, and run ``` make``` to compile the code for different configurations and version of the source code. Feel free to modify it with additional options for different architectures.

## Basic usage

Once the code has been compiled, the generated binary code can be run provided with the following arguments ```<n> <m> <d>```, where n is the size of the problem, i.e. the number of the queens, m is the number of workers and d is the depth where to stop sequential recursion.