#version 400

layout(location = 0) in vec3 VerPos;

out vec3 vert_eye;
out vec3 vert_dir;

uniform mat4 ViewMatrix;

uniform float fov_y_tangent;
uniform float aspect_ratio;

void main(void)
{
  gl_Position = vec4(VerPos,1.0);

  // http://blog.hvidtfeldts.net/index.php/2014/01/combining-ray-tracing-and-polygons/
  //---------------------------------------------------------------------------------
  vert_eye = - (ViewMatrix[3].xyz) * mat3(ViewMatrix);
  vert_dir = vec3(VerPos.x*fov_y_tangent*aspect_ratio, VerPos.y*fov_y_tangent, -1.0) * mat3(ViewMatrix);
  //---------------------------------------------------------------------------------
}