#include "flutter_gpu_texture_renderer_plugin.h"

#include <windows.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace flutter_gpu_texture_renderer {

// static
void FlutterGpuTextureRendererPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<FlutterGpuTextureRendererPlugin>(registrar);
  registrar->AddPlugin(std::move(plugin));
}

FlutterGpuTextureRendererPlugin::FlutterGpuTextureRendererPlugin(flutter::PluginRegistrarWindows* registrar)
: registrar_(registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_gpu_texture_renderer",
          &flutter::StandardMethodCodec::GetInstance());
  channel->SetMethodCallHandler(
      [&](const auto &call, auto result) {
        this->HandleMethodCall(call, std::move(result));
      });

}

FlutterGpuTextureRendererPlugin::~FlutterGpuTextureRendererPlugin() {
  for (auto& output: outputs_) output.reset();
  outputs_.clear();
}

void FlutterGpuTextureRendererPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  try {
    if (method_call.method_name().compare("registerTexture") == 0) {
      auto output = std::make_unique<D3D11Output>(
        registrar_->texture_registrar(),
        registrar_->GetView()->GetGraphicsAdapter());
      auto id = output->TextureId();
      outputs_.push_back(std::move(output));
      return result->Success(flutter::EncodableValue(id));
    } else if (method_call.method_name().compare("unregisterTexture") == 0) {
        auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
        auto id = std::get<int64_t>(args.at(flutter::EncodableValue("id")));
        auto new_end = std::remove_if(outputs_.begin(), outputs_.end(),
          [id](const std::unique_ptr<D3D11Output>& output) {
            return output->TextureId() == id;
        });
        outputs_.erase(new_end, outputs_.end());
        return result->Success();
    } else if (method_call.method_name().compare("device") == 0) {
      auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
      auto id = std::get<int64_t>(args.at(flutter::EncodableValue("id")));
      auto it = std::find_if(outputs_.begin(), outputs_.end(),
        [id] (const std::unique_ptr<D3D11Output>& output) {
          return output->TextureId() == id;
      });
      if (it != outputs_.end()) {
        return result->Success(flutter::EncodableValue((int64_t)(*it)->Device()));
      }
    } else if (method_call.method_name().compare("output") == 0) {
      auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
      auto id = std::get<int64_t>(args.at(flutter::EncodableValue("id")));
      auto it = std::find_if(outputs_.begin(), outputs_.end(),
        [id] (const std::unique_ptr<D3D11Output>& output) {
          return output->TextureId() == id;
      });
      if (it != outputs_.end()) {
        return result->Success(flutter::EncodableValue((int64_t)(*it).get()));
      }
    } else {
      return result->NotImplemented();
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return result->Error("", e.what());
  }

  return result->Error("", "");
}

}  // namespace flutter_gpu_texture_renderer
