# 医学图像处理大作业

## 项目名称

基于传统图像处理的胸部 CT 肺实质分割与 COVID-19 感染负荷定量分析

## 项目简介

本项目是医学图像处理课程大作业，目标是在不使用神经网络的前提下，使用传统图像处理方法完成胸部 CT 图像中的肺实质分割，并进一步结合 COVID-19 CT 数据集中的肺部标注和感染区域标注，完成感染负荷的定量分析。

项目采用 C++、Microsoft Visual Studio、MFC 和 OpenCV 开发。应用程序类型为 MFC MDI 多文档窗口程序。多窗口结构用于展示不同处理阶段的图像结果，例如原始 CT、预处理结果、阈值分割结果、连通域筛选结果、形态学修复结果、最终肺部 mask、人工 mask、感染区域叠加图等。

本项目强调传统医学图像处理流程的完整性和可解释性，而不是追求深度学习模型的最高精度。

## 课程约束

本项目遵守以下限制：

* 不使用神经网络。
* 不使用 CNN、U-Net、Transformer 等深度学习模型。
* 不使用 PyTorch、TensorFlow、Keras 等深度学习框架。
* 主要使用 C++、MFC 和 OpenCV。
* 以传统图像处理算法为主，包括阈值分割、连通域分析、形态学处理、孔洞填充、区域统计和指标评价。

## 技术栈

本项目主要使用：

* C++
* Microsoft Visual Studio
* MFC MDI
* OpenCV
* Git

可选辅助工具：

* Python / Excel：用于后期绘制统计曲线或整理 CSV。
* Kagglehub：用于下载 Kaggle 数据集。

第一版不强制支持 DICOM / NIfTI 等医学影像格式。优先处理 Kaggle 数据集中已经整理好的 PNG、JPG、BMP、TIFF 等普通图像格式。

## 项目目标

本项目的核心目标包括：

1. 实现胸部 CT 图像读取与显示。
2. 实现灰度归一化、肺窗显示、滤波等预处理功能。
3. 基于传统图像处理方法实现肺实质分割。
4. 展示肺部分割过程中的中间结果。
5. 读取人工肺部 mask，并计算 Dice、IoU、Precision、Recall、面积误差等评价指标。
6. 在 COVID-19 CT 数据集中，结合肺部 mask 与感染 mask 计算感染负荷。
7. 支持结果图像和统计 CSV 的导出。
8. 使用 MFC 多窗口界面直观展示完整处理流程。

## 主要功能

### 1. 图像加载与显示

支持加载常见图像格式：

* .png
* .jpg
* .jpeg
* .bmp
* .tif
* .tiff

程序应支持：

* 打开单张 CT 图像。
* 打开对应人工 mask。
* 显示灰度图和彩色图。
* 在 MFC MDI 子窗口中显示不同处理阶段结果。
* 保存当前显示结果。

### 2. 图像预处理

预处理模块包括：

* 灰度化。
* 灰度归一化。
* 肺窗显示。
* 高斯滤波。
* 中值滤波。
* 可选 CLAHE 对比度增强。

第一版若只处理 8-bit PNG / JPG 图像，则优先实现普通灰度归一化和滤波。若后续支持 HU 数据，可加入窗宽窗位映射。

肺窗参数建议：

* WL = -600
* WW = 1500
* lower = WL - WW / 2
* upper = WL + WW / 2

### 3. 肺实质分割

本项目采用传统图像处理流程完成肺实质分割：

原始 CT 图像
→ 灰度归一化 / 肺窗处理
→ 高斯滤波
→ 阈值分割
→ 连通域筛选
→ 形态学修复
→ 孔洞填充
→ 最终肺部 mask

其中：

* 阈值分割用于得到初始肺部候选区域。
* 连通域筛选用于去除外部空气、边界背景和小噪声区域。
* 形态学修复用于修复肺部 mask 的边界缺口和内部空洞。
* 孔洞填充用于补全肺内血管、支气管或局部灰度变化造成的空洞。
* 最终 mask 统一规定为：肺部区域 = 255，背景区域 = 0。

