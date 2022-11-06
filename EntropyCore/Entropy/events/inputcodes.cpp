#include <etpch.h>
#include "inputcodes.h"

namespace et
{
	using namespace Key;
	std::string KeyCodeToString(KeyCode key)
	{
		switch (key)
		{
		case Space:
			return "Space";
			break;
		case Apostrophe:
			return "Apostrophe";
			break;
		case Comma:
			return "Comma";
			break;
		case Minus:
			return "Minus";
			break;
		case Period:
			return "Period";
			break;
		case Slash:
			return "Slash";
			break;
		case D0:
			return "D0";
			break;
		case D1:
			return "D1";
			break;
		case D2:
			return "D2";
			break;
		case D3:
			return "D3";
			break;
		case D4:
			return "D4";
			break;
		case D5:
			return "D5";
			break;
		case D6:
			return "D6";
			break;
		case D7:
			return "D7";
			break;
		case D8:
			return "D8";
			break;
		case D9:
			return "D9";
			break;
		case Semicolon:
			return "Semicolon";
			break;
		case Equal:
			return "Equal";
			break;
		case A:
			return "A";
			break;
		case B:
			return "B";
			break;
		case C:
			return "C";
			break;
		case D:
			return "D";
			break;
		case E:
			return "E";
			break;
		case F:
			return "F";
			break;
		case G:
			return "G";
			break;
		case H:
			return "H";
			break;
		case I:
			return "I";
			break;
		case J:
			return "J";
			break;
		case K:
			return "K";
			break;
		case L:
			return "L";
			break;
		case M:
			return "M";
			break;
		case N:
			return "N";
			break;
		case O:
			return "O";
			break;
		case P:
			return "P";
			break;
		case Q:
			return "Q";
			break;
		case R:
			return "R";
			break;
		case S:
			return "S";
			break;
		case T:
			return "T";
			break;
		case U:
			return "U";
			break;
		case V:
			return "V";
			break;
		case W:
			return "W";
			break;
		case X:
			return "X";
			break;
		case Y:
			return "Y";
			break;
		case Z:
			return "Z";
			break;
		case LeftBracket:
			return "LeftBracket";
			break;
		case Backslash:
			return "Backslash";
			break;
		case RightBracket:
			return "RightBracket";
			break;
		case GraveAccent:
			return "GraveAccent";
			break;
		case World1:
			return "World1";
			break;
		case World2:
			return "World2";
			break;
		case Escape:
			return "Escape";
			break;
		case Enter:
			return "Enter";
			break;
		case Tab:
			return "Tab";
			break;
		case Backspace:
			return "Backspace";
			break;
		case Insert:
			return "Insert";
			break;
			case Delete:   
				return "Delete";
				break;
			case Right                :
				return "Right";
				break;
			case Left                 :
				return "Left";
				break;
			case Down                 :
				return "Down";
				break;
			case Up                   :
				return "Up";
				break;
			case PageUp               :
				return "PageUp";
				break;
			case PageDown             :
				return "PageDown";
				break;
			case Home                 :
				return "Home";
				break;
			case End                  :
				return "End";
				break;
			case CapsLock             :
				return "CapsLock";
				break;
			case ScrollLock           :
				return "ScrollLock";
				break;
			case NumLock              :
				return "NumLock";
				break;
			case PrintScreen          :
				return "PrintScreen";
				break;
			case Pause                :          
				return "Pause";
				break;
			case F1                   :
				return "F1";
				break;
			case F2                   :
				return "F2";
				break;
			case F3                   :
				return "F3";
				break;
			case F4                   :
				return "F4";
				break;
			case F5                   :
				return "F5";
				break;
			case F6                   :
				return "F6";
				break;
			case F7                   :
				return "F7";
				break;
			case F8                   :
				return "F8";
				break;
			case F9                   :
				return "F9";
				break;
			case F10                  :
				return "F10";
				break;
			case F11                  :
				return "F11";
				break;
			case F12                  :
				return "F12";
				break;
			case F13                  :
				return "F13";
				break;
			case F14                  :          
				return "F14";
				break;
			case F15                  :
				return "F15";
				break;
			case F16                  :
				return "F16";
				break;
			case F17                  :
				return "F17";
				break;
			case F18                  :
				return "F18";
				break;
			case F19                  :
				return "F19";
				break;
			case F20                  :
				return "F20";
				break;
			case F21                  :
				return "F21";
				break;
			case F22                  :
				return "F22";
				break;
			case F23                  :
				return "F23";
				break;
			case F24                  :
				return "F24";
				break;
			case F25                  :
				return "F25";
				break;
			case KP0                  :
				return "KP0";
				break;
			case KP1                  :
				return "v";
				break;
			case KP2                  :
				return "KP2";
				break;
			case KP3                  :
				return "KP3";
				break;
			case KP4                  :
				return "KP4";
				break;
			case KP5                  :
				return "KP5";
				break;
			case KP6                  :
				return "KP6";
				break;
			case KP7                  :
				return "KP7";
				break;
			case KP8                  :
				return "KP8";
				break;
			case KP9                  :
				return "KP9";
				break;
			case KPDecimal            :
				return "KPDecimal";
				break;
			case KPDivide             :
				return "KPDivide";
				break;
			case KPMultiply           :
				return "KPMultiply";
				break;
			case KPSubtract           :
				return "KPSubtract";
				break;
			case KPAdd                :
				return "KPAdd";
				break;
			case KPEnter              :
				return "KPEnter";
				break;
			case KPEqual              :
				return "KPEqual";
				break;
			case LeftShift            :
				return "LeftShift";
				break;
			case LeftControl          :
				return "LeftControl";
				break;
			case LeftAlt              :
				return "LeftAlt";
				break;
			case LeftSuper            :
				return "LeftSuper";
				break;
			case RightShift           :
				return "RightShift";
				break;
			case RightControl         :
				return "RightControl";
				break;
			case RightAlt             :
				return "RightAlt";
				break;
			case RightSuper           :
				return "RightSuper";
				break;
			case Menu                 :
				return "Menu";
				break;
			default:
				return "";
				break;
		}
		return "";
	}

	using namespace Mouse;
	std::string MouseCodeToString(MouseCode code)
	{
		switch (code)
		{
			case Button0:
				return "ButtonLeft";
				break;
			case Button1:
				return "ButtonRight";
				break;
			case Button2:
				return "ButtonMiddle";
				break;
			case Button3:
				return "Button3";
				break;
			case Button4:
				return "Button4";
				break;
			case Button5:
				return "Button5";
				break;
			case Button6:
				return "Button6";
				break;
			case Button7:
				return "ButtonLast";
				break;
			default:
				return "";
				break;
		}
		return "";
	}
}