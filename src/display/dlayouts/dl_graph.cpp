#include "display/dlayouts/dl_graph.h"
#include "main.h"

void DLayoutGraph::update(void* data) {
	DisplayLayout::update(data);
}
void DLayoutGraph::tick() {
	DisplayLayout::tick();
	if (!_btn1->tick() && !_btn2->tick()) {
		return;
	}

	if (!_btn2->down() && _btn1->click()) {
		_app->activateDisplayLayout(DisplayLayoutKeys::MAIN);
		return;
	}
	if (!_btn1->down() && _btn2->click()) {
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