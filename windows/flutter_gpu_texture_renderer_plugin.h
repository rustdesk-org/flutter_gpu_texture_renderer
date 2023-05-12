#ifndef FLUTTER_PLUGIN_FLUTTER_GPU_TEXTURE_RENDERER_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_GPU_TEXTURE_RENDERER_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <memory>
#include <mutex>
#include <wrl/client.h>
#include "d3d11_output.h"

using namespace Microsoft::WRL;

namespace flutter_gpu_texture_renderer {

class FlutterGpuTextureRendererPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterGpuTextureRendererPlugin(flutter::PluginRegistrarWindows* registrar);

  virtual ~FlutterGpuTextureRendererPlugin();

  // Disallow copy and assign.
  FlutterGpuTextureRendererPlugin(const FlutterGpuTextureRendererPlugin&) = delete;
  FlutterGpuTextureRendererPlugin& operator=(const FlutterGpuTextureRendererPlugin&) = delete;

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
 
 private:
    flutter::PluginRegistrarWindows* registrar_ = nullptr;
    HWND hwnd_ = nullptr;
    std::vector<std::unique_ptr<D3D11Output>> outputs_;
    std::mutex mutex_;   
};

}  // namespace flutter_gpu_texture_renderer

#endif  // FLUTTER_PLUGIN_FLUTTER_GPU_TEXTURE_RENDERER_PLUGIN_H_
