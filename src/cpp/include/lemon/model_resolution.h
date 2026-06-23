#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace lemon {

class ModelManager;

using json = nlohmann::json;

/// When `requested` is registered, returns it unchanged. Otherwise, if
/// `default_model` is configured in config.json and that model exists,
/// returns the default. If no fallback applies, returns `requested` as-is
/// (caller should check existence before loading).
std::string resolve_model_with_default(const std::string& requested,
                                       ModelManager* model_manager);

/// Chat/completion resolution: `override_model` (when set and registered) wins,
/// then registered `requested`, then `default_model` fallback, else `requested`.
std::string resolve_model_for_chat(const std::string& requested,
                                   ModelManager* model_manager);

/// Rewrites `request["model"]` when chat resolution changes the effective model.
void apply_chat_model_to_request(json& request_json, ModelManager* model_manager);

} // namespace lemon
