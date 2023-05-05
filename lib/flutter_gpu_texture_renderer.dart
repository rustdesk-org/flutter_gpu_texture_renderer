
import 'flutter_gpu_texture_renderer_platform_interface.dart';

class FlutterGpuTextureRenderer {
  Future<String?> getPlatformVersion() {
    return FlutterGpuTextureRendererPlatform.instance.getPlatformVersion();
  }
}
