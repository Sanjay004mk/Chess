#pragma once
#include <glm/glm.hpp>

#include "Entropy/events/inputcodes.h"

namespace et
{
	/// <summary>
	/// Static class used to query key and mouse states, get mouse position and lock / unlock cursor 
	/// </summary>
	class Input
	{
	public:
		/// <summary>
		/// Checks if key state == down
		/// </summary>
		/// <param name="key">Keycode defined in namespace &apos;Key&apos;</param>
		/// <returns>Return true if key state == down, else returns false</returns>
		static bool IsKeyDown(KeyCode key);

		/// <summary>
		/// Checks if mouse button state == down
		/// </summary>
		/// <param name="button">Mousecode defined in namespace &apos;MouseKey&apos;</param>
		/// <returns>Returs true if mouse button state == down, else returns false</returns>
		static bool IsMouseButtonDown(MouseCode button);
		
		/// <summary>
		/// Retrieves the position of the cursor relative to the content area of the window.
		/// Is Cursor is locked (Input::LockMouseCursor()), the cursor position is limited only by the max value of float
		/// </summary>
		/// <returns>Returns the position of the cursor, in screen coordinates, relative to the upper - left corner of the window</returns>
		static glm::vec2 GetMousePosition();

		/// <summary>
		/// Retrieves the position of the window
		/// </summary>
		/// <returns>Returns the position of the upper - left corner of the window, in screen cordinates</returns>
		static glm::vec2 GetWindowPosition();

		/// <summary>
		/// Retrieves the x position of the cursor relative to the content area of the window.
		/// Is Cursor is locked (Input::LockMouseCursor()), the cursor position is limited only by the max value of float
		/// </summary>
		/// <returns>Returns the x position of the cursor, in screen coordinates, relative to the upper - left corner of the window</returns>
		static float GetMouseX();

		/// <summary>
		/// Retrieves the y position of the cursor relative to the content area of the window.
		/// Is Cursor is locked (Input::LockMouseCursor()), the cursor position is limited only by the max value of float
		/// </summary>
		/// <returns>Returns the y position of the cursor, in screen coordinates, relative to the upper - left corner of the window</returns>
		static float GetMouseY();

		/// <summary>
		/// Hides the cursor. The cursor can still move and is unbouned (can take values upto float::max or float::min)
		/// </summary>
		static void LockMouseCursor();

		/// <summary>
		/// Shows the cursor. The cursor can move within bounds of the monitor(s)
		/// </summary>
		static void UnLockMouseCursor();
	};
}