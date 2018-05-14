#pragma once

#ifndef __PLANE_H
#define __PLANE_H

#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtc/type_ptr.hpp"

class Plane {
public:
	float pitch, bank, speed;
	glm::vec3 pos;
	Plane(glm::vec3 startPos);
	void changePitch(float change);
	void changeBank(float change);
	void changeSpeed(float change);

};


#endif
