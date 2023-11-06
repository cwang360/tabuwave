# tabuwave

Tabular waveform viewer. Instead of using the traditional signal-to-time dimensions, Tabuwave uses an index-to-signal dimension at a specific time to make it easier to compare the same bit/groups of bits across multibit signals, such as a scoreboard represented as masks or vectors. Currently only supports VCD files as input.

# Usage

[Install Boost libraries](https://www.boost.org/doc/libs/1_83_0/more/getting_started/index.html) and change the path to Boost in the Makefile if needed:
```
BOOST_DIR = /usr/local/boost_1_82_0/
```

Compile with `make`, run with (for example):
```
./tabuwave -f vcd/test.vcd
```

# Details

This is a custom project submission for ECE 4122, so below are what fulfill the grading requirements.

- Classes
    - `Parser` is a class for parsing a vcd file into manipulatable data structures. It's not the most robust VCD parser out there, but it's simple, independent of other parser/lexical analysis tools, and gets the job done.
    - `VcdScope` is a class to represent a "scope" which can have any number of children that are instances of `VcdScope` or `VcdVar`.
    - `VcdVar` is a derived class of `VcdScope` since both are intended to be part of a tree structure and share many of the same members (parent and children nodes, name, etc.). `VcdVar` has some additional members, such as time-value information. This is parsed from the vcd file and then stored into interval trees to be able to quickly query the value at any time within the simulation. `boost::icl::interval_map` is used for robustness.
- Multithreading
    - Multithreading is used to efficiently process vcd data for each `VcdVar` into its own interval_map simultaneously.

