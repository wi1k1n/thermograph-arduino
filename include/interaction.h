#ifndef INTERACTION_H__
#define INTERACTION_H__

// The interaction interface is a layer between the interaction implementation 
// (physical buttons with display or web-page sending commands) and the core
class InteractionInterface {

};

class HardwareInteraction : public InteractionInterface {

};

class WebserverInteraction : public InteractionInterface {

};

#endif // INTERACTION_H__