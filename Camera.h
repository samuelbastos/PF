#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
public:
	Camera(float _radius, float _min_rad = 1.0f, float _max_rad = 980.0f);
	~Camera();

	void setInitialConfig();

	int mouseButton(int bt, int st, int x, int y);
	int mouseMotion(int x, int y);

	void setSpeedRadius(float spd);

	glm::mat4 lookAt();

	glm::vec3 GetEye()
	{
		return eye;
	}

	glm::vec3 GetDir()
	{
		return glm::normalize(center - eye);
	}

	void UpdateAspectRatio(float w, float h)
	{
		aspect_ratio = w / h;
	}

protected:
	glm::vec3 eye, center, up;
	float radius;
	float speed;
	float speed_radius;

	float min_radius;
	float max_radius;

	float fovy;
	float aspect_ratio;
	float cam_near, cam_far;

private:
	int last_mx, last_my;
	int cur_mx, cur_my;
	bool arcball_on;
	bool changing_radius;
};

