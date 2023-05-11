#ifndef FLUTTER_PLUGIN_D3D11_OUTPUT_PLUGIN_H_
#define FLUTTER_PLUGIN_D3D11_OUTPUT_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <memory>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>
#include <thread>
#include <atomic>

using namespace Microsoft::WRL;

namespace flutter_gpu_texture_renderer {

class D3D11Output  {
 public:
  D3D11Output(flutter::TextureRegistrar *texture_registrar, IDXGIAdapter *adapter);
  virtual ~D3D11Output();
  int64_t TextureId() { return texture_id_; }
  bool SetTexture(ID3D11Texture2D *texture);
  bool Present();

 private:
  D3D11Output() = delete;
  D3D11Output(const D3D11Output&) = delete;
  D3D11Output& operator=(const D3D11Output&) = delete;
  bool EnsureTexture(ID3D11Texture2D *texture);
 
 private:
  flutter::TextureRegistrar *texture_registrar_ = nullptr;
  ComPtr<IDXGIAdapter> adapter_ = nullptr;
  ComPtr<ID3D11Device> dev_ = nullptr;
  ComPtr<ID3D11DeviceContext> ctx_ = nullptr;
  ComPtr<ID3D11Texture2D> tex_ = nullptr;
  std::unique_ptr<FlutterDesktopGpuSurfaceDescriptor> surface_desc_ = nullptr;
  std::unique_ptr<flutter::TextureVariant> variant_ = nullptr;
  int64_t texture_id_ = 0;
  UINT width_ = 0;
  UINT height_ = 0;
  std::atomic_bool allowInput_ = true;
};

}  // namespace flutter_gpu_texture_renderer

#endif  // FLUTTER_PLUGIN_D3D11_OUTPUT_PLUGIN_H_
