#include "display/dlayouts/dl_graph.h"
#include "application.h"

void DLayoutGraph::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutGraph::tick() {
	DisplayLayout::tick();
	
	PushButton* btn1 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON1));
	PushButton* btn2 = static_cast<PushButton*>(_inputs->getInput(HardwareInputs::HardwareInputChannel::BUTTON2));
	if (!btn1 || !btn2)
		return;

	if (!btn1->tick() && !btn2->tick()) {
		return;
	}

	if (!btn2->down() && btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
	if (!btn1->down() && btn2->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::SETTINGS);
		return;
	}
}
void DLayoutGraph::draw(bool doDisplay) {
	DisplayLayout::draw(doDisplay);
	// DLOGLN();
	display()->clearDisplay();
	
	display()->setCursor(0, 0);
	display()->setTextSize(1);
	display()->print(F("Graph Layout"));

	if (doDisplay) {
		display()->display();
	}
}