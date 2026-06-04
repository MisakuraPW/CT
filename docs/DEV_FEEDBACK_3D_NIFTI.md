# 3D NIfTI Support Feedback

## 本阶段目标

在继续做批量处理之前，先补齐 3D 医学图像数据支持，解决 `.nii` / `.nii.gz` 体数据无法打开、无法切片验证的问题。

## 已实现能力

### 1. 支持打开 NIfTI 体数据

新增支持格式：

- `.nii`
- `.nii.gz`

支持用途：

- CT 体数据
- 人工肺部 mask 体数据
- COVID 感染 mask 体数据

打开 `.nii/.nii.gz` 后，程序会把体数据拆成 2D 切片，并默认显示中间切片。

### 2. 支持切片切换

新增菜单：

```text
视图 -> 上一切片
视图 -> 下一切片
```

切换切片时会同步更新：

- 当前 CT 切片
- 当前人工 mask 切片
- 当前感染 mask 切片

切换切片后会清空当前切片派生结果：

- 肺部分割结果
- Dice / IoU 指标
- 感染负荷统计
- 感染叠加图

这样可以避免把上一张切片的分析结果误用于当前切片。

### 3. 现有 2D 流程可复用在当前切片上

打开 3D 数据后，当前切片仍然走已有 2D 流程：

1. `处理 -> 一键肺部分割`
2. `文件 -> 打开人工 mask`
3. `分析 -> 计算 Dice / IoU`
4. `文件 -> 打开感染 mask`
5. `分析 -> 感染负荷分析`
6. `文件 -> 保存当前结果`
7. `文件 -> 导出指标 CSV`
8. `文件 -> 导出感染 CSV`

### 4. 标注体数据自动二值化

NIfTI mask 不一定是 8-bit 图像，可能是 `int16`、`uint16`、`float32` 等类型。

程序读取人工 mask / 感染 mask 后会先转换到 8-bit，再二值化为：

```text
前景 = 255
背景 = 0
```

## 新增文件

- `src/大作业/NiftiIO.h`
- `src/大作业/NiftiIO.cpp`

## 修改文件

- `src/大作业/大作业Doc.h`
- `src/大作业/大作业Doc.cpp`
- `src/大作业/大作业.h`
- `src/大作业/大作业.cpp`
- `src/大作业/MainFrm.cpp`
- `src/大作业/My.rc`
- `src/大作业/Resource.h`
- `src/大作业/大作业.vcxproj`
- `src/大作业/大作业.vcxproj.filters`

## NIfTI 支持范围

当前实现支持 NIfTI-1 单文件格式：

- `.nii`
- `.nii.gz`

常见 datatype：

- `uint8`
- `int8`
- `int16`
- `uint16`
- `int32`
- `uint32`
- `float32`
- `float64`

当前未处理：

- `.hdr/.img` 双文件格式
- NIfTI-2
- 仿射矩阵方向重排
- 体素间距用于物理面积/体积计算

目前面积统计仍然是像素面积，不是 mm² 或 ml。

## `.nii.gz` 说明

OpenCV 预编译包没有暴露 zlib/libzip，因此 `.nii.gz` 当前通过本机 Python 标准库 `gzip` 临时解压。

要求：

```text
python
```

或：

```text
py -3
```

至少有一个命令可在 PATH 中调用。

如果 `.nii.gz` 打不开，但 `.nii` 可以打开，优先检查 Python 是否可用：

```powershell
python --version
```

或：

```powershell
py -3 --version
```

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

## 使用建议

### COVID `.nii` 数据

1. `文件 -> 打开`，选择 CT `.nii`。
2. `视图 -> 上一切片 / 下一切片` 浏览切片。
3. `文件 -> 打开感染 mask`，选择对应感染标注 `.nii`。
4. 当前切片执行肺部分割和感染负荷分析。

### Finding Lungs `.nii.gz` 数据

1. 确认本机 `python --version` 可用。
2. `文件 -> 打开`，选择 CT `.nii.gz`。
3. 使用 `视图 -> 上一切片 / 下一切片` 浏览。
4. 打开对应 3D mask 后可按当前切片计算 Dice / IoU。

## 下一阶段建议

3D 数据能打开后，下一阶段可以继续做批量处理：

1. 对 3D 体数据逐切片运行肺部分割。
2. 逐切片匹配人工 mask / 感染 mask。
3. 生成每层 Dice / IoU。
4. 生成每层感染负荷。
5. 导出整套体数据的汇总 CSV。
6. 可选：保存每层中间结果和叠加图。
