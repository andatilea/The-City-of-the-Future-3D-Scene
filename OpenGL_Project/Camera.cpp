#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
		this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
		this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

    }

    //return the view matrix, using the glm::lookAt() function
	glm::mat4 Camera::getViewMatrix()
	{
		return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f));
	}

    //update the camera internal parameters following a camera move event
	void Camera::move(MOVE_DIRECTION direction, float speed)
	{
		if (this->cameraPosition.y <= 1081949816 && this->cameraPosition.y >= -1065283000 && this->cameraPosition.x <= 1610612736 && this->cameraPosition.x >= -1073741824) {

			glm::vec3 v = speed * cameraFrontDirection;
			switch (direction) {
			case MOVE_FORWARD:
				cameraPosition += glm::vec3(v.x, 0.0f, v.z);
				break;

			case MOVE_BACKWARD:
				cameraPosition -= glm::vec3(v.x, 0.0f, v.z);
				break;

			case MOVE_RIGHT:
				cameraPosition += cameraRightDirection * speed;
				break;

			case MOVE_LEFT:
				cameraPosition -= cameraRightDirection * speed;
				break;
			}
		}
	}

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
		cameraFrontDirection.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		cameraFrontDirection.y = sin(glm::radians(pitch));
		cameraFrontDirection.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

		cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	}
}