# DEMO1 — Math & CAD Visualizer

> 跨平台数学与 CAD 可视化演示程序，基于 SDL2 + Dear ImGui，支持原生 Linux/Raspberry Pi 与 WebAssembly（浏览器）两种运行方式。
>
> A cross-platform math and CAD visualization demo built with SDL2 + Dear ImGui, running natively on Linux/Raspberry Pi and in the browser via WebAssembly.

---

## 目录 / Table of Contents

- [功能概览 / Features](#功能概览--features)
- [依赖 / Dependencies](#依赖--dependencies)
- [获取源码 / Get the Source](#获取源码--get-the-source)
- [原生编译 / Native Build](#原生编译--native-build)
- [WebAssembly 编译 / WASM Build](#webassembly-编译--wasm-build)
- [运行 / Run](#运行--run)
- [插件说明 / Plugin Guide](#插件说明--plugin-guide)
- [目录结构 / Project Layout](#目录结构--project-layout)

---

## 功能概览 / Features

| 菜单 | 插件 | 说明 |
|------|------|------|
| **Demo** | Lorenz Attractor | Lorenz 混沌吸引子 |
| | Mandelbrot Set | Mandelbrot 集，橡皮筋缩放 |
| | Julia Set | Julia 集，参数与视图分离 |
| | Newton Fractal | Newton 迭代分形 |
| | Ecosystem | 捕食者-猎物生态模拟 |
| | Henon Map | Hénon 映射 |
| | May (Logistic) | Logistic 映射分叉图 |
| | Trammel | Archimedes 椭圆规 |
| | L-System | L 系统龟形图 |
| | Julia Out | Julia 集反向迭代法 |
| | IFS Fractal | IFS 迭代函数系统分形 |
| | Eyes | 跟随鼠标的眼球动画 |
| **Evaluate** | Graph View | .gph 格式有/无向加权图可视化 |
| | Diagram | 多曲线绘图（折线 / 样条 / IFS） |
| | Geograph | .pan 格式地形高度图（2D 热力图、等高线、3D 线框、3D 遮掩） |
| **CAD** | GWB Solids | B-Rep 半边数据结构实体（圆柱、球、圆环、长方体），背面剔除 |

---

## 依赖 / Dependencies

### 原生构建 / Native

```bash
# Debian / Ubuntu / Raspberry Pi OS
sudo apt install cmake g++ libsdl2-dev
```

> Dear ImGui 已内置于 `third_party/imgui/`，无需额外安装。
> Dear ImGui is bundled in `third_party/imgui/` — no extra install needed.

### WASM 构建 / WebAssembly

需要 [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)：

```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ~/emsdk/emsdk_env.sh
```

---

## 获取源码 / Get the Source

```bash
git clone https://github.com/Azz1/DEMO1.git
cd DEMO1
```

---

## 原生编译 / Native Build

```bash
# 在项目根目录
cmake -B build
cmake --build build --parallel 4
```

编译产物 / Output:
- `build/bin/demo` — 主程序 / Main executable
- `build/lib/show.so` — 可视化插件库 / Visualization plugin library
- `build/bin/eyes` — 独立眼球程序 / Standalone eyes application

---

## WebAssembly 编译 / WASM Build

```bash
source ~/emsdk/emsdk_env.sh

cd build_wasm
emcmake cmake .
emmake make -j4
```

编译产物 / Output：`build_wasm/demo.html`、`demo.js`、`demo.wasm`

启动本地服务器（需要 COEP/COOP 头） / Serve locally (COEP/COOP headers required):

```bash
cd build_wasm
python3 serve.py 8080
# 浏览器打开 / Open in browser: http://localhost:8080/demo.html
```

> `serve.py` 已内置于 `build_wasm/` 目录。
> `serve.py` is included in `build_wasm/`.

---

## 运行 / Run

### 原生 / Native

```bash
cd build/bin
./demo
```

### 独立眼球程序 / Standalone Eyes

```bash
build/bin/eyes
# 双击左键 = 切换无边框/常驻顶层模式
# 右键 = 关闭（无边框模式下）
# ESC = 退出
```

### 键盘快捷键 / Keyboard Shortcuts

| 按键 | 功能 |
|------|------|
| `F12` | 保存截图（JPG） |
| `ESC` | 退出程序 |

---

## 插件说明 / Plugin Guide

### 🌀 Lorenz Attractor

经典 Lorenz 混沌吸引子三维点迹。  
Classic Lorenz chaotic attractor plotted in 3D.

**交互 / Controls:**
- 左键拖拽 / LMB drag — 旋转视角 / Rotate view
- 滚轮 / Scroll — 缩放 / Zoom
- 侧栏滑块 / Sidebar sliders — σ、ρ、β 参数及步数 / σ, ρ, β parameters and step count

---

### 🔵 Mandelbrot Set

Mandelbrot 集着色渲染。  
Mandelbrot set with iteration-depth coloring.

**交互 / Controls:**
- 左键框选 / LMB rubber-band — 放大选区 / Zoom into selection
- 右键 / RMB — 重置视图 / Reset view
- 侧栏 / Sidebar — 最大迭代次数、颜色模式 / Max iterations, color mode

---

### 🌈 Julia Set

Julia 集，参数 c 与视图中心独立控制。  
Julia set with separate c parameter and view center controls.

**交互 / Controls:**
- 左键框选 / LMB rubber-band — 放大 / Zoom
- 右键 / RMB — 重置 / Reset
- 侧栏 / Sidebar — 复数参数 (cx, cy)、视图中心、迭代次数 / Complex parameter (cx, cy), view center, iterations

---

### ✨ Newton Fractal

牛顿迭代法生成的分形，多项式根着色。  
Newton's method fractal with per-root coloring.

**交互 / Controls:**
- 左键框选 / LMB rubber-band — 放大 / Zoom
- 右键 / RMB — 重置 / Reset

---

### 🦊 Ecosystem

捕食者-猎物生态系统模拟，基因颜色编码适应度。  
Predator-prey ecosystem with gene-encoded fitness coloring.

**交互 / Controls:**
- 侧栏 / Sidebar — 初始种群、繁殖参数 / Initial populations, reproduction params

---

### 📐 Henon Map / May (Logistic) / Trammel

经典二维离散动力系统与机械曲线演示。  
Classic 2D discrete dynamical systems and mechanical curve demos.

**交互 / Controls:**
- 左键框选 / LMB rubber-band — 放大 / Zoom
- 右键 / RMB — 重置 / Reset

---

### 🌿 L-System

L 系统龟形图（Lindenmayer 系统），内置 6 种预设。  
L-System turtle graphics with 6 built-in presets.

**交互 / Controls:**
- 侧栏 / Sidebar — 预设选择、自定义公理和规则、迭代次数、步长、角度 / Preset, custom axiom & rules, iterations, step, angle

---

### 🔄 Julia Out / IFS Fractal

Julia 集反向迭代法 & IFS 迭代函数系统（Barnsley 蕨、Sierpinski 三角等）。  
Julia inverse iteration & IFS fractals (Barnsley fern, Sierpinski triangle, etc.)

---

### 👁️ Eyes

跟随鼠标移动的眼球，支持 1–4 只眼睛和背景颜色调整。  
Eyeballs that follow the mouse cursor; supports 1–4 eyes and custom background color.

---

### 🕸️ Graph View

可视化有向/无向加权图，支持节点权重和边权重显示。  
Directed/undirected weighted graph visualizer.

**数据格式 / Data Format (`.gph`):**
```
# comment
[Nodes]
label weight x y z
...
[DEdges]        # or [Edges] for undirected
label n0 n1 stress
...
```

**交互 / Controls:**
- 滚轮 / Scroll — 缩放 / Zoom
- 右键拖拽 / RMB drag — 平移 / Pan
- 内置测试数据 / Built-in test data; native: Load File button

---

### 📈 Diagram

多曲线绘图，三种模式：  
Multi-curve plotter with three modes:

| 模式 | 说明 |
|------|------|
| **Dline** | 折线 / Polyline |
| **Scurve** | 三次 Hermite 样条 / Cubic Hermite spline |
| **Fcurve** | IFS 迭代控制点分形 / IFS fractal via iterated control points |

**数据格式 / Data Format:**
```
# comment
ncurves
x  y0  y1  ...
x  y0  y1  ...
# value -999 = skip (gap in curve)
```

---

### 🏔️ Geograph

地形高度图可视化，支持五种显示模式。  
Terrain height-map visualizer with five display modes.

**显示模式 / Display Modes:**

| 模式 | 说明 |
|------|------|
| 2D Map | 高度热力图着色 / Height heatmap |
| 2D Contour | 等高线（Marching Squares） / Contour lines |
| 3D Wire | 透视线框 / Perspective wireframe |
| 3D Hidden | 射线检测遮掩线框 / Ray-march hidden surface removal |
| 3D+Cont | 3D 空间等高线 / 3D contour projection |

**数据格式 / Data Format (`.pan`):**
```
title N M stepX stepY
z00 z01 z02 ...   (M rows × N columns of float height values)
```

内置预设 / Built-in presets: **Wave**（50×50 衰减正弦波）、**Function**（50×50 函数曲面）、**Test**（11×10 稀疏测试）

**交互 / Controls (3D modes):**
- 左键拖拽 / LMB drag — 旋转 / Rotate
- 滚轮 / Scroll — 缩放 / Zoom
- Levels 滑块 / slider — 等高线层数（2–300）/ Contour levels (2–300)
- Apply 按钮 — 应用文本框中编辑的数据 / Apply edited data from text box
- Reset 按钮 — 重新加载当前预设 / Reload current preset

---

### 🧊 GWB Solids (CAD)

B-Rep 边界表示实体（半边数据结构 + Euler 算子），移植自经典 CAD 教学代码。  
B-Rep boundary representation solids (half-edge + Euler operators), ported from classic CAD educational code.

**内置场景 / Built-in Scenes:**

| 场景 | 内容 |
|------|------|
| Cylinder + Ball + Torus | 圆柱 + 球体 + 圆环（原始演示场景） |
| Block | 长方体 |
| Torus | 圆环体 |
| Ball | 球体 |
| Cylinder | 圆柱体 |

**显示模式 / Display Modes:**
- **3D Line** — 全部边线框（可看穿） / Full wireframe
- **3D Solid** — 背面剔除（仅显示正面边） / Back-face culled wireframe

**交互 / Controls:**
- 左键拖拽 / LMB drag — 旋转 / Rotate
- 滚轮 / Scroll — 缩放 / Zoom

---

## 目录结构 / Project Layout

```
DEMO1/
├── CMakeLists.txt          # 主构建文件 / Main build file
├── DEMO/
│   ├── main.cpp            # 原生入口 / Native entry point
│   └── main_wasm.cpp       # WASM 入口 / WASM entry point
├── SHOW/                   # 插件实现 / Plugin implementations
│   ├── IPlugin.hpp         # 插件接口 / Plugin interface
│   ├── ZoomSelector.hpp    # 橡皮筋缩放 / Rubber-band zoom
│   ├── Lorenz.{hpp,cpp}
│   ├── Mandelbrot.{hpp,cpp}
│   ├── Julia.{hpp,cpp}
│   ├── Newton.{hpp,cpp}
│   ├── Ecosys.{hpp,cpp}
│   ├── Henon.{hpp,cpp}
│   ├── May.{hpp,cpp}
│   ├── Trammel.{hpp,cpp}
│   ├── Lsys.{hpp,cpp}
│   ├── Jout.{hpp,cpp}
│   ├── Pfrac.{hpp,cpp}
│   ├── Eyes.{hpp,cpp}
│   ├── GraphView.{hpp,cpp}
│   ├── Diagram.{hpp,cpp}
│   ├── Geograph.{hpp,cpp}
│   ├── Geograph_presets.hpp
│   ├── Gwb.{hpp,cpp}
│   └── GWB/                # B-Rep 内核 C 代码 / B-Rep kernel C code
│       ├── gwb.h
│       ├── gwb_mat.{h,c}
│       ├── gwb_3d.{h,c}
│       ├── gwb_core.c
│       ├── gwb_euler.c
│       ├── gwb_adv.c
│       ├── gwb_geo.c
│       ├── gwb_trans.c
│       ├── gwb_undo.c
│       └── gwb_shgwb.c
├── EYES/
│   └── eyes.cpp            # 独立眼球程序 / Standalone eyes app
├── third_party/
│   └── imgui/              # Dear ImGui 源码 / Dear ImGui source
├── build/                  # 原生构建输出 / Native build output
│   └── bin/
│       ├── demo
│       └── eyes
└── build_wasm/             # WASM 构建输出 / WASM build output
    ├── CMakeLists.txt
    ├── serve.py            # 本地 HTTPS 服务器 / Local CORS-aware server
    ├── demo.html
    ├── demo.js
    └── demo.wasm
```

---

## 许可 / License

本项目为个人学习与演示用途。部分算法源自经典计算机图形学教材。  
This project is for personal learning and demonstration. Some algorithms are derived from classic computer graphics textbooks.
