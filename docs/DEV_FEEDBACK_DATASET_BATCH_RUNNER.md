# 阶段反馈：配置数据集一键批处理

日期：2026-06-04

## 本阶段完成内容

- 新增 `DatasetBatchRunner`，将 `DatasetScanner` 扫描出的病例任务接入现有逐切片处理流程。
- 支持统一处理：
  - finding-lungs 2D：`.tif`
  - finding-lungs 3D：`.nii.gz`
  - COVID-19 CT：`.nii`
- 新增菜单入口：
  - 主窗口：`数据集 -> 一键处理配置数据集...`
  - 文档窗口：`处理 -> 一键处理配置数据集...`
- 点击菜单后会先扫描 `configs/paths.local.ini` 中配置的数据集，并弹窗确认病例数量。
- 批处理完成后会生成跨病例汇总表：
  - `results/datasets/dataset_summary.csv`

## 输出结构

整批任务会按照数据集和病例编号组织输出：

```text
results/
  datasets/
    dataset_summary.csv
    finding-lungs-2d/
      ID_0000_Z_0142/
        masks/
        intermediate/
        overlays/
        csv/batch_summary.csv
    finding-lungs-3d/
      IMG_0002/
        masks/
        intermediate/
        overlays/
        csv/batch_summary.csv
    covid-19-ct/
      coronacases_001/
        masks/
        intermediate/
        overlays/
        csv/batch_summary.csv
```

其中：

- `masks/` 保存每张切片的最终肺部分割 mask。
- `intermediate/` 保存阈值、连通域、形态学等中间结果，是否保存受 `save_intermediate` 控制。
- `overlays/` 保存感染区域叠加图，只有存在感染标注时生成。
- `csv/batch_summary.csv` 是单个病例逐切片指标。
- `dataset_summary.csv` 是全部病例的总表，包含成功/失败状态、切片数、指标切片数、感染统计切片数和错误信息。

## 验证结果

项目编译验证：

```text
MSBuild Debug|x64: 0 warnings, 0 errors
```

## 当前能力边界

- 批处理目前在主线程中运行，处理完整数据集时窗口会暂时无响应；菜单已在运行前提示这一点。
- 本阶段没有自动启动完整数据集跑批，避免在开发环境里无意生成大量结果文件。
- 3D 数据处理沿用当前 NIfTI 读取能力：支持 `.nii` 和依赖 Python gzip 解压的 `.nii.gz`。

## 下一阶段建议

- 增加后台线程、进度条与取消按钮，让完整数据集跑批体验更稳。
- 增加 `dataset_summary.csv` 的统计汇总视图，例如平均 Dice、平均感染负荷、失败病例列表。
- 在报告阶段可以直接引用本阶段输出的 CSV 和结果目录结构。
