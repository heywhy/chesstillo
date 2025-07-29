# Chesstillo

**Chesstillo** is a modern, modular chess engine project implemented in C++20, focusing on clean architecture, extensibility, and advanced chess algorithmics. The repository is composed primarily of C++ code with CMake build configuration, supporting features like a UCI interface, engine logic, and a text-based user interface (TUI).

## Features

- **Chess Engine Core**: Board representation, move generation, parallel search (Young Brother Wait), evaluation, transposition tables, threading, and more.
- **UCI (Universal Chess Interface) Support**: Communicates using the UCI protocol for engine interoperability.
- **Text-based User Interface (TUI)**: Built on [FTXUI](https://github.com/ArthurSonzogni/ftxui) with modal editing (NORMAL, INTERRACT, VISUAL modes), supporting keyboard and mouse navigation.
- **Perft Validation**: Includes a standalone `perft` executable to verify that the engine generates only legal moves. Results have been cross-checked against official [Perft results](https://www.chessprogramming.org/Perft_Results) from chessprogramming wiki, ensuring correctness of move generation.
- **Modern C++ Standards**: Uses C++20 features and leverages modern libraries such as Boost and nlohmann/json.
- **Test Suite**: Unit tests and mocks via GoogleTest and GoogleMock.
- **CMake Build System**: Modular subdirectories (`engine`, `uci`, `tui`), dependency management using `FetchContent`.

## TUI Modal Editor

Chesstillo's TUI is designed as a **modal editor** inspired by Vim, supporting three modes:

- **NORMAL**: Default mode for basic commands.
- **INTERRACT**: Enables advanced keyboard navigation and shortcuts.
- **VISUAL**: Allows mouse-based interactions only.

Switch between modes using keyboard shortcuts for efficient navigation and editing.

## Directory Structure

- `engine/`: Core chess engine logic (board, search, parallel algorithms, evaluation, threading, etc.).
- `uci/`: UCI protocol implementation, command parsing, and engine communication.
- `tui/`: Text-based user interface components, utilities, and mapping for UI.
- `cmake/`: CMake scripts for dependency management (Boost, FTXUI, magic-bits, etc.).

## Dependencies

- [Boost](https://github.com/boostorg/boost) (asio, filesystem, process)
- [FTXUI](https://github.com/ArthurSonzogni/ftxui) (Terminal UI)
- [nlohmann/json](https://github.com/nlohmann/json) (JSON parsing)
- [GoogleTest](https://github.com/google/googletest) (Testing)
- [magic-bits](https://github.com/goutham/magic-bits) (Bitboard helpers)

Dependencies are automatically fetched via CMake's `FetchContent` module.

## Building

```bash
git clone https://github.com/heywhy/chesstillo.git
cd chesstillo
cmake -B build
cmake --build build
```

### Options

- `CMAKE_BUILD_TYPE`: Set to `Debug` or `Release`
- `DEBUG_THREADS`: Optionally enable ThreadSanitizer during debug

## Running

- All executables (`chesstillo_tui`, `perft`, and the main engine) will be built inside the `build` directory.
- To launch the text-based user interface, run:
  ```bash
  build/tui/chesstillo_tui
  ```
- To validate move generation (legal moves only), run the standalone perft tool:
  ```bash
  build/engine/perft
  ```
  The results produced should match those documented on [chessprogramming wiki](https://www.chessprogramming.org/Perft_Results).
- **Engine Analysis**: Currently, only [Stockfish](https://stockfishchess.org/) is supported for analysis. Ensure Stockfish is available in your system path or configured appropriately.
- **Board Representation**: For the chess board and pieces to display correctly in your terminal, you must have [Nerd Font](https://www.nerdfonts.com/) installed and configured for your terminal emulator.

## Reusable Subprojects & APIs

The `engine` and `uci` modules have user-friendly APIs and can be reused independently outside Chesstillo, making them suitable for integration with other chess-related applications or tooling.

## Contributing

Pull requests and issues are welcome! Please ensure your code is well-tested and adheres to the project's style guidelines.

## License

[MIT](LICENSE) or see repository for details.

## Acknowledgements

- [FTXUI](https://github.com/ArthurSonzogni/ftxui)
- [Boost](https://www.boost.org/)
- [GoogleTest](https://github.com/google/googletest)

---

*Chesstillo* showcases modern C++ systems programming, parallel algorithms, protocol design, reusable APIs, terminal UI/UX, and automated testing. Enjoy hacking, experimenting, and playing chess with code!
