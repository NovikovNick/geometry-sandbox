

#include <gtest/gtest.h>

#include "core/json_service.h"
#include "core/log_manager.h"
#include "core/settings.h"

#include <string>

namespace gs
{
namespace
{
auto& getTestDependencyInjectionContext()
{
	static auto ctx = []()
	{
		return boost::di::make_injector(boost::di::bind<Settings>().in(boost::di::singleton),  //
										di::logManager(),
										di::jsonService());
	}();
	return ctx;
};
}  // namespace

TEST(isValidCoord2dArrayString, validate_ratio_numbers)
{
	// arrange
	auto& ctx					= getTestDependencyInjectionContext();
	const IJsonService& service = ctx.create<const IJsonService&>();
	std::string input			= "[[3,42],[-4,0.5]]";

	// act
	bool valid = service.isValidCoord2dArrayString(input);

	// assert
	EXPECT_TRUE(valid);
}

TEST(isValidCoord2dArrayString, validate_with_spaces)
{
	// arrange
	auto& ctx					= getTestDependencyInjectionContext();
	const IJsonService& service = ctx.create<const IJsonService&>();
	std::string input			= "  [  [  3  ,  0  ]  ,  [  -4  ,  0.5  ]  ]  ";

	// act
	bool valid = service.isValidCoord2dArrayString(input);

	// assert
	EXPECT_TRUE(valid);
}

TEST(isValidCoord2dArrayString, validate_invalid_bracers)
{
	// arrange
	auto& ctx					= getTestDependencyInjectionContext();
	const IJsonService& service = ctx.create<const IJsonService&>();
	std::string input			= "[3,42],[-4,0.5]]";

	// act
	bool valid = service.isValidCoord2dArrayString(input);

	// assert
	EXPECT_FALSE(valid);
}

TEST(isValidCoord2dArrayString, validate_invalid_numbers)
{
	// arrange
	auto& ctx					= getTestDependencyInjectionContext();
	const IJsonService& service = ctx.create<const IJsonService&>();
	std::string input			= "[[3],[-4,0.5]]";

	// act
	bool valid = service.isValidCoord2dArrayString(input);

	// assert
	EXPECT_FALSE(valid);
}

TEST(isValidCoord2dArrayString, validate_valid_big_intput)
{
	// arrange
	auto& ctx					= getTestDependencyInjectionContext();
	const IJsonService& service = ctx.create<const IJsonService&>();
	std::string
		input = "[[14.21,3.87],[7.34,12.91],[0.56,8.23],[5.89,4.12],[13.45,1.78],[2.31,14.67],[9.87,6.54],[11.23,9.01],[3.45,2.89],[6."
				"78,13.4],[14.56,7.89],[1.23,5.67],[8.90,0.34],[12.34,14.78],[4.56,11.23],[10.12,3.45],[0.89,1.23],[13.67,10.34],[5."
				"43,7.12],[9.01,15],[2.78,9.87],[14.89,5.43],[7.12,2.34],[11.45,12.31],[3.67,0.98],[6.23,14.56],[12.78,8.90],[1.45,13."
				"67],[8.34,4.56],[10.6,1.4],[14.00,11.78],[4.12,6.23],[9.45,10.12],[2.34,3.67],[13.23,14.00],[6.54,8.34],[11.89,5.67],"
				"[0.34,12.78],[7.6,0.12],[12.1,7.3],[3.89,15.00],[8.56,13.89],[14.34,2.12],[5.78,9.45],[10.23,14.34],[1.67,7.67],[9."
				"34,1.78],[13.78,6.54],[4.90,4.90],[7.4,11.2],[11.67,0.56],[2.12,10.67],[6.90,3.89],[12.56,13.45],[3.34,8.90],[8.78,5."
				"78],[14.12,9.34],[5.01,1.23],[10.45,12.12],[0.7,4.3],[13.34,0.67],[7.89,14.89],[11.12,7.89],[4.34,13.34],[9.78,3.12],"
				"[2.89,0.45],[14.67,8.12],[6.12,5.01],[12.45,11.6],[3.1,14.1],[8.45,9.78],[13.89,4.56],[5.67,12.45],[10.89,2.89],[1."
				"34,6.12],[7.23,1.67],[11.78,13.89],[4.01,10.23],[9.23,7.4],[14.4,12.6],[2.67,2.34],[6.45,0.89],[12.89,3.78],[8.12,8."
				"12],[13.56,15.00],[5.34,5.34],[10.78,11.34],[3.01,1.78],[7.56,6.45],[11.34,4.1],[0.12,9.23],[14.78,13.67],[6.67,10."
				"78],[9.89,2.01],[4.78,7.56],[12.01,0.23],[8.67,14.34],[2.45,12.01],[3.4,12.18],[2.1,12.3]]";

	// act
	bool valid = service.isValidCoord2dArrayString(input);

	// assert
	EXPECT_TRUE(valid);
}

}  // namespace gs
