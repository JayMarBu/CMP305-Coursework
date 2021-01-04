#include "FreeCamera.h"


void FreeCamera::handleInput(float dt)
{
	setFrameTime(dt);
	// Handle the input.
	if (input->isKeyDown('W'))
	{
		// forward
		moveForward();
	}
	if (input->isKeyDown('S'))
	{
		// back
		moveBack();
	}
	if (input->isKeyDown('A'))
	{
		// Strafe Left
		moveLeft();
	}
	if (input->isKeyDown('D'))
	{
		// Strafe Right
		moveRight();
	}
	if (input->isKeyDown('Q'))
	{
		// Down
		moveDown();
	}
	if (input->isKeyDown(VK_SPACE))
	{
		// Up
		moveUp();
	}
	if (input->isKeyDown(VK_UP))
	{
		// rotate up
		turnUp();
	}
	if (input->isKeyDown(VK_DOWN))
	{
		// rotate down
		turnDown();
	}
	if (input->isKeyDown(VK_LEFT))
	{
		// rotate left
		turnLeft();
	}
	if (input->isKeyDown(VK_RIGHT))
	{
		// rotate right
		turnRight();
	}

	if (input->isMouseActive())
	{
		// mouse look is on
		deltax_ = input->getMouseX() - (winWidth / 2);
		deltay_ = input->getMouseY() - (winHeight / 2);
		turn(deltax_, deltay_);
		cursor_.x = winWidth / 2;
		cursor_.y = winHeight / 2;
		ClientToScreen(wnd, &cursor_);
		SetCursorPos(cursor_.x, cursor_.y);
	}

	if (input->isRightMouseDown() && !input->isMouseActive())
	{
		// re-position cursor
		cursor_.x = winWidth / 2;
		cursor_.y = winHeight / 2;
		ClientToScreen(wnd, &cursor_);
		SetCursorPos(cursor_.x, cursor_.y);
		input->setMouseX(winWidth / 2);
		input->setMouseY(winHeight / 2);

		// set mouse tracking as active and hide mouse cursor
		input->setMouseActive(true);
		ShowCursor(false);
	}
	else if (!input->isRightMouseDown() && input->isMouseActive())
	{
		// disable mouse tracking and show mouse cursor
		input->setMouseActive(false);
		ShowCursor(true);
	}

	//if (input->isKeyDown(VK_SPACE))
	//{
	//	// re-position cursor
	//	cursor.x = winWidth / 2;
	//	cursor.y = winHeight / 2;
	//	ClientToScreen(wnd, &cursor);
	//	SetCursorPos(cursor.x, cursor.y);
	//	input->setMouseX(winWidth / 2);
	//	input->setMouseY(winHeight / 2);
	//	input->SetKeyUp(VK_SPACE);
	//	// if space pressed toggle mouse
	//	input->setMouseActive(!input->isMouseActive());
	//	if (!input->isMouseActive())
	//	{
	//		ShowCursor(true);
	//	}
	//	else
	//	{
	//		ShowCursor(false);
	//	}
	//}
	update();
}

void FreeCamera::moveForward()
{
	float radians;

	// Update the forward movement based on the frame time
	float speed = frameTime * move_speed_;

	// Convert degrees to radians.
	radians = rotation.y * 0.0174532f;  

	// Update the position.
	position.x += sinf(radians) * speed;
	position.z += cosf(radians) * speed;
}

void FreeCamera::moveBack()
{
	float radians;

	// Update the backward movement based on the frame time
	float speed = frameTime * move_speed_;// *0.5f;

	// Convert degrees to radians.
	radians = rotation.y * 0.0174532f;

	// Update the position.
	position.x -= sinf(radians) * speed;
	position.z -= cosf(radians) * speed;
}

void FreeCamera::moveUp()
{
	// Update the upward movement based on the frame time
	float speed = frameTime * move_speed_;// *0.5f;

	// Update the height position.
	position.y += speed;
}

void FreeCamera::moveDown()
{
	// Update the downward movement based on the frame time
	float speed = frameTime * move_speed_;// *0.5f;

	// Update the height position.
	position.y -= speed;
}

void FreeCamera::moveLeft()
{
	float radians;

	// Update the forward movement based on the frame time
	float speed = frameTime * move_speed_;

	// Convert degrees to radians.
	radians = rotation.y * 0.0174532f;

	// Update the position.
	position.z += sinf(radians) * speed;
	position.x -= cosf(radians) * speed;
}

void FreeCamera::moveRight()
{
	float radians;

	// Update the forward movement based on the frame time
	float speed = frameTime * move_speed_;

	// Convert degrees to radians.
	radians = rotation.y * 0.0174532f;

	// Update the position.
	position.z -= sinf(radians) * speed;
	position.x += cosf(radians) * speed;
}

void FreeCamera::turnLeft()
{
	// Update the left turn movement based on the frame time 
	float speed = frameTime * rot_speed_;

	// Update the rotation.
	rotation.y -= speed;

	// Keep the rotation in the 0 to 360 range.
	if (rotation.y < 0.0f)
	{
		rotation.y += 360.0f;
	}
}

void FreeCamera::turnRight()
{
	// Update the right turn movement based on the frame time
	float speed = frameTime * rot_speed_;

	// Update the rotation.
	rotation.y += speed;

	// Keep the rotation in the 0 to 360 range.
	if (rotation.y > 360.0f)
	{
		rotation.y -= 360.0f;
	}
}

void FreeCamera::turnUp()
{
	// Update the upward rotation movement based on the frame time
	float speed = frameTime * rot_speed_;

	// Update the rotation.
	rotation.x -= speed;

	// Keep the rotation maximum 90 degrees.
	if (rotation.x > 90.0f)
	{
		rotation.x = 90.0f;
	}
}

void FreeCamera::turnDown()
{
	// Update the downward rotation movement based on the frame time
	float speed = frameTime * rot_speed_;

	// Update the rotation.
	rotation.x += speed;

	// Keep the rotation maximum 90 degrees.
	if (rotation.x < -90.0f)
	{
		rotation.x = -90.0f;
	}
}

void FreeCamera::turn(int x, int y)
{
	// Update the rotation.
	rotation.y += (float)x / (rot_speed_/5.0f);// m_speed * x;

	rotation.x += (float)y / (rot_speed_/5.0f);// m_speed * y;
}

void FreeCamera::drawGui()
{
	if (ImGui::CollapsingHeader("Camera Options"))
	{
		ImGui::Indent(10);
		ImGui::Text("Camera Controls:");
		ImGui::Text("\tWASD to move.");
		ImGui::Text("\tRight click and drag to turn");
		ImGui::Text("\tSpace to ascend");
		ImGui::Text("\tQ to decend");

		ImGui::Text(("Position: (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")").c_str());
		ImGui::SliderFloat("Camera move speed", getMoveSpeed(), 5.0f, 30.0f);
		ImGui::SliderFloat("Camera rotation speed", getRotSpeed(), 10.0f, 100.0f);
		ImGui::Indent(-10);
	}
}
