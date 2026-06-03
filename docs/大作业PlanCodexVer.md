# Codex 项目开发规划：医学图像处理大作业 MFC 程序

## 一、项目背景

我要开发一个 C++ / MFC / OpenCV 项目，用于医学图像处理课程大作业。项目主题是：

**基于传统图像处理的胸部 CT 肺实质分割与 COVID-19 感染负荷定量分析**

课程要求：
不允许使用神经网络。
可以使用 C++、OpenCV。
最终需要提交程序、代码、PDF 报告和 PPT 展示。

我已经使用 Visual Studio 创建了一个 **MFC 多文档应用程序（MDI）** 项目。希望在此基础上继续开发。项目需要支持多窗口显示，因为我希望同时展示原始 CT、预处理结果、阈值分割结果、连通域筛选结果、形态学修复结果、最终 mask、感染叠加图等多个阶段。

---

## 二、总体开发目标

请你协助我在现有 MFC MDI 工程中，逐步实现一个医学 CT 图像处理应用。核心功能包括：

1. 加载 CT 图像或普通灰度图像。
2. 对图像进行灰度归一化、窗宽窗位调整、滤波等预处理。
3. 使用传统图像处理方法完成肺实质分割。
4. 显示肺部分割过程中的中间结果。
5. 读取人工肺部 mask，并计算 Dice、IoU、Precision、Recall 等评价指标。
6. 在 COVID-19 CT 数据上结合感染 mask 计算感染负荷。
7. 支持多窗口可视化展示处理流程。
8. 支持导出处理结果图片和统计结果 CSV。

不要使用深度学习、神经网络、CNN、U-Net、PyTorch、TensorFlow 等内容。

---

## 三、推荐技术栈

主要使用：

```text
C++
MFC
OpenCV
Visual Studio
```

如果遇到医学图像格式读取问题，可以先不处理 DICOM / NIfTI，优先支持 Kaggle 数据集中已经转换好的 PNG / JPG / TIFF / BMP 图像。后续再考虑 SimpleITK 或 ITK。

第一版目标不是做复杂医学格式解析，而是先把传统图像处理流程跑通。

---

## 四、项目数据集设定

### 数据集 1：Finding and Measuring Lungs in CT Data

用途：
作为主数据集，用于肺部分割算法验证。

任务：
读取 CT 图像和对应人工肺部 mask。
执行肺部分割算法。
将算法输出 mask 与人工 mask 对比。
计算 Dice、IoU、Precision、Recall、面积误差。

### 数据集 2：COVID-19 CT scans

用途：
作为扩展数据集，用于 COVID-19 感染负荷定量分析。

任务：
读取原始 CT 图像、肺部 mask、感染 mask。
计算每层 CT 中：

```text
肺部面积
感染面积
感染面积 / 肺部面积
左肺感染比例
右肺感染比例
整体感染负荷
```

第一版可以直接使用数据集自带感染 mask 做定量分析，不强制要求自动分割感染区域。

---

## 五、MFC 多窗口设计目标

因为工程是 MDI 多文档应用，所以希望每个处理结果可以打开为一个独立子窗口，方便展示。

建议设计如下窗口类型：

```text
原始 CT 图像窗口
肺窗/灰度归一化窗口
阈值分割结果窗口
连通域筛选结果窗口
形态学修复结果窗口
最终肺部 mask 窗口
人工 mask 窗口
预测 mask 与人工 mask 叠加窗口
感染区域叠加窗口
感染负荷曲线窗口
```

第一版可以不做特别复杂的窗口类型，只要可以在 MDI 子窗口中显示不同 `cv::Mat` 结果即可。

---

## 六、代码结构建议

请尽量按照模块化方式组织代码，不要把所有逻辑写在 View 类里面。

建议目录结构：

