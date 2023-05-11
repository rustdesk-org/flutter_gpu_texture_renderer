#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>
#include <windows.h>
#include <thread>
#include <chrono>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>

#include "flutter_window.h"
#include "utils.h"
#include "DDAImpl.h"

using namespace Microsoft::WRL;

typedef void (*DuplicateCallback)(void* texture);

static DuplicateCallback duplicateCallback = nullptr;
static ComPtr<ID3D11Device> device = nullptr;
static ComPtr<ID3D11DeviceContext> deviceCtx = nullptr;

namespace {
void duplicateThread(const std::atomic_bool &stop) {
  std::unique_ptr<DDAImpl> dda = nullptr;

  while (!stop) {
    if (!duplicateCallback) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    if (!dda) {
      dda = std::make_unique<DDAImpl>(device.Get(), deviceCtx.Get());
      dda->Init();
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ComPtr<ID3D11Texture2D> texture = nullptr;
    if(SUCCEEDED(dda->GetCapturedFrame(texture.ReleaseAndGetAddressOf(), 100))) {
          duplicateCallback(texture.Get());
    } else {
      dda.reset();
    }
  }
}
}

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev,
                      _In_ wchar_t *command_line, _In_ int show_command) {
  // Attach to console when present (e.g., 'flutter run') or create a
  // new console when running with a debugger.
  if (!::AttachConsole(ATTACH_PARENT_PROCESS) && ::IsDebuggerPresent()) {
    CreateAndAttachConsole();
  }

  // Initialize COM, so that it is available for use in the library and/or
  // plugins.
  ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  flutter::DartProject project(L"data");

  std::vector<std::string> command_line_arguments =
      GetCommandLineArguments();

  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  FlutterWindow window(project);
  Win32Window::Point origin(10, 10);
  Win32Window::Size size(1280, 720);
  if (!window.Create(L"flutter_gpu_texture_renderer_example", origin, size)) {
    return EXIT_FAILURE;
  }
  window.SetQuitOnClose(true);

  std::atomic_bool stop = false;
  std::thread duplicate(duplicateThread, &stop);

  ::MSG msg;
  while (::GetMessage(&msg, nullptr, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  stop = true;
  duplicate.join();

  ::CoUninitialize();
  return EXIT_SUCCESS;
}
