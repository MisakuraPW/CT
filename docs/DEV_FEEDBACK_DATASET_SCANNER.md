# 阶段反馈：数据集配置读取与任务扫描

日期：2026-06-04

## 本阶段完成内容

- 新增 `AppConfig` 模块，程序可从 `configs/paths.local.ini` 读取：
  - `lung_dataset_root`
  - `covid_dataset_root`
  - `result_root`
  - `save_intermediate`
- 新增 `DatasetScanner` 模块，将不同数据集结构统一整理为病例任务：
  - finding-lungs 2D：`2d_images/*.tif` 自动匹配 `2d_masks/同名.tif`
  - finding-lungs 3D：`3d_images/IMG_*.nii.gz` 自动匹配 `3d_images/MASK_*.nii.gz`
  - COVID-19 CT：`ct_scans/*.nii` 自动匹配 `lung_mask/*.nii` 与 `infection_mask/*.nii`
- 新增菜单入口：
  - 主窗口：`数据集 -> 扫描数据集任务...`
  - 文档窗口：`处理 -> 扫描数据集任务...`
- 扫描结果会导出到 `results/dataset_tasks.csv`，采用 UTF-8 BOM，方便用 Excel 直接查看。

## 本机验证结果

使用当前 `configs/paths.local.ini` 中配置的数据集路径，独立扫描得到：

| 数据集 | 数量 | 标注缺失 |
| --- | ---: | ---: |
| finding-lungs 2D | 267 | 0 |
| finding-lungs 3D | 4 | 0 |
| COVID-19 CT | 20 | 0 |

项目编译验证：

```text
MSBuild Debug|x64: 0 warnings, 0 errors
```

## 当前能力边界

- 本阶段先完成“数据集识别与任务清单生成”，还没有把整个数据集的一键处理流水线接到这些任务上。
- COVID 数据集的命名规则已覆盖当前本机数据中的两类文件名：
  - `coronacases_org_*.nii`
  - `radiopaedia_org_covid-19-pneumonia-*-dcm.nii`
- 后续阶段可以直接基于 `DatasetCase` 做整批读取、分割、指标计算、感染负荷统计和总表导出。

## 下一阶段建议

- 新增 `DatasetBatchRunner`，读取 `DatasetScanner` 产出的病例任务并逐病例调用现有 `BatchProcessor`。
- 为整批任务增加输出目录结构：
  - `results/datasets/finding-lungs-2d/...`
  - `results/datasets/finding-lungs-3d/...`
  - `results/datasets/covid-19-ct/...`
- 输出一个跨数据集总表 `dataset_summary.csv`，用于最终报告和可视化统计。
