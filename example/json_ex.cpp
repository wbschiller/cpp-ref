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
enum class log_level : int {
  trace = SPDLOG_LEVEL_TRACE,
  debug = SPDLOG_LEVEL_DEBUG,
  info = SPDLOG_LEVEL_INFO,
  warn = SPDLOG_LEVEL_WARN,
  err = SPDLOG_LEVEL_ERROR,
  critical = SPDLOG_LEVEL_CRITICAL,
  off = SPDLOG_LEVEL_OFF,
  n_levels
};

struct system_settings {
  std::vector<Feature> features;
  bool feature_X_enabled;
  bool feature_Y_enabled;
  int calibration_param_a;
  std::optional<log_level> log_level; // this parameter is optional
};
struct cfg {
  factory_settings factory;
  system_settings system;
};
} // namespace example

namespace nlohmann {
template <typename T> struct adl_serializer<std::optional<T>> {
  static void to_json(json &j, const std::optional<T> &opt)
  {
    if (opt) {
      j = *opt;
    } else {
      j = nullptr;
    }
  }

  static void from_json(const json &j, std::optional<T> &opt)
  {
    if (j.is_null()) {
      opt = std::nullopt;
    } else {
      opt = j.get<T>();
    }
  }
};
} // namespace nlohmann

// Preferrably define the conversion to/from JSON in another header hide
// implementation
namespace example {
NLOHMANN_JSON_SERIALIZE_ENUM(log_level, {
                                            {log_level::trace, "trace"},
                                            {log_level::debug, "debug"},
                                            {log_level::info, "info"},
                                            {log_level::warn, "warn"},
                                            {log_level::err, "err"},
                                            {log_level::critical, "critical"},
                                            {log_level::off, "off"},
                                        })

NLOHMANN_JSON_SERIALIZE_ENUM(Feature, {
                                          {Feature::A, "A"},
                                          {Feature::B, "B"},
                                          {Feature::C, "C"},
                                      })

template <typename J, typename K, typename T>
constexpr void to_json_with_optional(J &json_j, K const &key, const T &val)
{
  json_j[key] = val;
}
template <typename J, typename K, typename T>
constexpr void to_json_with_optional(J &json_j, K const &key,
                                     std::optional<T> const &val)
{
  if (val) {
    json_j[key] = val;
  }
}
template <typename J, typename K, typename T>
constexpr void from_json_with_optional(J const &json_j, K const &key, T &val)
{
  json_j.at(key).get_to(val);
}
template <typename J, typename K, typename T>
constexpr void from_json_with_optional(J const &json_j, K const &key,
                                       std::optional<T> &val)
{
  if (auto it = json_j.find(key); it != json_j.end()) {
    val = *it;
  } else {
    val = std::nullopt;
  }
}

#define IO_JSON_JSON_TO_OPTIONAL(v1)                                           \
  to_json_with_optional(nlohmann_json_j, #v1, nlohmann_json_t.v1);
#define IO_JSON_JSON_FROM_OPTIONAL(v1)                                         \
  from_json_with_optional(nlohmann_json_j, #v1, nlohmann_json_t.v1);

#define IO_JSON_DEFINE_TYPE_NON_INTRUSIVE_WITH_OPTIONAL(Type, ...)             \
  inline void to_json(nlohmann::json &nlohmann_json_j,                         \
                      const Type &nlohmann_json_t)                             \
  {                                                                            \
    NLOHMANN_JSON_EXPAND(                                                      \
        NLOHMANN_JSON_PASTE(IO_JSON_JSON_TO_OPTIONAL, __VA_ARGS__))            \
  }                                                                            \
  inline void from_json(const nlohmann::json &nlohmann_json_j,                 \
                        Type &nlohmann_json_t)                                 \
  {                                                                            \
    NLOHMANN_JSON_EXPAND(                                                      \
        NLOHMANN_JSON_PASTE(IO_JSON_JSON_FROM_OPTIONAL, __VA_ARGS__))          \
  }

IO_JSON_DEFINE_TYPE_NON_INTRUSIVE_WITH_OPTIONAL(factory_settings, serial_number,
                                                hardware_version, build_date)
IO_JSON_DEFINE_TYPE_NON_INTRUSIVE_WITH_OPTIONAL(system_settings, features,
                                                feature_X_enabled,
                                                feature_Y_enabled,
                                                calibration_param_a, log_level)
IO_JSON_DEFINE_TYPE_NON_INTRUSIVE_WITH_OPTIONAL(cfg, factory, system)

} // namespace example

auto ConfigToJson(example::cfg &config) -> json
{
  json j = config;
  return j;
}

auto JsonToConfig(json &j) -> std::optional<example::cfg>
{
  try {
    return j.get<example::cfg>();
  } catch (std::exception ex) {
    spdlog::error("Exception getting cfg from json {}: {}", j.dump(),
                  ex.what());
  }
  return std::nullopt;
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

  using namespace example;
  cfg config1{{"SN12345", 1, "2022-11-28 08:30:00"},
              {{Feature::A, Feature::B}, true, false, 1001, log_level::info}};
  auto j3 = ConfigToJson(config1);
  fmt::print("String from json configuration:\n{}\n", j3.dump());

  cfg config2;
  auto j4 = ConfigToJson(config2);
  fmt::print("String from json default configuration:\n{}\n", j4.dump());

  std::string partial_cfg =
      "{\"factory\":{\"build_date\":\"2022-11-28 "
      "08:30:00\",\"hardware_version\":1},\"system\":{\"calibration_param_a\":"
      "1001}}";
  WriteJsonFile(partial_cfg, p);
  if (auto partial_json = ParseJsonFile(p)) {
    if (auto config3 = JsonToConfig(*partial_json)) {
      spdlog::error("Failure - read json from partial config: {}",
                    (*partial_json).dump());
    } else {
      spdlog::info(
          "Expected behavior - failed reading json from partial config: {}",
          (*partial_json).dump());
    }
  } else {
    spdlog::error("Failed parsing partial config file {}", p.c_str());
  }

  std::string valid_cfg =
      "{\"factory\":{\"build_date\":\"2022-11-28 "
      "08:30:00\",\"hardware_version\":1,\"serial_number\":\"SN12345\"},"
      "\"system\":{\"calibration_param_a\":1001,\"feature_X_enabled\":true,"
      "\"feature_Y_enabled\":false,\"features\":[\"A\",\"B\"]}}";
  WriteJsonFile(valid_cfg, p);
  if (auto valid_json = ParseJsonFile(p)) {
    if (auto config3 = JsonToConfig(*valid_json)) {
      auto has_level = ((*config3).system.log_level) ? true : false;
      spdlog::info("Read json from valid config: {} -- log level {}",
                   (*valid_json).dump(), has_level);
    } else {
      spdlog::error("Failed parsing valid config {}", p.c_str());
    }
  } else {
    spdlog::error("Failed parsing valid config file {}", p.c_str());
  }
  return 0;
}