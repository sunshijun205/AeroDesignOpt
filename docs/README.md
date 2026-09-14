# AeroDesignOpt 文档入口

气动设计优化平台（Qt Widgets）知识库。Agent 先读 [`../AGENTS.md`](../AGENTS.md)。

**现状**：四大功能 UI 原型为主；**翼型气动分析**已按 View→Presenter→Service→本地文件落地（见气动分析页）。其余按钮多为 `wireDummyAction`。无网络。

## 工程文档

| 文档 | 用途 |
|------|------|
| [frontend_constraints.md](frontend_constraints.md) | **前端约束（MUST）** — 分层/线程/风格/Checklist；含 MVP 分层样板流程 |
| [architecture.md](architecture.md) | 现状结构、目录、模块、数据流 |
| [ui.md](ui.md) | 主窗口、子页、内部接口、MVP 接口 |
| [build.md](build.md) | 编译、运行、排查 |

## 业务文档（按需加载）

业务能力说明在 [`business/`](business/README.md)。**改某个业务或排查其行为时**，从该目录索引定位对应 md 再读入（类职责、文件路径、关键函数）；不要与工程约束文档混为一谈。

| 业务 id | 文档 |
|---------|------|
| `airfoil_analysis` | [business/airfoil_analysis.md](business/airfoil_analysis.md) |

完整列表与加载约定见 [business/README.md](business/README.md)。

## 阅读顺序

1. `AGENTS.md`
2. `frontend_constraints.md`（工程 MUST；§10 为分层样板 = 翼型气动分析）
3. 若任务绑定具体业务 → **`business/<id>.md`**
4. 需要总览时再读 `architecture.md` / `ui.md`；构建问题读 `build.md`
