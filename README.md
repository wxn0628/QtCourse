# samp2_4App — Qt 主窗口示例作业

课程作业：在示例代码中增加一个工具按钮，点击时显示一个 About 窗口，输出姓名、学号等信息。

## 开发环境

- Qt 6.12.0（MinGW 64-bit 套件）
- Qt Creator 20.0.1
- qmake 工程（samp2_4.pro）

## 本次修改内容

在原示例 `samp2_4`（QWMainWind 主窗口程序）基础上：

1. `qwmainwind.ui`：新增 `actAbout`（关于）Action，添加到主工具栏（"新建"按钮之后），
   并新增"帮助"菜单（内含"关于"项）；按钮图标使用资源中的 `images/430.bmp`。
2. `qwmainwind.h`：新增槽函数声明 `on_actAbout_triggered()`。
3. `qwmainwind.cpp`：实现该槽函数，使用 `QMessageBox::about()` 弹出 About 窗口，
   显示开发人员姓名与学号；并包含 `<QMessageBox>` 头文件。
4. `res.qrc`：注册 `images/430.bmp` 资源。

点击工具栏"关于"按钮后效果：弹出标题为 `samp2_4` 的对话框，
内容为「开发人员信息：吴湘楠 / 学号：2024414290333」。

## 编译运行

- Qt Creator：直接打开 `samp2_4.pro`，选择 Qt 6.12.0 MinGW 64-bit 套件，点击运行。
- 命令行：

```
mkdir build-qt6 && cd build-qt6
E:\QT\6.12.0\mingw_64\bin\qmake.exe ..\samp2_4.pro "CONFIG+=release"
mingw32-make -j8
```

## 作者

吴湘楠　2024414290333
