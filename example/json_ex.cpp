#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>
namespace fs = std::filesystem;

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
using json = nlohmann::json;

auto JsonToStringExample() -> json
{
  json j = {{"pi", 3.141},
            {"happy", true},
            {"name", "Niels"},
            {"nothing", nullptr},
            {"answer", {{"everything", 42}}},
            {"list", {1, 0, 2}},
            {"object", {{"currency", "USD"}, {"value", 42.99}}}};

  fmt::print("String from json type:\n{}\n", j.dump(2));
  // Line below compiles but throws runtime exception
  // fmt::print("fmt supports json type:\n{}\n", j);

  return j;
}

auto WriteJsonFile(const std::string &contents, const fs::path &p)
{
  std::error_code ec;
  fs::remove(p, ec);
  std::ofstream ofs(p.c_str());
  if (ofs) {
    ofs << contents << std::endl;
  }
  ofs.close();
  if (ofs.bad()) {
    spdlog::error("Failed writing json file: {}", p.c_str());
  }
}

auto WriteJsonFile(json j, const fs::path &p) { WriteJsonFile(j.dump(2), p); }

auto ParseJsonFile(const fs::path &p) -> std::optional<json>
{
  try {
    std::ifstream ifs(p.c_str());
    if (ifs) {
      json j;
      ifs >> j;
      if (ifs.good()) {
        return j;
      }
    }
  } catch (std::exception ex) {
    spdlog::error("Exception parsing json file {}: {}", p.c_str(), ex.what());
  }

  return std::nullopt;
}

namespace example {
struct factory_settings {
  std::string serial_number;
  int hardware_version;
  std::string build_date;
};
enum class Feature {
  A,
  B,
  C,
};
struct system_settings {
  std::vector<Feature> features;
  bool feature_X_enabled;
  bool feature_Y_enabled;
  int calibration_param_a;
};
struct cfg {
  factory_settings factory;
  system_settings system;
};

// Conversions to/from JSON
NLOHMANN_JSON_SERIALIZE_ENUM(Feature, {
                                          {Feature::A, "A"},
                                          {Feature::B, "B"},
                                          {Feature::C, "C"},
                                      })
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(factory_settings, serial_number,
                                   hardware_version, build_date)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(system_settings, features, feature_X_enabled,
                                   feature_Y_enabled, calibration_param_a)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(cfg, factory, system)

} // namespace example

auto ConfigToJsonStringExample() -> json
{
  using namespace example;
  cfg config{{"SN12345", 1, "2022-11-28 08:30:00"},
             {{Feature::A, Feature::B}, true, false, 1001}};
  json j = config;
  fmt::print("String from json configuration:\n{}\n", j.dump(2));

  return j;
}

/**
 * A simple example for using nlohmann/json library
 */
auto main() -> int
{
  std::error_code ec;
  auto p = fs::temp_directory_path() / "example.json";

  auto j1 = JsonToStringExample();
  WriteJsonFile(j1, p);
  if (auto j2 = ParseJsonFile(p)) {
    spdlog::info("Read json from file:{}", (*j2).dump());
    spdlog::info("Comparison of serialization/deserialziation is {}",
                 j1 == *j2);
  }
  fs::remove(p, ec);

  std::string junk =
      "{\"answer\":{\"everything\":42},\"happy\":true,\"list\":[1,0,2],";
  auto bad_file = fs::temp_directory_path() / "invalid.json";
  WriteJsonFile(junk, bad_file);
  auto bad_json = ParseJsonFile(bad_file);
  if (bad_json) {
    spdlog::error("Read json from corrupt file: {}", (*bad_json).dump());
  } else {
    spdlog::info("Expected behavior: failed parsing corrupt file {}",
                 bad_file.c_str());
  }

  fs::remove(bad_file, ec);

  auto j3 = ConfigToJsonStringExample();
  WriteJsonFile(j3, p);

  return 0;
}