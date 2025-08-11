/**
 * Copyright (C) 2025 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#pragma once

#include <TmxMessageManager.h>
// #include <tmx/j2735_messages/MessageFrame.hpp>
// #include <jer_encoder.h>

namespace JSONMessageLoggerPlugin {
      using buffer_structure_t = struct buffer_structure
    {
        char *buffer;          // buffer array
        size_t buffer_size;    // this is really where we will write next.
        size_t allocated_size; // this is the total size of the buffer.
    };
    class JSONMessageLoggerPlugin : public tmx::utils::TmxMessageManager {
        public:
            JSONMessageLoggerPlugin(const std::string &name);
            void OnStateChange(IvpPluginState state) override;
            void OnConfigChanged(const char *key, const char *value) override;
            void OnMessageReceived(tmx::routeable_message &msg) override;

        protected:
            void UpdateConfigSettings();

        private:
            std::mutex _configMutex;
    };

    int DynamicBufferAppend(const void *buffer, size_t size, void *app_key);

} /* namespace JSONMessageLoggerPlugin */