/**
 * @file handle.h
 * @brief Lightweight handle for controlling a playing animation instance.
 * @author MetalHeart
 */
#ifndef GEOMETRY_SANDBOX_ANIMATION_HANDLE_H
#define GEOMETRY_SANDBOX_ANIMATION_HANDLE_H

#include "animation/types.h"

namespace gs::animation
{
class IManager;

/**
 * @brief Lightweight handle for controlling a playing animation instance.
 *
 * Acts as a "smart reference" to an active animation, hiding the internal
 * structure of the animation manager and raw identifier access from client code.
 * Binds together a pointer to the owning gs::animation::IManager and a unique @ref gs::animation::InstanceId.
 *
 * @author MetalHeart
 */
struct Handle
{
	IManager* manager;
	InstanceId animationId;

	/** @return true if animation active in manager */
	bool isValid() const;

	const Instance& getAnimationInstance() const;
	void setProgress(float progress) const;

	void pause() const;
	void resume() const;

	/** @brief stops and removes the playing animation, invalidates this handle */
	void stop();

	/** @brief resume if paused and play forward */
	void playForward() const;

	/** @brief resume if paused and play backward */
	void playBackward() const;

	/** @brief sets the progress value to the value of the next marker, relative to the current progress */
	void stepForwardToNextMarker() const;

	/** @brief sets the progress value to the value of the previous marker, relative to the current progress */
	void stepBackToPrevMarker() const;
};
}  // namespace gs::animation
#endif	// GEOMETRY_SANDBOX_ANIMATION_HANDLE_H
