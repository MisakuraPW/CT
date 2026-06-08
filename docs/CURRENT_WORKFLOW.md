# 当前可运行工作流与验收清单

本文记录当前代码框架已经打通的功能链条，用于后续开发、报告撰写和演示验收。

## 1. 本地准备

1. 复制 `configs/paths.example.ini` 为 `configs/paths.local.ini`。
2. 填写本机数据集路径：
   - `lung_dataset_root`
   - `covid_dataset_root`
   - `result_root`
3. 复制 `configs/opencv.example.props` 为 `configs/opencv.local.props`。
4. 填写 OpenCV 配置：
   - `OpenCVRoot`
   - `OpenCVLibDir`
   - `OpenCVDllDir`
   - `OpenCVWorldVersion`
5. 使用 Visual Studio 打开 `src/大作业/大作业.vcxproj`，选择 `x64 / Debug` 或 `x64 / Release` 编译。

## 2. 单张图像流程

菜单路径：

```text
文件 -> 打开
处理 -> 灰度/归一化
处理 -> 肺窗显示
处理 -> 高斯滤波 / 中值滤波 / CLAHE 增强
处理 -> 一键肺部分割
文件 -> 打开人工 mask
分析 -> 计算 Dice / IoU
文件 -> 导出指标 CSV
文件 -> 保存当前结果
```

验收点：

- 可以打开 PNG/JPG/BMP/TIF/TIFF。
- 可以显示原图、预处理结果、阈值结果、连通域结果、形态学结果、最终 mask。
- 可以计算 Dice、IoU、Precision、Recall、Area Error。
- 可以保存当前显示图像和导出指标 CSV。

## 3. 3D 体数据流程

支持格式：

```text
.nii
.nii.gz
```

菜单路径：

```text
文件 -> 打开
视图 -> 上一切片 / 下一切片
文件 -> 打开人工 mask
文件 -> 打开感染 mask
处理 -> 一键肺部分割
分析 -> 计算 Dice / IoU
分析 -> 感染负荷分析
```

验收点：

- COVID 数据集的 CT、肺部标注、感染标注可以以 NIfTI 体数据读取。
- finding-lungs 数据集中的 3D `.nii.gz` 可以读取。
- 切片切换时，人工 mask 和感染 mask 会跟随当前切片同步。
- 对当前切片可以运行分割、指标计算和感染负荷分析。

## 4. 当前数据批处理

菜单路径：

```text
处理 -> 批量处理当前数据...
```

输出结构：

```text
输出目录/
  masks/
  intermediate/
  overlays/
  csv/
    batch_summary.csv
    run_manifest.txt
```

验收点：

- 单张图像会作为 1 个切片处理。
- 3D 体数据会逐切片处理。
- 如果已加载人工 mask，会输出分割评价指标。
- 如果已加载感染 mask，会输出感染负荷统计。
- 运行期间会显示进度窗口，可取消。

## 5. 配置数据集批处理

菜单路径：

```text
数据集 -> 扫描数据集任务...
数据集 -> 一键处理配置数据集...
```

输出结构：

```text
results/
  dataset_tasks.csv
  datasets/
    dataset_summary.csv
    <dataset_kind>/
      <case_id>/
        masks/
        intermediate/
        overlays/
        csv/
```

验收点：

- 能扫描 finding-lungs 2D、finding-lungs 3D 和 COVID NIfTI 数据。
- 能逐病例批处理配置中的数据集。
- `dataset_summary.csv` 记录每个病例的状态、输出目录、批处理 CSV 和 manifest。
- 运行期间会显示病例/切片进度，可取消。

## 6. 报告素材生成

命令行工具：

```text
python tools/generate_report_figures.py
python tools/collect_report_assets.py
```

推荐流程：

1. 先运行数据集批处理，生成 `results/datasets/dataset_summary.csv`。
2. 运行 `tools/generate_report_figures.py` 生成统计 SVG。
3. 运行 `tools/collect_report_assets.py` 收集报告/PPT 需要的精选图和 manifest。

验收点：

- `results/figures/` 中生成统计图。
- `report/figures/` 中生成精选素材和 `ASSET_MANIFEST.md`。
- `results/` 和 `report/figures/` 默认不提交 Git，避免把批量输出塞进仓库。

## 7. 已验证构建命令

```powershell
& cmd.exe /d /s /c 'set PATH=& set "Path=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0;G:\MicrosoftVisualStudio\VSC\MSBuild\Current\Bin"& "G:\MicrosoftVisualStudio\VSC\MSBuild\Current\Bin\MSBuild.exe" "src\大作业\大作业.vcxproj" /p:Configuration=Debug /p:Platform=x64 /m'
```

最近一次验证结果：

```text
已成功生成。
0 个警告
0 个错误
```

## 8. 当前边界

- NIfTI 支持覆盖 `.nii` 和 `.nii.gz`，不覆盖 NIfTI-2、DICOM、Analyze `.hdr/.img`。
- `.nii.gz` 依赖本机可用的 Python gzip 解压流程。
- 取消是协作式取消，会等待当前切片处理完成后停止。
- 项目坚持传统图像处理路线，不引入 PyTorch、TensorFlow、CNN、U-Net 或 Transformer。
