import 'dart:ffi';

import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_gpu_texture_renderer/flutter_gpu_texture_renderer.dart';
import 'package:window_manager/window_manager.dart';

import 'generated_bindings.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> with WindowListener {
  int? _textureId;
  final _flutterGpuTextureRendererPlugin = FlutterGpuTextureRenderer();
  final _duplicationLib = NativeLibrary(DynamicLibrary.open("duplication.dll"));

  @override
  void initState() {
    windowManager.addListener(this);
    super.initState();
    initPlatformState();
  }

  @override
  void dispose() {
    windowManager.removeListener(this);
    super.dispose();
  }

  @override
  void onWindowClose() {
    super.onWindowClose();
    _duplicationLib.StopDuplicateThread();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    try {
      _textureId = await _flutterGpuTextureRendererPlugin.registerTexture();
      if (_textureId != null) {
        final device = await _flutterGpuTextureRendererPlugin.device();
        final output = await _flutterGpuTextureRendererPlugin.output();
        debugPrint("textureId: $_textureId, output: $output, device: $device");
        setState(() {});

        if (device != null && output != null) {
          _duplicationLib.SetAndStartDuplicateThread(
              Pointer.fromAddress(output), Pointer.fromAddress(device));
        }
      }
    } on PlatformException {
      debugPrint("platform exception");
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: _textureId != null
              ? Texture(textureId: _textureId!)
              : const Text("_textureId is null"),
        ),
      ),
    );
  }
}
