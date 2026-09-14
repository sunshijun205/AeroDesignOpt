# 构建

## 已验证环境

Windows · Qt **5.15.2** mingw81_64 · g++ **8.1.0** · CMake **3.30.5** · Generator **MinGW Makefiles** · Debug · 产物 `build-mingw64/AeroDesignOpt.exe`

路径因机器而异。Kit 与 Generator 勿混用；Debug/Release 勿混链 Qt。

## 命令

```powershell
$env:Path = "C:\Qt\Tools\mingw810_64\bin;C:\Qt\5.15.2\mingw81_64\bin;C:\Qt\Tools\CMake_64\bin;" + $env:Path

cmake -S . -B build-mingw64 -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_64"
cmake --build build-mingw64 --target AeroDesignOpt
```

Release：换目录 `build-mingw64-release` 与 `-DCMAKE_BUILD_TYPE=Release`（Release 全链路 **待确认**）。

清理：`Remove-Item -Recurse -Force build-mingw64` 或 `cmake --build build-mingw64 --target clean`。

运行：PATH 含 Qt `bin` 后执行 `.\build-mingw64\AeroDesignOpt.exe`。**约定：Build 通过后先 `Stop-Process -Name AeroDesignOpt -Force -ErrorAction SilentlyContinue` 清理旧实例，再用 `Start-Process ".\build-mingw64\AeroDesignOpt.exe"` 后台启动，免手动开 Qt；仅在 Build 成功时运行，且不阻塞会话。**

新增源文件 → 写入 `PROJECT_SOURCES` → 重新 build（必要时重新 cmake）。

气动分析落盘路径（Windows 典型）：`%AppData%/AeroDesignOpt/气动设计优化平台/airfoil_analysis.json`。

## 依赖

当前仅依赖 `Qt::Widgets`，无第三方库。新增第三方库时须在 `CMakeLists.txt` 显式登记，并在本文件补充获取方式（find_package / vendored / FetchContent）。

## 排查

| 问题 | 处理 |
|------|------|
| 找不到 Qt | 检查 `CMAKE_PREFIX_PATH` |
| 改了文件无效果 | 是否写入 `PROJECT_SOURCES` |
| moc/链接异常 | `Q_OBJECT` + 源文件已入目标 |
| 缺 DLL | PATH 加 Qt bin / windeployqt（**待确认**） |
| 样式不对 | `objectName` ↔ `Theme`；动态改名后 polish |
| 中文乱码 | UTF-8 源文件 + MinGW UTF-8 编译选项 |
| 按钮「没功能」 | 原型预期：`QMessageBox` |
| 子线程改 UI 崩溃/警告 | 改用 `Qt::QueuedConnection` / 信号回 UI 线程 |
