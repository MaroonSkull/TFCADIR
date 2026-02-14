# TFCADIR

for "the first CAD in Russia[, which has no alternatives]"

## Данный репозитарий содержит реализацию следующей задачи:

> Необходимо привести пример программы или библиотеки на C++, которая выводит на экран различные геометрические фигуры: круг, квадрат, треугольник, прямоугольник и т.п. Глубина проработки примера (количество фигур, параметры, методы) на Ваше усмотрение. Программа не обязательно должна запускаться и работать (хотя это будет плюсом).
> В задании Вам необходимо продемонстрировать умение использовать ООП. Просим не пользоваться библиотекой QT при выполнении задания.

---

## Features

TFCADIR is a full-featured CAD application with comprehensive UI/UX capabilities:

### Core Features
- **2D/3D Mode Switching** - Toggle between 2D sketch and 3D view modes (F2/F3)
- **Drawing Tools** - Line, Rectangle, Circle, Polygon, Triangle, Arc, Ellipse, Spline
- **Modify Tools** - Move, Rotate, Scale, Mirror, Fillet
- **Undo/Redo System** - Full command history with Ctrl+Z/Ctrl+Y

### UI Panels
- **Tools Panel** - Tool selection and configuration
- **Properties Panel** - Context-aware property editing
- **Outliner Panel** - Scene hierarchy view
- **Log Panel** - Application logging
- **Help Panel** - F1 context-sensitive help

### Advanced Features
- **Measurement Tools** - Distance, Angle, Area measurements with real-time display
- **Layer Management** - Create, delete, reorder layers with visibility/lock controls
- **Import/Export** - DXF and SVG format support
- **Grid & Snap** - Configurable grid with snap-to-grid functionality
- **Keyboard Shortcuts** - 30+ customizable shortcuts (F9 to configure)
- **Context Menus** - Right-click menus for canvas and objects
- **Themes** - Dark/Light theme support
- **Settings Persistence** - Automatic save/load of user preferences

### Help System
- **F1 Context Help** - Press F1 for context-sensitive help
- **Tooltips** - Hover over UI elements for hints
- **Tutorials** - Interactive step-by-step guides
- **Keyboard Shortcuts Reference** - F9 or Help menu

---

## Зависимости

Проанализировав задачу, было решено использоовать систему сборки CMake, OpenGL, а так же следующий набор библиотек:

| Библиотека                                                      | Причина выбора |
| ---                                                             | --- |
| [Embed Resource](https://github.com/MaroonSkull/embed-resource) | Предоставляет возможность запаковать ресурсы (шейдеры glsl) в бинарный файл при помощи CMake. |
| [glad](https://github.com/Dav1dde/glad)                         | Простой OpenGL loader, удобный в работе. |
| [glfwpp](https://github.com/janekb04/glfwpp)                    | Аналог классического glfw, но в более подходящем для C++ виде, в т.ч. с поддержкой удобных исключений. |
| [glm](https://github.com/g-truc/glm)                            | OpenGL Mathematics - математическая библиотека для графических приложений на базе спецификации OpenGL Shading Language. |
| [Dear ImGui](https://github.com/ocornut/imgui)                  | Мощная библиотека для быстрой разработки GUI, предоставляет приличное количество готовых элементов UI. |
| [spdlog](https://github.com/gabime/spdlog)                      | Очень быстрая библиотека для логгирования. |


### Для Ubuntu / Linux Mint / Debian

```bash
sudo apt update && sudo apt-get install xorg-dev libglu1-mesa-dev libspdlog-dev
```

Все прочие зависимости поставляются вместе с проектом в виде статических библиотек с помощью [git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules), за исключением glad.
Glad решено предварительно собрать и добавить в проект as is, т.к. его сборка требует Python в зависимостях.

---

### Arch Linux

```bash
sudo pacman -Sy glad spdlog glfw
```

## Сборка

На Windows исполнение команд в пунктах 1-2 требует установленного в систему git. Команды можно вводить в git bash, либо же в командную строку, если git прописан в PATH.

1. __Клонируйте репозиторий__
```bash
git clone --recurse-submodules https://github.com/MaroonSkull/TFCADIR
```

Следующие пункты специфичны для linux, на прочих платформах можно использовать аналогичные команды.

В Windows можно использовать VS2019+ для сборки cmake и компиляции цели TFCADIR.exe. Или можно использовать cmake с графическим интерфейсоум, с последующей компиляцией проекта в любой удобной IDE.

2. __Выполните конфигурацию и сборку проекта с помощью cmake__
```bash
cmake -B build && cmake --build build
```

3. __Запустите__
```
build/TFCADIR
```

---

## Архитектура проекта

В качестве основного архитектурного паттерна применён MVC
- Модель на данный момент хранит в себе данные о геометрических фигурах в 2D пространстве и позволяет выполнять их конструирование.
- Контроллер использует imgui, OpenGL (GLFWPP, glm) и служит для обработки событий пользовательского ввода, хранит в себе его состояние.
- Вью использует imgui, OpenGL (glad, GLFWPP, glm, Embed Resource) для отображения пользовательского интерфейса.


Запланирована возможность смены моделей, вью и контроллеров, что позволит в будущем масштабировать решение и внедрять поддержку:
- новых моделей (к примеру для поддержки 3д фигур и т.д.);
- новых способов отображения и взаимодействия с пользователем, к примеру - иные библиотеки для рендера GUI или библиотеки для работы с иными графическими API (vulkan, metal и проч.)


- [x] Реализован паттерн `Factory method`, который позволяет конструировать различные модели, контроллеры и вью.
- [x] Реализован паттерн `Memento`, который позволяет сохранять состояние модели и восстанавливать его, что даёт возможность реализовать действия `undo` (<kbd>Ctrl</kbd>+<kbd>Z</kbd>) и `redo` (<kbd>Ctrl</kbd>+<kbd>Y</kbd>)
- [x] Реализован паттерн `Observer`, который позволяет автоматически обновлять canvas при изменении данных модели (Вместо обновления канваса в каждом кадре)
- [x] Обработка ввода выполняется с помощью иерархических конечных автоматов

---

## Keyboard Shortcuts

| Category | Action | Shortcut |
|----------|--------|----------|
| **File** | New | Ctrl+N |
| | Open | Ctrl+O |
| | Save | Ctrl+S |
| | Save As | Ctrl+Shift+S |
| **Edit** | Undo | Ctrl+Z |
| | Redo | Ctrl+Y |
| | Cut | Ctrl+X |
| | Copy | Ctrl+C |
| | Paste | Ctrl+V |
| | Delete | Delete |
| | Duplicate | Ctrl+D |
| **View** | 2D Mode | F2 |
| | 3D Mode | F3 |
| | Zoom In | Ctrl+= |
| | Zoom Out | Ctrl+- |
| | Fit View | Ctrl+F |
| | Toggle Grid | G |
| | Toggle Snap | Shift+G |
| **Tools** | Select | Q |
| | Line | L |
| | Rectangle | R |
| | Circle | C |
| | Polygon | P |
| | Triangle | T |
| | Arc | A |
| | Ellipse | E |
| **Help** | Context Help | F1 |
| | Shortcuts Dialog | F9 |

Press **F9** in the application to customize keyboard shortcuts.

---

## Documentation

- [User Manual](docs/user_manual.md) - Complete user guide
- [API Reference](docs/api_reference.md) - Developer API documentation
- [Architecture](docs/architecture.md) - System architecture overview
- [Keyboard Shortcuts](docs/shortcuts_reference.md) - Full shortcuts reference