#pragma once
#include "CustomCamera.h"
class FreeCamera : public CustomCamera
{
public:
	void handleInput(float ft);

	void moveForward();
	void moveBack();
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();

	void turnLeft();
	void turnRight();
	void turnUp();
	void turnDown();
	void turn(int x, int y);

	void setMoveSpeed(float ms) { move_speed_ = ms; }
	void setRotSpeed(float rs) { rot_speed_ = rs; }

	float* getMoveSpeed() { return &move_speed_; }
	float* getRotSpeed() { return &rot_speed_; }

	// method to draw camera related debug options to gui
	void drawGui();

private:
	POINT cursor_;
	int deltax_, deltay_;

	// floats to allow customisability of cameras movement
	float move_speed_ = 5.0f;
	float rot_speed_ = 25.0f;
};

