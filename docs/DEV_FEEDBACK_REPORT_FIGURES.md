# 阶段反馈：报告统计图生成工具

日期：2026-06-04

## 本阶段完成内容

- 新增 `tools/generate_report_figures.py`。
- 该工具读取应用程序批处理导出的 CSV，并生成报告/PPT 可直接引用的 SVG 图。
- 仅使用 Python 标准库，不依赖 `matplotlib`、`pandas` 或任何深度学习框架。

## 默认输入输出

默认输入：

```text
results/datasets/dataset_summary.csv
```

默认输出：

```text
results/figures/
```

可生成的图包括：

- `dataset_case_status.svg`：各数据集病例成功/失败数量。
- `mean_dice_by_dataset.svg`：按数据集统计的平均 Dice。
- `mean_infection_ratio_by_dataset.svg`：按数据集统计的平均感染负荷。
- `covid_infection_ratio_curves.svg`：最多 5 个 COVID 病例的逐切片感染负荷曲线。

## 使用方式

在项目根目录运行：

```powershell
python tools\generate_report_figures.py
```

如果 CSV 或输出目录不使用默认位置：

```powershell
python tools\generate_report_figures.py `
  --dataset-summary results\datasets\dataset_summary.csv `
  --output-dir results\figures
```

## 验证结果

已完成以下验证：

```text
python tools\generate_report_figures.py --help
python -B -m py_compile tools\generate_report_figures.py
```

并使用临时目录中的最小 CSV 样例实际生成了 4 张 SVG：

- `dataset_case_status.svg`
- `mean_dice_by_dataset.svg`
- `mean_infection_ratio_by_dataset.svg`
- `covid_infection_ratio_curves.svg`

## 当前能力边界

- SVG 图是静态图，不提供交互式缩放。
- 如果某类数据缺少对应字段，例如没有感染统计，就不会生成对应图。
- 图表样式以报告可读性为主，后续可以按课程报告模板继续调整配色和尺寸。

## 下一阶段建议

- 增加后台批处理进度条与取消功能。
- 增加一键导出“报告素材包”的菜单，把关键 PNG/SVG 统一复制到 `report/figures/`。
