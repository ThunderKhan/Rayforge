#include "inclde/world/Map.hpp"
#include <vector>
#include <utility>

namespace world {
  Map::Map(int width, int height, std::vector<cell_t> cells)
      : m_width(width), m_height(height), m_cells(std::move(cells)) {}

  int Map::width() const noexcept {
      return m_width;
  }

  int Map::height() const noexcept {
      return m_height;
  }

  int Map::index(int x, int y) const noexcept {
      return y * m_width + x;
  }
  
  bool Map::in_bounds(int x, int y) const noexcept {
      return x >= 0 && x < m_width && y >= 0 && y < m_height;
  }

  Map::cell_t Map::cell(int x, int y) const noexcept {
      if (!in_bounds(x, y)) {
          return 1;
      }
      return m_cells[index(x, y)];
  }

  bool Map::is_wall(int x, int y) const noexcept {
      return cell(x, y) != 0;
  }

}
