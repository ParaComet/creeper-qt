# MyiMusicPlayer

一个基于 `creeper-qt + Qt Multimedia` 的小型音乐播放器示例。

## 目录结构

- `src/main.cc`: 程序入口
- `src/player_window.hh`: 播放器窗口声明
- `src/player_window.cc`: UI 构建、播放逻辑、事件绑定

## 功能

- 打开并播放本地音频文件（支持多选）
- 播放列表显示与双击切歌
- 上一首 / 播放-暂停 / 下一首
- 拖动进度条跳转
- 音量调节

## 构建

```bash
cd MyiMusicPlayer
cmake -B build
cmake --build build -j
./build/MyiMusicPlayer
```

> 依赖：`Qt6 Widgets`、`Qt6 Multimedia`。
