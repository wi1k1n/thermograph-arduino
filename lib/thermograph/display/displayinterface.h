#ifndef DISPLAYINTERFACE_H__
#define DISPLAYINTERFACE_H__

#include <Arduino.h>

class DisplayInterface {
public:
	virtual bool init();
	virtual void tick();
	virtual void disable();

	virtual bool isEnabled() const;
};

#endif // DISPLAYINTERFACE_H__