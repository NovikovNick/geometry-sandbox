#include "animation/service.h"

#include "animation/dsl.h"
#include "animation/handle.h"
#include "animation/manager.h"
#include "animation/types.h"
#include "core/ecs.h"
#include "core/types.h"
#include "easing.h"

#include <algorithm>
#include <chrono>
#include <functional>

namespace gs::animation
{
ReversableAnimation Service::createHoverPointAnimation(entt::entity entity)
{
	// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
	using namespace animation::dsl;

	const auto& color		 = registry_->get<ecs::component::MainColor>(entity);
	const Color hoveredColor = Color::cian();

	// it looks like a clang-tidy bug to expect keyframes as const variable. Need to investigate it properly
	KeyframeCollection keyframes;  // NOLINT(misc-const-correctness)
	keyframes += keyframe(0ms) | accessor::point(entity) / point::color << color.val;
	keyframes += keyframe(300ms) | accessor::point(entity) / point::color << hoveredColor;

	return ReversableAnimation{animationManager_->buildAndPlay(keyframes)};
	// NOLINTEND(*-using-namespace, *-magic-numbers)
}

std::function<void()> Service::createSelectPointAnimation(entt::entity entity)
{
	return [&, entity]
	{
		// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
		using namespace animation::dsl;

		const auto& radius = registry_->get<ecs::component::Radius>(entity);

		KeyframeCollection keyframes;
		keyframes += keyframe(0s) | accessor::point(entity) / point::radius << radius.val;
		keyframes += keyframe(100ms) | accessor::point(entity) / point::radius << radius.val + 0.05F;
		keyframes += keyframe(700ms) | accessor::point(entity) / point::radius << key(radius.val, bounce::out);
		animationManager_->buildAndPlay(keyframes, [](Clip& clip) { clip.removeOnComplete = true; });
		// NOLINTEND(*-using-namespace, *-magic-numbers)
	};
}

ReversableAnimation Service::createHoverPlaneAnimation(entt::entity entity)
{
	// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
	using namespace animation::dsl;

	const auto& planeColor	 = registry_->get<ecs::component::MainColor>(entity);
	const float opacityBegin = planeColor.val.a;
	const float opacityEnd	 = std::min(planeColor.val.a * 2, 255.0F);

	// it looks like a clang-tidy bug to expect keyframes as const variable. Need to investigate it properly
	KeyframeCollection keyframes;  // NOLINT(misc-const-correctness)
	keyframes += keyframe(0s) | accessor::plane(entity) / plane::color / color::a << opacityBegin;
	keyframes += keyframe(300ms) | accessor::plane(entity) / plane::color / color::a << opacityEnd;

	return ReversableAnimation{animationManager_->buildAndPlay(keyframes)};
	// NOLINTEND(*-using-namespace, *-magic-numbers)
}

ReversableAnimation Service::createHoverButtonAnimation(const ui::Element& elem)
{
	constexpr PropertyPath arg0			= dsl::accessor::button(0);	 // strange name cause it is placeholder

	const static AssetCollection assets = [&]
	{
		// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
		using namespace animation::dsl;

		KeyframeCollection keyframes;
		keyframes += keyframe(0s) | arg0 / btn::color << settings_->buttonColor;
		keyframes += keyframe(300ms) | arg0 / btn::color << settings_->buttonColorHover;
		return animationManager_->build(keyframes);
		// NOLINTEND(*-using-namespace, *-magic-numbers)
	}();

	Clip clip{.duration = assets.duration};
	clip.animations.reserve(assets.map.size());
	clip.animations.emplace_back(dsl::accessor::button(elem.id).target, assets.map.find(arg0.target)->second);

	const Handle handle = animationManager_->play(clip);
	return ReversableAnimation{handle};
}

void Service::playAppearUIAnimation()
{
	// NOLINTBEGIN(*-using-namespace, *-magic-numbers)
	using namespace animation::dsl;

	KeyframeCollection keyframes;
	keyframes += keyframe(0s) | dsl::accessor::ui() / dsl::layout::opacity << 0.0F;
	keyframes += keyframe(2s) | dsl::accessor::ui() / dsl::layout::opacity << key(1.0F, cubic::in);
	animationManager_->buildAndPlay(keyframes, [](Clip& clip) { clip.removeOnComplete = true; });
	// NOLINTEND(*-using-namespace, *-magic-numbers)
}

}  // namespace gs::animation
