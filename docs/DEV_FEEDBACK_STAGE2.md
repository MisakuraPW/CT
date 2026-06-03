# Stage 2 Feedback: Infection Burden and CSV Export

## 本阶段目标

在第一版 MVP 的基础上，补全下一阶段的基础分析能力：

1. 打开 COVID 感染区域 mask。
2. 基于最终肺部 mask 计算感染负荷。
3. 粗略拆分图像坐标意义上的左右肺。
4. 生成感染区域叠加图。
5. 导出 Dice / IoU 指标 CSV。
6. 导出感染负荷统计 CSV。

## 修改文件

新增文件：

- `src/大作业/InfectionAnalyzer.h`
- `src/大作业/InfectionAnalyzer.cpp`
- `src/大作业/CsvExporter.h`
- `src/大作业/CsvExporter.cpp`

修改文件：

- `src/大作业/大作业Doc.h`
- `src/大作业/大作业Doc.cpp`
- `src/大作业/MainFrm.cpp`
- `src/大作业/My.rc`
- `src/大作业/Resource.h`
- `src/大作业/大作业.vcxproj`
- `src/大作业/大作业.vcxproj.filters`

## 新增功能

### 1. 感染 mask 读取

新增菜单：

```text
文件 -> 打开感染 mask
```

读取后会二值化感染 mask，并可通过视图菜单显示。

### 2. 感染负荷分析

新增菜单：

```text
分析 -> 感染负荷分析
```

前置条件：

- 已打开 CT 图像。
- 已执行一键肺部分割，得到最终肺部 mask。
- 已打开感染 mask。

输出指标：

- `lung_area`
- `infection_area`
- `infection_ratio`
- `left_lung_area`
- `left_infection_area`
- `left_ratio`
- `right_lung_area`
- `right_infection_area`
- `right_ratio`

说明：当前左右肺拆分使用图像坐标意义上的左右，不等同于医学解剖左右。

### 3. 感染叠加图

新增视图：

```text
视图 -> 显示感染叠加图
```

显示规则：

- 肺部轮廓：绿色
- 感染区域：红色半透明叠加

### 4. CSV 导出

新增菜单：

```text
文件 -> 导出指标 CSV
文件 -> 导出感染 CSV
```

指标 CSV 在执行 `分析 -> 计算 Dice / IoU` 后可用。

感染 CSV 在执行 `分析 -> 感染负荷分析` 后可用。

CSV 使用 UTF-8 BOM，方便 Excel 直接打开。

## 关键类和函数

### `CInfectionAnalyzer`

主要函数：

- `Analyze(const cv::Mat& lungMask, const cv::Mat& infectionMask)`
- `SplitLeftRightLung(const cv::Mat& lungMask, cv::Mat& leftMask, cv::Mat& rightMask)`
- `MakeInfectionOverlay(const cv::Mat& source, const cv::Mat& lungMask, const cv::Mat& infectionMask)`

当前左右肺拆分策略：

1. 对肺部 mask 做连通域分析。
2. 若至少有两个主要连通域，按质心 x 坐标区分图像左侧肺和图像右侧肺。
3. 若只有一个连通域，则按图像中线粗略拆分。

### `CsvExporter`

主要函数：

- `ExportMetrics(...)`
- `ExportInfectionStats(...)`

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

1. 感染负荷分析目前面向单张切片，不是批量序列。
2. 左右肺拆分是图像坐标左右，报告中需要说明。
3. 感染区域不做自动分割，只读取数据集自带 infection mask。
4. CSV 导出一次只导出当前图像的结果。

## 下一阶段建议

下一阶段可以进入批量处理：

1. 打开一个 CT 图像文件夹。
2. 自动匹配肺部 mask / 感染 mask。
3. 对每张切片批量运行分割、指标计算和感染负荷统计。
4. 输出汇总 CSV。
5. 保存每张切片的中间结果和叠加图。
