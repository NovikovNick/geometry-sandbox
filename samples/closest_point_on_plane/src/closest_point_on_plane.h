/**
 * @file closest_point_on_plane.h
 * @brief Functions for running the closest_point_on_plane visualization.
 *
 * Provides functions to operate in both cyclic mode (infinite loop) and
 * explicit per-frame call mode.
 *
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_CLOSEST_POINT_ON_PLANE_EXAMPLE_H
#define GEOMETRY_SANDBOX_CLOSEST_POINT_ON_PLANE_EXAMPLE_H

namespace gs
{
void init();
void drawNextFrame();
void startInfiniteLoop();
}  // namespace gs

#endif	// GEOMETRY_SANDBOX_CLOSEST_POINT_ON_PLANE_EXAMPLE_H