### 4. 连通域筛选

连通域筛选是本项目的重要处理步骤。

初始阈值分割结果中可能包含：

* 肺部区域。
* 体外空气。
* 图像边界背景。
* 扫描床或其他低灰度区域。
* 小噪声区域。
* 气管或其他非肺结构。

因此需要使用连通域分析进行筛选。

推荐筛选规则：

* 去除面积过小的连通域。
* 去除大面积接触图像边界的连通域。
* 保留胸腔内部的主要连通域。
* 优先保留面积最大的两个肺部候选区域。
* 根据质心位置辅助区分左右肺。

OpenCV 相关函数：

* cv::connectedComponentsWithStats()

### 5. 形态学修复

形态学处理用于将粗分割结果修复成更完整的肺部 mask。

推荐流程：

* 开运算：去除小噪声和毛刺。
* 闭运算：连接断裂区域，补齐边界缺口。
* 孔洞填充：填补肺部内部空洞。
* 边界平滑：改善 mask 轮廓质量。

推荐初始参数：

* Opening kernel: ellipse 3x3
* Closing kernel: ellipse 5x5
* Iterations: 1

OpenCV 相关函数：

* cv::morphologyEx()
* cv::erode()
* cv::dilate()
* cv::getStructuringElement()
* cv::floodFill()

### 6. 分割评价

程序需要支持读取人工肺部 mask，并将算法输出的预测 mask 与人工 mask 进行比较。

评价指标包括：

* TP
* FP
* TN
* FN
* Dice
* IoU
* Precision
* Recall
* Area Error

公式：

Dice = 2TP / (2TP + FP + FN)

IoU = TP / (TP + FP + FN)

Precision = TP / (TP + FP)

Recall = TP / (TP + FN)

AreaError = abs(predArea - gtArea) / gtArea

计算前需要保证：

* 预测 mask 和人工 mask 尺寸一致。
* 预测 mask 和人工 mask 类型一致。
* 前景定义一致。
* 前景 = 255。
* 背景 = 0。

### 7. COVID-19 感染负荷分析

在 COVID-19 CT 数据集中，使用肺部 mask 和感染 mask 进行感染负荷分析。

第一版不强制自动分割感染区域，而是优先使用数据集自带感染 mask 完成定量分析。

需要计算：

* 每层肺部面积。
* 每层感染面积。
* 感染面积 / 肺部面积。
* 左肺感染面积。
* 右肺感染面积。
* 左肺感染比例。
* 右肺感染比例。
* 整体感染负荷。

多切片统计结果建议导出为 CSV，字段包括：

* slice
* lung_area
* infection_area
* infection_ratio
* left_lung_area
* left_infection_area
* left_ratio
* right_lung_area
* right_infection_area
* right_ratio

### 8. 可视化输出

项目需要保存和展示以下图像结果：

* 原始 CT 图像。
* 灰度归一化图像。
* 阈值分割结果。
* 连通域筛选结果。
* 形态学修复结果。
* 最终肺部 mask。
* 人工肺部 mask。
* 预测 mask 与人工 mask 叠加图。
* 感染区域叠加图。
* 连通域伪彩色图。

推荐可视化颜色：

* 肺部 mask：绿色。
* 感染 mask：红色。
* 人工 mask：蓝色。
* 预测 mask：绿色。
* 重叠区域：黄色或白色。

注意 OpenCV 默认颜色顺序是 BGR，不是 RGB。

## 使用数据集

### 1. Finding and Measuring Lungs in CT Data

用途：肺实质分割主数据集。

本项目在该数据集上完成：

* CT 图像读取。
* 肺部区域分割。
* 人工肺部 mask 读取。
* 预测 mask 与人工 mask 对比。
* Dice / IoU / Precision / Recall / 面积误差计算。

配置项：

* lung_dataset_root

### 2. COVID-19 CT scans

用途：COVID-19 感染负荷定量分析。

本项目在该数据集上完成：

