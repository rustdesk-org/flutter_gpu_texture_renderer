#include "include/flutter_gpu_texture_renderer/flutter_gpu_texture_renderer_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_gpu_texture_renderer_plugin.h"

void FlutterGpuTextureRendererPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_gpu_texture_renderer::FlutterGpuTextureRendererPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
