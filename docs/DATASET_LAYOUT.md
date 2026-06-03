# Dataset Layout

本项目不将 Kaggle 数据集提交到 Git。数据集通过本地路径配置文件指定。

## 本地配置文件

请复制：

configs/paths.example.ini

并重命名为：

configs/paths.local.ini

然后填写本机数据集路径。

## 当前使用的数据集

### 1. Finding and Measuring Lungs in CT Data

用途：肺实质分割算法验证。

本地路径通过配置项指定：

lung_dataset_root = ...

该数据集用于读取：
- CT 图像
- 人工肺部 mask

### 2. COVID-19 CT scans

用途：COVID-19 感染负荷定量分析。

本地路径通过配置项指定：

covid_dataset_root = ...

该数据集用于读取：
- 原始 CT 图像
- 肺部 mask
- 感染 mask

## 路径规则

代码中禁止硬编码绝对路径。

所有数据路径必须从 configs/paths.local.ini 读取。

如果 paths.local.ini 不存在，程序应提示用户复制 paths.example.ini 并填写路径。