* CT 图像读取。
* 肺部 mask 读取。
* 感染 mask 读取。
* 每层肺面积统计。
* 每层感染面积统计。
* 感染面积 / 肺面积计算。
* 左右肺感染比例计算。
* 感染区域叠加可视化。
* CSV 结果导出。

配置项：

* covid_dataset_root

## 数据管理原则

完整 Kaggle 数据集不提交到 Git 仓库。

原因：

1. 医学图像数据体积较大，不适合 Git 管理。
2. 不同成员本机数据路径不同。
3. 避免仓库臃肿、同步困难和 push 失败。
4. 代码应该通过配置文件读取路径，而不是硬编码绝对路径。

本项目使用本地配置文件指定数据路径：

configs/paths.local.ini

该文件只在本机存在，不提交 Git。

仓库中只提交路径模板文件：

configs/paths.example.ini

队友拿到项目后，需要复制 paths.example.ini，并重命名为 paths.local.ini，然后填写自己电脑上的真实数据集路径。

## 推荐项目目录结构

项目根目录结构建议如下：

* src/
  Visual Studio / MFC 项目代码。

* configs/
  路径配置文件目录。

* configs/paths.example.ini
  路径配置模板，需要提交 Git。

* configs/paths.local.ini
  本机私有路径配置，不提交 Git。

* docs/
  项目说明文档。

* docs/DATASET_LAYOUT.md
  数据集说明文档。

* docs/PROJECT_SETUP.md
  项目配置、协作和 Git 使用说明。

* sample_data/
  少量样例数据，可选，可提交 Git。

* results/
  程序输出结果，不提交 Git。

* README.md
  项目总说明。

* .gitignore
  Git 忽略规则。

## 路径配置说明

### 1. 配置模板

仓库中保留：

configs/paths.example.ini

该文件作为路径配置模板。

### 2. 本地配置

每个成员需要在本机创建：

configs/paths.local.ini

创建方法：

复制 configs/paths.example.ini，然后重命名为 configs/paths.local.ini。

### 3. 配置内容示例

paths.local.ini 中至少包含：

[dataset]

lung_dataset_root = G:/Kaggle/kagglehub/datasets/kmader/finding-lungs-in-ct-data/versions/1

covid_dataset_root = G:/Kaggle/kagglehub/datasets/andrewmvd/covid19-ct-scans/versions/4

[output]

result_root = ../results

[app]

save_intermediate = 1

image_extensions = .png,.jpg,.jpeg,.bmp,.tif,.tiff

mask_foreground_value = 255

注意：上面的路径只是示例。每个成员应根据自己电脑上的真实路径修改。

## Git 管理规则

### 应该提交的内容

* 源代码。
* Visual Studio 工程文件。
* README.md。
* docs/ 文档。
* configs/paths.example.ini。
* .gitignore。
* 少量 sample_data。
* 报告和 PPT。
* 精选的结果展示图。

### 不应该提交的内容

* 完整 Kaggle 数据集。
* configs/paths.local.ini。
* results/ 输出目录。
* Visual Studio 编译产物。
* Visual Studio 用户缓存。
* 大型医学影像文件。
* 压缩包数据集。

尤其不要提交：

* .vs/
* x64/
* Debug/
* Release/
* configs/paths.local.ini
* data/
* dataset/
* datasets/
* results/
* outputs/
* *.dcm
* *.nii
* *.nii.gz
* *.zip

## 队友第一次使用项目的流程

### 第一步：克隆仓库

使用 Git 克隆项目到本机。

### 第二步：创建本地路径配置

复制：

configs/paths.example.ini

重命名为：

configs/paths.local.ini

### 第三步：修改本机数据路径

打开 configs/paths.local.ini，修改：

* lung_dataset_root
* covid_dataset_root
* result_root

路径必须改成自己电脑上的真实路径。

### 第四步：打开 Visual Studio 工程

进入 src/ 目录，打开 .sln 文件。

### 第五步：检查 OpenCV 配置

确认 Visual Studio 项目已经配置好：

* OpenCV include 目录。
* OpenCV lib 目录。
* OpenCV .lib 依赖项。
* OpenCV .dll 运行路径。

