// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <algorithm>
#include <iostream>
#include <string>
#include <ie_common.h>
#include <legacy/ie_layers.h>
#include <iomanip>
#include <caseless.hpp>
#include <layers/gna_copy_layer.hpp>

#include "dnn_components.hpp"
#include "log/log.hpp"

using namespace ov::intel_gna;
using namespace GNAPluginNS;
using namespace GNAPluginNS::backend;

intel_dnn_component_t & DnnComponents::addComponent(const std::string layerName, const std::string layerMetaType) {
    auto isDelayed = InferenceEngine::details::CaselessEq<std::string>()(layerMetaType, DelayedCopyLayerName);
    delayedOperations += isDelayed ? 1 : 0;
    components.emplace_back(DnnComponentExtra{layerName, {}, isDelayed});
    auto &currentComponent = components.back().dnnComponent;

    log::trace() << "IR layer : " << std::left << std::setw(20) << layerName << " " << layerMetaType << "_" << components.size() - 1 << std::endl;

    currentComponent.original_layer_name = components.back().name.c_str();
    int execOrder = 0;
    if (!isDelayed) {
        execOrder = static_cast<int>(components.size() - 1 - delayedOperations);
    } else {
        // todo: not perfect - propose to create mapping table that will be printed out by extra request
        execOrder = - static_cast<int>(delayedOperations);
    }

    log::debug() << "IR layer : " << std::left << std::setw(20) << layerName << " " << layerMetaType << "_" << execOrder << std::endl;
    return currentComponent;
}

intel_dnn_component_t* DnnComponents::findComponent(InferenceEngine::CNNLayerPtr layer) {
    if (layer) {
        return findComponent(layer->name);
    }

    return nullptr;
}

intel_dnn_component_t* GNAPluginNS::backend::DnnComponents::findComponent(const std::string& layerName) {
    auto component = std::find_if(begin(components), end(components), [&](const storage_type ::value_type& comp) {
        return comp.name == layerName;
    });
    if (component != components.end()) {
        return &component->dnnComponent;
    }
    return nullptr;
}

const intel_dnn_component_t* GNAPluginNS::backend::DnnComponents::findComponent(
    const InferenceEngine::CNNLayerPtr layer) const {
    if (layer) {
        return findComponent(layer->name);
    }

    return nullptr;
}

const intel_dnn_component_t* GNAPluginNS::backend::DnnComponents::findComponent(const std::string& layerName) const {
    auto component = std::find_if(begin(components), end(components), [&](const storage_type ::value_type& comp) {
        return comp.name == layerName;
    });
    if (component != components.end()) {
        return &component->dnnComponent;
    }
    return nullptr;
}

std::vector<intel_dnn_component_t> DnnComponents::getExecutionOrder() {
    std::vector<intel_dnn_component_t> result(components.size());

    uint32_t direct_id = 0;
    uint32_t delayed_id = static_cast<uint32_t>(components.size() - delayedOperations);

    for (auto &&c : components) {
        uint32_t &id = c.isDelayed ? delayed_id : direct_id;
        result[id] = c.dnnComponent;
        id++;
    }
    return result;
}
