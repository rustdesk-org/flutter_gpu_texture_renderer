#include "d3d11_output.h"
#include "DDAImpl.h"

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

D3D11Output::D3D11Output(flutter::TextureRegistrar *texture_registrar,IDXGIAdapter *adapter) :
  texture_registrar_(texture_registrar) {
  adapter_ = adapter;
  if (adapter_) {
    DXGI_ADAPTER_DESC desc;
    if (SUCCEEDED(adapter_->GetDesc(&desc))) {
      std::wcout << "Graphics adapter: " << desc.Description << std::endl;
    }
  }
  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  MS_THROW(D3D11CreateDevice(
    adapter_.Get(),
    (adapter_ ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE),
    NULL, creationFlags, NULL, 0, D3D11_SDK_VERSION,
    dev_.ReleaseAndGetAddressOf(), NULL,
    ctx_.ReleaseAndGetAddressOf()));

  ComPtr<ID3D10Multithread> mt;
  MS_THROW(dev_.As(&mt));
  mt->SetMultithreadProtected(TRUE);

  surface_desc_ = std::make_unique<FlutterDesktopGpuSurfaceDescriptor>();

  variant_ = std::make_unique<flutter::TextureVariant>(
    flutter::GpuSurfaceTexture(kFlutterDesktopGpuSurfaceTypeDxgiSharedHandle,
    [&](size_t width, size_t height) {
      return surface_desc_.get();
    }));
  
  texture_id_ = texture_registrar_->RegisterTexture(variant_.get());

  dupThread_ = std::make_unique<std::thread>(&D3D11Output::runDup, this);
}

D3D11Output::~D3D11Output() {
  if (dupThread_) {
    stopThread_ = true;
    dupThread_->join();
  }
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
  if (!(!tex_ || newDesc.Width != width_ || newDesc.Height != height_)) return true;

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
  };

  width_ = newDesc.Width;
  height_ = newDesc.Height;

  return true;
}

bool D3D11Output::Present() {
  return texture_registrar_->MarkTextureFrameAvailable(texture_id_);
}

void D3D11Output::runDup() {
  
  std::unique_ptr<DDAImpl> dda = std::make_unique<DDAImpl>(dev_.Get(), ctx_.Get());
  dda->Init();

  while (!stopThread_) {
    ComPtr<ID3D11Texture2D> texture = nullptr;
    if(SUCCEEDED(dda->GetCapturedFrame(texture.ReleaseAndGetAddressOf(), 100))) {
          SetTexture(texture.Get());
    } else {
      dda.reset();
      dda = std::make_unique<DDAImpl>(dev_.Get(), ctx_.Get());
      dda->Init();
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

}

