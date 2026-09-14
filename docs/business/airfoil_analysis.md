# 翼型气动分析（airfoil_analysis）

分层样板：唯一完整的 View→Presenter→Service→本地数据闭环。

## 业务概述

给定翼型与工况参数，估算单点气动系数与升阻比，支持保存/载入上次输入。**真实计算**（非 MOCK），采用工程级简化模型，可作为后续接入面元法 / CFD 的接口占位。

## 输入 / 输出（POD）

`model/airfoiltypes.h`：

- `AirfoilInput`
  - `naca`（QString，如 `"2412"`，用于提取厚度/弯度示意）
  - `alphaDeg`（迎角，°）
  - `reynolds`（雷诺数）
  - `mach`（马赫数）
  - `aspectRatio`（展弦比；用于有限翼升力线斜率修正）
  - `alphaKnown` 等 `*Known` 标志：区分“未载入”与“0”
- `AeroResult`
  - `cl` / `cd` / `cm` / `lOverD`
  - `clAlpha`（升力线斜率，1/rad）
  - `stalled`（是否达到失速夹断）
  - `valid`（计算是否成功）

## 计算模型（AeroAnalysisService）

`service/aeroanalysisservice.{h,cpp}`：`AeroResult run(const AirfoilInput&, QString* err)`

1. 零升迎角 `α0 ≈ −1.1 × 弯度%`（由 NACA 首位百分弯度估算）。
2. 二维升力线斜率 `a0 = 2π`；有限翼修正 `a = a0 / (1 + a0/(π·e·AR))`（`e≈0.9`）。
3. `Cl = a·(α − α0)`，超过 `Clmax≈1.5` 则夹断并置 `stalled=true`。
4. 阻力极曲线 `Cd = Cd0 + Cl²/(π·e·AR)`，`Cd0≈0.008 + 0.01×厚度%/12`。
5. `L/D = Cl/Cd`；`Cm ≈ −0.05 − 0.01×弯度%`（示意）。

> 模型为工程估算，**界面已标注“工程估算”**，不得冒充高保真结果。替换求解器时保持 `run()` 签名与 `AeroResult` 契约。

## 持久化（AirfoilStore）

`model/airfoilstore.{h,cpp}`：

- `bool save(const AirfoilInput&, QString* err)` / `AirfoilInput load(bool* ok)`
- 路径：`QStandardPaths::AppDataLocation` → `气动设计优化平台/airfoil_analysis.json`
- 用 `QJsonDocument`；字段缺失时对应 `*Known=false`

## 编排（AnalysisPresenter）

`controller/analysispresenter.{h,cpp}`：

- 构造时 `load()` → 若 ok 调 `view->setInput`
- `onRunRequested(input)`：`service.run` → `store.save` → `view->setResult` + `setStatus`；失败 `view->showError` 且 `setBusy(false)`
- `onSaveRequested()`：仅 `store.save(view->snapshotInput())`

## View（AnalysisPage）

`ui/analysispage.{h,cpp}`：

- 三子页：工况 `condition` / 求解设置 `solver`（原型）/ 结果 `result`
- 信号：`runRequested(AirfoilInput)`、`saveRequested()`
- 槽：`setInput(AirfoilInput)`、`setResult(AeroResult)`、`setStatus(QString,bool warn)`、`showError(QString)`、`setBusy(bool)`
- `snapshotInput()` 从控件读回 `AirfoilInput`
- 主窗转发：`requestRun()` / `requestSave()`（顶栏主次按钮）

## 落点速查

| 改什么 | 去哪 |
|--------|------|
| 计算模型 | `service/aeroanalysisservice.cpp` |
| 输入/输出字段 | `model/airfoiltypes.h`（同步 Store 读写与 View 控件） |
| 存储路径/格式 | `model/airfoilstore.cpp` |
| 表单/结果控件 | `ui/analysispage.cpp` |
| 编排/刷新时机 | `controller/analysispresenter.cpp` |
