#include "polyscope/point_cloud.h"

#include "polyscope/gl/colors.h"
#include "polyscope/gl/shaders.h"
#include "polyscope/gl/shaders/sphere_shaders.h"
#include "polyscope/polyscope.h"

#include "imgui.h"

using namespace geometrycentral;

namespace polyscope {

PointCloud::PointCloud(std::string name, const std::vector<Vector3>& points_)
    : Structure(name), points(points_) {
  prepare();
}

void PointCloud::draw() {
  if (!enabled) {
    return;
  }

  // Set uniforms
  glm::mat4 viewMat = view::getViewMatrix();
  program->setUniform("u_viewMatrix", glm::value_ptr(viewMat));

  glm::mat4 projMat = view::getPerspectiveMatrix();
  program->setUniform("u_projMatrix", glm::value_ptr(projMat));

  Vector3 eyePos = view::getCameraWorldPosition();
  program->setUniform("u_eye", eyePos);

  Vector3 lightPos = view::getLightWorldPosition();
  program->setUniform("u_light", lightPos);

  program->draw();
}

void PointCloud::drawPick() {}

void PointCloud::prepare() {
  // Create the GL program
  program = new gl::GLProgram(&PASSTHRU_SPHERE_VERT_SHADER, &SPHERE_GEOM_SHADER,
                              &SHINY_SPHERE_FRAG_SHADER, gl::DrawMode::Points);

  // Constant color
  std::vector<Vector3> colorData(points.size());
  std::fill(colorData.begin(), colorData.end(), gl::RGB_ORANGE);

  // Constant size
  std::vector<double> sizeData(points.size());
  std::fill(sizeData.begin(), sizeData.end(), .01 * state::lengthScale);

  // Store data in buffers
  program->setAttribute("a_position", points);
  program->setAttribute("a_color", colorData);
  program->setAttribute("a_pointRadius", sizeData);
}

void PointCloud::teardown() {
  if (program != nullptr) {
    delete program;
  }
}

void PointCloud::drawUI() {

  ImGui::TextUnformatted(name.c_str());
  ImGui::Checkbox("Enabled", &enabled);

}

double PointCloud::lengthScale() {

  // Measure length scale as twice the radius from the center of the bounding box
  auto bound = boundingBox(); 
  Vector3 center = 0.5 * (std::get<0>(bound) + std::get<1>(bound));

  double lengthScale = 0.0;
  for(Vector3& p : points) {
    lengthScale = std::max(lengthScale, geometrycentral::norm2(p - center));
  }

  return 2 * std::sqrt(lengthScale);
}

std::tuple<geometrycentral::Vector3, geometrycentral::Vector3>
PointCloud::boundingBox() {

  Vector3 min = Vector3{1,1,1}*std::numeric_limits<double>::infinity();
  Vector3 max = -Vector3{1,1,1}*std::numeric_limits<double>::infinity();

  for(Vector3& p : points) {
    min = geometrycentral::componentwiseMin(min, p);
    max = geometrycentral::componentwiseMax(max, p);
  }

  return {min, max};
}

}  // namespace polyscope