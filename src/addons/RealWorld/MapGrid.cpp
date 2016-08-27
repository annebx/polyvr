#include "MapGrid.h"
#include <boost/format.hpp>

using namespace OSG;

MapGrid::Box::Box() {}

MapGrid::Box::Box(Vec2f min, float size) {
    this->min = min;
    this->max = min + Vec2f(size, size);
    this->str = (boost::format("%.3f") % (round(min[0]*1000) / 1000)).str() + "-" +
                (boost::format("%.3f") % (round(min[1]*1000) / 1000)).str();
}

bool MapGrid::Box::same(MapGrid::Box* b) { return str == b->str; }


MapGrid::MapGrid(int d, float s) : dim(d), size(s) {
    for (int i=0; i<dim*dim; i++) grid.push_back( Box() );
    update();
}

void MapGrid::set(Vec2f p) { position = p; update(); }

vector<MapGrid::Box>& MapGrid::getBoxes() { return grid; }

bool MapGrid::has(Box& box) {
    for(auto b : grid) if (b.str == box.str) return true;
    return false;
}

void MapGrid::update() {
    for (int i=0; i<dim; i++) {
        for (int j=0; j<dim; j++) {
            int k = i*dim+j;
            int x = i-dim*0.5;
            int y = j-dim*0.5;
            grid[k] = Box(position + Vec2f(size*x, size*y), size);
        }
    }
}
