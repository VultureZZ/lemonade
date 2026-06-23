#include "lemon/model_resolution.h"

#include "lemon/model_manager.h"
#include "lemon/runtime_config.h"
#include "lemon/utils/aixlog.hpp"

namespace lemon {

namespace {

std::string resolve_default_fallback(const std::string& requested,
                                     ModelManager* model_manager) {
    if (model_manager && model_manager->model_exists(requested)) {
        return requested;
    }

    if (auto* cfg = RuntimeConfig::global()) {
        const std::string fallback = cfg->default_model();
        if (!fallback.empty() && model_manager && model_manager->model_exists(fallback)) {
            LOG(INFO, "ModelResolution")
                << "Model '" << requested << "' not found, using default model '"
                << fallback << "'" << std::endl;
            return fallback;
        }
    }

    return requested;
}

} // namespace

std::string resolve_model_with_default(const std::string& requested,
                                       ModelManager* model_manager) {
    return resolve_default_fallback(requested, model_manager);
}

std::string resolve_model_for_chat(const std::string& requested,
                                   ModelManager* model_manager) {
    if (auto* cfg = RuntimeConfig::global()) {
        const std::string override_model = cfg->override_model();
        if (!override_model.empty() && model_manager &&
            model_manager->model_exists(override_model)) {
            if (override_model != requested) {
                LOG(INFO, "ModelResolution")
                    << "Overriding model '" << requested << "' with '"
                    << override_model << "'" << std::endl;
            }
            return override_model;
        }
    }

    return resolve_default_fallback(requested, model_manager);
}

void apply_chat_model_to_request(json& request_json, ModelManager* model_manager) {
    if (!request_json.contains("model") || !request_json["model"].is_string()) {
        return;
    }

    const std::string requested = request_json["model"].get<std::string>();
    const std::string resolved = resolve_model_for_chat(requested, model_manager);
    if (resolved != requested) {
        request_json["model"] = resolved;
    }
}

} // namespace lemon
