# Batch Processing Feedback

## 本阶段目标

在单张切片和 3D NIfTI 浏览能力之上，补齐批量处理框架，使程序能够对当前打开的数据逐切片运行处理流程，并输出可用于报告/PPT/后续统计的文件。

## 新增能力

### 1. 批量处理当前数据

新增菜单：

```text
处理 -> 批量处理当前数据
```

适用输入：

- 当前打开的单张 2D 图像。
- 当前打开的 3D NIfTI CT 体数据。
- 可选：当前打开的人工 mask。
- 可选：当前打开的感染 mask。

处理内容：

1. 对每张 CT 切片执行肺部分割。
2. 保存最终肺部 mask。
3. 保存中间结果。
4. 若人工 mask 存在，则逐切片计算 Dice / IoU / Precision / Recall / Area Error。
5. 若感染 mask 存在，则逐切片计算感染负荷。
6. 若感染 mask 存在，则保存感染区域叠加图。
7. 导出汇总 CSV。

### 2. 输出目录结构

用户选择输出目录后，程序会创建：

```text
输出目录/
  masks/
  intermediate/
  overlays/
  csv/
```

主要输出文件：

```text
masks/slice_0000_final_mask.png
intermediate/slice_0000_threshold.png
intermediate/slice_0000_connected.png
intermediate/slice_0000_morphology.png
overlays/slice_0000_infection_overlay.png
csv/batch_summary.csv
```

### 3. 汇总 CSV

`csv/batch_summary.csv` 字段：

```text
slice
has_metrics
dice
iou
precision
recall
area_error
tp
fp
tn
fn
has_infection
lung_area
infection_area
infection_ratio
left_lung_area
left_infection_area
left_ratio
right_lung_area
right_infection_area
right_ratio
```

## 新增文件

- `src/大作业/BatchProcessor.h`
- `src/大作业/BatchProcessor.cpp`

## 修改文件

- `src/大作业/大作业Doc.h`
- `src/大作业/大作业Doc.cpp`
- `src/大作业/MainFrm.cpp`
- `src/大作业/My.rc`
- `src/大作业/Resource.h`
- `src/大作业/大作业.vcxproj`
- `src/大作业/大作业.vcxproj.filters`

## 关键类

### `CBatchProcessor`

核心函数：

```cpp
bool ProcessSlices(
    const std::vector<cv::Mat>& sourceSlices,
    const std::vector<cv::Mat>& gtMaskSlices,
    const std::vector<cv::Mat>& infectionMaskSlices,
    const BatchOptions& options,
    BatchProcessResult& result,
    CString& errorMessage) const;
```

复用模块：

- `CLungSegmenter`
- `CMetricsCalculator`
- `CInfectionAnalyzer`
- `ImageIO`

## 验证结果

已执行：

```text
MSBuild src/大作业/大作业.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

结果：

```text
已成功生成。
0 个警告
0 个错误
```

## 当前限制

1. 批量处理目前针对“当前已打开数据”，还没有做完整数据集目录自动匹配。
2. 3D mask 如果切片数与 CT 不一致，只会处理索引范围内能对应上的切片。
3. 感染 mask 仍来自标注，不做自动感染分割。
4. 面积统计仍是像素面积，不是基于 spacing 的物理面积。

## 下一阶段建议

下一阶段可以做数据集级自动匹配：

1. 从 `configs/paths.local.ini` 读取数据集根目录。
2. 对 COVID 数据集自动识别 CT、lung mask、infection mask。
3. 对 Finding Lungs 数据集自动识别 2D/3D 数据。
4. 一键跑完整数据集并导出总汇总 CSV。