### 第六步：编译运行

推荐使用：

* x64 / Debug

或：

* x64 / Release

## Codex / 代码 Agent 使用规则

交给 Codex、Claude Code 或其他代码 Agent 前，请先让它阅读：

* README.md
* docs/DATASET_LAYOUT.md
* docs/PROJECT_SETUP.md
* configs/paths.example.ini

代码 Agent 必须遵守以下规则：

1. 不要硬编码本机绝对路径。
2. 不要扫描整个磁盘寻找数据。
3. 所有数据路径必须从 configs/paths.local.ini 读取。
4. configs/paths.local.ini 不提交 Git。
5. 完整 Kaggle 数据集不提交 Git。
6. 输出结果默认写入 results/。
7. results/ 不提交 Git。
8. 不使用神经网络。
9. 不引入 PyTorch、TensorFlow 等深度学习库。
10. MFC View 类只负责显示和交互。
11. 图像处理算法应拆成独立类。
12. 每次修改代码应小步提交、小步测试，不要一次性大规模重构。

## 推荐代码模块

后续代码建议尽量拆分为独立模块，不要把所有算法逻辑写进 MFC View 类。

推荐模块包括：

* CImageLoader
  负责图像读取。

* CPreprocessor
  负责灰度归一化、肺窗显示、滤波等预处理。

* CLungSegmenter
  负责肺部分割主流程。

* CConnectedComponentFilter
  负责连通域筛选。

* CMorphologyProcessor
  负责开闭运算、孔洞填充和边界修复。

* CMetricsCalculator
  负责 Dice、IoU、Precision、Recall 等指标计算。

* CInfectionAnalyzer
  负责感染负荷定量分析。

* COverlayVisualizer
  负责 mask 叠加、轮廓显示和伪彩色显示。

* CResultExporter
  负责结果图像和 CSV 导出。

## MFC 多窗口设计原则

本项目使用 MFC MDI 多文档结构。多窗口主要用于展示不同处理阶段结果。

建议展示窗口包括：

* 原始 CT 图像窗口。
* 灰度归一化窗口。
* 阈值分割结果窗口。
* 连通域筛选结果窗口。
* 形态学修复结果窗口。
* 最终肺部 mask 窗口。
* 人工 mask 窗口。
* 预测 mask 与人工 mask 叠加窗口。
* 感染区域叠加窗口。
* 感染负荷统计窗口。

开发原则：

* View 负责显示。
* Document 负责当前图像状态。
* 图像处理算法放在独立类中。
* 不要把算法逻辑全部写进 View 类。
* 每一步中间结果都应可以保存，方便报告和 PPT 展示。

## 最小可行版本 MVP

第一阶段优先完成 MVP，不要一开始追求所有功能。

MVP 必须包含：

1. 打开单张 CT 图像。
2. 显示原始图像。
3. 灰度化 / 归一化。
4. Otsu 阈值分割。
5. 连通域筛选。
6. 形态学修复。
7. 显示最终肺部 mask。
8. 打开人工 mask。
9. 计算 Dice 和 IoU。
10. 保存结果图。

MVP 完成后再扩展：

1. 批量处理。
2. 感染负荷分析。
3. 左右肺分离。
4. CSV 导出。
5. 曲线图绘制。
6. 多窗口展示优化。
7. 自动感染候选区域检测。

## 推荐开发顺序

建议按以下顺序开发：

1. 确认 MFC MDI 工程可以正常编译运行。
2. 配置 OpenCV。
3. 实现 cv::Mat 到 MFC View 的显示。
4. 实现单张图像读取。
5. 实现灰度化和归一化。
6. 实现 Otsu 阈值分割。
7. 实现连通域筛选。
8. 实现形态学修复。
9. 实现孔洞填充。
10. 实现最终肺 mask 显示和保存。
11. 实现人工 mask 读取。
12. 实现 Dice 和 IoU 计算。
13. 实现 Precision、Recall 和面积误差计算。
14. 实现 CSV 导出。
15. 实现 COVID 感染负荷分析。
16. 实现多窗口可视化优化。
17. 整理报告和 PPT 需要的结果图。

