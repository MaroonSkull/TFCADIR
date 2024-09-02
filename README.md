# TFCADIR

for "the first CAD in Russia[, which has no alternatives]"

## Данный репозитарий содержит реализацию следующей задачи:

> Необходимо привести пример программы или библиотеки на C++, которая выводит на экран различные геометрические фигуры: круг, квадрат, треугольник, прямоугольник и т.п. Глубина проработки примера (количество фигур, параметры, методы) на Ваше усмотрение. Программа не обязательно должна запускаться и работать (хотя это будет плюсом).
> В задании Вам необходимо продемонстрировать умение использовать ООП. Просим не пользоваться библиотекой QT при выполнении задания.

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

```
sudo apt update
```
```
sudo apt-get install xorg-dev libglu1-mesa-dev
```

Все прочие зависимости поставляются вместе с проектом в виде статических библиотек с помощью [git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules), за исключением glad.
Glad решено предварительно собрать и добавить в проект as is, т.к. его сборка требует Python в зависимостях.

---

## Сборка

На Windows исполнение команд в пунктах 1-2 требует установленного в систему git. Команды можно вводить в git bash, либо же в командную строку, если git прописан в PATH.

1. __Клонируйте репозиторий__
```
git clone -b dev https://github.com/MaroonSkull/TopSystems
```
2. __Инициализируйте сабмодули__
```
cd TopSystems/
```
```
git submodule update --init --recursive
```

Следующие пункты специфичны для linux, на прочих платформах можно использовать аналогичные команды.

В Windows можно использовать VS2019+ для сборки cmake и компиляции цели TopSystems.exe. Или можно использовать cmake с графическим интерфейсоом, с последующей компиляцией проекта в любой удобной IDE.

3. __Выполните конфигурацию и сборку проекта с помощью cmake__
```
cmake .
```

sudo необходим для сборки spdlog, подключенного с помощью `ExternalProject_Add`.

```
sudo cmake --build .
```
4. __Запустите__
```
./TopSystems
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


- [ ] Реализован паттерн `Factory method`, который позволяет конструировать различные модели, контроллеры и вью.
- [ ] Реализован паттерн `Memento`, который позволяет сохранять состояние модели и восстанавливать его, что даёт возможность реализовать действия `undo` (<kbd>Ctrl</kbd>+<kbd>z</kbd>) и `redo` (<kbd>Ctrl</kbd>+<kbd>y</kbd>)
- [ ] Реализован паттерн `Observer`, который позволяет автоматически обновлять canvas при изменении данных модели (Вместо обновления канваса в каждом кадре)