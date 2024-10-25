#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Transform {
  public:
    Transform();

    // avoiding copying while making sure they canot be modified
    const glm::vec3 &get_position() const;
    const glm::vec3 &get_rotation() const;
    const glm::vec3 &get_scale() const;

    void set_position(const glm::vec3 &pos);
    void set_rotation(const glm::vec3 &rot);
    void set_scale(const glm::vec3 &scl);

    glm::mat4 get_transform_matrix() const;

    void print() const;

  private:
    glm::vec3 position; // Position in 3D space
    glm::vec3 rotation; // Euler angles in degrees (pitch, yaw, roll)
    glm::vec3 scale;    // Scale factors
};

#endif // TRANSFORM_HPP
