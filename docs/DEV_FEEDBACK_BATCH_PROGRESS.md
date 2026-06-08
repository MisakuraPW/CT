# 阶段反馈：批处理进度与取消

## 本阶段完成内容

本阶段补齐了长时间批处理的交互框架，让“批量处理当前数据”和“一键处理配置数据集”在运行时显示进度，并支持用户取消。

主要改动：

- 新增 `CBatchProgressDialog` 进度对话框。
- `CBatchProcessor` 增加 `BatchProgressInfo` 和 `BatchProgressCallback`。
- 当前体数据批处理按切片上报进度。
- 数据集批处理按病例和切片上报进度。
- 用户点击“取消”后，会在当前切片处理结束后的下一个回调点停止。

## 修改文件

- `src/大作业/BatchProgressDialog.h`
- `src/大作业/BatchProgressDialog.cpp`
- `src/大作业/BatchProcessor.h`
- `src/大作业/BatchProcessor.cpp`
- `src/大作业/DatasetBatchRunner.h`
- `src/大作业/DatasetBatchRunner.cpp`
- `src/大作业/大作业.cpp`
- `src/大作业/大作业Doc.cpp`
- `src/大作业/Resource.h`
- `src/大作业/My.rc`
- `src/大作业/大作业.vcxproj`
- `src/大作业/大作业.vcxproj.filters`

## 关键设计

`BatchProgressCallback` 返回 `bool`：

- 返回 `true` 表示继续处理。
- 返回 `false` 表示用户请求取消。

这样底层批处理模块不依赖 MFC UI，可以继续作为独立算法/导出框架使用。MFC 层只负责把回调连接到进度对话框。

## 使用方式

在程序中：

1. 打开单张图像或 3D 体数据。
2. 选择“处理 -> 批量处理当前数据...”。
3. 选择输出目录。
4. 进度窗口会显示当前切片进度，可点击“取消”。

对于配置数据集：

1. 确认 `configs/paths.local.ini` 中数据集路径有效。
2. 选择“数据集 -> 一键处理配置数据集...”。
3. 进度窗口会显示病例和切片进度，可点击“取消”。

## 验证结果

已执行 Debug x64 编译：

```text
MSBuild src/大作业/大作业.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

结果：

```text
已成功生成。
0 个警告
0 个错误
```

## 注意事项

当前取消是协作式取消，不会强行中断正在运行的单张切片算法。也就是说，点击“取消”后会等待当前切片处理完成，再停止后续切片或病例。
