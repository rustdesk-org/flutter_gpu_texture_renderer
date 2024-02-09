#include "d3d11_output.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#define MS_ENSURE(f, ...) MS_CHECK(f, return __VA_ARGS__;)
#define MS_WARN(f) MS_CHECK(f)
#define MS_CHECK(f, ...)                                                       \
  do {                                                                         \
    HRESULT __ms_hr__ = (f);                                                   \
    if (FAILED(__ms_hr__)) {                                                   \
      std::clog                                                                \
          << #f "  ERROR@" << __LINE__ << __FUNCTION__ << ": (" << std::hex    \
          << __ms_hr__ << std::dec << ") "                                     \
          << std::error_code(__ms_hr__, std::system_category()).message()      \
          << std::endl                                                         \
          << std::flush;                                                       \
      __VA_ARGS__                                                              \
    }                                                                          \
  } while (false)

namespace flutter_gpu_texture_renderer {

D3D11Output::D3D11Output(flutter::TextureRegistrar *texture_registrar)
    : texture_registrar_(texture_registrar) {
  for (int i = 0; i < 2; i++) {
    surface_desc_[i] = std::make_unique<FlutterDesktopGpuSurfaceDescriptor>();
    if (surface_desc_[i].get() == nullptr) {
      unusable_ = true;
    }
  }

  variant_ = std::make_unique<flutter::TextureVariant>(
      flutter::GpuSurfaceTexture(kFlutterDesktopGpuSurfaceTypeDxgiSharedHandle,
                                 [&](size_t width, size_t height) {
                                   std::lock_guard<std::mutex> lock(mutex_);
                                   busy_ = free_;
                                   return surface_desc_[busy_].get();
                                 }));

  if (texture_registrar_) {
    texture_id_ = texture_registrar_->RegisterTexture(variant_.get());
  } else {
    unusable_ = true;
  }
}

D3D11Output::~D3D11Output() {
  if (texture_id_)
    texture_registrar_->UnregisterTexture(texture_id_);
}

bool D3D11Output::SetTexture(void *texture) {
  if (unusable_) {
    return false;
  }
  if (!texture)
    return false;

  if (!EnsureTexture((ID3D11Texture2D *)texture))
    return false;

  return Present();
}

bool D3D11Output::EnsureTexture(ID3D11Texture2D *texture) {
  std::lock_guard<std::mutex> lock(mutex_);
  tex_ = texture;
  ComPtr<IDXGIResource> resource = nullptr;
  MS_ENSURE(tex_.As(&resource), false);
  HANDLE shared_handle = nullptr;
  MS_ENSURE(resource->GetSharedHandle(&shared_handle), false);
  D3D11_TEXTURE2D_DESC desc;
  tex_->GetDesc(&desc);
  free_ = (busy_ + 1) % 2;
  tex_buffers_[free_] = tex_;

  surface_desc_[free_]->struct_size =
      sizeof(FlutterDesktopGpuSurfaceDescriptor);
  surface_desc_[free_]->handle = shared_handle;
  surface_desc_[free_]->width = surface_desc_[free_]->visible_width =
      desc.Width;
  surface_desc_[free_]->height = surface_desc_[free_]->visible_height =
      desc.Height;
  surface_desc_[free_]->format = kFlutterDesktopPixelFormatBGRA8888;
  surface_desc_[free_]->release_context = this;
  surface_desc_[free_]->release_callback = [](void *release_context) {
    D3D11Output *self = (D3D11Output *)release_context;
    self->SetFPS();
  };

  return true;
}

bool D3D11Output::Present() {
  return texture_registrar_->MarkTextureFrameAvailable(texture_id_);
}

void D3D11Output::SetFPS() {
  this_fps_++;
  auto now = std::chrono::high_resolution_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                     now - fps_time_point_.load())
                     .count();
  if (elapsed >= 1) {
    fps_time_point_ = now;
    last_fps_.store(this_fps_);
    this_fps_ = 0;
    // std::cout << "fps:" << (int)last_fps_.load() << std::endl;
  }
}

} // namespace flutter_gpu_texture_renderer
