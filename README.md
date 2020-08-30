# n-queens

## Requirements

* C++11 compiler, e.g GNU g++ / Intel icc. In my experiments I used the Intel compiler for perfomance reasons.

* [FastFlow](https://github.com/fastflow/fastflow), a library offering high-performance parallel patterns and building blocks in C++.

## Installation

Modify ```src/Makefile``` to provide the name of the chosen C++11 compiler and the home directory to FastFlow.
Then ```cd src/```, and run ``` make``` to compile the code for different configurations and version of the source code. Feel free to modify it with additional options for different architectures.

## Basic usage

Once the code has been compiled, the generated binary code can be run provided with the following arguments ```<n> <m> <d>```, where n is the size of the problem, i.e. the number of the queens, m is the number of workers and d is the depth where to stop sequential recursion.

## Introduction

The n-queens problems stems from a generalization of the [eight queens puzzle](https://en.wikipedia.org/wiki/Eight_queens_puzzle).

In the following will be introduced two parallel implementations of a Divide-And-Conquer algorithm to solve the n-queens problem, using different computational models. The first using the [FastFlow](https://github.com/fastflow/fastflow) framework, and the second pure C++1 Threads. The experiments I made, were ran on an Intel Xeon architecture equipped with two Intel Xeon Phi co-processors.

### 1.1. The sequental algorithm

Before going further into the details of the sequential algorithm, we introduce the following definition

> Def.1: Let an abstract tree model all possible configurations, or states, a chessboard B of size n x n, and with n queens, it can assume. The root node, with height h=0, represents the empty configuration. A path from the root node to a generic internal node at height h > 0, represents a partial configuration of B with h <= n queens. Furthermore, the generic node represents the position (i, j) in B, assuming B to be indexed using a matrix-like notation, in which it can ben found a queen. Thus, the number of paths of size n, namely all possible permutations of the n queens is given by

$$
    P\left(n\right) = \frac{n^{2}!}{\left(n^{2} - n\right)!n!}
$$

The naive implementation of the sequential algorithm simply generates all the P(n) states to verify which solution satifies the constraints of the n-queens puzzle.

In order to reduce time complexity, it can be found that during the generation of the states, it can be forced to have that no queen can be found on the diagonal line or the horizontal line of any other queen. To that end, it can be used a simple array of size n. Let R be an array with n components such that

$$
    R: Q \rightarrow C (1)
$$

where Q = {1, ..., n} is the set of labels of the queens, and C = {1, ..., n} is the set of column indexes of the chessboard. Then, if R(i) = j then queen i is in position B_{ij}. By definition, only one queen can be found in every row and in every column. Then, it matters only of verifying which of the O(n!) permutations of array R satisfy the aforementioned constrain over the diagonals, namely

$$
    \| R(i+d) - R(i) \| \neq d, \forall i, d. i = 1, ..., n-d, d = 1, ..., n-1 (2)
$$

### 1.1.1 Implementation

The exploration of the tree of solutions proceeds only in the partial paths, i.e. of height h <= n, satisfying equation (2). Also, at each step of the algorithm, R can be represented using a bit array.

<img src="https://github.com/luigidisotto/n-queens/blob/master/img/bit-array.png" width="250" height="250" />