```text
ProjectRoot/
  src/
    core/
      ImageTypes.h
      ImageUtils.h
      ImageUtils.cpp

    io/
      ImageLoader.h
      ImageLoader.cpp
      MaskLoader.h
      MaskLoader.cpp

    preprocess/
      Windowing.h
      Windowing.cpp
      Filters.h
      Filters.cpp

    segmentation/
      LungSegmenter.h
      LungSegmenter.cpp
      ConnectedComponentFilter.h
      ConnectedComponentFilter.cpp
      MorphologyProcessor.h
      MorphologyProcessor.cpp

    analysis/
      Metrics.h
      Metrics.cpp
      InfectionAnalyzer.h
      InfectionAnalyzer.cpp

    visualization/
      Overlay.h
      Overlay.cpp
      ColorMap.h
      ColorMap.cpp

    export/
      ResultExporter.h
      ResultExporter.cpp
```

如果 MFC 工程不方便使用这种完整目录，也可以至少按类拆分：

```text
CLungSegmenter
CPreprocessor
CConnectedComponentFilter
CMorphologyProcessor
CMetricsCalculator
CInfectionAnalyzer
COverlayVisualizer
CResultExporter
```

---

## 七、核心算法流程

### 1. 图像加载

实现功能：

```text
打开单张图像
打开一个文件夹中的多张 CT 切片
打开对应 mask 图像
显示图像尺寸、通道数、灰度范围
```

优先支持：

```text
.png
.jpg
.bmp
.tif
.tiff
```

读取后统一转成 OpenCV 的 `cv::Mat`。

灰度图统一为：

```cpp
CV_8UC1
```

彩色显示图统一为：

```cpp
CV_8UC3
```

---

### 2. 灰度归一化与窗宽窗位

实现一个预处理模块。

如果输入是普通 8-bit 图像，则做：

```text
灰度归一化
可选 CLAHE
可选 GaussianBlur
可选 MedianBlur
```

如果后续支持 HU 数据，则加入窗宽窗位处理：

```text
lung window:
WL = -600
WW = 1500
lower = WL - WW / 2
upper = WL + WW / 2
```

映射到 0-255：

```cpp
dst = (src - lower) / (upper - lower) * 255
```

第一版如果只处理 PNG/JPG，可以先只实现 8-bit 版本。

---

### 3. 初始肺部分割

实现传统阈值分割。

需要支持几种模式：

```text
固定阈值
Otsu 阈值
自适应阈值
```

建议第一版优先实现：

```text
GaussianBlur + Otsu
```

流程：

```text
输入预处理后的灰度 CT
高斯滤波
Otsu 阈值
根据肺部在 CT 中偏暗的特点获得二值 mask
```

需要注意：
阈值结果可能需要反色。肺区域可能是黑色，也可能是白色。请根据图像均值或用户选项判断是否取反，最终统一保证：

```text
肺部区域 = 255
背景区域 = 0
```

---

### 4. 连通域筛选

实现连通域分析模块。

目的：
去除阈值分割后混入的外部空气、边界背景、小噪声区域，只保留左右肺主体。

使用 OpenCV：

```cpp
cv::connectedComponentsWithStats()
```

筛选规则：

```text
去除面积太小的连通域
去除明显接触图像边界的大型区域
保留面积最大的两个或几个非边界连通域
根据质心位置筛选位于胸腔内部的区域
```

输出：

```text
连通域伪彩色图
筛选后的二值 mask
```

这一步很重要，后续报告和 PPT 需要展示“阈值分割结果 → 连通域筛选结果”的变化。

---

### 5. 形态学修复

实现形态学后处理模块。

使用 OpenCV：

```cpp
cv::morphologyEx()
cv::erode()
cv::dilate()
cv::getStructuringElement()
```

默认流程：

```text
开运算：去除小噪声
闭运算：连接断裂区域，补小缝隙
孔洞填充：填补肺内部血管/支气管造成的空洞
边界平滑：让 mask 更自然
```

推荐参数初值：

```text
Opening kernel: ellipse 3x3
Closing kernel: ellipse 5x5
Iterations: 1
```

孔洞填充可以使用 flood fill 思路：

```text
对 mask 取反
从图像边界 floodFill 背景
未被 floodFill 到的区域视为内部孔洞
将孔洞填回肺部区域
```

输出：

```text
形态学修复前 mask
形态学修复后 mask
孔洞填充后 mask
```

---

### 6. 左右肺分离

实现可选模块。

目标：
把最终肺部 mask 分成左肺和右肺，方便计算左右肺感染比例。

第一版简单做法：

