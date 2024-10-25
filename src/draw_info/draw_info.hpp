#ifndef DRAW_INFO_HPP
#define DRAW_INFO_HPP

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "../transform/transform.hpp"

class IndexedVertexPositions {
  public:
    IndexedVertexPositions(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions)
        : indices(indices), xyz_positions(xyz_positions) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
};

class IVPSolidColor {
  public:
    IVPSolidColor(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                  std::vector<glm::vec3> rgb_colors)
        : indices(indices), xyz_positions(xyz_positions), rgb_colors(rgb_colors) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<glm::vec3> rgb_colors;
};

class IVPTextured {
  public:
    IVPTextured(std::vector<unsigned int> indices, std::vector<glm::vec3> xyz_positions,
                std::vector<glm::vec2> texture_coordinates)
        : indices(indices), xyz_positions(xyz_positions), texture_coordinates(texture_coordinates) {};
    Transform transform;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> xyz_positions;
    std::vector<glm::vec2> texture_coordinates;
    std::string texture;
};

#endif // DRAW_INFO_HPP
