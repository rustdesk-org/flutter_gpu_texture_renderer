#include "include/flutter_gpu_texture_renderer/flutter_gpu_texture_renderer_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_gpu_texture_renderer_plugin.h"

#include <cassert>

void FlutterGpuTextureRendererPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_gpu_texture_renderer::FlutterGpuTextureRendererPlugin::
      RegisterWithRegistrar(
          flutter::PluginRegistrarManager::GetInstance()
              ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

void FlutterGpuTextureRendererPluginCApiSetTexture(void *output,
                                                   void *texture) {
  if (!output || !texture)
    return;
  // The pointer may already have been unregistered by the Dart side; the push
  // validates it against the live-object set instead of dereferencing.
  flutter_gpu_texture_renderer::D3D11OutputSetTexture(output, texture);
}

uint64_t FlutterGpuTextureRendererPluginCApiGetConsumed(void *output) {
  if (!output)
    return 0;
  return flutter_gpu_texture_renderer::D3D11OutputConsumed(output);
}

int64_t FlutterGpuTextureRendererPluginCApiGetAdapterLuid() {
  return flutter_gpu_texture_renderer::FlutterGpuTextureRendererPlugin::
      GetAdapterLuid();
}