```text
对最终肺 mask 做连通域分析
若存在两个主要连通域，则按质心 x 坐标区分左右肺
左侧连通域为图像左侧肺区域，右侧连通域为图像右侧肺区域
```

注意医学左右与图像左右可能相反，报告里可以说明这里使用的是图像坐标意义上的左右。

如果两个肺连在一起：

```text
尝试轻微腐蚀后再做连通域
或者用图像中线粗略分割
```

---

## 八、评价指标模块

实现 `CMetricsCalculator`。

输入：

```cpp
cv::Mat predMask;
cv::Mat gtMask;
```

要求二者都是二值图，前景为 255，背景为 0。

计算：

```text
TP
FP
TN
FN
Dice
IoU
Precision
Recall
Area Error
```

公式：

```text
Dice = 2TP / (2TP + FP + FN)
IoU = TP / (TP + FP + FN)
Precision = TP / (TP + FP)
Recall = TP / (TP + FN)
AreaError = abs(predArea - gtArea) / gtArea
```

注意处理分母为 0 的情况，避免程序崩溃。

输出可以显示在：

```text
状态栏
弹窗
结果窗口
CSV 文件
```

CSV 格式建议：

```csv
filename,dice,iou,precision,recall,pred_area,gt_area,area_error
case001.png,0.94,0.89,0.95,0.93,12345,12400,0.0044
```

---

## 九、感染负荷分析模块

实现 `CInfectionAnalyzer`。

输入：

```cpp
cv::Mat lungMask;
cv::Mat infectionMask;
cv::Mat leftLungMask;
cv::Mat rightLungMask;
```

第一版先支持单张切片。

计算：

```text
lung_area
infection_area
infection_ratio = infection_area / lung_area

left_lung_area
left_infection_area
left_infection_ratio

right_lung_area
right_infection_area
right_infection_ratio
```

对于多切片序列，计算每一层：

```text
slice_index
lung_area
infection_area
infection_ratio
```

输出 CSV：

```csv
slice,lung_area,infection_area,infection_ratio,left_ratio,right_ratio
0,12000,200,0.0167,0.010,0.023
1,12500,260,0.0208,0.012,0.030
```

后续可以根据 CSV 生成曲线。MFC 内部第一版可以不画复杂曲线，先导出 CSV，PPT 中再用 Python / Excel 绘图也可以。

---

## 十、可视化模块

实现 `COverlayVisualizer`。

需要支持：

```text
mask 叠加到原图
肺部 mask 轮廓叠加
感染 mask 半透明叠加
预测 mask 与人工 mask 对比叠加
连通域伪彩色显示
```

推荐颜色逻辑：

```text
肺部 mask：绿色
感染 mask：红色
预测 mask：绿色
人工 mask：蓝色
重叠区域：黄色或白色
```

注意 OpenCV 默认是 BGR，不是 RGB。

可视化函数示例：

```cpp
cv::Mat OverlayMask(const cv::Mat& gray, const cv::Mat& mask, cv::Scalar color, double alpha);
cv::Mat DrawContourOverlay(const cv::Mat& gray, const cv::Mat& mask, cv::Scalar color);
cv::Mat MakeConnectedComponentColorMap(const cv::Mat& labels);
```

---

## 十一、MFC 菜单功能规划

请在 MFC 菜单栏中增加以下功能：

```text
文件
  打开图像
  打开图像文件夹
  打开人工 mask
  保存当前结果
  导出 CSV

处理
  灰度归一化
  肺窗显示
  高斯滤波
  Otsu 阈值分割
  连通域筛选
  形态学修复
  一键肺部分割

分析
  计算 Dice / IoU
  感染负荷分析
  左右肺感染比例分析

视图
  显示原图
  显示阈值结果
  显示连通域结果
  显示最终 mask
  显示叠加图
```

第一版重点完成：

```text
打开图像
一键肺部分割
显示中间结果
打开 mask
计算 Dice / IoU
保存结果图
导出 CSV
```

---

## 十二、开发阶段安排

### 阶段 1：工程基础与图像显示

目标：

```text
确认 MFC MDI 工程能正常编译运行
接入 OpenCV
实现打开单张图像
在 MDI 子窗口中显示 cv::Mat
实现 Mat 到 CImage / Bitmap 的转换
```

