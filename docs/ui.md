# UI 与内部接口

- Widgets + **代码布局**（`QLayout`）+ `objectName` + `Theme` QSS。强调色 `#2f6fed`。
- 无 `.ui` 文件。无 HTTP/对外 SDK。无独立业务 Dialog（仅 `QMessageBox`）。
- View 层约束见 [frontend_constraints.md](frontend_constraints.md)。

## MainWindow

顶栏（品牌/项目/主次按钮）→ 四大 `ModeButton` → `QStackedWidget`（四 Page）。
`switchMode(int)` 切页并 `updateActions(int)` 改按钮文案（均为 private）。
气动分析页：主按钮「运行气动分析」、次按钮「保存分析」转发到 `AnalysisPage::requestRun` / `requestSave`。其余模式仍为信息框示意。
成员：`m_pages`、`m_analysisPage`、`m_primary`、`m_secondary`、`m_modeButtons`。

## 子页

多数 Page：`makeHeading` + `SubTabBar`（`currentChanged`）+ 内层 `QStackedWidget` + `wrapScroll`。
除气动分析外，各 Page 构造时组装静态 UI；无对外业务 signal。

| Page | 子页 id |
|------|---------|
| Airfoil | naca / geometry / preview |
| Analysis | condition / solver / result |
| Optimization | objective / variables / algorithm |
| Results | polar / convergence / compare |

## 气动分析（分层样板）

| 层 | 类型 | 要点 |
|----|------|------|
| View | `AnalysisPage` | 工况表单可编辑；信号 `runRequested`/`saveRequested`；`setResult` / `setStatus`；`snapshotInput` |
| Presenter | `AnalysisPresenter` | 连接信号；调用 Service 计算与 Store 落盘；刷新结果 |
| Service | `AeroAnalysisService` | 薄翼理论升力 + 阻力极曲线 + L/D + 力矩 |
| 数据 | `AirfoilInput` / `AeroResult` + `AirfoilStore` | `%AppData%/AeroDesignOpt/气动设计优化平台/airfoil_analysis.json` |

组装：`AnalysisPage` 持有 Store/Service（`unique_ptr`）与 Presenter（`QObject` 子对象），**View 不直接调 Service**。

业务细节见 [business/airfoil_analysis.md](business/airfoil_analysis.md)。

## uihelpers

`makeButton` / `makeInput` / `makeSelect` / `makeField` / `makeSelectField` / `makePanel` / `makePanelTitle` / `makeKpis` / `makeTable` / `makeHeading` / `wrapScroll` / `makeChip` / `makeStatusText` / `wireDummyAction`

结构体：`KpiItem`、`TableOptions`。
`SubTabBar`：`currentChanged(QString id)`。

连接以函数指针 + lambda 为主。

## Theme

`Theme::styleSheet()` 与颜色辅助。强调色 `#2f6fed`（区别于 AMDO 的青绿色）。
