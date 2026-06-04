# 阶段反馈：处理参数配置化

日期：2026-06-04

## 本阶段完成内容

- 新增 `ProcessingOptions.h`，集中定义：
  - `PreprocessingOptions`
  - `LungSegmentationOptions`
- 扩展 `AppConfig`，从 `configs/paths.local.ini` 读取预处理与分割参数。
- 更新 `configs/paths.example.ini`，新增 `[preprocess]` 和 `[segmentation]` 配置示例。
- `Preprocessor` 菜单功能开始使用配置参数：
  - 肺窗窗宽窗位
  - Gaussian kernel
  - Median kernel
  - CLAHE clip limit / tile grid size
- `CLungSegmenter` 开始使用配置参数：
  - Otsu 前高斯核大小
  - 连通域最小面积阈值
  - 保留主连通域数量
  - 开闭运算核大小
  - 形态学迭代次数
- 单图处理、当前体数据批处理、配置数据集一键批处理均已贯通同一套分割参数。

## 新增配置项

```ini
[preprocess]
lung_window_level = -600
lung_window_width = 1500
gaussian_kernel_size = 5
median_kernel_size = 5
clahe_clip_limit = 2.0
clahe_tile_grid_size = 8

[segmentation]
threshold_gaussian_kernel_size = 5
min_component_area = 64
min_component_area_divisor = 1000
keep_component_count = 2
open_kernel_size = 3
close_kernel_size = 7
morphology_iterations = 1
```

旧版 `configs/paths.local.ini` 不需要立刻修改；如果缺少这些字段，程序会自动使用默认参数。

## 验证结果

```text
MSBuild Debug|x64: 0 warnings, 0 errors
```

## 当前能力边界

- 参数目前通过 ini 文件配置，还没有做图形化参数对话框。
- 配置读取支持默认值回退；如果字段写错成非数字，会使用默认值。
- kernel size 会在算法内部自动修正为不小于 3 的奇数。

## 下一阶段建议

- 增加一个参数设置对话框，允许运行时调整并保存到本地配置。
- 增加处理日志文件，记录每次批处理实际使用的参数，增强实验复现性。
