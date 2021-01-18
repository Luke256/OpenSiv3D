//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2021 Ryo Suzuki
//	Copyright (c) 2016-2021 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

# include <Siv3D/Monitor.hpp>
# include <Siv3D/Common/OpenGL.hpp>

extern"C"
{
	GLFWAPI void glfwGetMonitorInfo_Siv3D(GLFWmonitor* handle, uint32_t* displayID, uint32_t* unitNumber,
										  int* xpos, int* ypos, int* w, int* h,
										  int* wx, int* wy, int* ww, int* wh);
}

namespace s3d
{
	namespace System
	{
		Array<Monitor> EnumerateActiveMonitors()
		{
			Array<Monitor> results;
			
			int32 numMonitors;
			GLFWmonitor** monitors = ::glfwGetMonitors(&numMonitors);
			
			for (int32 i = 0; i < numMonitors; ++i)
			{
				GLFWmonitor* monitor = monitors[i];

				uint32 displayID, unitNumber;
				int32 xPos, yPos, width, height;
				int32 wx, wy, ww, wh;
				glfwGetMonitorInfo_Siv3D(monitor, &displayID, &unitNumber,
										 &xPos, &yPos, &width, &height,
										 &wx, &wy, &ww, &wh);
				
				Optional<Size> sizeMillimeter;
				{
					int32 pw = 0, ph = 0;
					::glfwGetMonitorPhysicalSize(monitor, &pw, &ph);
					if (pw > 0 && ph > 0)
					{
						sizeMillimeter = Size(pw, ph);
					}
				}

				Optional<double> scaling;
				{
					float xscale = 1.0f, yscale = 1.0f;
					::glfwGetMonitorContentScale(monitor, &xscale, &yscale);
					if (const double scale = Max(xscale, yscale); scale > 0.0)
					{
						scaling = scale;
					}
				}
				
				Optional<double> refreshRate;
				{
					if (const GLFWvidmode* vidMode = ::glfwGetVideoMode(monitor))
					{
						if (vidMode->refreshRate > 0)
						{
							refreshRate = vidMode->refreshRate;
						}
					}
				}

				const Monitor info =
				{
					.name				= Unicode::Widen(::glfwGetMonitorName(monitor)),
					.id					= Format(displayID),
					.displayDeviceName	= Format(unitNumber),
					.displayRect		= Rect(xPos, yPos, width, height),
					.workArea			= Rect(wx, wy, ww, wh),
					.isPrimary			= (i == 0),
					.sizeMillimeter		= sizeMillimeter,
					.scaling			= scaling,
					.refreshRate		= refreshRate,
				};
				
				results.push_back(info);
			}
			
			return results;
		}
	}
}