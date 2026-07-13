本项目基于 STM32F407 微控制器与 LVGL 图形库开发，采用 Makefile 组织工程架构，用于手持地面站的数据监控与交互。

## 项目架构与目录结构
项目代码采用模块化分层设计，目录结构如下：
App/：应用层，包含 LVGL UI 界面设计及地面站核心业务逻辑。
BSP/：板级支持包，包含屏幕、触摸屏及其他板载外设的底层驱动。
Middlewares/：中间件，包含 FreeRTOS 实时操作系统与 LVGL 图形库源码以及 Mavlink 协议。
Platform/STM32F4/：平台相关代码，包含 STM32F4 标量启动文件、标准库底层驱动及硬件初始化配置。
Tasks/：任务管理层，处理 FreeRTOS 的多任务调度（如 UI 刷新任务、数据通信任务等）。
Makefile：项目构建脚本，用于管理编译与链接配置。

## 开发与构建环境
核心硬件：STM32F407
软件组件：FreeRTOS + LVGL
构建工具：GNU Arm Embedded Toolchain (GCC) + Make
