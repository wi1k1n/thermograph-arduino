#ifndef DISPLAY_MANAGER_H__
#define DISPLAY_MANAGER_H__

#include <Arduino.h>

#include <unordered_map>
#include <memory>

#include "software/display_layout.h"

enum class DisplayLayoutKey {
	NONE = -1,
	WELCOME = 0,
};

template<class HWDISPLAY>
class DisplayManager {
public:
	DisplayManager();

	bool init();
	void tick();

	bool activateLayout(DisplayLayoutKey key);
private:
	HWDISPLAY _display;
	std::unordered_map<DisplayLayoutKey, std::unique_ptr<DisplayLayoutInterface>> _layouts;
	DisplayLayoutInterface* _activeLayout = nullptr;
};

#endif // DISPLAY_MANAGER_H__