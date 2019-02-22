# 1. duilib
关于`duilib`的介绍可以访问：[duilib](https://github.com/duilib/duilib)

该项目对qdtroy版本的[DuiLib_Ultimate](https://github.com/qdtroy/DuiLib_Ultimate)进行了BUG修复、功能增强。

# 2. 如何编译
duilib使用Nuget包管理器进行依赖库管理：
依赖[ppx](https://github.com/winsoft666/ppx)库提供基础功能，如String、函数闭合体等。

使用`Visual Studio 2015`打开`src\DuiLib.sln`，选择对应的编译选项进行编译即可。

# 3. 属性
属性文档见：`src\doc\Attributes.xml`

# 4. BUG修复及新增功能

- 修复：在高DPI模式下, CLabelUI(含派生于它的CButtonUI控件)的size不正确的BUG。
- 修复：Edit控件在没有指定高度时，宽度无法适应DPI的BUG。
- 修复：List控件在拖动鼠标改变表头宽度时，表头宽度成倍数级增长的BUG。
- 修复：List控件在当用户指定了表头高度时，没有将高度进行DPI缩放的BUG。
- 修复：在未指定字体，使用默认字体时，文字被进行2次DPI Scale的BUG。
- 修复：Menu控件在高DPI下Menu窗体Size计算错误的BUG。
- 修复：Windows的maxinfo属性不支持DPI缩放的BUG。
- 修复：Caption 属性不能适应DPI缩放的BUG。
- 优化：不同DPI图片选择机制，在对应的DPI图片(***@200.png)不存在时，使用1倍原图缩放。
- 新增：ListHeader新增sepwidthadaptdpiscale属性，用于指定分隔符宽是否适应DPI缩放。
- 新增：bkimage等属性新增adaptdpiscale子属性，用于指定该图片是否适应DPI缩放。
- 新增：Window新增dpi属性，支持dpi数字、system等2种类型取值。
- 修复：绘制矩形边框时，右边框和下边框有1px的间距误差的BUG。
- 修复：压缩包内文件不支持中文的BUG。
- 修复：Groupbox的上边框不能位于文字中间的BUG。
- 新增：新增Windows size的取值max，支持在启动时即最大化。
- 新增：发送任务（含lamda表达式）到UI线程的功能。

-------------------------------
**感谢您的使用，欢迎提交BUG**

