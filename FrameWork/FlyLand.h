﻿#pragma once
#include "../FrameWork/Animation.h"
#include "BaseObject.h"
#include "IComponent.h"
#include "CollisionBody.h"
//#include "Scenes/PlayScene.h"

#define FLYLAND_HORIZONTAL_VELOC		GVector2(48.0f, 0.0f)		// vận tốc ngang
#define FLYLAND_HORIZONTAL_VELOC_PRE	GVector2(-48.0f, 0.0f)		// vận tốc ngang

class FlyLand : public BaseObject {
public:
	// leftPos: vị trí ngoài cùng bên trái 
	// rightPos: vị trí ngoài cùng bên phải
	FlyLand(GVector2 leftPos, GVector2 rightPos);
	~FlyLand();

	// Các phương thức kế thừa từ BaseObject.
	void init() override;
	void update(float deltatime) override;
	void draw(LPD3DXSPRITE, Viewport*) override;
	void release() override;
	GVector2 getVelocity() override;

	IComponent* getComponent(string componentName);
	//RECT getBounding() override;

private:
	map<string, IComponent*> _listComponent;

	GVector2	_leftPosition;
	GVector2	_rightPosition;

	void checkPosition();

};
