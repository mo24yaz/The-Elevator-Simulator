# Elevator Simulator

A multi-elevator, multi-passenger simulation written in C using POSIX threads. This project models concurrent elevator and passenger controllers coordinating via mutexes and condition variables to serve up to 30 elevators and 60 passengers in a building simulation.

## Table of Contents

* [Features](#features)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
* [Usage](#usage)
* [Project Structure](#project-structure)
* [Testing](#testing)
* [Performance](#performance)
* [Contributing](#contributing)
* [License](#license)

## Features

* **Concurrent Simulation**: Coordinates multiple elevators and passengers using pthreads.
* **Threaded Controllers**: Separate `passenger_controller` and `elevator_controller` modules manage behavior.
* **Synchronization Primitives**: Uses mutexes and condition variables to ensure safe boarding, movement, and disembarking.
* **Event-Driven Logging**: Structured logs of elevator movements and passenger actions.
* **Performance-Tuned**: Meets strict runtime requirements through lock-contention tuning.

## Prerequisites

* GCC (or compatible C compiler)
* POSIX Threads library (`-pthread`)
* GNU Make

## Installation

1. Obtain the starter code (e.g., clone or unzip into a single folder).

2. In the project root, simply run:

   ```bash
   make
   ```

   This will compile `main.c`, `hw5.c`, and produce the simulator executable.

## Usage

```
./elevator_simulator <num_elevators> <num_floors> <input_file>
```

* `<num_elevators>`: Number of elevator threads (max 30)
* `<num_floors>`: Number of floors in the building
* `<input_file>`: Path to a passenger request file, where each line contains:<br>
  `<start_time> <origin_floor> <destination_floor>`

Example:

```bash
./elevator_simulator 5 10 test/requests.txt
```

## Project Structure

```
├── elevator.h        # Elevator data structures and declarations
├── main.c           # Program entry point and argument parsing
├── hw5.c            # Core simulation logic and thread controllers
├── elevator.log      # Sample output log of simulation events
├── Makefile         # Build rules for compilation
└── runtests.sh      # Script to run correctness and performance tests
```

## Testing

Run the provided test harness to execute both correctness and performance checks:

```bash
./runtests.sh
```

## Performance

* All test cases complete within the required maximum runtime (≤2304 ms). Lock-contention tuning ensures efficient thread scheduling under load.

## License

This repository contains academic coursework for CS 361 @ UIC.
