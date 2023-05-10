#include "flutter_gpu_texture_renderer_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

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
  if (method_call.method_name().compare("create") == 0) {
    try {
      output_ = std::make_unique<D3D11Output>(
        registrar_->texture_registrar(),
        registrar_->GetView()->GetGraphicsAdapter());
      result->Success(flutter::EncodableValue(output_->TextureId()));
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      result->Error("", e.what());
    }
  } 
  else {
    result->NotImplemented();
  }
}

}  // namespace flutter_gpu_texture_renderer
