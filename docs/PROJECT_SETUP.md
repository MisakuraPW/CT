# Project Setup

本文档记录本项目进入正式开发前需要保持一致的基础配置。

## 1. Git 仓库

项目根目录需要是 Git 仓库，默认分支建议使用 `main`。

不要提交以下内容：

- `configs/paths.local.ini`
- `configs/*.local.props`
- `.vs/`
- `x64/`, `Debug/`, `Release/`
- 完整 Kaggle 数据集
- `results/` 中生成的输出文件

可以提交以下内容：

- MFC / Visual Studio 工程文件
- `src/` 源码
- `configs/paths.example.ini`
- `configs/opencv.example.props`
- `docs/` 文档
- 少量 `sample_data/` 示例数据
- 报告、PPT 和精选展示图片

## 2. 本地路径配置

复制：

```text
configs/paths.example.ini
```

为：

```text
configs/paths.local.ini
```

然后修改本机数据集路径。

所有代码都应从 `configs/paths.local.ini` 读取数据集路径，不要硬编码绝对路径，也不要扫描整个磁盘查找数据。

## 3. OpenCV 配置

复制：

```text
configs/opencv.example.props
```

为：

```text
configs/opencv.local.props
```

然后修改：

- `OpenCVRoot`
- `OpenCVLibDir`
- `OpenCVDllDir`
- `OpenCVWorldVersion`

`opencv.local.props` 是本机私有配置，不进入 Git。Visual Studio 工程会在该文件存在时自动导入它。

如果你安装的 OpenCV 不使用 `opencv_world*.lib`，请把 `AdditionalDependencies` 改成本机实际的 OpenCV `.lib` 列表。

## 4. Visual Studio 调试工作目录

工程已配置调试工作目录为项目根目录：

```text
$(MSBuildProjectDirectory)\..\..
```

这样程序运行时可以用相对路径读取：

```text
configs/paths.local.ini
results/
sample_data/
```

## 5. 开发顺序

当前阶段完成后，后续建议按以下顺序开发：

1. 确认 MFC MDI 工程可编译运行。
2. 配置 OpenCV。
3. 实现 `cv::Mat` 到 MFC View 的显示。
4. 实现单张图像读取。
5. 实现灰度化、归一化和基础滤波。
6. 实现 Otsu 阈值分割。
7. 实现连通域筛选。
8. 实现形态学修复和孔洞填充。
9. 实现 Dice / IoU 等评价指标。
10. 实现结果保存和 CSV 导出。
