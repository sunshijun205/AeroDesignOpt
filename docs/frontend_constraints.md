# AeroDesignOpt 前端开发约束

自《Qt 桌面前端开发约束模板》裁剪落地。**形态：纯前端（单机）**。
当前仓库多为 UI 原型；下列 MUST 约束 **新增业务能力时强制遵守**，改现有原型页时尽量向其靠拢、禁止继续扩大越界。

---

## 1. 目标分层

```text
用户操作 → View（信号）→ Presenter（槽）→ Service（用例）→ 本地数据层
         ← View 更新槽 ← Presenter ← 结果/通知 ←
```

| 层 | 目录（目标） | MUST | MUST NOT |
|---|---|---|---|
| View | `ui/` | 展示与交互；发意图信号；`setXxx`/`updateXxx` | 调 Service；写业务；直接取数 |
| Presenter | `controller/` | 唯一协调者；编排用例与刷新范围 | 依赖具体 QWidget 细节；直接取数 |
| Service | `service/` | 用例编排、状态、缓存、计算调度 | 写 UI；堆存储细节 |
| 数据 | `model/` + 本地持久 | 纯数据 / 文件 IO | 反向依赖上层；做业务判断 |

**现状**：**翼型气动分析**已按分层落地（见 §10），是唯一完整业务栈样板。其余逻辑与演示数据多在 `*Page` 内。引入真实能力时**沿用分层目录**，勿继续堆进 Page 构造函数。

通信层 / 后端协议章节：**不适用**（无服务端）。若将来有后端，另开文档。

---

## 2. 技术栈（MUST）

- **C++17**，全文统一
- **Qt 5.x**（本机 5.15.2）；选定前禁止混用 Qt6-only API
- CMake ≥ 3.16；工程描述以根目录 `CMakeLists.txt` 为准（仅此一套）
- 第三方库须在 CMake 显式登记；禁止隐式依赖
- 允许标准库：`shared_ptr/unique_ptr/thread/mutex/condition_variable/atomic/function/chrono` 等
- 无自动化测试时：回归 = 静态检查 + Build + 人工走查

---

## 3. 线程与异步（MUST）

- 跨线程更新 UI：**显式** `Qt::QueuedConnection`，禁止依赖 AutoConnection 碰运气
- 子线程禁止调用任何 `QWidget` 成员、禁止直接改 UI 状态
- Worker：`QObject` + `moveToThread`，**不继承 QThread**；收尾：`finished→quit/deleteLater`
- UI 线程禁止耗时计算 / 大文件 IO / 阻塞等待（交互目标 < 100ms）
- 长任务走 worker：UI 只发请求 + 收进度/完成；须有 busy/进度，结束或失败都恢复
- 异步完成语义：函数返回 ≠ 业务完成；以任务结束通知 / IO 落盘为准

---

## 4. 工程与文件（MUST）

- 新增/删除/重命名源文件 → 同步 `CMakeLists.txt` `PROJECT_SOURCES`，必要时重新 cmake
- 含 `Q_OBJECT` 的头须能被 AUTOMOC 处理
- 类主文件：全小写连续名（现有 `mainwindow.cpp`）；按域拆分可用小写下划线
- 大类按域拆文件，**不膨胀根文件**
- include 基于 include path（工程已含 `.`、`ui/`、`model/`、`service/`、`controller/`）：工程头 `"..."` 后跟 `<Qt>`，**禁止新增 `../`**
- 参考资料/原型 HTML/调研材料不进源码目录

目标目录（已部分创建；空目录勿预建）：

```text
ui/  controller/  service/  model/  util/
```

---

## 5. 命名与风格（MUST / SHOULD）

- 类 `PascalCase`；函数 `lowerCamelCase`（工厂 `make*`）；成员 `m_`；常量 `k` 前缀或全大写下划线（新代码统一一种）
- 头保护 `#ifndef FOO_H`
- 用户文案中文；标识符英文；标识符字符串用 `QStringLiteral` / `QLatin1String`
- QObject 子控件按 Qt 父子所有权；非 QObject 所有权用智能指针；禁止无主裸 `new/delete`
- 可失败函数返回错误状态，禁止 silent failure（建议 `bool xxx(..., QString* errorMessage = nullptr)`）
- 跨层配置/状态用 POD；可空字段配 `xxxKnown`，区分“未返回”与“返回 0”
- 现有原型：中文用 `QString::fromUtf8`；样式靠 `objectName` + `Theme::styleSheet()`；连接用函数指针/lambda

---

## 6. 错误、日志、提示（MUST）

- 错误沿 数据→Service→Presenter→View 上抛，View 统一提示
- 错误信息：底层原文 + 本层动作语境（如“保存失败：…”）
- 失败必须恢复 UI（解禁按钮、收尾进度）
- 日志：当前无统一框架（**待确认**）；新增时需含时间/线程/模块/级别

---

## 7. 状态与刷新（MUST）

- **真实数据优先**：禁止用前端伪数据冒充真实结果（原型演示页可保留硬编码，但不得标成“已计算/已保存”）
- **最小刷新**：只更新受影响部分；禁止任意改动整页重建（结构性增删、标识失效除外）
- 局部更新：`Known=false` 的字段不覆盖用户未提交编辑
- 新功能按闭环分步：读 → 提交 → 刷新 → 展示

---

## 8. 可选：可视化绘制

自定义绘制控件（如翼型/极曲线）：

- 绘制调用在 UI 线程；重数据解析在计算线程
- 渲染控件不理解业务语义，只按传入数据增量绘制

---

## 9. Checklist

**写前**：哪一层？有无越界？要不要 worker？能否复用 `uihelpers`/已有用例？

**提交前**：语言/Qt 合规；CMake 已更新；QueuedConnection；无 `../` include；最小刷新；无 silent failure；文档已同步；Build 通过。

---

## 10. 分层示例流程（翼型气动分析）

对照样板：气动分析页的「翼型参数 → 计算 → 结果」闭环。

```text
用户点击「运行气动分析」/「保存分析」
  → AnalysisPage 发 runRequested(AirfoilInput) / saveRequested
  → AnalysisPresenter 槽
  → AeroAnalysisService（计算）+ AirfoilStore（读/写 airfoil_analysis.json）
  ← Presenter 调 View：setResult / setStatus / showError；失败时 setBusy(false) 恢复 UI
```

| 步骤 | 做法 |
|------|------|
| 读 | Presenter 构造时 → Store 载入上次输入；`*Known` 为真才回填控件 |
| 提交 | View 只 `snapshotInput` 发结构体意图，不拼路径、不写文件 |
| 刷新 | 只更新结果面板/表格与状态行，不重建整页 |
| 展示 | 成功显示落盘路径与结果；失败 `showError` + 状态行标错 |

新功能请复制此闭环，勿把 Service 调用写进 Page。
