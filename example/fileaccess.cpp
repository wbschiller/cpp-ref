#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

#include "spdlog/fmt/chrono.h"
#include "spdlog/spdlog.h"

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
 * A simple example for file accessing using std::filesystem
 */
auto main() -> int
{
  // Create a file and manipulate its write time
  auto p = fs::temp_directory_path() / "example.bin";
  std::ofstream{p.c_str()}.put('a'); // create file

  std::error_code ec;
  auto ftime = fs::last_write_time(p, ec);
  spdlog::info("File write time is {}", to_system_time(ftime));

  // move file write time 1 hour to the future
  fs::last_write_time(p, ftime + 1h, ec);

  // read back from the filesystem
  ftime = fs::last_write_time(p, ec);
  spdlog::info("File write time is {}", to_system_time(ftime));

  fs::remove(p);

  // Read the contents of the log.txt file in the current director
  auto log = fs::path("log.txt");
  std::ifstream ifs(log);

  if (!fs::exists(log, ec)) {
    spdlog::error("File does not exits {}", log.string());
  } else if (ifs) {
    fmt::print("----------- File contents start: {} -----------\n",
               log.string());
    while (!ifs.eof()) {
      std::string str;
      std::getline(ifs, str);
      fmt::print("{}\n", str);
    }
    fmt::print("----------- File contents end: {}   -----------\n",
               log.string());
    ifs.close();
  } else {
    spdlog::error("Unable to open {}", log.string());
  }

  return 0;
}