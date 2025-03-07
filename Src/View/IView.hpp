#pragma once

// Observer
// #include <boost/signals2.hpp>

// static boost::signals2::signal<void(void)> mustBeRedrawSignal;

class IView {
public:
	virtual inline bool shouldClose() const = 0;
	virtual void draw() = 0;
};