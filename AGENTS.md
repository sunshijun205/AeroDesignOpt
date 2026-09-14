# AeroDesignOpt — Cursor Agent 规则

强制上下文。细节见 [`docs/`](docs/README.md)。**前端约束全文**：[`docs/frontend_constraints.md`](docs/frontend_constraints.md)。

## 1. 项目信息

| 项 | 值 |
|---|---|
| 名称 | AeroDesignOpt / 气动设计优化平台 |
| 形态 | **纯前端单机** Qt Widgets；UI 原型为主；**翼型气动分析**已示范 MVP 分层（无网络） |
| 语言 | **C++17**（全文统一，禁止混用约定以外的特性） |
| Qt | **以 Qt 5.15.2 为准**；禁止混用 Qt6-only API。CMake 虽可找 Qt6，升级前不得混用 |
| 构建 | CMake ≥ 3.16；本机验证：MinGW Makefiles + g++ 8.1.0 |
| 依赖 | 仅 `Qt::Widgets`（无第三方库）；新增第三方库须登记到 CMake 并说明获取方式 |

## 2. 开发红线（摘要）

1. 改前读相关 `docs/`，用搜索确认真实类/文件；**勿假设已有业务层**。
2. 新业务代码按分层落地（View → Presenter → Service → 本地数据），见 `frontend_constraints.md` §10（翼型气动分析样板）。原型期改 UI 时仍优先保持现有 `MainWindow` → `*Page` → `uihelpers`。
3. View **不写业务、不直接取数**；跨线程更新 UI **必须** `Qt::QueuedConnection`。
4. 新增/删除源文件必须改 `CMakeLists.txt` 的 `PROJECT_SOURCES`；含 `Q_OBJECT` 靠 AUTOMOC。
5. UI 以代码布局 + `objectName` + `Theme` QSS 为主；无 `.ui` 入构建。
6. 禁止删改四大导航语义；改导航时同步 `switchMode` / `updateActions` 文案数组。
7. 最小改动；禁止 silent failure；失败要恢复 UI。
8. 无自动化测试：回归 = Build + 人工走查。

## 3. Build

```powershell
$env:Path = "C:\Qt\Tools\mingw810_64\bin;C:\Qt\5.15.2\mingw81_64\bin;C:\Qt\Tools\CMake_64\bin;" + $env:Path
cmake -S . -B build-mingw64 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_64"
cmake --build build-mingw64 --target AeroDesignOpt
# Build 成功后自动运行：先清理旧实例，再后台启动（勿阻塞；需 Qt bin 已在 PATH）
Stop-Process -Name AeroDesignOpt -Force -ErrorAction SilentlyContinue
Start-Process -FilePath ".\build-mingw64\AeroDesignOpt.exe"
```

路径因机器而异。详见 [`docs/build.md`](docs/build.md)。

改完必须 Build；失败则分析→修复→再编，禁止只报“编译失败”。**Build 成功后须直接运行生成的 `build-mingw64\AeroDesignOpt.exe`（启动前先 `Stop-Process -Name AeroDesignOpt` 清理旧实例，再用 `Start-Process` 后台启动，不阻塞会话；仅在 Build 通过时运行），免去手动打开 Qt。**

## 4. 文档

- 知识库：`docs/frontend_constraints.md`、`architecture.md`、`ui.md`、`build.md`；业务说明见 `docs/business/`（入口 [`docs/README.md`](docs/README.md)）。
- 改业务时按需加载 `docs/business/<id>.md`（如 `airfoil_analysis`），勿一次读完全部业务文。
- 结构/接口/业务 API 变更须同步对应 docs。
- 文档与代码冲突时：**以代码为准**并改文档。

## 5. Checklist

写代码前：所属层？是否越界？长任务要不要 worker？有无已有复用？

提交前：

- [ ] C++17 / Qt5 API 合规
- [ ] CMake 源列表与依赖已更新
- [ ] 无子线程直接操作 UI；跨线程显式 QueuedConnection
- [ ] 无新增 `../` include；刷新范围最小必要
- [ ] 无伪数据兜底冒充真实结果；错误有提示且 UI 可恢复
- [ ] Build 通过；相关 docs 已更新