## 结果输出规则

所有输出结果默认写入：

results/

推荐输出结构：

* results/intermediate/
  保存中间处理结果。

* results/overlays/
  保存叠加可视化图。

* results/masks/
  保存最终预测 mask。

* results/csv/
  保存评价指标和感染负荷统计表。

建议输出文件包括：

* 01_original.png
* 02_preprocess.png
* 03_threshold.png
* 04_connected_components.png
* 05_morphology.png
* 06_final_mask.png
* lung_overlay.png
* infection_overlay.png
* metrics.csv
* infection_stats.csv

results/ 目录不提交 Git。

如果某些结果图需要放入报告或 PPT，可以复制少量精选图片到 docs/figures/ 或 report/figures/ 中，并提交这些精选图片。

## 报告与展示建议

报告建议结构：

1. 项目背景与医学意义。
2. 数据集说明。
3. 方法流程。
4. 图像预处理。
5. 肺实质分割算法。
6. 连通域筛选与形态学修复。
7. 分割评价指标。
8. COVID-19 感染负荷分析。
9. 实验结果。
10. 消融实验。
11. 可视化展示。
12. 局限性分析。
13. 总结与展望。

PPT 展示建议围绕以下流程展开：

原始 CT
→ 肺窗 / 灰度归一化
→ 阈值分割
→ 连通域筛选
→ 形态学修复
→ 最终肺部 mask
→ 与人工 mask 对比
→ Dice / IoU 指标
→ 感染区域叠加
→ 感染负荷统计

展示时应重点突出：

* 本项目不是简单分类，而是医学图像处理流程。
* 肺部分割是感染负荷分析的前置步骤。
* 连通域筛选和形态学修复体现了传统图像处理方法的可解释性。
* 感染负荷统计提供了医学上更直观的定量结果。

## 常见问题

### 1. 为什么不把 Kaggle 数据集放进项目仓库？

因为数据集体积较大，不适合 Git 管理。不同成员本机路径也不同，直接放进仓库会导致仓库臃肿、同步困难甚至 push 失败。

### 2. 队友的数据路径和我的不一样怎么办？

每个成员自己创建 configs/paths.local.ini，并填写自己电脑上的路径。代码不应该硬编码任何人的绝对路径。

### 3. Codex 找不到数据怎么办？

不要让 Codex 扫描整个磁盘。程序应该从 configs/paths.local.ini 读取数据路径。如果该文件不存在，应提示用户创建。

### 4. 可以使用软链接吗？

可以，但不是必须。本项目优先使用 ini 配置文件管理路径。软链接可以作为本地辅助方案，但代码不应该依赖软链接。

### 5. 可以提交 sample_data 吗？

可以，但只能提交少量小样例，用于测试程序基本功能。不要把完整 Kaggle 数据集放入 sample_data。

### 6. 第一版是否必须支持 DICOM / NIfTI？

不必须。第一版优先支持 Kaggle 数据集中整理好的普通图像格式，例如 PNG、JPG、BMP、TIFF。后续如有需要，再扩展 SimpleITK 或 ITK 支持医学影像格式。

## 当前优先任务

当前开发优先级如下：

1. 完成项目路径配置文件。
2. 完成 README 和 docs 文档。
3. 配置 .gitignore，避免误提交数据和编译产物。
4. 确认 Visual Studio MFC 工程可以正常运行。
5. 配置 OpenCV。
6. 实现图像读取与显示。
7. 实现肺部分割 MVP。
8. 实现 Dice / IoU 评价。
9. 实现结果保存。
10. 实现感染负荷分析。

## 备注

本项目的核心不是“使用复杂模型”，而是基于传统图像处理方法构建一条完整、可解释、可展示的医学图像分析流程。

肺实质分割是基础能力，COVID-19 感染负荷分析是医学应用。通过多窗口展示中间结果和最终定量指标，本项目能够体现医学图像处理课程中图像增强、分割、形态学、连通域分析和定量评价等核心内容。
