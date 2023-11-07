# tabuwave

Tabular waveform viewer as a simple terminal user interface. Instead of using the traditional signal-to-time dimensions, Tabuwave uses an index-to-signal dimension at a specific time to make it easier to compare the same bit/groups of bits across multibit signals, such as a scoreboard or buffer of entries represented as masks or vectors. Currently only supports VCD files as input.

# Compilation

## General compilation

[Install Boost libraries](https://www.boost.org/doc/libs/1_83_0/more/getting_started/index.html) and [change the path to Boost in the Makefile](Makefile#L17) or specify the `BOOST_DIR` when compiling with `make` if needed.
```
make BOOST_DIR=<your boost path>
```

To use OpenMP, add `USE_OMP=1` to your `make` command:
```
make BOOST_DIR=<your boost path> USE_OMP=1
```
If `OpenMP` is not specified, `std::thread` is used for multithreading by default.

Run with (for example):
```
./tabuwave -f vcd/test.vcd 
```
See help text with `-h`

## PACE-ICE instructions

The Makefile has been designed to work on my local machine and the PACE-ICE cluster. To compile and run on PACE-ICE:
```
module load boost

make            # to use std::thread multithreading
make USE_OMP=1  # to use OpenMP

./tabuwave -f vcd/test.vcd 
```

# Usage

Commands when viewing table:
| Command       | Description             |
| -----------   | --------------------    |
| `:<#> + ENTER`         | jump to time `<#>`      |
| `LEFT_ARROW`           | next timestamp     |
| `RIGHT_ARROW`           | previous timestamp      |
| `/<#> + ENTER`        | highlight line at index `<#>`      |
| `DOWN_ARROW`           | highlight next line          |
| `UP_ARROW`           | highlight previous line      |
| `t`           | toggle table with/without horizontal lines      |
| `Q`           | quit      |


# Details

This is a custom project submission for ECE 4122, so below are what fulfill the grading requirements.

- Classes
    - `Parser` is a class for parsing a vcd file into manipulatable data structures. It's not the most robust VCD parser out there, but it's simple, independent of other parser/lexical analysis tools, and gets the job done.
    - `VcdScope` is a class to represent a "scope" which can have any number of children that are instances of `VcdScope` or `VcdVar`.
    - `VcdVar` is a derived class of `VcdScope` since both are intended to be part of a tree structure and share many of the same members (parent and children nodes, name, etc.). `VcdVar` has some additional members, such as time-value information. This is parsed from the vcd file and then stored into interval trees to be able to quickly query the value at any time within the simulation. `boost::icl::interval_map` is used for robustness.
- Multithreading
    - Multithreading is used to efficiently process vcd data for each `VcdVar` into its own interval_map simultaneously.
    - Both hand-threading with `std::thread` (distributing work as evenly as possible among the available concurrent threads supported by hardware) and multithreading using `OpenMP` are implemented and can be switched/selected during compile time. See usage instructions.

