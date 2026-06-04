# 阶段反馈：处理参数设置对话框

日期：2026-06-04

## 本阶段完成内容

- 新增 `ProcessingOptionsDialog`，提供图形化参数编辑入口。
- 新增菜单：
  - 主窗口：`数据集 -> 处理参数设置...`
  - 文档窗口：`处理 -> 处理参数设置...`
- 参数保存到本机私有配置：
  - `configs/paths.local.ini`
- 新增 `AppConfigLoader::SaveDefaultProcessingOptions`，只写处理参数，不改数据集路径。

## 可编辑参数

预处理参数：

- 肺窗 `WL`
- 肺窗 `WW`
- Gaussian kernel size
- Median kernel size
- CLAHE clip limit
- CLAHE tile grid size

肺部分割参数：

- Otsu 前高斯核大小
- 最小连通域面积
- 连通域面积自适应除数
- 保留主连通域数量
- 开运算核大小
- 闭运算核大小
- 形态学迭代次数

## 参数保护

对话框保存时会做基础范围修正：

- `WW` 必须大于 0。
- kernel size 不小于 3。
- 连通域数量、面积、迭代次数不小于 1。
- CLAHE clip limit 不小于 0.1。

算法内部仍会把 kernel size 修正为奇数，因此即使输入偶数也不会导致 OpenCV 调用失败。

## 验证结果

```text
MSBuild Debug|x64: 0 warnings, 0 errors
```

## 当前能力边界

- 对话框目前只保存到已有的 `paths.local.ini`；如果本地配置文件不存在，会提示用户先创建。
- 对话框不负责立即刷新已经生成的图像结果；修改参数后，重新执行预处理、分割或批处理即可使用新参数。

## 下一阶段建议

- 增加后台批处理进度条和取消功能。
- 增加“一键导出报告素材包”，把关键 PNG/SVG 统一复制到 `report/figures/`。
