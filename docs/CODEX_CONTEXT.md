# Codex Context

这是一个 C++ / MFC / OpenCV 医学图像处理课程大作业。

主题：
基于传统图像处理的胸部 CT 肺实质分割与 COVID-19 感染负荷定量分析。

约束：
- 不使用神经网络
- 不使用 PyTorch / TensorFlow
- 以 OpenCV 传统图像处理为主
- Visual Studio MFC MDI 项目
- 数据集不进入 Git 仓库

重要路径：
- 源码在 src/
- 数据路径从 configs/paths.local.ini 读取
- paths.local.ini 不进 Git
- paths.example.ini 进 Git
- 输出结果默认写入 results/

开发原则：
- 不要硬编码本机绝对路径
- 不要扫描整个磁盘找数据
- 不要把大数据文件加入 Git
- 每次新增模块都要保持可独立测试