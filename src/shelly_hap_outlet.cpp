/*
 * Copyright (c) 2020 Deomid "rojer" Ryabkov
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "shelly_hap_outlet.h"

namespace shelly {
namespace hap {

Outlet::Outlet(int id, Input *in, Output *out, PowerMeter *out_pm,
               struct mgos_config_sw *cfg, HAPAccessoryServerRef *server,
               const HAPAccessory *accessory)
    : ShellySwitch(id, in, out, out_pm, cfg, server, accessory) {
}

Outlet::~Outlet() {
}

Status Outlet::Init() {
  auto st = ShellySwitch::Init();
  if (!st.ok()) return st;

  const int id1 = id() - 1;  // IDs used to start at 0, preserve compat.
  uint16_t iid = IID_BASE_OUTLET + (IID_STEP_OUTLET * id1);
  svc_.iid = iid++;
  svc_.serviceType = &kHAPServiceType_Outlet;
  svc_.debugDescription = kHAPServiceDebugDescription_Outlet;
  // Name
  AddNameChar(iid++, cfg_->name);
  // On
  auto *on_char = new BoolCharacteristic(
      iid++, &kHAPCharacteristicType_On,
      [this](HAPAccessoryServerRef *, const HAPBoolCharacteristicReadRequest *,
             bool *value) {
        *value = out_->GetState();
        return kHAPError_None;
      },
      true /* supports_notification */,
      [this](HAPAccessoryServerRef *, const HAPBoolCharacteristicWriteRequest *,
             bool value) {
        SetState(value, "HAP");
        return kHAPError_None;
      },
      kHAPCharacteristicDebugDescription_On);
  state_notify_char_ = on_char->GetBase();
  AddChar(on_char);
  // In Use
  AddChar(new BoolCharacteristic(
      iid++, &kHAPCharacteristicType_OutletInUse,
      [](HAPAccessoryServerRef *, const HAPBoolCharacteristicReadRequest *,
         bool *value) {
        *value = true;
        return kHAPError_None;
      },
      true /* supports_notification */, nullptr /* write_handler */,
      kHAPCharacteristicDebugDescription_OutletInUse));

  return Status::OK();
}

HAPError Outlet::HandleOnRead(HAPAccessoryServerRef *server,
                              const HAPBoolCharacteristicReadRequest *request,
                              bool *value) {
  *value = out_->GetState();
  (void) server;
  (void) request;
  return kHAPError_None;
}

HAPError Outlet::HandleOnWrite(HAPAccessoryServerRef *server,
                               const HAPBoolCharacteristicWriteRequest *request,
                               bool value) {
  SetState(value, "HAP");
  (void) server;
  (void) request;
  return kHAPError_None;
}

}  // namespace hap
}  // namespace shelly
