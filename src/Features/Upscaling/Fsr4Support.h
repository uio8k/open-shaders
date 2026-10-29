#pragma once

#include <cstdint>

/** @brief Adapter classification and settings-migration rules for the runtime FSR4 provider. */
namespace Fsr4
{
	/** @brief AMD adapter classes eligible to request a runtime FSR4 provider. */
	enum class AdapterClass
	{
		Unsupported,
		RadeonRx6000,
		RadeonRx7000,
		RadeonRx9000
	};

	/** @brief Bumped whenever the FSR4 selection rule changes so existing settings re-run the migration. */
	constexpr uint32_t kRuntimeSelectionSchemaVersion = 3;

	/**
	 * @brief True when a pre-migration settings blob should have fsr4RuntimeEnable turned on.
	 *
	 * RX 7000 stays on only for blobs that never ran the schema-1 migration; re-running it at
	 * schema 2 after the user turned the setting off would override that choice.
	 */
	constexpr bool ShouldAutoEnable(AdapterClass a_support, uint32_t a_schemaVersion, bool a_currentlyEnabled)
	{
		if (a_currentlyEnabled || a_schemaVersion >= kRuntimeSelectionSchemaVersion)
			return false;

		return a_support == AdapterClass::RadeonRx6000 ||
		       (a_schemaVersion == 0 && a_support == AdapterClass::RadeonRx7000);
	}

	/**
	 * @brief True when a pre-schema-3 blob should have its upscale method promoted kFSR -> kFSR4.
	 *
	 * A blob that had the legacy toggle on is promoted unconditionally (the user already chose
	 * runtime FSR4); otherwise the fresh-config auto-enable rules decide.
	 */
	constexpr bool ShouldPromoteMethodToFsr4(AdapterClass a_support, uint32_t a_schemaVersion, bool a_legacyRuntimeEnabled)
	{
		return a_legacyRuntimeEnabled || ShouldAutoEnable(a_support, a_schemaVersion, a_legacyRuntimeEnabled);
	}
}
