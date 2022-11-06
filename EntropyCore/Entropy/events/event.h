#pragma once
#include <string>
#include <sstream>

#include "Entropy/core/utils.h"
#include "inputcodes.h"

namespace et
{
	/* Event enums and flags */
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	using EventCategory = uint32_t;
	namespace EventCategories
	{
		enum : uint32_t
		{
			None			= 0,
			Application		= 1,
			Input			= 1 << 1,
			Keyboard		= 1 << 2,
			Mouse			= 1 << 3,
			MouseButton		= 1 << 4
		};
	}

	/* macros to make defining event easier */
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
virtual EventType GetEventType() const override { return GetStaticType(); }\
virtual const char* GetName() const override { return #type;}

#define EVENT_CLASS_CATEGORY(category) virtual int32_t GetCategoryFlags() const override { return category; }

	/* base event */
	class Event
	{
	public:
		virtual ~Event() = default;

		bool handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int32_t GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

	};

	/* event dispatcher */
	class EventDispatcher
	{
	public:
		EventDispatcher(Event& e)
			: mEvent(e)
		{}

		template <typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (mEvent.GetEventType() == T::GetStaticType())
			{
				mEvent.handled |= func(static_cast<T&>(mEvent));
				return true;
			}
			return false;
		}
	private:
		Event& mEvent;
	};

	/* operator overload */
	template <typename ostream>
	inline ostream& operator<<(ostream& stream, const Event& e)
	{
		return stream << e.ToString();
	}
}