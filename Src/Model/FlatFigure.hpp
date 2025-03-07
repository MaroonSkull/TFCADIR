#pragma once
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <spdlog/spdlog.h>

#include <string>
#include <variant>
#include <vector>

namespace model {

	// todo добавить выбор точности, попробовать концепты из C++20
	struct Point {
		float x{ 0.f };
		float y{ 0.f };
		float z{ 0.f };
	};

	struct Triangle {
		const static std::string name;
		Point first;
		Point second;
		Point third;
	};

	struct Quad {
		const static std::string name;
		Point first;
		Point second;
		Point third;
		Point fourth;
	};

	struct Circle {
		const static std::string name;
		Point center;
		float radius;
	};

	struct Ngon {
		const static std::string name;
		Point center;
		Point first;
		float n;
		float radius;
	};

	struct CurveBezier3 {
		const static std::string name;
		Point start;
		Point end;
		Point first;
	};

	struct CurveBezier4 {
		const static std::string name;
		Point start;
		Point end;
		Point first;
		Point second;
	};

	enum class isScribed {
		no,
		inscribed,
		circumscribed
	};

	template <isScribed _scribed>
	struct ScribedTraits {
		static const bool value = true;
		float angle = 0.0f; // in radians, start from 0 at positive x axis across positive y to positive x = 2*pi, counterclock-wise
		float r = 0.0f;
	};
	template <> struct ScribedTraits<isScribed::no> {
		static const bool value = false;
	};

	// интрузивная структура, которая хранит сервисные данные о конкретной фигуре
	template <class _T, isScribed _scribed = isScribed::no>
	struct Figure : public _T {
	private:
		inline static uint32_t counter_{ 0 };
	public:
		// const приводит к C2280 в MSVC, т.к. конструктор присваивания автоматически удаляется
		uint32_t id_{};
		std::string name_{ _T::name + " №" + std::to_string(id_) };
		_T obj_;
		[[no_unique_address]] ScribedTraits<_scribed> Round_;
		Figure(float x = 0.0f, float y = 0.0f, float z = 0.0f) {
			id_ = counter_++;
			if constexpr (Round_.value) {
				if constexpr (std::is_same_v<_T, Triangle>) {
					spdlog::info("triangle by round");
				}
			}
			else {
				if constexpr (std::is_same_v<_T, Triangle>) {
					spdlog::info("triangle by points");
				}
			}
		}
	};

	class Memento;

	/**
	 * @brief Полная хуйня, которая хранит в себе всё.
	 * @details Хранит в себе:
	 * - все фигуры в виде вариантов.
	 * - координатную систему как таковую.
	 * - координаты мыши.
	 * - инвариантная матрица вьюпроекции и модели.
	 * @todo рефакторинг обязателен, перейти на интерфейсы
	 */
	class FlatFigures {
	public:
		// Ещё хочу добавить мнимые точки для привязок
		using allFigures_t = std::variant<
			Figure<Triangle>,
			Figure<Triangle, isScribed::inscribed>,
			Figure<Triangle, isScribed::circumscribed>,
			Figure<Quad>,
			Figure<Quad, isScribed::inscribed>,
			Figure<Quad, isScribed::circumscribed>,
			Figure<Circle>,
			Figure<Ngon, isScribed::inscribed>,
			Figure<Ngon, isScribed::circumscribed>,
			Figure<CurveBezier3>,
			Figure<CurveBezier4>
		>;

	private:
		std::vector<allFigures_t> Figures_;
		glm::mat4 inverseMVP_{};// = glm::inverse(MVP); // = projection * view * model

	public:
		glm::mat4 model_{ 1.0f };
		glm::mat4 projection_{ 1.0f };

		struct Camera_t {
			glm::vec3 position{ 0.0f, 0.0f, -1.0f };
			glm::vec3 target{ 0.0f, 0.0f, 0.0f };
			glm::vec3 up{ 0.0f, 1.0f, 0.0f };
		};

		Camera_t camera_{}; // view = glm::lookAt(camera_.position, camera_.target, camera_.up);

