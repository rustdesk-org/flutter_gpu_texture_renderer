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
  final _flutterGpuTextureRendererPlugin = FlutterGpuTextureRenderer();
  final _duplicationLib = NativeLibrary(DynamicLibrary.open("duplication.dll"));
  var _count = 1;

  @override
  void initState() {
    windowManager.addListener(this);
    super.initState();
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

  @override
  Widget build(BuildContext context) {
    final items = List<Widget>.empty(growable: true);
    for (int i = 0; i < _count; i++) {
      items.add(SizedBox(
          width: 1920 / 2,
          height: 1080 / 2,
          child: VideoOutput(
              plugin: _flutterGpuTextureRendererPlugin,
              library: _duplicationLib)));
    }
    bool isSingleItem = _count == 1;
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Row(
            children: [
              const Text('Plugin example app'),
              Padding(
                  padding: const EdgeInsets.symmetric(horizontal: 20),
                  child: Text("Count:$_count")),
              ElevatedButton(
                  onPressed: () {
                    setState(() {
                      _count++;
                    });
                  },
                  child: const Text('+')),
              ElevatedButton(
                  onPressed: () {
                    if (_count > 0) {
                      setState(() {
                        _count--;
                      });
                    }
                  },
                  child: const Text('-')),
            ],
          ),
        ),
        body: GridView.count(
          crossAxisCount: isSingleItem ? 1 : 2,
          childAspectRatio: isSingleItem ? 2 : 1,
          children: List.generate(
            _count,
            (index) => Padding(
              padding: EdgeInsets.all(isSingleItem ? 0 : 5),
              child: SizedBox(
                width: 1920,
                height: 1080,
                child: VideoOutput(
                    plugin: _flutterGpuTextureRendererPlugin,
                    library: _duplicationLib),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class VideoOutput extends StatefulWidget {
  final FlutterGpuTextureRenderer plugin;
  final NativeLibrary library;
  const VideoOutput({Key? key, required this.plugin, required this.library})
      : super(key: key);

  @override
  State<VideoOutput> createState() => _VideoOutputState();
}

class _VideoOutputState extends State<VideoOutput> {
  int? _textureId;
  get plugin => widget.plugin;
  get library => widget.library;

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> initPlatformState() async {
    try {
      _textureId = await widget.plugin.registerTexture();
      if (_textureId != null) {
        final device = await plugin.device(_textureId!);
        final output = await plugin.output(_textureId!);
        setState(() {});
        if (device != null && output != null) {
          library.StartDuplicateThread(Pointer.fromAddress(device));
          library.AddOutput(Pointer.fromAddress(output));
        }
      }
    } on PlatformException {
      debugPrint("platform exception");
    }
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      child: _textureId != null
          ? Texture(textureId: _textureId!)
          : const Text("_textureId is null"),
    );
  }
}
