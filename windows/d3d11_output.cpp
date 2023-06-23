#include "d3d11_output.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#define MS_ENSURE(f, ...) MS_CHECK(f, return __VA_ARGS__;)
#define MS_THROW(f, ...) MS_CHECK(f, throw std::runtime_error(#f);)
#define MS_WARN(f) MS_CHECK(f)
#define MS_CHECK(f, ...)  do { \
        HRESULT __ms_hr__ = (f); \
        if (FAILED(__ms_hr__)) { \
            std::clog << #f "  ERROR@" << __LINE__ << __FUNCTION__ << ": (" << std::hex << __ms_hr__ << std::dec << ") " << std::error_code(__ms_hr__, std::system_category()).message() << std::endl << std::flush; \
            __VA_ARGS__ \
        } \
    } while (false)

namespace flutter_gpu_texture_renderer {

D3D11Output::D3D11Output(flutter::TextureRegistrar *texture_registrar, ID3D11Device *device) :
  texture_registrar_(texture_registrar) {
  dev_ = device;
  dev_->GetImmediateContext(ctx_.ReleaseAndGetAddressOf());
  surface_desc_ = std::make_unique<FlutterDesktopGpuSurfaceDescriptor>();

  variant_ = std::make_unique<flutter::TextureVariant>(
    flutter::GpuSurfaceTexture(kFlutterDesktopGpuSurfaceTypeDxgiSharedHandle,
    [&](size_t width, size_t height) {
      return surface_desc_.get();
    }));
  
  texture_id_ = texture_registrar_->RegisterTexture(variant_.get());
}

D3D11Output::~D3D11Output() {
  if (texture_id_) texture_registrar_->UnregisterTexture(texture_id_);
#ifdef _DEBUG
  if (dev_) {
    ComPtr<ID3D11Debug> debug = nullptr;
    if (SUCCEEDED(dev_.As(&debug))) {
      debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
  }
#endif
}

bool D3D11Output::SetTexture(HANDLE shared_handle) {
  if (!shared_handle) return false;
  if (!allowInput_)  return false;

  if (!EnsureTexture(shared_handle)) return false;
  allowInput_ = false;

  return Present();
}

bool D3D11Output::EnsureTexture(HANDLE shared_handle) {
  ComPtr<IDXGIResource> resource = nullptr;
  MS_THROW(dev_->OpenSharedResource(shared_handle, __uuidof(ID3D10Texture2D), (void**)resource.ReleaseAndGetAddressOf()));
  MS_THROW(resource.As(&tex_));
  D3D11_TEXTURE2D_DESC desc;
  tex_->GetDesc(&desc);
  
  surface_desc_->struct_size = sizeof(FlutterDesktopGpuSurfaceDescriptor);
  surface_desc_->handle = shared_handle;;
  surface_desc_->width = surface_desc_->visible_width = desc.Width;
  surface_desc_->height = surface_desc_->visible_height = desc.Height;
  surface_desc_->format = kFlutterDesktopPixelFormatBGRA8888;
  surface_desc_->release_context = this;
  surface_desc_->release_callback = [](void* release_context) {
    D3D11Output *self = (D3D11Output*) release_context;
    self->allowInput_ = true;
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
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - fps_time_point_.load()).count();
  if (elapsed >= 1) {
    fps_time_point_ = now;
    last_fps_.store(this_fps_);
    this_fps_ = 0;
    // std::cout << "fps:" << (int)last_fps_.load() << std::endl;
  }
}

}

