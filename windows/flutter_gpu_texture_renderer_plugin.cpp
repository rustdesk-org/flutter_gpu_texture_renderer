#include "flutter_gpu_texture_renderer_plugin.h"

#include <windows.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace flutter_gpu_texture_renderer {

ComPtr<ID3D11Device> FlutterGpuTextureRendererPlugin::dev_ = nullptr;

// static
void FlutterGpuTextureRendererPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto plugin = std::make_unique<FlutterGpuTextureRendererPlugin>(registrar);
  registrar->AddPlugin(std::move(plugin));
}

FlutterGpuTextureRendererPlugin::FlutterGpuTextureRendererPlugin(flutter::PluginRegistrarWindows* registrar)
: registrar_(registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_gpu_texture_renderer",
          &flutter::StandardMethodCodec::GetInstance());
  channel->SetMethodCallHandler(
      [&](const auto &call, auto result) {
        this->HandleMethodCall(call, std::move(result));
      });
  CreateDevice(registrar_->GetView()->GetGraphicsAdapter());
}

FlutterGpuTextureRendererPlugin::~FlutterGpuTextureRendererPlugin() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto& output: outputs_) output.reset();
  outputs_.clear();
}

void FlutterGpuTextureRendererPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::lock_guard<std::mutex> lock(mutex_);
  try {
    if (method_call.method_name().compare("registerTexture") == 0) {
      if (dev_) {
        auto output = std::make_unique<D3D11Output>(registrar_->texture_registrar(), dev_.Get());
        auto id = output->TextureId();
        outputs_.push_back(std::move(output));
        return result->Success(flutter::EncodableValue(id));
      }
    } else if (method_call.method_name().compare("unregisterTexture") == 0) {
        auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
        auto id = std::get<int64_t>(args.at(flutter::EncodableValue("id")));
        auto new_end = std::remove_if(outputs_.begin(), outputs_.end(),
          [id](const std::unique_ptr<D3D11Output>& output) {
            return output->TextureId() == id;
        });
        outputs_.erase(new_end, outputs_.end());
        return result->Success();
    } else if (method_call.method_name().compare("device") == 0) {
      auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
      auto id = std::get<int64_t>(args.at(flutter::EncodableValue("id")));
      auto it = std::find_if(outputs_.begin(), outputs_.end(),
        [id] (const std::unique_ptr<D3D11Output>& output) {
          return output->TextureId() == id;
      });
      if (it != outputs_.end()) {
        return result->Success(flutter::EncodableValue((int64_t)(*it)->Device()));
      }
    } else if (method_call.method_name().compare("output") == 0) {
      auto args = std::get<flutter::EncodableMap>(*method_call.arguments());
      auto id = std::get<int64_t>(args.at(flutter::EncodableValue("id")));
      auto it = std::find_if(outputs_.begin(), outputs_.end(),
        [id] (const std::unique_ptr<D3D11Output>& output) {
          return output->TextureId() == id;
      });
      if (it != outputs_.end()) {
        return result->Success(flutter::EncodableValue((int64_t)(*it).get()));
      }
    } else {
      return result->NotImplemented();
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return result->Error("", e.what());
  }

  return result->Error("", "");
}

bool FlutterGpuTextureRendererPlugin::CreateDevice(IDXGIAdapter *adapter) {
  if (dev_) return true;
  if (adapter) {
    DXGI_ADAPTER_DESC desc;
    if (SUCCEEDED(adapter->GetDesc(&desc))) {
      std::wcout << "Graphics adapter: " << desc.Description << std::endl;
    }
  }
  UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  auto hr = D3D11CreateDevice(
    adapter,
    (adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE),
    NULL, creationFlags, NULL, 0, D3D11_SDK_VERSION,
    dev_.ReleaseAndGetAddressOf(), NULL,
    NULL);
  if (FAILED(hr)) {
    std::cerr << "D3D11CreateDevice failed, hr = " << std::hex << hr << std::dec << std::endl;
    return false;
  }

  ComPtr<ID3D10Multithread> mt;
  hr = dev_.As(&mt);
  if (FAILED(hr)) {
    std::cerr << "Get ID3D10Multithread failed, hr = " << std::hex << hr << std::dec << std::endl;
    return false;
  }
  if (!mt->SetMultithreadProtected(TRUE)) {
    std::cerr << "SetMultithreadProtected failed" << std::endl;
    // return false;
  }
  return true;
}

}  // namespace flutter_gpu_texture_renderer
