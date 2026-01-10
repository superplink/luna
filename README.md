## Introduction
This is a collection of low level data structure classes. I found myself remaking or reusing these in my own personal projects, so I decided to formalize them and create more robust and general purpose implimentations of them.

## Integration
Luna is a header only library.

## Motivation
I created Luna because I found stadard library data structures to often have very inflexible interfaces, and is often needlesly slowed down by its requirements.

## Requirements:
Luna requires C++23.

## Tests
Luna uses premake5. In order to compile and run the tests, you first need to install premake5, and then run the following commands:
```
premake5 gmake
make
```

## Performance
The hash table performs around 2x faster with insertion, 2x faster with lookups, 10x faster with deletions, and around 20x faster with iteration.