		struct Mouse_t {
			glm::vec2 NDC{ 0.0f, 0.0f };
			glm::vec2 clipSpace{ 0.0f, 0.0f };
			glm::vec2 viewSpace{ 0.0f, 0.0f };
			glm::vec2 worldSpace{ 0.0f, 0.0f };
		};

		Mouse_t mouse_{};

		void setMouseCoordinates(glm::vec2 absoluteMousePos) {
			// Преобразование абсолютных координат мыши в NDC
			glm::vec4 ndcPosition = glm::inverse(getInverseMVP()) * glm::vec4(absoluteMousePos.x, absoluteMousePos.y, 0.0f, 1.0f);
			ndcPosition /= ndcPosition.w;

			// Заполнение NDC координат
			mouse_.NDC = glm::vec2(ndcPosition);

			// Преобразование NDC координат обратно в clipSpace
			glm::vec4 clipPosition = glm::vec4(ndcPosition.x, ndcPosition.y, -1.0f, 1.0f);
			clipPosition = clipPosition * 0.5f + 0.5f; // Нормализация в диапазон [0, 1]

			// Заполнение clipSpace координат
			mouse_.clipSpace = glm::vec2(clipPosition);

			// Преобразование clipSpace координат в viewSpace
			glm::vec4 viewPosition = glm::inverse(projection_) * glm::vec4(clipPosition.x, clipPosition.y, 0.0f, 1.0f);

			// Заполнение viewSpace координат
			mouse_.viewSpace = glm::vec2(viewPosition);

			// Заполнение worldSpace координат и установка z = 0
			mouse_.worldSpace = glm::vec2(viewPosition.x, viewPosition.y);
		}

		glm::mat4 getInverseMVP() {
			inverseMVP_ = glm::inverse(projection_ * getView() * model_);
			return inverseMVP_;
		}

		glm::mat4 getView() {
			return glm::lookAt(camera_.position, camera_.target, camera_.up);
		}

		allFigures_t getFlatFigure(uint32_t id) {
			return Figures_[id];
		}

		allFigures_t findFlatFigureByCoords(glm::vec2 coords, float delta) {
			for (const auto & figure : Figures_) {
				if (std::holds_alternative<Figure<Triangle>>(figure)) {
					/*auto triangle = std::get<Figure<Triangle>>(figure);
					if (triangle.obj_.first.x - triangle.obj_.radius < coords.x + delta && triangle.obj_.first.x + triangle.obj_.radius > coords.x - delta &&
						triangle.obj_.first.y - triangle.obj_.radius < coords.y + delta && triangle.obj_.first.y + triangle.obj_.radius > coords.y - delta) {
						return figure;	
					}*/
				}
			}
			return Figures_[0]; // todo: remove
		}

	private:

		template<class... Ts>
		struct overloaded : Ts... { using Ts::operator()...; };
		template<class... Ts>
		overloaded(Ts...) -> overloaded<Ts...>;

	public:
		FlatFigures() = default;
		~FlatFigures() = default;

		template <class T>
		void createFigure(float x, float y) {
			Figures_.push_back(Figure<T>{});
			auto figure = Figures_.back();
			if constexpr (std::is_same_v<T, Triangle>) {
				spdlog::info("Triangle in model");
				// тут сохраняем все три точки в одном месте
			}
		}

		/*std::unique_ptr<Memento> createMemento() {
			return std::make_unique<Memento>(Memento(Figures_));
		}

		void restoreFromMemento(const Memento& Mem) {
			Figures_ = Mem.Figures_;
		}*/
	};

	class Memento {
		friend class FlatFigures;
	private:
		std::vector<FlatFigures::allFigures_t> Figures_;
	public:
		Memento(const std::vector<FlatFigures::allFigures_t>& Figures) : Figures_(Figures) {}
		~Memento() = default;
	};

}

// наследовать всё от фигуры, полиморфизм вместо варианта
// вектор полиморфных указателей, тогда надо сильно меньше шаблонов, код проще