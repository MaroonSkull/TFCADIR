#pragma once
#include <spdlog/spdlog.h>

#include <string>
#include <memory>
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

	class FlatFigures {
	public:
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