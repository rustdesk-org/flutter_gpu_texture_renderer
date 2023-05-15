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

bool D3D11Output::SetTexture(ID3D11Texture2D *texture) {
  if (!texture) return false;
  if (!allowInput_) return false;

  ComPtr<ID3D11Texture2D> tex = texture;
  if (!EnsureTexture(tex.Get())) return false;
  ctx_->CopyResource(tex_.Get(), tex.Get());
  ctx_->Flush();
  allowInput_ = false;

  return Present();
}

bool D3D11Output::EnsureTexture(ID3D11Texture2D *texture) {
  D3D11_TEXTURE2D_DESC newDesc;
  texture->GetDesc(&newDesc);
  if (tex_ && newDesc.Width == width_ && newDesc.Height == height_) return true;

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = newDesc.Width;
  desc.Height = newDesc.Height;
  desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
  MS_THROW(dev_->CreateTexture2D(&desc, nullptr, tex_.ReleaseAndGetAddressOf()));

  // clear view only
  ComPtr<ID3D11RenderTargetView> rtv;
  D3D11_RENDER_TARGET_VIEW_DESC rtd;
  ZeroMemory(&rtd, sizeof(rtd));
  rtd.Format = desc.Format;
  rtd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  MS_THROW(dev_->CreateRenderTargetView(tex_.Get(), &rtd, &rtv));
  const float c[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  ctx_->ClearRenderTargetView(rtv.Get(), c);
  ID3D11RenderTargetView* const targets[1] = { rtv.Get()};
  ctx_->OMSetRenderTargets(1, targets, nullptr);

  ComPtr<IDXGIResource> res;
  MS_THROW(tex_.As(&res));
  HANDLE shared_handle = nullptr;
  MS_THROW(res->GetSharedHandle(&shared_handle));
  
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

  width_ = newDesc.Width;
  height_ = newDesc.Height;

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
  }
}

}

