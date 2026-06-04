# 阶段反馈：报告素材包收集工具

日期：2026-06-04

## 本阶段完成内容

- 新增 `tools/collect_report_assets.py`。
- 可从批处理输出中自动收集报告/PPT 常用素材：
  - `results/figures/*.svg`
  - 每个病例 `overlays/` 下的连通域伪彩色图
  - 预测/人工 mask 对比图
  - 感染叠加图
- 默认输出到：

```text
report/figures/
```

- 自动生成：

```text
report/figures/ASSET_MANIFEST.md
```

用于记录本次收集了哪些素材。

## 使用方式

推荐流程：

1. 在程序中运行数据集批处理，生成 `results/datasets/dataset_summary.csv`。
2. 运行统计图生成：

```powershell
python tools\generate_report_figures.py
```

3. 收集报告素材：

```powershell
python tools\collect_report_assets.py
```

可选参数：

```powershell
python tools\collect_report_assets.py `
  --dataset-summary results\datasets\dataset_summary.csv `
  --figures-dir results\figures `
  --output-dir report\figures `
  --max-cases-per-dataset 2 `
  --max-images-per-case 6
```

## 验证结果

已完成：

```text
python tools\collect_report_assets.py --help
python -B -m py_compile tools\collect_report_assets.py
```

并使用临时目录模拟 `dataset_summary.csv`、SVG 图和 overlay PNG，实际验证复制结果与 manifest 生成成功。

## Git 管理

- `report/figures/` 已加入 `.gitignore`，避免自动收集的大量结果图误提交。
- 如果最终需要提交精选报告图片，建议人工挑选后放入单独目录，例如 `docs/figures/`。

## 下一阶段建议

- 增加后台批处理进度条与取消功能。
- 最后统一更新 README，把完整运行流程、批处理流程和报告素材流程写清楚。
