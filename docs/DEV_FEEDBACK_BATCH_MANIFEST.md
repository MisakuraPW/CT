# 阶段反馈：批处理运行参数 Manifest

日期：2026-06-04

## 本阶段完成内容

- `BatchProcessor` 新增运行参数记录文件：
  - `csv/run_manifest.txt`
- 每个病例批处理开始时会写入实际使用的参数，便于复现实验结果。
- `BatchProcessResult` 新增 `manifestPath`。
- `DatasetBatchRunner` 的 `dataset_summary.csv` 新增 `run_manifest` 列，可从数据集总表追溯到每个病例的参数文件。
- “批量处理当前数据...”完成弹窗会同时显示：
  - `batch_summary.csv`
  - `run_manifest.txt`

## Manifest 内容

当前记录字段包括：

```text
case_name
source_slice_count
gt_mask_slice_count
infection_mask_slice_count
save_intermediate

[segmentation]
threshold_gaussian_kernel_size
min_component_area
min_component_area_divisor
keep_component_count
open_kernel_size
close_kernel_size
morphology_iterations
```

## 输出位置

单病例或单体数据批处理：

```text
<selected_output_dir>/csv/run_manifest.txt
```

配置数据集一键批处理：

```text
results/datasets/<dataset_kind>/<case_id>/csv/run_manifest.txt
```

## 验证结果

```text
MSBuild Debug|x64: 0 warnings, 0 errors
```

## 当前能力边界

- Manifest 当前记录分割参数；预处理菜单的独立展示参数由配置文件本身体现，尚未单独写入 manifest。
- Manifest 是文本文件，方便人工检查；后续如需机器汇总，可改为 CSV 或 JSON。

## 下一阶段建议

- 数据集级批处理完成后，额外输出一个总的参数快照，记录整批任务使用的配置文件来源。
- 增加图形化参数设置对话框，保存后同步更新本地配置。
