#pragma once
#include "event.h"

namespace et
{
	/* input keyboard base event */
	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return mKeyCode; }

		EVENT_CLASS_CATEGORY(EventCategories::Input | EventCategories::Keyboard)
	protected:
		KeyEvent(KeyCode code)
			: mKeyCode(code)
		{}

		KeyCode mKeyCode;
	};

	/* input key pressed event */
	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode code, bool isRepeated = false)
			: KeyEvent(code), mIsRepeated(isRepeated)
		{}

		bool IsRepeated() const { return mIsRepeated; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << ::et::KeyCodeToString(mKeyCode) << " [REPEATED = " << mIsRepeated << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		bool mIsRepeated;
	};

	/* input key released event */
	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode code)
			: KeyEvent(code)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << ::et::KeyCodeToString(mKeyCode);
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	/* input key typed event */
	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode code)
			: KeyEvent(code)
		{}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << (char)mKeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}