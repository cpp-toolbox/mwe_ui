#include "transform.hpp"

Transform::Transform() : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f) {}

const glm::vec3 &Transform::get_position() const { return position; }

const glm::vec3 &Transform::get_rotation() const { return rotation; }

const glm::vec3 &Transform::get_scale() const { return scale; }

void Transform::set_position(const glm::vec3 &pos) { position = pos; }

void Transform::set_rotation(const glm::vec3 &rot) { rotation = rot; }

void Transform::set_scale(const glm::vec3 &scl) { scale = scl; }

glm::mat4 Transform::get_transform_matrix() const {
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotate = glm::rotate(rotate, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotate = glm::rotate(rotate, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scale_mat = glm::scale(glm::mat4(1.0f), scale);

    return translate * rotate * scale_mat;
}

void Transform::print() const {
    std::cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")\n"
              << "Rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ")\n"
              << "Scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")\n";
}
