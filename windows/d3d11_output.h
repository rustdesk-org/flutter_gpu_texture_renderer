#ifndef FLUTTER_PLUGIN_D3D11_OUTPUT_PLUGIN_H_
#define FLUTTER_PLUGIN_D3D11_OUTPUT_PLUGIN_H_

#include <atomic>
#include <d3d11.h>
#include <dxgi.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <memory>
#include <thread>
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace flutter_gpu_texture_renderer {

class D3D11Output {
public:
  D3D11Output(flutter::TextureRegistrar *texture_registrar,
              ID3D11Device *device);
  virtual ~D3D11Output();
  int64_t TextureId() { return texture_id_; }
  ID3D11Device *Device() { return dev_.Get(); }
  bool SetTexture(HANDLE shared_handle);
  bool Present();
  int16_t Fps() { return last_fps_; }

private:
  D3D11Output() = delete;
  D3D11Output(const D3D11Output &) = delete;
  D3D11Output &operator=(const D3D11Output &) = delete;
  bool EnsureTexture(HANDLE shared_handle);
  void SetFPS();

private:
  flutter::TextureRegistrar *texture_registrar_ = nullptr;
  ComPtr<ID3D11Device> dev_ = nullptr;
  ComPtr<ID3D11DeviceContext> ctx_ = nullptr;
  ComPtr<ID3D11Texture2D> tex_ = nullptr;
  ComPtr<ID3D11Texture2D> tex_buffers_[2] = {nullptr, nullptr};
  std::atomic<int> current_tex_buffer_index_ = 0;
  std::atomic<int> tex_occupied_count_ = 0;
  std::unique_ptr<FlutterDesktopGpuSurfaceDescriptor> surface_desc_ = nullptr;
  std::unique_ptr<flutter::TextureVariant> variant_ = nullptr;
  int64_t texture_id_ = 0;
  std::atomic_char16_t last_fps_ = 0;
  std::atomic_char16_t this_fps_ = 0;
  std::atomic<std::chrono::steady_clock::time_point> fps_time_point_ =
      std::chrono::steady_clock::now();
};

} // namespace flutter_gpu_texture_renderer

#endif // FLUTTER_PLUGIN_D3D11_OUTPUT_PLUGIN_H_
