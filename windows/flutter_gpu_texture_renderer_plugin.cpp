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

FlutterGpuTextureRendererPlugin::~FlutterGpuTextureRendererPlugin() {}

void FlutterGpuTextureRendererPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  try {
    if (method_call.method_name().compare("registerTexture") == 0) {
        output_ = std::make_unique<D3D11Output>(
          registrar_->texture_registrar(),
          registrar_->GetView()->GetGraphicsAdapter());
        return result->Success(flutter::EncodableValue(output_->TextureId()));
    } else if (method_call.method_name().compare("unregisterTexture") == 0) {
        output_.reset();
        return result->Success();
    } else if (method_call.method_name().compare("device") == 0) { 
      if (output_) {
        return result->Success(flutter::EncodableValue((int64_t)output_->Device()));
      }
    } else if (method_call.method_name().compare("output") == 0) { 
      if (output_) {
        return result->Success(flutter::EncodableValue((int64_t)output_.get()));
      }
    } else if (method_call.method_name().compare("setTexture") == 0) { 
      if (output_) {
        auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
        auto texture = std::get<int64_t>(args.at(flutter::EncodableValue("texture")));
        if (output_->SetTexture(((ID3D11Texture2D*)texture))) return result->Success();
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
