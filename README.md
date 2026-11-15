# Lightning Search
A lightning-fast, native C++ code search engine designed for developers who need instant full-text search across large codebases.
## Features
- **Blazing Fast:** Sub-100 microsecond query execution times.
- **BM25 Ranking:** Industry-standard relevance scoring to surface the most accurate results first.
- **Multithreading:** Parallel directory scanning and indexing using standard C++ threads (`std::thread`, `std::mutex`).
- **Zero Dependencies:** Written in pure modern C++ (C++17), requiring no external libraries or bloated runtimes.
## Getting Started
1. Clone or copy the repository into your project folder.
2. Open `FastSearchEngine.slnx` in Visual Studio (or compile manually with a C++17 compiler).
3. Build in **Release** mode for maximum performance.
4. Run the executable and search your codebase instantly.
## Performance
- **Index time:** Instantaneous for local source trees.
- **Search latency:** ~80-87 µs per query.
