#pragma once

#include <cstdint>
#include <vector>

namespace world {

class Map{

public:
  using cell_t = std::uint8_t;

  Map() = default;
  Map(int width, int height, std::vector<cell_t> cells);

  int width() const noexcept;
  int height() const noexcept;
  
  bool in_bounds(int x, int y) const noexcept;

  cell_t cell(int x, int y) const noexcept;
  bool is_wall(int x, int y) const noexcept;

private:
  int m_width = 0;
  int m_height = 0;
  std::vector<cell_t> m_cells;

  int index(int x, int y) const noexcept;

};

}
