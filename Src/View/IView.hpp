#pragma once

class IView {
public:
	virtual inline bool shouldClose() const = 0;
	virtual void draw() = 0;
};