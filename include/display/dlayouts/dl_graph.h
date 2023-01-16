#ifndef DL_GRAPH_H__
#define DL_GRAPH_H__

#include "display/display_layout.h"

class DLayoutGraph : public DisplayLayout {
	float _temp1;
public:
	void draw(bool doDisplay = true) override;
	void update(void* data) override;
	void tick() override;
};

#endif // DL_GRAPH_H__