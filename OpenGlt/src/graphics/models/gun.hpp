#include "../model.h"
#include "../../io/Camera.h"

class Gun : public Model
{
	public:
	Gun()
		:Model(glm::vec3(0.0f), glm::vec3(0.5f),true){ }


};