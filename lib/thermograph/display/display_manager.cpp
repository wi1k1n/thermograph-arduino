#include "display_manager.h"
#include "sdk.h"

#include "hardware/hwdisplay_interface.h"

#include "software/display_layouts/dlwelcome.h"

#include "hardware/hwdisplay_ssd1351.h"
template class DisplayManager<HWDisplaySSD1351>;

template <class HWDISPLAY>
DisplayManager<HWDISPLAY>::DisplayManager() {
	static_assert(std::is_base_of<HWDisplayInterface, HWDISPLAY>::value, "HWDISPLAY is not derived from HDDisplayInterface");
}

template <class HWDISPLAY>
bool DisplayManager<HWDISPLAY>::init() {
	if (!_display.init())
		return false;
	
	_layouts.clear();
	_layouts.insert({DisplayLayoutKey::WELCOME, std::unique_ptr<DLWelcome>(new DLWelcome)});
	for (auto& dlayoutPair : _layouts)
		if (!dlayoutPair.second || !dlayoutPair.second->init(&_display))
			return false;

	return true;
}

template <class HWDISPLAY>
void DisplayManager<HWDISPLAY>::tick() {
	if (_activeLayout)
		_activeLayout->tick();
}

template <class HWDISPLAY>
bool DisplayManager<HWDISPLAY>::activateLayout(DisplayLayoutKey key) {
	auto entry = _layouts.find(key);
	if (entry == _layouts.end())
		return false;
	
	_activeLayout = entry->second.get();
	_activeLayout->draw();
	return true;
}
