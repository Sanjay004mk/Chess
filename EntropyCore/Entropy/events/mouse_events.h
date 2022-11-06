#pragma once
#include "event.h"

namespace et
{
	/* input mouse moved event */
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float posX, float posY)
			: mX(posX), mY(posY)
		{}

		float GetX() const { return mX; }
		float GetY() const { return mY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << mX << ", " << mY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategories::Input | EventCategories::Mouse)

	private:
		float mX, mY;
	};

	/* input mouse scrolled event */
	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xoffs, float yoffs)
			: mXOffs(xoffs), mYOffs(yoffs)
		{}

		float GetXOffset() const { return mXOffs; }
		float GetYOffset() const { return mYOffs; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << mXOffs << ", " << mYOffs;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategories::Input | EventCategories::Mouse)

	private:
		float mXOffs, mYOffs;
	};

	/* input mouse button base event */
	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return mMouseCode; }

		EVENT_CLASS_CATEGORY(EventCategories::Input | EventCategories::MouseButton)

	protected:
		MouseButtonEvent(MouseCode code)
			: mMouseCode(code)
		{}

		MouseCode mMouseCode;
	};

	/* input mouse button pressed event */
	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode code)
			: MouseButtonEvent(code)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << ::et::MouseCodeToString(mMouseCode);
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	/* input mouse button released event */
	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseCode code)
			: MouseButtonEvent(code)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << ::et::MouseCodeToString(mMouseCode);
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}