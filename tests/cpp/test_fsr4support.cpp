// Unit tests for the FSR4 runtime-selection auto-enable rule.

#include "Features/Upscaling/Fsr4Support.h"

#include <catch2/catch_test_macros.hpp>

using Fsr4::AdapterClass;

TEST_CASE("RX 6000 and 7000 auto-enable on a fresh settings blob", "[fsr4]")
{
	REQUIRE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx6000, 0, false));
	REQUIRE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx7000, 0, false));
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx9000, 0, false));
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::Unsupported, 0, false));
}

TEST_CASE("RX 6000 auto-enables on a blob that already ran the schema-1 migration", "[fsr4]")
{
	REQUIRE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx6000, 1, false));
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx7000, 1, false));
}

TEST_CASE("Migration does not re-run once the schema version is current", "[fsr4]")
{
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx6000, Fsr4::kRuntimeSelectionSchemaVersion, false));
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx6000, Fsr4::kRuntimeSelectionSchemaVersion + 1, false));
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx7000, Fsr4::kRuntimeSelectionSchemaVersion, false));
}

TEST_CASE("Migration leaves an already-enabled setting alone", "[fsr4]")
{
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx6000, 0, true));
	REQUIRE_FALSE(Fsr4::ShouldAutoEnable(AdapterClass::RadeonRx7000, 0, true));
}

TEST_CASE("A legacy enabled toggle promotes the method unconditionally", "[fsr4]")
{
	REQUIRE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx6000, 0, true));
	REQUIRE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx7000, 2, true));
	REQUIRE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx9000, 2, true));
	REQUIRE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::Unsupported, 0, true));
}

TEST_CASE("Method promotion without the legacy toggle follows the auto-enable rules", "[fsr4]")
{
	REQUIRE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx6000, 0, false));
	REQUIRE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx7000, 0, false));
	REQUIRE_FALSE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx7000, 2, false));
	REQUIRE_FALSE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx9000, 0, false));
	REQUIRE_FALSE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::Unsupported, 0, false));
	REQUIRE_FALSE(Fsr4::ShouldPromoteMethodToFsr4(AdapterClass::RadeonRx6000, Fsr4::kRuntimeSelectionSchemaVersion, false));
}