验收标准：

```text
可以打开一张 CT PNG 图像
可以在窗口中正常显示
支持灰度图和彩色图显示
```

---

### 阶段 2：预处理与阈值分割

目标：

```text
实现灰度归一化
实现高斯滤波
实现 Otsu 阈值
实现固定阈值
实现阈值结果反色
```

验收标准：

```text
能够打开 CT 图像并生成初始二值肺区域候选 mask
可以新开 MDI 子窗口显示阈值结果
```

---

### 阶段 3：连通域筛选

目标：

```text
实现 connectedComponentsWithStats
实现面积筛选
实现边界接触筛选
实现保留左右肺主要区域
实现连通域伪彩色图
```

验收标准：

```text
阈值结果中的外部空气和小噪声明显减少
可以显示连通域筛选前后对比
```

---

### 阶段 4：形态学修复与孔洞填充

目标：

```text
实现开运算
实现闭运算
实现孔洞填充
实现最终肺 mask 输出
```

验收标准：

```text
肺部 mask 内部孔洞减少
边界更完整
可以保存最终 mask
```

---

### 阶段 5：评价指标

目标：

```text
支持打开人工 mask
对预测 mask 和人工 mask 统一尺寸、统一二值化
计算 Dice、IoU、Precision、Recall、面积误差
导出 CSV
```

验收标准：

```text
输入一张 CT 和对应人工 mask 后，可以得到完整评价指标
批量处理时可以生成 results.csv
```

---

### 阶段 6：COVID 感染负荷分析

目标：

```text
读取肺部 mask
读取感染 mask
计算感染面积占比
计算左右肺感染比例
导出每层统计 CSV
生成感染叠加图
```

验收标准：

```text
可以对一组 COVID CT 切片输出感染比例统计表
可以显示感染区域叠加图
```

---

### 阶段 7：展示优化

目标：

```text
整理一键处理流程
保存所有中间结果
生成报告/PPT 所需图片
优化界面菜单和状态栏
增加错误提示
```

验收标准：

```text
演示时可以流畅展示：
原始 CT → 预处理 → 阈值分割 → 连通域筛选 → 形态学修复 → 最终 mask → 指标计算 → 感染负荷分析
```

---

## 十三、第一版最小可行版本 MVP

请优先实现 MVP，不要一开始追求复杂功能。

MVP 必须包含：

```text
1. 打开单张 CT 图像
2. 显示原图
3. Otsu 阈值分割
4. 连通域筛选
5. 形态学修复
6. 显示最终肺 mask
7. 打开人工 mask
8. 计算 Dice 和 IoU
9. 保存结果图
```

MVP 完成后再做：

```text
批量处理
感染负荷分析
左右肺分离
CSV 导出
曲线图
复杂 MDI 多窗口管理
```

---

## 十四、注意事项

1. 所有 mask 必须统一为二值图：

```text
前景 = 255
背景 = 0
```

2. OpenCV 中显示和处理要注意通道：

```text
灰度图 CV_8UC1
彩色图 CV_8UC3
```

3. 计算指标前必须确保：

```text
预测 mask 和人工 mask 尺寸一致
类型一致
前景定义一致
```

4. MFC 显示图像时，不要直接在 View 里写所有算法逻辑。View 只负责显示和交互，算法放到独立类里。

5. 每一步中间结果都要能保存，因为报告和 PPT 需要展示处理链条。

6. 不要引入神经网络，不要使用深度学习库。

---

## 十五、请你执行时的工作方式

请按照以下顺序协助开发：

```text
第一步：检查当前 MFC 工程结构，确认 OpenCV 是否已配置。
第二步：实现 cv::Mat 在 MFC View 中显示。
第三步：实现单张图像加载。
第四步：实现肺部分割 MVP。
第五步：实现评价指标。
第六步：实现结果保存。
第七步：实现批量处理和感染负荷分析。
```

每一步完成后，请先给出：

```text
修改了哪些文件
新增了哪些类
关键函数说明
如何测试
可能的错误点
```

不要一次性生成过大的代码改动。优先小步提交、小步测试。
