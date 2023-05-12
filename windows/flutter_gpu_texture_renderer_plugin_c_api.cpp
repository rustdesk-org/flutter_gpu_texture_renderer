#include "include/flutter_gpu_texture_renderer/flutter_gpu_texture_renderer_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_gpu_texture_renderer_plugin.h"

#include <cassert>

void FlutterGpuTextureRendererPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_gpu_texture_renderer::FlutterGpuTextureRendererPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

using flutter_gpu_texture_renderer::D3D11Output;

void FlutterGpuTextureRendererPluginCApiSetTexture(void *output, void *texture) {
    assert(output != nullptr);
    assert(texture != nullptr);
    D3D11Output *d3d11Output = (D3D11Output*)(output);
    d3d11Output->SetTexture((ID3D11Texture2D*)texture);
}