#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std::chrono_literals;

#include "spdlog/fmt/chrono.h"
#include "spdlog/spdlog.h"

auto file_time_to_sys() {}

/**
 * An approximation from file time to system time.  This is as good as you can
 * do with C++17.  In C++20, replace with std::chrono::file_clock::to_sys()
 */
template <typename TP>
auto to_system_time(TP tp) -> std::chrono::time_point<std::chrono::system_clock>
{
  using namespace std::chrono;
  return time_point_cast<system_clock::duration>(tp - TP::clock::now() +
                                                 system_clock::now());
}

/**
 * A simple example for using spdlog as a logger for a programm
 */
auto main() -> int
{
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  auto p = std::filesystem::temp_directory_path() / "example.bin";
  std::ofstream{p.c_str()}.put('a'); // create file

  auto ftime = std::filesystem::last_write_time(p);
  spdlog::info("File write time is {}", to_system_time(ftime));

  // move file write time 1 hour to the future
  std::filesystem::last_write_time(p, ftime + 1h);

  // read back from the filesystem
  ftime = std::filesystem::last_write_time(p);
  spdlog::info("File write time is {}", to_system_time(ftime));

//  std::filesystem::remove(p);

  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  return 0;
}